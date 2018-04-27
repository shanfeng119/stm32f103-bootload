
#include "util.h"
#include "stm32f10x.h"
#include "spi_flash.h"

#include <stdio.h>
#include <string.h>

// ====================== spi总线底层配置======================================
/* 定义SPI总线的 GPIO端口 */
#define FLASH_PORT_SCK		GPIOB
#define FLASH_PIN_SCK		GPIO_Pin_13

#define FLASH_PORT_MISO		GPIOB
#define FLASH_PIN_MISO		GPIO_Pin_14

#define FLASH_PORT_MOSI		GPIOB
#define FLASH_PIN_MOSI		GPIO_Pin_15

/* 串行Flsh的片选GPIO端口  */
#define SF_PIN_RCC			RCC_APB2Periph_GPIOA
#define SF_PORT_CS			GPIOA
#define SF_PIN_CS			GPIO_Pin_4
#define SF_CS_0()			SF_PORT_CS->BRR = SF_PIN_CS
#define SF_CS_1()			SF_PORT_CS->BSRR = SF_PIN_CS

#define FLASH_USE_SPI		SPI1


void spi_flash_init(void)
{    
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
    
 //   SPI_I2S_DeInit(SPI1);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7; // Not used

    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_Init(SPI1, &SPI_InitStructure);
    
    SPI_Cmd(SPI1, DISABLE);
    
    SPI_Cmd(SPI1, ENABLE);
    SF_CS_1();
}


uint8_t spi_readwrite_byte(uint8_t data)
{  
    uint32_t timeout = 0xFFFFFF;

    while ((SPI_I2S_GetFlagStatus(FLASH_USE_SPI, SPI_I2S_FLAG_TXE) == RESET) &&(timeout--));
    
    SPI_I2S_SendData(FLASH_USE_SPI, data); 
    
    timeout = 0xFFFFFF;
    while ((SPI_I2S_GetFlagStatus(FLASH_USE_SPI, SPI_I2S_FLAG_RXNE) == RESET)&&(timeout--));
   
    return SPI_I2S_ReceiveData(FLASH_USE_SPI);  
}


// ======================SPI Flash配置相关================================
#define   Write_EN                0x06		/* 写使能命令 */
#define   Write_Dis               0x04		/* 禁止写 */
#define   Read_Data               0x03		/* 读数据区命令 */
#define   Read_Reg1               0x05		/* 读状态寄存器命令 */
#define   Read_Reg2               0x35
#define   Write_Reg               0x01		/* 写状态寄存器命令 */
#define   Page_Program            0x02
#define   Quad_Page_Program       0x32
#define   Block_Erase_64K         0xD8
#define   Block_Erase_32K         0x52
#define   Sector_Erase_4K         0x20		/* 擦除扇区命令 */
#define   chip_Erase              0xC7		/* 批量擦除命令 */
#define   Erase_Suspend           0x75
#define   Erase_Resume            0x7A
#define   Power_Down              0xB9
#define   Release_Power_Down      0xAB
#define   Mode_Bit_Reset          0xFF
#define   Device_ID               0x9F		/* 读器件ID命令 */

#define   Dummy_Byte              0xA5  	/* 哑命令，可以为任意值，用于读操作 */


/* Delay definition */   
#define BlockErase_Timeout    ((int)0x00001000)			// max 400 ms
#define ChipErase_Timeout     ((int)0x00010000) 		// max 30  s
#define Program_Timeout       ((int)0x00000100) 		// max 3   ms


#define CMD_EWRSR	 			0x50		/* 允许写状态寄存器的命令 */

#define WIP_FLAG      			0x01		/* 状态寄存器中的正在编程标志（WIP) */


void spi_flash_write_enable(void)
{
    /* Select the FLASH: Chip Select low */
    SF_CS_0();
    /* Send "Write Enable" instruction */
    spi_readwrite_byte(Write_EN);
    /* Deselect the FLASH: Chip Select high */
    SF_CS_1();
}

void spi_flash_write_disable(void)
{
    /* Select the FLASH: Chip Select low */
     SF_CS_0();
    /* Send "Write Enable" instruction */
    spi_readwrite_byte(Write_Dis);
    /* Deselect the FLASH: Chip Select high */
     SF_CS_1();
}

void spi_flash_get_status(void)
{
    uint8_t state;
    
	SF_CS_0();
	spi_readwrite_byte(Read_Reg1);
	
    do
    {
        state = spi_readwrite_byte(Dummy_Byte);
    }
    while( (state & 0x01) );
	
	SF_CS_1();
}


void spi_flash_erase_sector(int sec_addr)
{
    spi_flash_write_enable();
	
    SF_CS_0();
    spi_readwrite_byte(Sector_Erase_4K);
    
    spi_readwrite_byte((sec_addr & 0xFF0000) >> 16);
    spi_readwrite_byte((sec_addr & 0xFF00) >> 8);
    spi_readwrite_byte(sec_addr & 0xFF);
    SF_CS_1();
    
    spi_flash_get_status();
	
	spi_flash_write_disable();
	
}

