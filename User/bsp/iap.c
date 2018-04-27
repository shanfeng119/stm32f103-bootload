
#include "stm32f10x.h"

#include "iap.h"

#include "dwt.h"
#include "download.h"
#include "upload.h"
#include "iwdg.h"

#include "spi_flash.h"
#include "md5.h"

#include <stdio.h>
#include <string.h>
#include "util.h"

#define USE_USART3 1

#if USE_USART3
#define COM1 			USART3
#define COM1_RCC		RCC_APB1Periph_USART3 
#define COM1_AFIO_RCC	RCC_APB2Periph_AFIO
#define COM1_GPIO_RCC	RCC_APB2Periph_GPIOB
#define COM1_TX_PIN		GPIO_Pin_10
#define COM1_GPIO_PORT	GPIOB
#define COM1_RX_PIN		GPIO_Pin_11
#define COM1_GPIO_PORT	GPIOB
#else
#define COM1 			USART1
#define COM1_RCC		RCC_APB2Periph_USART1 
#define COM1_AFIO_RCC	RCC_APB2Periph_AFIO
#define COM1_GPIO_RCC	RCC_APB2Periph_GPIOA
#define COM1_TX_PIN		GPIO_Pin_9
#define COM1_GPIO_PORT	GPIOA
#define COM1_RX_PIN		GPIO_Pin_10
#define COM1_GPIO_PORT	GPIOA

#endif



void IAP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	
	RCC_APB2PeriphClockCmd(COM1_GPIO_RCC, ENABLE);
#if USE_USART3
    RCC_APB1PeriphClockCmd(COM1_RCC, ENABLE);
#else
    RCC_APB2PeriphClockCmd(COM1_RCC, ENABLE);    
