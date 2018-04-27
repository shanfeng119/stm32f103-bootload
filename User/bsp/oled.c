

#include "includes.h"


/*
	SPI 模式接线定义 (只需要6根杜邦线连接)  本例子采用软件模拟SPI时序
*/
#define RCC_OLED_PORT (RCC_AHB1Periph_GPIOB)		/* GPIO端口时钟 */

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

/* 定义IO = 1和 0的代码 （不用更改） */
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


// 命令
#define SetColumnAddressLSB		0x00			//地址低位
#define SetColumnAddressMSB		0x10			//地址高位
#define SetPowerControl			0x28			//电源控制
#define SetScrollLine			0x40			//滚动行
#define SetPageAddress			0xB0			//页地址
#define SetLCDContrast			0x20			//对比度

#define SetContrastRefBase		0x81			//对比度基地址
#define SetContrastRef			0x00			//对比度可变值

#define SetAllPielOn			0xA4			
#define SetInverseDisplay		0xA6			//泛白显示
#define SetDisplayEnable		0xAE			//使能显示
#define SetXDirection			0xA0			//水平方向
#define SetYDirection			0xC0			//垂直方向
#define SetReset				0xE2			//软件复位
#define Nop						0xE3			//空操作
#define SetLcdBiasRatio			0xA2			//设置偏压
#define SetCursorUpdateMode		0xE0			//置位光标更新模式
#define ResetCursorUpdateMode	0xEE			//复位光标更新模式
#define SetIndicatorOff			0xAC			//关闭指针
#define SetIndicatorOn			0xAD00			//打开指针
#define SetBoosterRatio			0xF800			//Booster速率
#define AdvanceProgramControl0	0xFA10			//高级控制

enum Direction
{
	LeftToRight = SetXDirection | 0,			//左到右显示
	RightToLeft = SetXDirection | 0x01,			//右到左显示
	UpToDown	= SetYDirection | 0x08,			//上到下显示
	DownToUp	= SetYDirection | 0,			//下到上显示
};

enum InverseDisplay
{
	EnableInverse  = SetInverseDisplay | 1,		//使能泛白显示
	DisableInverse = SetInverseDisplay |0		//关闭泛白显示
};

enum DisplayEnable
{
	EnableDisplay	= SetDisplayEnable | 1,			//打开显示
	DisableDisplay	= SetDisplayEnable | 0			//关闭显示
};

/* 12864 OLED的显存镜像，占用1K字节. 共8行，每行128像素 */
static uint8_t s_ucGRAM[8][128];

/* 为了避免刷屏拉幕感太强，引入刷屏标志。
0 表示显示函数只改写缓冲区，不写屏。1 表示直接写屏（同时写缓冲区） */
static uint8_t s_ucUpdateEn = 1;

static void OLED_ConfigGPIO(void);
static void OLED_WriteCmd(uint8_t _ucCmd);
static void OLED_WriteData(uint8_t _ucData);
static void OLED_BufToPanel(void);

/*
*********************************************************************************************************
*	函 数 名: OLED_InitHard
*	功能说明: 初始化OLED屏
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_InitHard(void)
{
	OLED_ConfigGPIO();

	/* 上电延迟 */
	SSD_RST_0();
	delay_ms(100);
	SSD_RST_1();

	 /* 模块厂家提供初始化代码 */
	OLED_WriteCmd(SetReset);			// soft reset
	delay_ms(5);
	OLED_WriteCmd(0x2C);				// step1
	delay_ms(5);
	OLED_WriteCmd(0x2E);				// step2
	delay_ms(5);
	OLED_WriteCmd(0x2F);				// step3
	delay_ms(5);
	
	OLED_WriteCmd(DisableInverse);
	
	OLED_WriteCmd(SetLCDContrast|4);				// 粗调对比度 20-27
	OLED_WriteCmd(SetContrastRefBase);				// 微调对比度 
	OLED_WriteCmd(SetContrastRef|0x1A);				// 00 - 3F
	OLED_WriteCmd(0xA2);							// 1/9偏压比
	OLED_WriteCmd(UpToDown);						// 行扫描 从上到下
	OLED_WriteCmd(LeftToRight);						// 列扫描 从左到右
	OLED_WriteCmd(SetScrollLine);					// 起始行 第一行
	OLED_WriteCmd(0xAF);							// 开显示
	
	OLED_ClrScr(0x00);								// 清屏
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
*	函 数 名: OLED_SetDir
*	功能说明: 设置显示方向
*	形    参: _ucDir = 0 表示正常方向，1表示翻转180度
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_SetDir(unsigned char left_right, unsigned char up_down)
{

	OLED_WriteCmd(left_right);	/* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
	OLED_WriteCmd(up_down);	/* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
}


void OLED_SetContrast(uint8_t ucValue)
{
	OLED_WriteCmd(0x81);	/* 设置对比度命令(双字节命令），第1个字节是命令，第2个字节是对比度参数0-255 */
	OLED_WriteCmd(ucValue);	/* 设置对比度参数,缺省CF */
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
		OLED_WriteCmd (0xB0 + i);	/* 设置页地址（0~7）  */
		OLED_WriteCmd (0x00);		/* 设置列地址的低地址 */
		OLED_WriteCmd (0x10);		/* 设置列地址的高地址 */

		for (j = 0 ; j < 128; j++)
		{
			OLED_WriteData(s_ucGRAM[i][j]);
		}
	}
}

