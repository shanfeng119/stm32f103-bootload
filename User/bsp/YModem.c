
#include <string.h>
#include <stdlib.h>

#include "stm32f10x.h"

#include "ymodem.h"
#include "iap.h"
#include "led.h"

#include "iwdg.h"

u8 file_name[FILE_NAME_LENGTH];



//����ת�����ַ���
void YModem_Int2Str(u8* str, s32 intnum)
{
    u32 i, Div = 1000000000, j = 0, Status = 0;

    for (i = 0; i < 10; i++)
    {
        str[j++] = (intnum / Div) + '0';//???????

        intnum = intnum % Div;              
        Div /= 10;
        if ((str[j-1] == '0') & (Status == 0))//??????'0'
        {
            j = 0;
        }
        else
        {
            Status++;
        }
    }
}

static s8 YModem_RecvByte(u8 *c, u32 timeout)
{
    while(timeout-- > 0)
    {
        if(IAP_SerialGetByte(c) == 1)
        {
            return 0;
        }
    }
    return -1;
}

static void YModem_SendByte(u8 c)
{
    IAP_SerialSendByte(c);
}


// 0 normal  -1 error
s8 YModem_RecvPacket(u8 *data, s32 *length, u32 timeout)
{
    u16 i, packet_size;
    u8 c;

    *length = 0;
    if(YModem_RecvByte(&c, timeout) != 0)//�������ݰ��ĵ�һ���ֽ�
    {
        return -1;
    }
    switch(c)
    {
    case SOH:               //128�ֽ����ݰ�
        packet_size = PACKET_SIZE;
        //IAP_SerialSendStr("SOH Packet Size 128\r\n");
        break;
    case STX:               //1024�ֽ����ݰ�
        packet_size = PACKET_1K_SIZE;   
        //IAP_SerialSendStr("SOH Packet Size 1024\r\n");
        break;
    case EOT:               //���ݽ��ս����ַ�
        return 0;           //���ս���       
    case CA:                //������ֹ��־
        if((YModem_RecvByte(&c, timeout) == 0) && (c == CA))//�ȴ�������ֹ
        {
            *length = -1;       //�յ�
            return 0;                    
        }
        else                //��ʱ
        {
            return -1;
        }
    case ABORT1:                //�û���ֹ���û�����'A'
    case ABORT2:                //�û���ֹ���û�����'a'
        return 1;                       //������ֹ
    default:
        return -1;                      //���մ���
    }
    *data = c;                          //�����һ���ֽ�
    for(i = 1; i < (packet_size + PACKET_OVERHEAD); i++)//��������
    {
        if(YModem_RecvByte(data + i, timeout) != 0)
        {
            return -1;
        }
    }   
    if(data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
    {
        return -1;                  //���մ��� 
    }
    *length = packet_size;               //������յ������ݳ���
    return 0;                            //��������
}


// 0���Ͷ���ֹ        -1 �̼�����  -2 flash��д����        -3 �û���ֹ -4 ��������  
s32 YModem_Receive(u8 *buf)
{
    u8 packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH];  
    u8 session_done, file_done, session_begin, packets_received;
    u8 errors = 0;
    u8 *file_ptr, *buf_ptr;
    s32 packet_length = 0, size = 0;
    u32 i = 0,RamSource = 0;
    u8 stu = 0;
    s8 ret = 0;
    u32 total_received = 0;

    for (session_done = 0, errors = 0, session_begin = 0; ;)//��ѭ����һ��ymodem����  
    {
        for (packets_received = 0, file_done = 0, buf_ptr = buf; ; )//��ѭ�������Ͻ�������
        {
            ret = YModem_RecvPacket(packet_data, &packet_length, NAK_TIMEOUT);//�������ݰ�,һ������
            switch(ret)
            {
                case 0: /* ����һ���������� */
                {
                    errors = 0;
                    switch (packet_length) 
                    {
                        case -1: //���Ͷ���ֹ����
                        {
                            YModem_SendByte(ACK);//�ظ�ACK
                            return -4;
                        }
                        case 0: //���ս������߽��մ���
                        {
                            YModem_SendByte(ACK);
                            file_done = 1;//�������
                            stu = 2;
                            break;
                        }
                        default: //�������� ���� 
                        {
                            if((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff)) 
                            {
                                YModem_SendByte(NAK);//���մ��������NAK
                            }
                            else//���յ���ȷ�����ݡ�
                            {
                                ledToggleAll();
                                feed_iwdg();
                                
                                if(stu == 0)//��һ֡
                                {
                                    if( (packets_received == 0) && (packet_data[PACKET_HEADER] != 0) )//���յ�һ֡���� //�����ļ���Ϣ���ļ������ļ����ȵ�
                                    {
                                        for(i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH); )
                                        {
                                            file_name[i++] = *file_ptr++;//�����ļ���
                                        }
                                        file_name[i++] = '\0';//�ļ�����'\0'����

                                        for(i = 0, file_ptr++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH); )
                                        {
                                            file_size[i++] = *file_ptr++;//�����ļ���С
                                        }
                                        file_size[i++] = '\0';//�ļ���С��'\0'����
                                        size = atoi((const char *)file_size);//�ļ���С

                                        if (size > IAP_APP_PROGRAME_MAX_SIZE)//�����̼�����
                                        {
                                            YModem_SendByte(CA);             
                                            YModem_SendByte(CA);//��������2����ֹ��CA
                                            return -1;//����
                                        }
                                        
                                        if (size == 0) /* ����secureCRT6.2.0�汾���������ļ���С ��������ʧ������ 2018.4.26 */
                                        {                                            
                                            IAP_FlashEease(IAP_APP_PROGRAME_MAX_SIZE);//��������app��flash�ռ�
                                        }
                                        else
                                        {
                                            IAP_FlashEease(size);//������Ӧ��flash�ռ�
                                        }
                                        IAP_UpdataParam(&size);//��size��С��д��Flash��Parameter��
                                        YModem_SendByte(ACK);//�ظ�ACk
                                        YModem_SendByte(CRC16);//����'C',ѯ������
                                        stu = 1;
                                    }
                                }
                                else if(stu == 1)//�յ����ݰ�
                                {
                                    memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);//��������
                                    RamSource = (u32)buf;//8λǿ��ת����32Ϊ����
                                    if(IAP_UpdataProgram(RamSource, packet_length) != 0)        //��д�������� 
                                    {
                                        YModem_SendByte(CA);     
                                        YModem_SendByte(CA);//flash��д������������2����ֹ��CA
                                        return -2;//��д����
                                    }
                                    YModem_SendByte(ACK);//flash��д�ɹ����ظ�ACK

                                    total_received += packet_length;
                                }
                                else if(stu == 2){
                                    if(  (packets_received == 0) && (packet_data[PACKET_HEADER] == 0) ){//�ļ���Ϊ�ա�
                                        YModem_SendByte(ACK);//??ACK
                                        file_done = 1;
                                        session_done = 1;//????
                                        break;
                                    }
                                }
                                packets_received++;//�յ����ݰ��ĸ���
                                session_begin = 1;//���ý����б�־

                                
                            }
                        }
                    }
                    break;
                }
                case 1:  /* ���յ��û���ֹ���� */
                {
                    YModem_SendByte(CA);
                    YModem_SendByte(CA);    //��������2����ֹ��CA
                    return -3;      //��д��ֹ
                }
                default: /* ����һ�����ݳ��� */
                {
                    if(session_begin > 0)   //��������з�������
                    {
                        errors++;
                    }
                    if(errors > MAX_ERRORS) //���󳬹�����
                    {
                        YModem_SendByte(CA);
                        YModem_SendByte(CA);//��������������ֹ��CA
                        return -4;  //������̷�����δ���
                    }
                    YModem_SendByte(CRC16); //����'C',��������
                    break;
                }
            }  
            if(file_done != 0)//�ļ�������ϣ��˳�ѭ��
            {
                break;
            }
        }

        
        if(session_done != 0)//�Ի�����������ѭ��
        {
            break;
        }
    }

    if ((size == 0) && (total_received > 1)) /* �ϰ汾6.2.0��secureCRT����Ymodemʱ��sizeΪ0������������� */
    {
        size = total_received; 
    }
    return (s32)size;
}

