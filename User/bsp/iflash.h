
#ifndef __IFLASH_H
#define __IFLASH_H

void iflash_Init(void);

int iflash_erase(unsigned int addr, int size);

int if_flash_write(unsigned int _ulFlashAddr, unsigned char *_ucpSrc, unsigned int _ulSize);

unsigned short if_readHalfWord(unsigned int addr);

#endif

