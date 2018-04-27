

#include "stm32f10x.h"
#include "led.h"

#if 0 /* 正点mini开发板*/
#define GPIO_PORT_LED_SYS  	    GPIOA				
#define GPIO_PIN_LED_SYS		GPIO_Pin_8	
#define RCC_LED_SYS             RCC_APB2Periph_GPIOA
#endif
#define GPIO_PORT_LED_SYS  	    GPIOB				
#define GPIO_PIN_LED_SYS		GPIO_Pin_12	
#define RCC_LED_SYS             RCC_APB2Periph_GPIOB

#define GPIO_PORT_LED_ERR  	    GPIOB				
#define GPIO_PIN_LED_ERR		GPIO_Pin_13	
#define RCC_LED_ERR             RCC_APB2Periph_GPIOB

#define GPIO_PORT_LED_NET  	    GPIOB				
#define GPIO_PIN_LED_NET		GPIO_Pin_14	
#define RCC_LED_NET             RCC_APB2Periph_GPIOB

#define GPIO_PORT_LED_POWERDOWN GPIOB				
#define GPIO_PIN_LED_POWERDOWN  GPIO_Pin_15	
#define RCC_LED_POWERDOWN       RCC_APB2Periph_GPIOB

GPIO_TypeDef * LED_Port[LED_MAX] = { GPIO_PORT_LED_SYS, GPIO_PORT_LED_ERR, GPIO_PORT_LED_NET, GPIO_PORT_LED_POWERDOWN};
const unsigned short LED_Pin[LED_MAX] = {GPIO_PIN_LED_SYS, GPIO_PIN_LED_ERR, GPIO_PIN_LED_NET, GPIO_PIN_LED_POWERDOWN};


void ledInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_LED_SYS, ENABLE);    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_PIN_LED_SYS;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIO_PORT_LED_SYS, &GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(RCC_LED_ERR, ENABLE);    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_PIN_LED_ERR;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIO_PORT_LED_ERR, &GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(RCC_LED_NET, ENABLE);    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_PIN_LED_NET;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIO_PORT_LED_NET, &GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(RCC_LED_POWERDOWN, ENABLE);    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_PIN_LED_POWERDOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIO_PORT_LED_POWERDOWN, &GPIO_InitStruct);

    ledOff(LED_SYS);
    ledOff(LED_ERR);
    ledOff(LED_NET);
    ledOff(LED_POWERDOWN);
    
}

void ledOn(unsigned int _no)
{
	if(_no >= LED_MAX)
	{
		return ;	
	}
	
	LED_Port[_no]->BRR = LED_Pin[_no];

}

void ledOff(unsigned int _no)
{
	if(_no >= LED_MAX)
	{
		return ;	
	}
	
	LED_Port[_no]->BSRR = LED_Pin[_no];
}

void ledToggleAll(void)
{
    unsigned int _no;
    
    for (_no=0; _no<LED_MAX; _no++)
    {
	    LED_Port[_no]->ODR ^= LED_Pin[_no];
    }
}


void ledToggle(unsigned int _no)
{
	if(_no >= LED_MAX)
	{
		return ;
	}
	
	LED_Port[_no]->ODR ^= LED_Pin[_no];
}

