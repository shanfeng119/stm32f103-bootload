

#ifndef __YMODEM_H
#define __YMODEM_H

#include "stm32f10x.h"

#define PACKET_SEQNO_INDEX      (1)       //���ݰ����
#define PACKET_SEQNO_COMP_INDEX (2)       //����ȡ��

#define PACKET_HEADER           (3)	  //�ײ�3λ
#define PACKET_TRAILER          (2)	  //CRC�����2λ
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)//3λ�ײ�+2λCRC
#define PACKET_SIZE             (128)     //128�ֽ�
#define PACKET_1K_SIZE          (1024)    //1024�ֽ�

#define FILE_NAME_LENGTH        (256)     //�ļ���󳤶�
#define FILE_SIZE_LENGTH        (16)      //�ļ���С

#define SOH                     (0x01)    //128�ֽ����ݰ���ʼ
#define STX                     (0x02)    //1024�ֽڵ����ݰ���ʼ
#define EOT                     (0x04)    //��������
#define ACK                     (0x06)    //��Ӧ
#define NAK                     (0x15)    //û��Ӧ
#define CA                      (0x18)    //�����������ֹת��
#define CRC16                   (0x43)    //'C'��0x43, ��Ҫ 16-bit CRC 

#define ABORT1                  (0x41)    //'A'��0x41, �û���ֹ 
#define ABORT2                  (0x61)    //'a'��0x61, �û���ֹ

#define NAK_TIMEOUT             (0x100000)//���ʱʱ��
#define MAX_ERRORS              (5)       //��������


void YModem_Int2Str(uint8_t* str, int32_t intnum);
s32 YModem_Receive(u8 *buf);
s8 YModem_Transmit(u8 *buf, u8 *sendFileName, u32 fileSize);


#endif