void YModem_PrepareFirstPacket(u8 *data, const u8 *fileName, u32 *length)
{
    u16 i, j;
    u8 file_size[10];

    data[0] = SOH;                 //128?????
    data[1] = 0x00;                //??????
    data[2] = 0xFF;            //data[2] = ~data[1]
    for(i = 0; (fileName[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
    {
        data[i + PACKET_HEADER] = fileName[i];//?????
    }
    data[i + PACKET_HEADER] = '\0';//????'\0'??

    YModem_Int2Str (file_size, *length);//???????????
    for (j =0, i = i + PACKET_HEADER + 1; file_size[j] != '\0' ; ) 
    {
        data[i++] = file_size[j++];//??????
    }
    for(j = i; j < PACKET_SIZE + PACKET_HEADER; j++)
    {
        data[j] = 0;               //0??
    }
}

void YModem_PrepareDataPacket(u8 *sourceBuf, u8 *data, u8 pktNo, u32 sizeBlk)
{
     u16 i, size, packetSize;
     u8 *file_ptr;

     packetSize = sizeBlk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;//????128?????????1024?????
     size = sizeBlk < packetSize ? sizeBlk : packetSize; //???????????
     if(packetSize == PACKET_1K_SIZE)//1K????
     {
        data[0] = STX;          //???????STX,1024?????
     }
     else               //128????
     {
        data[0] = SOH;          //???????SOH,128????
     }
     data[1] = pktNo;                //?????
     data[2] = (~pktNo);
     file_ptr = sourceBuf;           //?????????

     for(i = PACKET_HEADER; i < size + PACKET_HEADER; i++)
     {
        data[i] = *file_ptr++;  //????????
     }
     if(size <= packetSize)     //??????128??
     {
        for(i = size + PACKET_HEADER; i < packetSize + PACKET_HEADER; i++)
        {
            data[i] = 0x1A; //????,?0x1A??
        }
     }
}


void YModem_PrepareLastPacket(u8 *data)
{
    u8 i = 0;

    data[0] = SOH;   //128?????
    data[1] = 0;     //??
    data[2] = 0xFF;

    for(i = PACKET_HEADER; i < (PACKET_SIZE + PACKET_HEADER); i++)
    {
        data[i] = 0x00;//???0??,?????
    }   
}

u16 UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
    uint32_t crc = crcIn;
    uint32_t in = byte|0x100;
    do
    {
        crc <<= 1;
        in <<= 1;
        if (in&0x100)
            ++crc;      //crc |= 0x01
        if (crc&0x10000)
            crc ^= 0x1021;
    }
    while (!(in&0x10000));
    return crc&0xffffu;
}

u16 Cal_CRC16(const uint8_t* data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = data+size;
    while (data<dataEnd)
        crc = UpdateCRC16(crc,*data++);

    crc = UpdateCRC16(crc,0);
    crc = UpdateCRC16(crc,0);
    return crc&0xffffu;
}


void YModem_SendPacket(u8 *data, u16 length)
{
    u16 i = 0;

    while(i < length)
    {
        YModem_SendByte(data[i]);
        i++;
    }
}

// 0 ok   -1 err
s8 YModem_TransmitFirstPacket(u8 *sendFileName, u32 fileSize)
{
    u8 i, ackReceived = 0, errors = 0, receiveChar = 0;
    u8 firstPacket[PACKET_SIZE + PACKET_HEADER];
    u8 fileName[FILE_NAME_LENGTH];
    u16 tempCRC = 0;    
    
    for(i = 0; i < (FILE_NAME_LENGTH - 1); i++)      
    {
        fileName[i] = sendFileName[i];   //?????
    }   
    YModem_PrepareFirstPacket(firstPacket, fileName, &fileSize);//????????
    do
    {
        YModem_SendPacket(firstPacket, PACKET_SIZE + PACKET_HEADER);//????????
        tempCRC = Cal_CRC16(&firstPacket[3], PACKET_SIZE);//?????
        YModem_SendByte(tempCRC >> 8);                    //??CRC??
        YModem_SendByte(tempCRC & 0xFF);                  //??CRC??

        if(((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == ACK))
         && ((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == CRC16)))
        {
            ackReceived = 1; //????ACK?C??,??????????
        }
        else
        {
            errors++;
        }
    }while(!ackReceived && (errors < 0x0A)); 
    if(errors >= 0x0A)
    {
        return -1;
    }  
    return 0;
}


//  return     : 0-successed -1 -failed    
s8 YModem_TransmitDataPacket(u8 *buf, u32 size)
{
    u8 *buf_ptr = buf;
    u8 blkNumber = 0x01; 
    u8 ackReceived, errors, receiveChar;
    u8 packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
    u16 tempCRC = 0;
    u32 pktSize = 0;
    u32 fileSize = size;

    while(fileSize)       //???????
    {
        ackReceived = 0;
        receiveChar = 0;
        errors = 0;
        YModem_PrepareDataPacket(buf_ptr, packet_data, blkNumber, fileSize);//?????
        do
        {
            if(fileSize >= PACKET_1K_SIZE) //1024?????
            {
                pktSize = PACKET_1K_SIZE;
            }
            else        //128?????
            {
                pktSize = PACKET_SIZE;
            }
            YModem_SendPacket(packet_data, pktSize + PACKET_HEADER);//?????

            tempCRC = Cal_CRC16(&packet_data[3], pktSize);//?????
            YModem_SendByte(tempCRC >> 8);                //??CRC??
            YModem_SendByte(tempCRC & 0xFF);              //??CRC??

            if((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == ACK))
            {
                ackReceived = 1;//??ACK
                if(fileSize > pktSize)
                {
                    buf_ptr += pktSize;//??????????
                    fileSize -= pktSize;//???????
                    if(blkNumber == (size/1024 + 3))//?????????
                    {
                        return -1;
                    }
                    else 
                    {
                        blkNumber++;                      
                    }
                } 
                else
                {
                    buf_ptr += pktSize;
                    fileSize = 0;
                }
            }
            else
            {
                errors++;
            }
        }while (!ackReceived && (errors < 0x0A));
        if(errors >= 0x0A)
        {
            return -1;
        }
    }
    return 0;
}


//  return     : 0-successed -1 -failed    
s8 YModem_TransmitLastPacket(void)
{
    u8 ackReceived = 0, receiveChar = 0, errors = 0;
    u8 lastPacket[PACKET_SIZE + PACKET_OVERHEAD];
    u16 tempCRC = 0;

    YModem_PrepareLastPacket(lastPacket);
    do
    {
        YModem_SendPacket(lastPacket, PACKET_SIZE + PACKET_HEADER);//?????
        tempCRC = Cal_CRC16(&lastPacket[3], PACKET_SIZE);//??CRC???
                YModem_SendByte(tempCRC >> 8);  //??CRC??
                YModem_SendByte(tempCRC & 0xFF);//??CRC??

        if((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == ACK))
        {
            ackReceived = 1; //??ACK
        }
        else
        {
            errors++;
        }
    }while(!ackReceived && (errors < 0x0A));
    if(errors >= 0x0A)
    {
        return -1;
    }
    return 0;
}

s8 YModem_TransmitFirstEOT(void)
{
    u8 ackReceived = 0, receiveChar = 0, errors = 0;
    do
    {
        YModem_SendByte(EOT);//??????
        if((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == NAK))
        {
            ackReceived = 1;//??ACK, ??????EOT??
        }
        else
        {
            errors++;
        }
    }while(!ackReceived && (errors < 0x0A));
    if(errors >= 0x0A)
    {
        return -1;
    }
    return 0;
}

s8 YModem_TransmitSecondEOT(void)
{
    u8 ackReceived = 0, receiveChar = 0, errors = 0;
    do
    {
        YModem_SendByte(EOT);     //??????
        if(((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == ACK))
        && ((YModem_RecvByte(&receiveChar, NAK_TIMEOUT) == 0) && (receiveChar == CRC16)))
        {
            ackReceived = 1;//????ACK?C??,?????????
        }
        else
        {
            errors++;
        }
    }while(!ackReceived && (errors < 0x0A));
    if(errors >= 0x0A)
    {
        return -1;
    }
    return 0;
}

s8 YModem_Transmit(u8 *buf, u8 *sendFileName, u32 fileSize)
{
    s8 result = 0;
    result = YModem_TransmitFirstPacket(sendFileName, fileSize);//???????
    if(result != 0) return result;
    result = YModem_TransmitDataPacket(buf, fileSize);//??????
    if(result != 0) return result;
    result = YModem_TransmitFirstEOT();//?????????
    if(result != 0) return result;  
    result = YModem_TransmitSecondEOT();//?????????
    if(result != 0) return result;  
    result = YModem_TransmitLastPacket();//?????????
    if(result != 0) return result;
    return 0;;
}



