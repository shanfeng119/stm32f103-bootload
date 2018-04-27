

#ifndef __IAP_H
#define __IAP_H

#include "stm32f10x.h"


#define IAP_ADDR				0x08010000
#define FLASH_BASE_ADDR			0x08000000


#define IAP_PARAM_SIZE  1
#define IAP_PARAM_ADDR  (FLASH_BASE_ADDR + FLASH_SIZE - PAGE_SIZE) //Flash最后一页存参数

#define IAP_APP_PROGRAME_MAX_SIZE       (256 - 64 - 2)*1024 /* 芯片256k  64k为boot  2k为参数保存区 */

#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
 #define PAGE_SIZE          (0x400)    
 #define FLASH_SIZE         (0x20000)  
#elif defined STM32F10X_CL
 #define PAGE_SIZE          (0x800)    
 #define FLASH_SIZE         (0x40000)  
#elif defined STM32F10X_HD || defined (STM32F10X_HD_VL)
 #define PAGE_SIZE          (0x800)    
 #define FLASH_SIZE         (0x80000)  
#elif defined STM32F10X_XL
 #define PAGE_SIZE          (0x800)    
 #define FLASH_SIZE         (0x40000) //0xC0000
#else 
 #error "Please select first the STM32 device to be used (in stm32f10x.h)"    
#endif


void IAP_Init(void);
void IAP_SerialSendByte(u8 c);
void IAP_SerialSendStr(char *s);
u8 IAP_SerialGetByte(u8 *c);
u8 IAP_GetKey(void);
u8 IAP_GetMagic(void);
s8 IAP_UpdataParam(s32 *param);
s8 IAP_UpdataProgram(u32 addr, u32 size);
void IAP_FlashEease(u32 size);
void IAP_ShowMenu(void);
void IAP_ShowTitle(void);
void IAP_ShowApp(void);
void IAP_WiatForChoose(void);
void IAP_JumpToApplication(void);

void spi_flash_iap(void);

#endif

