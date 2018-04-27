
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H



#define Page_Size               (256)
#define NOR_FLASH_SIZE          (8<<20)            //8M
#define NOR_Sector_SIZE         (4<<10)            //4KB
#define NOR_BLOCK_SIZE          (32<<10)           //32KB



// ���������ļ���Ϣ
#define UPDATE_FILE_INFO_ADDR		(NOR_Sector_SIZE*4)

// ���������ļ�����  �ܴ�СΪ256K
#define UPDATE_FILE_DATA_ADDR		(5<<20)
#define UPDATE_FILE_DATA_END_ADDR	((5<<20)+(256<<10))


void spi_flash_init(void);

// ��ȡ�����ļ�����
void flash_read_update_data(int addr, unsigned char *data, int data_len);

// ��ȡ�����ļ���Ϣ
void flash_read_update_info(unsigned char *buf, unsigned char *info_len);
void flash_erase_update_info(void);



#endif