#endif
    RCC_APB2PeriphClockCmd(COM1_AFIO_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = COM1_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM1_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = COM1_RX_PIN;
	GPIO_Init(COM1_GPIO_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No; 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    //USART_ITConfig(COM1, USART_IT_RXNE, ENABLE);	
    
	USART_Init(COM1,&USART_InitStructure);
	USART_Cmd(COM1,ENABLE);	
}

void IAP_SerialSendByte(u8 c)
{
	USART_SendData(COM1, c);
	while (USART_GetFlagStatus(COM1, USART_FLAG_TXE) == RESET) {}
}

void IAP_SerialSendStr(char *s)
{
	while(*s != '\0')
	{
		IAP_SerialSendByte(*s);
		s++;
	}
}

// 0 NO DATA  1 data recv.
u8 IAP_SerialGetByte(u8 *c)
{
	if(USART_GetFlagStatus(COM1, USART_FLAG_RXNE) != RESET)
	{
		*c = USART_ReceiveData(COM1);
        //USART_ClearITPendingBit(COM1, USART_IT_RXNE);
		
		return 1;
	}  
	return 0;
}


/* 测试使用，获取特定关键字
成功返回 1，失败返回0 */
u8 IAP_GetMagic(void)
{
	u8 data;
    u16 timeout = 3000;

    while (timeout)
    {
        timeout--;
        if (IAP_SerialGetByte(&data))
        {

            if ((data == 'b') || (data == 'B'))/* ESC 27    F8 119*/
            {
                return 1;
            }            
        }
        else
        {
            delay_ms(1);
        }
        
    }

    return 0;
		
}


u8 IAP_GetKey(void)
{
	u8 data;

	while( !IAP_SerialGetByte(&data) );
	
	return data;
		
}
void IAP_ShowTitle(void)
{
    IAP_SerialSendStr("\r\n    Press \"B\" or \"b\" in 3 seconds, will Enter bootload mode.\r\n");
}

void IAP_ShowApp(void)
{
    IAP_SerialSendStr("\r\n    system has Entered app mode.\r\n");
}


void IAP_ShowMenu(void)
{
	IAP_SerialSendStr("\r\n|    In-Application Programing Application (Version 1.0)    		|");
	IAP_SerialSendStr("\r\n+----command----+-----------------function-----------------------+");
	IAP_SerialSendStr("\r\n|  1: FWUPDATA  | Update the firmware to flash by YModem    		|");
//	IAP_SerialSendStr("\r\n|  2: FWDWLOAD  | Download the firmware from Flash by YModem		|");
//	IAP_SerialSendStr("\r\n|  3: FWERASE   | Erase the current firmware                		|");
//	IAP_SerialSendStr("\r\n|  4: BOOT      | Excute the current firmware               		|");
	IAP_SerialSendStr("\r\n|  5: REBOOT    | Reboot                                    		|");
//	IAP_SerialSendStr("\r\n|  6: FWSTORE   | Store the firmware to spi flash by YModem		|");
	IAP_SerialSendStr("\r\n|  ?: HELP      | Display this help                         		|");
	IAP_SerialSendStr("\r\n+================================================================+");
	IAP_SerialSendStr("\r\n\r\n");
	IAP_SerialSendStr("STM32-IAP>>");
}

// 关闭flash写保护
void IAP_DisableFlashWPR(void)
{
	u32 blockNum = 0, UserMemoryMask = 0;

    blockNum = (IAP_ADDR - FLASH_BASE_ADDR) >> 12;   
	UserMemoryMask = ((u32)(~((1 << blockNum) - 1)));
	
	//查看是否写保护
	if((FLASH_GetWriteProtectionOptionByte() & UserMemoryMask) != UserMemoryMask)
	{
		FLASH_EraseOptionBytes ();
	}
}


s8 IAP_UpdataParam(s32 *param)
{
	u32 i;
	u32 flashptr = IAP_PARAM_ADDR;

	FLASH_Unlock();//flash解锁
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清楚标志
	for(i = 0; i < IAP_PARAM_SIZE; i++)
	{
		FLASH_ProgramWord(flashptr + 4 * i, *param);
		if(*(u32 *)(flashptr + 4 * i) != *param)
		{
			return -1;
		}	
		param++;
	}
	FLASH_Lock();
	return 0;
}

// 0  OK     1  err
s8 IAP_UpdataProgram(u32 addr, u32 size)
{
	u32 i;
	static u32 flashptr = IAP_ADDR;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	for(i = 0; i < size; i += 4)
	{	
		FLASH_ProgramWord(flashptr, *(u32 *)addr);	// 烧写一个字节
		if(*(u32 *)flashptr != *(u32 *)addr)		// 判断是否完成
		{
			return -1;
		}
		flashptr += 4;
		addr += 4;
	}
	FLASH_Lock();//flash??
	return 0;
}

// 
void IAP_FlashEease(u32 size)
{
	u16 eraseCounter = 0;
	u16 nbrOfPage = 0;
	FLASH_Status FLASHStatus = FLASH_COMPLETE;	  

	if(size % PAGE_SIZE != 0)//计算擦的页数
	{										  
		nbrOfPage = size / PAGE_SIZE + 1; 
	}
	else
	{
		nbrOfPage = size / PAGE_SIZE;
	}

	FLASH_Unlock();//
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清除flash相关标志位
	for(eraseCounter = 0; (eraseCounter < nbrOfPage) && ((FLASHStatus == FLASH_COMPLETE)); eraseCounter++)//开始擦除
	{
		FLASHStatus = FLASH_ErasePage(IAP_ADDR + (eraseCounter * PAGE_SIZE));//擦除
		IAP_SerialSendStr(".");
	}
	FLASH_ErasePage(IAP_PARAM_ADDR);//擦除参数所在的flash页
	FLASH_Lock();
}

void IAP_FlashEease_spi(u32 size)
{
	u16 eraseCounter = 0;
	u16 nbrOfPage = 0;
	FLASH_Status FLASHStatus = FLASH_COMPLETE;	  

	if(size % PAGE_SIZE != 0)//计算擦的页数
	{										  
		nbrOfPage = size / PAGE_SIZE + 1; 
	}
	else
	{
		nbrOfPage = size / PAGE_SIZE;
	}

	FLASH_Unlock();//
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//??flash?????
	for(eraseCounter = 0; (eraseCounter < nbrOfPage) && ((FLASHStatus == FLASH_COMPLETE)); eraseCounter++)//????
	{
		FLASHStatus = FLASH_ErasePage(IAP_ADDR + (eraseCounter * PAGE_SIZE));//??
	}
	FLASH_ErasePage(IAP_PARAM_ADDR);//
	FLASH_Lock();
}


typedef  void (*pFunction)(void);			
pFunction			Jump_To_Application;

void IAP_JumpToApplication(void)
{
	u32 JumpAddress;//

	if(((*(__IO u32 *)IAP_ADDR) & 0x2FFE0000) == 0x20000000)
	{
		JumpAddress = *(__IO u32 *)(IAP_ADDR + 4);
		Jump_To_Application = (pFunction)JumpAddress;
		__set_MSP(*(__IO u32*)IAP_ADDR);
		Jump_To_Application();
	}
}

static void ShwHelpInfo(void)
{
	IAP_SerialSendStr("\r\nEnter '1' to updtate you apllication code!");
//	IAP_SerialSendStr("\r\nRnter '2' to download the firmware from interal flash!");
//	IAP_SerialSendStr("\r\nRnter '3' to erase the current application code!");
//	IAP_SerialSendStr("\r\nEnter '4' to go to excute the current application code!");
	IAP_SerialSendStr("\r\nEnter '5' to restart the system!");
//	IAP_SerialSendStr("\r\nEnter '6' to store the firmware to spi flash!");
	IAP_SerialSendStr("\r\nEnter '?' to show the help infomation!\r\n");
}

void IAP_WiatForChoose(void)
{
	u8 c = 0; 
		
	while (1)
	{
		c = IAP_GetKey();
		
		IAP_SerialSendByte(c);
		switch(c)
		{
		case '1'://
			if(IAP_GetKey() == '\r')
			{
				IAP_DisableFlashWPR();
				DownloadFirmware();
				IAP_SerialSendStr("Reset the device.\r\n");
				delay_ms(500);
				NVIC_SystemReset();
			}
			break;
//		case '2'://FWDWLOAD
//			if(IAP_GetKey() == '\r')  //
//			{
////				UploadFirmware();//
//				IAP_SerialSendStr("\r\nnot support...");
//				return;//
//			}
//			break;
//		case '3'://FWERASE
//			if(IAP_GetKey() == '\r')//
//			{
//				IAP_SerialSendStr("\r\nErasing...");
//				IAP_SerialSendStr("\r\nForbidden...");
////				IAP_FlashEease(FLASH_SIZE + FLASH_BASE_ADDR - IAP_ADDR);//Flash,flash
//				IAP_SerialSendStr("\r\nErase done!\r\n");
//				return;//
//			}
//			break;
//		case '4'://BOOT
//			if(IAP_GetKey() == '\r')//
//			{
//				IAP_SerialSendStr("\r\nBotting...\r\n");
//				if(((*(__IO u32 *)IAP_ADDR) & 0x2FFE0000) != 0x20000000)//
//				{
//					IAP_SerialSendStr("No user program! Please download a firmware!\r\n");
//				}
//				delay_ms(500);
//				NVIC_SystemReset();
//				return;//
//			}
//			break;
		case '5'://REBOOT
			if(IAP_GetKey() == '\r')//
			{
				IAP_SerialSendStr("\r\nRebooting...\r\n");
				delay_ms(500);
				NVIC_SystemReset();
				return;//
			}
			break;
		case '?'://HELP
			if(IAP_GetKey() == '\r')
			{
				ShwHelpInfo();//
				return;//
			}
			break;
		
		default:
			IAP_SerialSendStr("\r\nInvalid Number! The number should be either 1or5\r\n");
			return;//
		}
	}
}





// ===================== spi flash 相关iap。===================

#define VER_ADDRESS		0x08020000

__packed typedef struct{
	char 	ver;
	int		size;
	char	md5[33];
}update_info_t;

update_info_t 	update_info = {0};
unsigned char 	info_len = 0;

unsigned char 	read_buf[512];
unsigned int	read_len = 0;

int read_update_info(void)
{
	unsigned char local_ver = 0;
	
	flash_read_update_info((unsigned char *)&update_info, &info_len);
	
	if(info_len != sizeof(update_info))
		return false;
	
	if(update_info.size == 0)
		return false;
	
	local_ver = *((unsigned char *)VER_ADDRESS);
	
	if(local_ver == 0xFF)
		return true;
	
	if(update_info.ver > local_ver)
		return true;
	else
		return false;
}

int check_flash_md5(void)
{
	MD5_CTX 	mdContext;  
	int 		i;	
	char 		md5_buf[33];
	int			read_remain_len, read_total_len;
	
	read_remain_len = update_info.size;
	read_total_len = 0;
	
	MD5Init (&mdContext);  
	while(read_remain_len){
		read_len = (read_remain_len > 512) ? 512:read_remain_len ;
		flash_read_update_data(read_total_len, read_buf, read_len);
		MD5Update (&mdContext, read_buf, read_len);
		read_total_len += read_len;
		read_remain_len -= read_len;
	}	
	MD5Final (&mdContext);	
	
	memset(md5_buf, 0, sizeof(md5_buf));
	for(i=0; i<16; i++)  
	{  
		sprintf(&md5_buf[i*2], "%02X", mdContext.digest[i]);
	} 
	
	if(memcmp(md5_buf, update_info.md5, 32) == 0){
		return true;
	}else{
		return false;
	}
}

void flash_program(void)
{
	int	read_remain_len, read_total_len;
	unsigned int 	source;
	
	IAP_FlashEease_spi(update_info.size);
	
	read_remain_len = update_info.size;
	read_total_len = 0;
	
	while(read_remain_len){
		read_len = (read_remain_len > 512) ? 512:read_remain_len ;
		flash_read_update_data(read_total_len, read_buf, read_len);
		
		source = (unsigned int)read_buf;
		IAP_UpdataProgram(source, read_len);
		read_total_len += read_len;
		read_remain_len -= read_len;
		
		delay_ms(30);
		GPIOA->ODR ^= GPIO_Pin_8;
	}	
}

void spi_flash_iap(void)
{
	if( !read_update_info() )
		return ;
	
	if( !check_flash_md5() )
		return ;
	
	flash_program();
	
	delay_ms(500);
	
	flash_erase_update_info();
	
	soft_reset();
}
