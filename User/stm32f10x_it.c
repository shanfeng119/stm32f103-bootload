/*
*********************************************************************************************************
*	                                  
*	ģ������ : �ж�ģ��
*	�ļ����� : stm32f10x_it.c
*	��    �� : V2.0
*	˵    �� : ���ļ�������е��жϷ�������Ϊ�˱��������˽�����õ����жϣ����ǲ����齫�жϺ����Ƶ�����
*			���ļ���
*			
*			����ֻ��Ҫ������Ҫ���жϺ������ɡ�һ���жϺ������ǹ̶��ģ��������޸��������ļ���
*				Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\startup_stm32f10x_hd.s
*			
*			�����ļ��ǻ�������ļ�������ÿ���жϵķ���������Щ����ʹ����WEAK �ؼ��֣���ʾ�����壬�����
*			��������c�ļ����ض����˸÷��������������ͬ��������ô�����ļ����жϺ������Զ���Ч����Ҳ��
*			�����ض���ĸ�����C++�еĺ������ص��������ơ�
*				
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f10x_it.h"

#define ERR_INFO "\r\nEnter HardFault_Handler, System Halt.\r\n"

/*
*********************************************************************************************************
*	Cortex-M3 �ں��쳣�жϷ������
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	�� �� ��: NMI_Handler
*	����˵��: ���������жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/  
void NMI_Handler(void)
{
}

/*
*********************************************************************************************************
*	�� �� ��: HardFault_Handler
*	����˵��: Ӳ��ʧЧ�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
void HardFault_Handler(void)
{
  /* ��Ӳ��ʧЧ�쳣����ʱ������ѭ�� */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	�� �� ��: MemManage_Handler
*	����˵��: �ڴ�����쳣�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void MemManage_Handler(void)
{
  /* ���ڴ�����쳣����ʱ������ѭ�� */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	�� �� ��: BusFault_Handler
*	����˵��: ���߷����쳣�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/    
void BusFault_Handler(void)
{
  /* �������쳣ʱ������ѭ�� */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	�� �� ��: UsageFault_Handler
*	����˵��: δ�����ָ���Ƿ�״̬�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void UsageFault_Handler(void)
{
  /* ���÷��쳣ʱ������ѭ�� */
  while (1)
  {
	  
  }
}

/*
*********************************************************************************************************
*	�� �� ��: DebugMon_Handler
*	����˵��: ���Լ������жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void DebugMon_Handler(void)
{
}

