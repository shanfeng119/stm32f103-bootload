

#include "includes.h"


#define FLASH_BASE_ADDR			0x08000000			
#define	FLASH_SIZE				(256*1024)			
#define PAGE_SIZE			(2*1024)

/* F103RC,  256K FLASH , PAGE = 2K ,  256 PAGE  */
#define SECTOR_MASK				0xFFFFF800			

#define FLASH_IS_EQU		0   /* 相等 不用擦除 */
#define FLASH_REQ_WRITE		1	/* 直接写 */
#define FLASH_REQ_ERASE		2	/* 需要擦除 */
#define FLASH_PARAM_ERR		3	/* 参数错误 */

void iflash_Init(void)
{
	FLASH_Unlock();
	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
}

unsigned short if_readHalfWord(unsigned int addr)
{
	return *(volatile unsigned short*)addr; 
}

uint32_t FLASH_PagesMask(__IO uint32_t Size)
{
    uint32_t pagenumber = 0x0;
    uint32_t size = Size;

    if ((size % PAGE_SIZE) != 0)
    {
        pagenumber = (size / PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;

}


int iflash_erase(unsigned int addr, int size)
{
	unsigned int NbrOfPage, i = 0;
	FLASH_Status FLASHStatus = FLASH_COMPLETE;
	
	if((addr % PAGE_SIZE) != 0)
		return false;
	
	if(addr < FLASH_BASE_ADDR)
		return false;
	
	if(addr >= (FLASH_BASE_ADDR+FLASH_SIZE))
		return false;
	
	if(size == 0)
		return true;
	
	NbrOfPage = FLASH_PagesMask(size);
	
	for (i = 0; (i < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); i++)
	{
		FLASHStatus = FLASH_ErasePage(addr + (PAGE_SIZE * i));
	}
	
	return true;
}

// size 需要是2的倍数
int if_flash_write(unsigned int _ulFlashAddr, unsigned char *_ucpSrc, unsigned int _ulSize)
{
	uint32_t i;
	FLASH_Status status = FLASH_COMPLETE;

	if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return false;
	}

	if (_ulSize == 0)
	{
		return true;
	}

	if ((_ulSize % 2) != 0)	// 长度为偶数 半字操作
	{
		_ulSize += 1;
	}

	for (i = 0; i < _ulSize / 2; i++)
	{	
		status = FLASH_ProgramHalfWord(_ulFlashAddr, *(uint16_t*)_ucpSrc);
		if (status != FLASH_COMPLETE)
		{ 	
			return false;
		}
		
		if (*(uint32_t*)_ulFlashAddr != *(uint16_t*)_ucpSrc)
		{
			return false;
		}
		
		_ulFlashAddr += 2;
		_ucpSrc += 2;
	}	

	return true;
}
