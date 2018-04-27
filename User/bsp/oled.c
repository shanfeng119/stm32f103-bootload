

#include "includes.h"


/*
	SPI ģʽ���߶��� (ֻ��Ҫ6���Ű�������)  �����Ӳ������ģ��SPIʱ��
*/
#define RCC_OLED_PORT (RCC_AHB1Periph_GPIOB)		/* GPIO�˿�ʱ�� */

#define OLED_CS_PORT	GPIOB
#define OLED_CS_PIN		GPIO_Pin_0

#define OLED_RS_PORT	GPIOB
#define OLED_RS_PIN		GPIO_Pin_1

#define OLED_SDIN_PORT	GPIOB
#define OLED_SDIN_PIN	GPIO_Pin_10

#define OLED_SCK_PORT	GPIOB
#define OLED_SCK_PIN	GPIO_Pin_11

#define OLED_LED_PORT	
#define OLED_LED_PIN	

#define OLED_RST_PORT	GPIOB
#define OLED_RST_PIN	GPIO_Pin_9

/* ����IO = 1�� 0�Ĵ��� �����ø��ģ� */
#define SSD_CS_1()		(GPIO_SetBits(OLED_CS_PORT, OLED_CS_PIN))
#define SSD_CS_0()		(GPIO_ResetBits(OLED_CS_PORT, OLED_CS_PIN))

#define SSD_SCK_1()		(GPIO_SetBits(OLED_SCK_PORT, OLED_SCK_PIN))
#define SSD_SCK_0()		(GPIO_ResetBits(OLED_SCK_PORT, OLED_SCK_PIN))

#define SSD_SDIN_1()	(GPIO_SetBits(OLED_SDIN_PORT, OLED_SDIN_PIN))
#define SSD_SDIN_0()	(GPIO_ResetBits(OLED_SDIN_PORT, OLED_SDIN_PIN))

#define SSD_RS_1()		(GPIO_SetBits(OLED_RS_PORT, OLED_RS_PIN))
#define SSD_RS_0()		(GPIO_ResetBits(OLED_RS_PORT, OLED_RS_PIN))

#define SSD_LED_1()		
#define SSD_LED_0()		

#define SSD_RST_1()		(GPIO_SetBits(OLED_RST_PORT, OLED_RST_PIN))
#define SSD_RST_0()		(GPIO_SetBits(OLED_RST_PORT, OLED_RST_PIN))


// ����
#define SetColumnAddressLSB		0x00			//��ַ��λ
#define SetColumnAddressMSB		0x10			//��ַ��λ
#define SetPowerControl			0x28			//��Դ����
#define SetScrollLine			0x40			//������
#define SetPageAddress			0xB0			//ҳ��ַ
#define SetLCDContrast			0x20			//�Աȶ�

#define SetContrastRefBase		0x81			//�ԱȶȻ���ַ
#define SetContrastRef			0x00			//�Աȶȿɱ�ֵ

#define SetAllPielOn			0xA4			
#define SetInverseDisplay		0xA6			//������ʾ
#define SetDisplayEnable		0xAE			//ʹ����ʾ
#define SetXDirection			0xA0			//ˮƽ����
#define SetYDirection			0xC0			//��ֱ����
#define SetReset				0xE2			//�����λ
#define Nop						0xE3			//�ղ���
#define SetLcdBiasRatio			0xA2			//����ƫѹ
#define SetCursorUpdateMode		0xE0			//��λ������ģʽ
#define ResetCursorUpdateMode	0xEE			//��λ������ģʽ
#define SetIndicatorOff			0xAC			//�ر�ָ��
#define SetIndicatorOn			0xAD00			//��ָ��
#define SetBoosterRatio			0xF800			//Booster����
#define AdvanceProgramControl0	0xFA10			//�߼�����

enum Direction
{
	LeftToRight = SetXDirection | 0,			//������ʾ
	RightToLeft = SetXDirection | 0x01,			//�ҵ�����ʾ
	UpToDown	= SetYDirection | 0x08,			//�ϵ�����ʾ
	DownToUp	= SetYDirection | 0,			//�µ�����ʾ
};