void oled_test(void)
{
	FONT_T  tFont16;
	
	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		tFont16.FrontColor = 1;		/* 字体颜色 0 或 1 */
		tFont16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		tFont16.Space = 0;			/* 文字间距，单位 = 像素 */
	}
	
	OLED_SetDir(LeftToRight, UpToDown);
	
	OLED_DispStr(0, 0, "故人西辞黄鹤楼", &tFont16);
	OLED_DispStr(0, 16, "烟花三月下扬州", &tFont16);
	
	OLED_DispStr(0, 32, "孤帆远影碧空尽", &tFont16);
	OLED_DispStr(0, 48, "唯见长江天际流", &tFont16);
}
/*
*********************************************************************************************************
*	函 数 名: OLED_DispStr
*	功能说明: 在屏幕指定坐标（左上角为0，0）显示一个字符串
*	形    参:
*		_usX : X坐标，对于12864屏，范围为【0 - 127】
*		_usY : Y坐标，对于12864屏，范围为【0 - 63】
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address = 0;
	uint8_t buf[32 * 32 / 8];	/* 最大支持32点阵汉字 */
	uint8_t m, width;
	uint8_t font_width,font_height, font_bytes;
	uint16_t x, y;
	const uint8_t *pAscDot;	
	
	const uint8_t *pHzDot;


	/* 如果字体结构为空指针，则缺省按16点阵 */
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
		/* 缺省是16点阵 */
		font_height = 16;
		font_width = 16;
		font_bytes = 32;
		pAscDot = g_Ascii16;	
		pHzDot = g_Hz16;
	}

	/* 开始循环处理字符 */
	while (*_ptr != 0)
	{
		code1 = *_ptr;	/* 读取字符串数据， 该数据可能是ascii代码，也可能汉字代码的高字节 */
		if (code1 < 0x80)
		{
			/* 将ascii字符点阵复制到buf */
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

			/* 计算16点阵汉字点阵地址
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
					/* 字库搜索完毕，未找到，则填充全FF */
					memset(buf, 0xFF, font_bytes);
					break;
				}
			}

			width = font_width;
		}

		y = _usY;
		/* 开始刷LCD */
		for (m = 0; m < font_height; m++)	/* 字符高度 */
		{
			x = _usX;
			for (i = 0; i < width; i++)	/* 字符宽度 */
			{
				if ((buf[m * ((2 * width) / font_width) + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
				{
					OLED_PutPixel(x, y, _tFont->FrontColor);	/* 设置像素颜色为文字色 */
				}
				else
				{
					OLED_PutPixel(x, y, _tFont->BackColor);	/* 设置像素颜色为文字背景色 */
				}
				
				x++;
			}
			y++;
		}

		if (_tFont->Space > 0)
		{
			/* 如果文字底色按_tFont->usBackColor，并且字间距大于点阵的宽度，那么需要在文字之间填充(暂时未实现) */
		}
		_usX += width + _tFont->Space;	/* 列地址递增 */
		_ptr++;			/* 指向下一个字符 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: OLED_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_ucColor  ：像素颜色
*	返 回 值: 无
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
		OLED_WriteCmd (0xB0 + ucPageAddr);					/* 设置页地址（0~7） */
		OLED_WriteCmd (0x00 + (ucColAddr & 0x0F));			/* 设置列地址的低地址 */
		OLED_WriteCmd (0x10 + ((ucColAddr >> 4) & 0x0F));	/* 设置列地址的高地址 */
		OLED_WriteData(ucValue);
	}
}

/*
*********************************************************************************************************
*	函 数 名: OLED_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*	返 回 值: 颜色值 (0, 1)
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
*	函 数 名: OLED_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 ：起始点坐标
*			_usX2, _usY2 ：终止点Y坐标
*			_ucColor     ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint8_t _ucColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* 采用 Bresenham 算法，在2点间画一条直线 */

	OLED_PutPixel(_usX1 , _usY1 , _ucColor);

	/* 如果两点重合，结束后面的动作。*/
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

	if ( dx < dy )   /*如果dy为计长方向，则交换纵横坐标。*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* 循环画点 */
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
*	函 数 名: OLED_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     ：坐标数组
*			_ucColor ：颜色
*	返 回 值: 无
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
*	函 数 名: OLED_DrawRect
*	功能说明: 绘制矩形。
*	形    参:
*			_usX,_usY：矩形左上角的坐标
*			_usHeight ：矩形的高度
*			_usWidth  ：矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_DrawRect(uint16_t _usX, uint16_t _usY, uint8_t _usHeight, uint16_t _usWidth, uint8_t _ucColor)
{
	/*
	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	OLED_DrawLine(_usX, _usY, _usX + _usWidth - 1, _usY, _ucColor);	/* 顶 */
	OLED_DrawLine(_usX, _usY + _usHeight - 1, _usX + _usWidth - 1, _usY + _usHeight - 1, _ucColor);	/* 底 */

	OLED_DrawLine(_usX, _usY, _usX, _usY + _usHeight - 1, _ucColor);	/* 左 */
	OLED_DrawLine(_usX + _usWidth - 1, _usY, _usX + _usWidth - 1, _usY + _usHeight, _ucColor);	/* 右 */
}

/*
*********************************************************************************************************
*	函 数 名: OLED_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  ：圆心的坐标
*			_usRadius  ：圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void OLED_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint8_t _ucColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

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
*	函 数 名: OLED_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：单色图片点阵指针，每个像素占用1个字节
*	返 回 值: 无
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
*	函 数 名: OLED_ConfigGPIO
*	功能说明: 配置OLED控制口线，设置为8位80XX总线控制模式或SPI模式
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void OLED_ConfigGPIO(void)
{
	/* 12.配置GPIO */
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: OLED_WriteCmd
*	功能说明: 向SSD1306发送一字节命令
*	形    参:  命令字
*	返 回 值: 无
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
*	函 数 名: OLED_WriteData
*	功能说明: 向SSD1306发送一字节数据
*	形    参:  命令字
*	返 回 值: 无
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
