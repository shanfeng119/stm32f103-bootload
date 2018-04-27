

#include "stm32f10x.h"

#include "dwt.h"
#include "iap.h"
#include "iwdg.h"
#include "led.h"

#include "spi_flash.h"


void gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOC);
	GPIO_DeInit(GPIOD);
	GPIO_DeInit(GPIOE);
	GPIO_DeInit(GPIOF);
	GPIO_DeInit(GPIOG);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	// sos key.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	// power open
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
	// spi2 sck miso mosi cs === spi flash.
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// led r.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
	GPIOA->BRR = GPIO_Pin_0;
}

void power_on(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

void power_off()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

u8 GetKey (void)
{
	u8 a;
	
	a = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);
	
	if( a == 0 ) 
		return 0;
	else 
		return 1;
}

int main(void)
{	
    DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);
	dwt_init();
	gpio_init();
    
    ledInit();
    
    /* 闪烁一次，检查灯*/
    ledToggleAll();
    delay_ms(500);
    ledToggleAll();

	
IAP_SET:
	IAP_Init();
    IAP_ShowTitle();
    
    if ( !IAP_GetMagic())
    {
       goto JUMP_2_APP;
    }

    /* 进入boot模式，灯全亮 */
    ledToggleAll(); 
	init_iwdg();
    
IAP_SHOW:	
	IAP_ShowMenu();	
    IAP_WiatForChoose();	
	goto IAP_SHOW;
	

JUMP_2_APP:	
    IAP_ShowApp();
    IAP_JumpToApplication();
    
    // usually cannot go here.
    goto IAP_SET;
}