enum InverseDisplay
{
	EnableInverse  = SetInverseDisplay | 1,		//ʹ�ܷ�����ʾ
	DisableInverse = SetInverseDisplay |0		//�رշ�����ʾ
};

enum DisplayEnable
{
	EnableDisplay	= SetDisplayEnable | 1,			//����ʾ
	DisableDisplay	= SetDisplayEnable | 0			//�ر���ʾ
};

/* 12864 OLED���Դ澵��ռ��1K�ֽ�. ��8�У�ÿ��128���� */
static uint8_t s_ucGRAM[8][128];

/* Ϊ�˱���ˢ����Ļ��̫ǿ������ˢ����־��
0 ��ʾ��ʾ����ֻ��д����������д����1 ��ʾֱ��д����ͬʱд�������� */
static uint8_t s_ucUpdateEn = 1;

static void OLED_ConfigGPIO(void);
static void OLED_WriteCmd(uint8_t _ucCmd);
static void OLED_WriteData(uint8_t _ucData);
static void OLED_BufToPanel(void);

/*
*********************************************************************************************************
*	�� �� ��: OLED_InitHard
*	����˵��: ��ʼ��OLED��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_InitHard(void)
{
	OLED_ConfigGPIO();

	/* �ϵ��ӳ� */
	SSD_RST_0();
	delay_ms(100);
	SSD_RST_1();

	 /* ģ�鳧���ṩ��ʼ������ */
	OLED_WriteCmd(SetReset);			// soft reset
	delay_ms(5);
	OLED_WriteCmd(0x2C);				// step1
	delay_ms(5);
	OLED_WriteCmd(0x2E);				// step2
	delay_ms(5);
	OLED_WriteCmd(0x2F);				// step3
	delay_ms(5);
	
	OLED_WriteCmd(DisableInverse);
	
	OLED_WriteCmd(SetLCDContrast|4);				// �ֵ��Աȶ� 20-27
	OLED_WriteCmd(SetContrastRefBase);				// ΢���Աȶ� 
	OLED_WriteCmd(SetContrastRef|0x1A);				// 00 - 3F
	OLED_WriteCmd(0xA2);							// 1/9ƫѹ��
	OLED_WriteCmd(UpToDown);						// ��ɨ�� ���ϵ���
	OLED_WriteCmd(LeftToRight);						// ��ɨ�� ������
	OLED_WriteCmd(SetScrollLine);					// ��ʼ�� ��һ��
	OLED_WriteCmd(0xAF);							// ����ʾ
	
	OLED_ClrScr(0x00);								// ����
}


void OLED_DispOn(void)
{
	OLED_WriteCmd(0xAF);	
}

void OLED_DispOff(void)
{
	OLED_WriteCmd(0xAE);	
}