void spi_flash_erase_block(int blo_addr)
{
    spi_flash_write_enable();
   
    SF_CS_0();
    spi_readwrite_byte(Block_Erase_32K);
    
    spi_readwrite_byte((blo_addr & 0xFF0000) >> 16);
    spi_readwrite_byte((blo_addr & 0xFF00) >> 8);
    spi_readwrite_byte(blo_addr & 0xFF);
    
    SF_CS_1();
    
    spi_flash_get_status();
	
	spi_flash_write_disable();
}

void spi_flash_erase_chip(void)
{	
    spi_flash_write_enable();
    
    SF_CS_0();
    spi_readwrite_byte(chip_Erase);
    SF_CS_1();
    
	spi_flash_get_status();
	
	spi_flash_write_disable();
}

void spi_flash_write_page(int write_addr, unsigned char *buffer, int len)
{
    spi_flash_write_enable();
    
	SF_CS_0();
    spi_readwrite_byte(Page_Program);
    
    spi_readwrite_byte((write_addr & 0xFF0000) >> 16);
    spi_readwrite_byte((write_addr & 0xFF00) >> 8);
    spi_readwrite_byte(write_addr & 0xFF);
    
    while(len--)
    {
        spi_readwrite_byte(*buffer);
        buffer++;
    }
    
    SF_CS_1();
	
	spi_flash_get_status();
	
	spi_flash_write_disable();
}

void spi_flash_write_buf(int write_addr, unsigned char *buffer, int len)
{
    int size;
	
    while(len)
    {
        size = (Page_Size - write_addr%Page_Size);
        if(len < size)
        {
            size = len;
        }
        spi_flash_write_page(write_addr, buffer, size);

        len = len - size;
        write_addr = write_addr + size;
        buffer = buffer + size;
    }
}

void spi_flash_read_buf(int read_addr, unsigned char *buffer, int len)
{

    SF_CS_0();
    spi_readwrite_byte(Read_Data);
    
    spi_readwrite_byte((read_addr & 0xFF0000) >> 16);
    spi_readwrite_byte((read_addr & 0xFF00) >> 8);
    spi_readwrite_byte(read_addr & 0xFF);
    
    while(len--)
    {
        *buffer = spi_readwrite_byte(Dummy_Byte);
        buffer++;
    }
    
    SF_CS_1();
}


// 
int nor_flash_write(int write_addr, unsigned char *buffer, int len)
{    
	if(write_addr >= NOR_FLASH_SIZE)
		return false;
	
    if(buffer == NULL)
        return false;
    
    if(len == 0)
        return false;
    
    spi_flash_write_buf(write_addr, buffer, len);
	
	return true;
}

int nor_flash_read(int read_addr, unsigned char *buffer, int len)
{ 
	if(read_addr >= NOR_FLASH_SIZE)
		return false;
	
    if(buffer == NULL)
        return false;
    
    if(len == 0)
        return false;
    
    spi_flash_read_buf(read_addr, buffer, len);
	
    return true;
}

int nor_flash_erase_sector(int sec_addr)
{

    if(sec_addr >= NOR_FLASH_SIZE)
        return false;
    
    if( (sec_addr % NOR_Sector_SIZE) != 0)
        return false;
    
    spi_flash_erase_sector(sec_addr);
	
	return true;
}

int nor_flash_erase_block(int blo_addr)
{

    if(blo_addr >= NOR_FLASH_SIZE)
        return false;
    
    if( (blo_addr % NOR_BLOCK_SIZE) != 0)
        return false;
    
    spi_flash_erase_block(blo_addr);
	
	return true;
}

//===================================spi flash应用接口======================================

#define CHECK_BYTE0		0x5A
#define CHECK_BYTE1 	0xA5




void flash_read_update_data(int addr, unsigned char *data, int data_len)
{
	nor_flash_read(addr+UPDATE_FILE_DATA_ADDR, data, data_len);
}

void flash_erase_update_info(void)
{
	nor_flash_erase_sector(UPDATE_FILE_INFO_ADDR);
}

void flash_read_update_info(unsigned char *buf, unsigned char *info_len)
{
	unsigned char read_buf[34];
	unsigned char len = 0;
	unsigned char sum_read = 0;
	unsigned char sum_calc = 0;
	
	memset(read_buf, 0, sizeof(read_buf));
	nor_flash_read(UPDATE_FILE_INFO_ADDR, read_buf, 4);
	if(read_buf[0] != CHECK_BYTE0 && read_buf[1] != CHECK_BYTE1){
		*info_len = 0;
		return ;
	}

	len = read_buf[2];
	sum_read = read_buf[3];
	// id data   and sum	
	memset(read_buf, 0, sizeof(read_buf));
	nor_flash_read(UPDATE_FILE_INFO_ADDR+4, read_buf, len);

	sum_calc = sum_verify(read_buf, len);
	if(sum_calc != sum_read){
		*info_len = 0;
		return ;
	}
	*info_len = len;
	memcpy(buf, read_buf, len);
}



