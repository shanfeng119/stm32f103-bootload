/*
*********************************************************************************************************
*                                  
*模块名称 : 独立看门狗驱动
*文件名称 : bsp_iwdg.c
*版    本 : V1.0
*说    明 : IWDG驱动程序
*修改记录 :
*版本号   日期        作者     说明
*V1.0    2013-12-02  armfly   正式发布
*
*Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "stm32f10x.h"

 
void init_iwdg(void)
{
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{}

	/* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
	dispersion) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: LSI/32 */
	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/* Set counter reload value to obtain 250ms IWDG TimeOut.
	Counter Reload Value = 250ms/IWDG counter clock period
	                  = 250ms / (LSI/32)
	                  = 0.25s / (LsiFreq/32)
	                  = LsiFreq/(32 * 4)
	                  = LsiFreq/128
	*/
	IWDG_SetReload(0xFFF);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
}

void feed_iwdg(void)
{
	IWDG_ReloadCounter();
}