void OLED_SetInverse(int Inverse)
{
	if(Inverse == 0){
		OLED_WriteCmd(DisableInverse);
	}else{
		OLED_WriteCmd(EnableInverse);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_SetDir
*	����˵��: ������ʾ����
*	��    ��: _ucDir = 0 ��ʾ��������1��ʾ��ת180��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_SetDir(unsigned char left_right, unsigned char up_down)
{

	OLED_WriteCmd(left_right);	/* A0 ���е�ַ0ӳ�䵽SEG0; A1 ���е�ַ127ӳ�䵽SEG0 */
	OLED_WriteCmd(up_down);	/* C0 ������ɨ��,��COM0��COM63;  C8 : ����ɨ��, �� COM63�� COM0 */
}


void OLED_SetContrast(uint8_t ucValue)
{
	OLED_WriteCmd(0x81);	/* ���öԱȶ�����(˫�ֽ��������1���ֽ��������2���ֽ��ǶԱȶȲ���0-255 */
	OLED_WriteCmd(ucValue);	/* ���öԱȶȲ���,ȱʡCF */
}


void OLED_StartDraw(void)
{
	s_ucUpdateEn = 0;
}


void OLED_EndDraw(void)
{
	s_ucUpdateEn = 1;
	OLED_BufToPanel();
}


void OLED_ClrScr(uint8_t _ucMode)
{
	uint8_t i,j;

	for (i = 0 ; i < 8; i++)
	{
		for (j = 0 ; j < 128; j++)
		{
			s_ucGRAM[i][j] = _ucMode;
		}
	}

	if (s_ucUpdateEn == 1)
	{
		OLED_BufToPanel();
	}
}


static void OLED_BufToPanel(void)
{
	uint8_t i,j;

	for (i = 0 ; i< 8; i++)
	{
		OLED_WriteCmd (0xB0 + i);	/* ����ҳ��ַ��0~7��  */
		OLED_WriteCmd (0x00);		/* �����е�ַ�ĵ͵�ַ */
		OLED_WriteCmd (0x10);		/* �����е�ַ�ĸߵ�ַ */

		for (j = 0 ; j < 128; j++)
		{
			OLED_WriteData(s_ucGRAM[i][j]);
		}
	}
}

void oled_test(void)
{
	FONT_T  tFont16;
	
	/* ����������� */
	{
		tFont16.FontCode = FC_ST_16;	/* ������� 16���� */
		tFont16.FrontColor = 1;		/* ������ɫ 0 �� 1 */
		tFont16.BackColor = 0;		/* ���ֱ�����ɫ 0 �� 1 */
		tFont16.Space = 0;			/* ���ּ�࣬��λ = ���� */
	}
	
	OLED_SetDir(LeftToRight, UpToDown);
	
	OLED_DispStr(0, 0, "�������ǻƺ�¥", &tFont16);
	OLED_DispStr(0, 16, "�̻�����������", &tFont16);
	
	OLED_DispStr(0, 32, "�·�ԶӰ�̿վ�", &tFont16);
	OLED_DispStr(0, 48, "Ψ�����������", &tFont16);
}
/*
*********************************************************************************************************
*	�� �� ��: OLED_DispStr
*	����˵��: ����Ļָ�����꣨���Ͻ�Ϊ0��0����ʾһ���ַ���
*	��    ��:
*		_usX : X���꣬����12864������ΧΪ��0 - 127��
*		_usY : Y���꣬����12864������ΧΪ��0 - 63��
*		_ptr  : �ַ���ָ��
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address = 0;
	uint8_t buf[32 * 32 / 8];	/* ���֧��32������ */
	uint8_t m, width;
	uint8_t font_width,font_height, font_bytes;
	uint16_t x, y;
	const uint8_t *pAscDot;	
	
	const uint8_t *pHzDot;


	/* �������ṹΪ��ָ�룬��ȱʡ��16���� */
	if (_tFont->FontCode == FC_ST_12)
	{
		font_height = 12;
		font_width = 12;
		font_bytes = 24;
		pAscDot = g_Ascii12;
		pHzDot = g_Hz12;
	}
	else
	{
		/* ȱʡ��16���� */
		font_height = 16;
		font_width = 16;
		font_bytes = 32;
		pAscDot = g_Ascii16;	
		pHzDot = g_Hz16;
	}

	/* ��ʼѭ�������ַ� */
	while (*_ptr != 0)
	{
		code1 = *_ptr;	/* ��ȡ�ַ������ݣ� �����ݿ�����ascii���룬Ҳ���ܺ��ִ���ĸ��ֽ� */
		if (code1 < 0x80)
		{
			/* ��ascii�ַ������Ƶ�buf */
			memcpy(buf, &pAscDot[code1 * (font_bytes / 2)], (font_bytes / 2));
			width = font_width / 2;
		}
		else
		{
			code2 = *++_ptr;
			if (code2 == 0)
			{
				break;
			}

			/* ����16�����ֵ����ַ
				ADDRESS = [(code1-0xa1) * 94 + (code2-0xa1)] * 32
				;
			*/
			m = 0;
			while(1)
			{
				address = m * (font_bytes + 2);
				m++;
				if ((code1 == pHzDot[address + 0]) && (code2 == pHzDot[address + 1]))
				{
					address += 2;
					memcpy(buf, &pHzDot[address], font_bytes);
					break;
				}
				else if ((pHzDot[address + 0] == 0xFF) && (pHzDot[address + 1] == 0xFF))
				{
					/* �ֿ�������ϣ�δ�ҵ��������ȫFF */
					memset(buf, 0xFF, font_bytes);
					break;
				}
			}

			width = font_width;
		}

		y = _usY;
		/* ��ʼˢLCD */
		for (m = 0; m < font_height; m++)	/* �ַ��߶� */
		{
			x = _usX;
			for (i = 0; i < width; i++)	/* �ַ���� */
			{
				if ((buf[m * ((2 * width) / font_width) + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
				{
					OLED_PutPixel(x, y, _tFont->FrontColor);	/* ����������ɫΪ����ɫ */
				}
				else
				{
					OLED_PutPixel(x, y, _tFont->BackColor);	/* ����������ɫΪ���ֱ���ɫ */
				}
				
				x++;
			}
			y++;
		}

		if (_tFont->Space > 0)
		{
			/* ������ֵ�ɫ��_tFont->usBackColor�������ּ����ڵ���Ŀ�ȣ���ô��Ҫ������֮�����(��ʱδʵ��) */
		}
		_usX += width + _tFont->Space;	/* �е�ַ���� */
		_ptr++;			/* ָ����һ���ַ� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_PutPixel
*	����˵��: ��1������
*	��    ��:
*			_usX,_usY : ��������
*			_ucColor  ��������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_PutPixel(uint16_t _usX, uint16_t _usY, uint8_t _ucColor)
{
	uint8_t ucValue;
	uint8_t ucPageAddr;
	uint8_t ucColAddr;

	const uint8_t aOrTab[8]  = {0x01, 0x02, 0x04, 0x08,0x10,0x20,0x40,0x80};
	const uint8_t aAndTab[8] = {0xFE, 0xFD, 0xFB, 0xF7,0xEF,0xDF,0xBF,0x7F};

	ucPageAddr = _usY / 8;
	ucColAddr = _usX;

	ucValue = s_ucGRAM[ucPageAddr][ucColAddr];
	if (_ucColor == 0)
	{
		ucValue &= aAndTab[_usY % 8];
	}
	else
	{
		ucValue |= aOrTab[_usY % 8];
	}
	s_ucGRAM[ucPageAddr][ucColAddr] = ucValue;

	if (s_ucUpdateEn == 1)
	{
		OLED_WriteCmd (0xB0 + ucPageAddr);					/* ����ҳ��ַ��0~7�� */
		OLED_WriteCmd (0x00 + (ucColAddr & 0x0F));			/* �����е�ַ�ĵ͵�ַ */
		OLED_WriteCmd (0x10 + ((ucColAddr >> 4) & 0x0F));	/* �����е�ַ�ĸߵ�ַ */
		OLED_WriteData(ucValue);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_GetPixel
*	����˵��: ��ȡ1������
*	��    ��:
*			_usX,_usY : ��������
*	�� �� ֵ: ��ɫֵ (0, 1)
*********************************************************************************************************
*/
uint8_t OLED_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint8_t ucValue;
	uint8_t ucPageAddr;
	uint8_t ucColAddr;

	ucPageAddr = _usY / 8;
	ucColAddr = _usX;

	ucValue = s_ucGRAM[ucPageAddr][ucColAddr];
	if (ucValue & (_usY % 8))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_DrawLine
*	����˵��: ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
*	��    ��:
*			_usX1, _usY1 ����ʼ������
*			_usX2, _usY2 ����ֹ��Y����
*			_ucColor     ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint8_t _ucColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* ���� Bresenham �㷨����2��仭һ��ֱ�� */

	OLED_PutPixel(_usX1 , _usY1 , _ucColor);

	/* ��������غϣ���������Ķ�����*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*���dyΪ�Ƴ������򽻻��ݺ����ꡣ*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* ȷ������1���Ǽ�1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* ѭ������ */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			OLED_PutPixel ( y , x , _ucColor) ;
		}
		else
		{
			OLED_PutPixel ( x , y , _ucColor) ;
		}
		x += tx ;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_DrawPoints
*	����˵��: ���� Bresenham �㷨������һ��㣬������Щ�����������������ڲ�����ʾ��
*	��    ��:
*			x, y     ����������
*			_ucColor ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint8_t _ucColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		OLED_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _ucColor);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_DrawRect
*	����˵��: ���ƾ��Ρ�
*	��    ��:
*			_usX,_usY���������Ͻǵ�����
*			_usHeight �����εĸ߶�
*			_usWidth  �����εĿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_DrawRect(uint16_t _usX, uint16_t _usY, uint8_t _usHeight, uint16_t _usWidth, uint8_t _ucColor)
{
	/*
	 ---------------->---
	|(_usX��_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	OLED_DrawLine(_usX, _usY, _usX + _usWidth - 1, _usY, _ucColor);	/* �� */
	OLED_DrawLine(_usX, _usY + _usHeight - 1, _usX + _usWidth - 1, _usY + _usHeight - 1, _ucColor);	/* �� */

	OLED_DrawLine(_usX, _usY, _usX, _usY + _usHeight - 1, _ucColor);	/* �� */
	OLED_DrawLine(_usX + _usWidth - 1, _usY, _usX + _usWidth - 1, _usY + _usHeight, _ucColor);	/* �� */
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_DrawCircle
*	����˵��: ����һ��Բ���ʿ�Ϊ1������
*	��    ��:
*			_usX,_usY  ��Բ�ĵ�����
*			_usRadius  ��Բ�İ뾶
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint8_t _ucColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{
		OLED_PutPixel(_usX + CurX, _usY + CurY, _ucColor);
		OLED_PutPixel(_usX + CurX, _usY - CurY, _ucColor);
		OLED_PutPixel(_usX - CurX, _usY + CurY, _ucColor);
		OLED_PutPixel(_usX - CurX, _usY - CurY, _ucColor);
		OLED_PutPixel(_usX + CurY, _usY + CurX, _ucColor);
		OLED_PutPixel(_usX + CurY, _usY - CurX, _ucColor);
		OLED_PutPixel(_usX - CurY, _usY + CurX, _ucColor);
		OLED_PutPixel(_usX - CurY, _usY - CurX, _ucColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_DrawBMP
*	����˵��: ��LCD����ʾһ��BMPλͼ��λͼ����ɨ����򣺴����ң����ϵ���
*	��    ��:
*			_usX, _usY : ͼƬ������
*			_usHeight  ��ͼƬ�߶�
*			_usWidth   ��ͼƬ���
*			_ptr       ����ɫͼƬ����ָ�룬ÿ������ռ��1���ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void OLED_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t *_ptr)
{
	uint16_t x, y;

	for (x = 0; x < _usWidth; x++)
	{
		for (y = 0; y < _usHeight; y++)
		{
			OLED_PutPixel(_usX + x, _usY + y, *_ptr);
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_ConfigGPIO
*	����˵��: ����OLED���ƿ��ߣ�����Ϊ8λ80XX���߿���ģʽ��SPIģʽ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void OLED_ConfigGPIO(void)
{
	/* 12.����GPIO */
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_WriteCmd
*	����˵��: ��SSD1306����һ�ֽ�����
*	��    ��:  ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void OLED_WriteCmd(uint8_t _ucCmd)
{
	uint8_t i;

	SSD_CS_0();
	
	SSD_RS_0();

	for (i = 0; i < 8; i++)
	{
		SSD_SCK_0();
		if (_ucCmd & 0x80)
		{
			SSD_SDIN_1();
		}
		else
		{
			SSD_SDIN_0();
		}
		SSD_SCK_1();
		_ucCmd <<= 1;
	}

	SSD_CS_1();
}

/*
*********************************************************************************************************
*	�� �� ��: OLED_WriteData
*	����˵��: ��SSD1306����һ�ֽ�����
*	��    ��:  ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void OLED_WriteData(uint8_t _ucData)
{
	uint8_t i;

	SSD_CS_0();

	SSD_RS_1();

	for (i = 0; i < 8; i++)
	{
		SSD_SCK_0();
		if (_ucData & 0x80)
		{
			SSD_SDIN_1();
		}
		else
		{
			SSD_SDIN_0();
		}
		SSD_SCK_1();
		_ucData <<= 1;
	}

	SSD_CS_1();

}
