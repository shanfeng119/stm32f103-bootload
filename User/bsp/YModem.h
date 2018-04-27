

#ifndef __YMODEM_H
#define __YMODEM_H

#include "stm32f10x.h"

#define PACKET_SEQNO_INDEX      (1)       //数据包序号
#define PACKET_SEQNO_COMP_INDEX (2)       //包序取反

#define PACKET_HEADER           (3)	  //首部3位
#define PACKET_TRAILER          (2)	  //CRC检验的2位
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)//3位首部+2位CRC
#define PACKET_SIZE             (128)     //128字节
#define PACKET_1K_SIZE          (1024)    //1024字节

#define FILE_NAME_LENGTH        (256)     //文件最大长度
#define FILE_SIZE_LENGTH        (16)      //文件大小

#define SOH                     (0x01)    //128字节数据包开始
#define STX                     (0x02)    //1024字节的数据包开始
#define EOT                     (0x04)    //结束传输
#define ACK                     (0x06)    //回应
#define NAK                     (0x15)    //没回应
#define CA                      (0x18)    //这两个相继中止转移
#define CRC16                   (0x43)    //'C'即0x43, 需要 16-bit CRC 

#define ABORT1                  (0x41)    //'A'即0x41, 用户终止 
#define ABORT2                  (0x61)    //'a'即0x61, 用户终止

#define NAK_TIMEOUT             (0x100000)//最大超时时间
#define MAX_ERRORS              (5)       //错误上限


void YModem_Int2Str(uint8_t* str, int32_t intnum);
s32 YModem_Receive(u8 *buf);
s8 YModem_Transmit(u8 *buf, u8 *sendFileName, u32 fileSize);


#endif

