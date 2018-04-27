
#include "iap.h"

#include "ymodem.h"


void UploadFirmware(void)
{
	u32 status = 0;
	u32 imageSize = 0;

	IAP_SerialSendStr("\r\nBeginning to receive file...(press any key to abort)\r\n");
	if(IAP_GetKey() == CRC16)//????'C',??ymodem????
	{
		imageSize = *(u32 *)IAP_PARAM_ADDR;//???IAP_PARAM_ADDR??????????
		status = YModem_Transmit((u8 *)IAP_ADDR, (u8 *)"Firmware.bin", imageSize);
		if(status != 0)	//????
		{
			IAP_SerialSendStr("\r\nError Occured while transmitting file!\r\n");
		}
		else//????
		{
			IAP_SerialSendStr("\r\nFile Transmitted successfully!\r\n");
		}
	}
	else//????
	{
		IAP_SerialSendStr("\r\nAbort by user!\r\n");
	}
}
