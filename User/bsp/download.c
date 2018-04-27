
#include "stm32f10x.h"
#include "iap.h"
#include "dwt.h"

#include "ymodem.h"

#include "iwdg.h"

extern u8 file_name[FILE_NAME_LENGTH];   
u8 tab_1024[1024 + PACKET_OVERHEAD] = {0};


void DownloadFirmware(void)
{
    char number[10]= "          ";        
    s32 size = 0;

    IAP_SerialSendStr("\r\nWaiting for the file to be send...(press 'a+enter' or 'A+enter' to abort)\r\n");
    feed_iwdg();
    size = YModem_Receive(&tab_1024[0]);
    feed_iwdg();
    delay_ms(2000);
    if(size > 0)
    {
        IAP_SerialSendStr("+-----------------------------------+\r\n");
        IAP_SerialSendStr("Proramming completed successfully!\r\nName: ");
        IAP_SerialSendStr((char *)file_name);
        YModem_Int2Str((uint8_t *)number, size);
        IAP_SerialSendStr("\r\nSize:");
        IAP_SerialSendStr(number);
        IAP_SerialSendStr("Bytes\r\n"); 
        IAP_SerialSendStr("+-----------------------------------+\r\n"); 
    }
    else if(size == -1)//      -1 固件过大  
    {
       IAP_SerialSendStr("\r\nThe image size is higher than the allowed space memory!\r\n");
    }
    else if(size == -2)//-2 flash烧写错误
    {
        IAP_SerialSendStr("\r\nVerification failed!\r\n");
    }
    else if(size == -3)//   -3 用户终止
    {
        IAP_SerialSendStr("\r\nAborted by user!\r\n");
    }
    else if(size == -4)//其他错误
    {
        IAP_SerialSendStr("\r\nFailed to receive the file!\r\n");   
    }

    return;
}
