
#include "gpio.h"
#include "OLED_Font.h"
#include <string.h>
#include "stdio.h"
#include <math.h>
#include "OLED.h"

/*引脚配置*/
#define OLED_SCL_PORT    GPIOB
#define OLED_SCL_PIN     GPIO_PIN_6
#define OLED_SDA_PORT    GPIOB
#define OLED_SDA_PIN     GPIO_PIN_7
#define OLED_W_SCL(x)		HAL_GPIO_WritePin(OLED_SCL_PORT,OLED_SCL_PIN,(x)?GPIO_PIN_SET:GPIO_PIN_RESET)
#define OLED_W_SDA(x)		HAL_GPIO_WritePin(OLED_SDA_PORT,OLED_SDA_PIN,((x)?GPIO_PIN_SET:GPIO_PIN_RESET))

/**
  * @brief  引脚初始化
  * @param  无
  * @retval 无
  */
void OLED_I2C_Init(void)
{
	MX_GPIO_Init();
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(!!(Byte & (0x80 >> i)));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

//*************************************************************************************************************************//


/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示一个字符(但是mini版)
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */


void OLED_ShowMinChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t trans[8]={0};
	OLED_SetCursor((Line - 1), (Column - 1) * 8);		//设置光标位置在上半部分
	for (uint8_t z = 0; z < 8; z++)
	{
		for(uint8_t y = 0;y<8;y++)
		{
			trans[z]=trans[z]<<1;
			if(OLEDMin_F8x8[Char - ' '][7-y]&(1<<z))
			{
				trans[z]=trans[z]|0x01;
			}
		}
	}
	for (uint8_t z = 0; z < 8; z++)
	{
		OLED_WriteData(trans[7-z]);			//显示上半部分内容
	}
}

/**
  * @brief  OLED显示一帧画面
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowPicture(void)
{
	
	OLED_SetCursor(0,0);
	for(int j=0;j<128;j++)
	{
			OLED_WriteData(0X00);
	}
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<128;j++)
		{
			OLED_WriteData(OLEDPicture_total.OLEDPicture_8[j][i]);
		}
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED显示字符串(min)
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowMinString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowMinChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED显示一行字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowRowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2,0);		//设置光标位置在上半部分
	for (i = 0; i < 128; i++)
	{
		OLED_WriteData(0x00);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2+1,0);		//设置光标位置在上半部分
	for (i = 0; i < 128; i++)
	{
		OLED_WriteData(0x00);			//显示上半部分内容
	}
	for (i = 0; String[i] != '\0'&&i<15; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);	// 从机地址
  	OLED_I2C_SendByte(0x00);	// 写命令标识位
 	 OLED_I2C_SendByte(0x20);	// 内存地址模式命令
  	OLED_I2C_SendByte(0x00);	// 水平模式（行+列自增）
	OLED_I2C_Stop();
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}

/**
  * @brief  OLED显示地平线,地平线底下的像素亮起
	* @param  x,y,z加速度
  * @retval 无
  */
void OLED_Horizon(int X,int Y,int Z){
	int Axis_k=0;
	int Axis_h=((Z+20000)*32/20000)%64;
	int Axis_l=Axis_h;
	uint64_t Fx16 = (uint64_t)-1;
	memset(OLEDPicture_total.OLEDPicture_8[0],0,1024);
	for(int i=0;i<128;i++){
		if(X<=0){
			if(Y>=0){
				Axis_k=((i-Axis_l*2)*Y/X)+(Axis_h%64);
			}else{
				Axis_k=((i-128+Axis_l*2)*Y/X)+(Axis_h%64);
			}
			if(Axis_k<0){
				continue;
			}else if(Axis_k>64){
				memset(OLEDPicture_total.OLEDPicture_8[i],0xff,8);
			}else{
				OLEDPicture_total.OLEDPicture_64[i]=Fx16<<(64-Axis_k);
			}
		}else{
			if(Y>=0){
				Axis_k=((i-Axis_l*2)*Y/X)+((64-Axis_h)%64);
			}else{
				Axis_k=((i-128+Axis_l*2)*Y/X)+((64-Axis_h)%64);
			}
			if(Axis_k<0){
				memset(OLEDPicture_total.OLEDPicture_8[i],0xff,8);
			}else if(Axis_k>64){
				continue;
			}else{
				OLEDPicture_total.OLEDPicture_64[i]=Fx16>>(Axis_k);
			}
		}
	}
}

/**
  * @brief  画一个圆
	* @param  x、y坐标，r圆半径，color颜色
  * @retval 无
  */

void OLED_DrawARound(int x_a,int y_a,int r,int r_a,Color color)
{
	int x,y;
	int x_max,y_max,x_min,y_min;
	x_max=x_a+r;
	x_min=x_a-r;
	y_max=y_a+r;
	y_min=y_a-r;
	for(x=x_min;x<=x_max;x++){
		for(y=y_min;y<=y_max;y++){
			if(x < 0 || x >= 128 || y <0 || y >=64) continue;
			if (( (x-x_a)*(x-x_a) + (y-y_a)*(y-y_a) >= r_a*r_a ) &&( (x-x_a)*(x-x_a) + (y-y_a)*(y-y_a) <= r*r ) &&r > 0){
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x]&=~(1ULL<<y);break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x]|= (1ULL<<y);break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x]^= (1ULL<<y);break;
				}
			}
		}
	}
}

/**
  * @brief  显示字符
	* @param  x、y坐标，CH字符，color颜色
  * @retval 无
  */

void OLED_DrawAWord(int x,int y,char CH,Color color){
	for(int i=0;i<8;i++){
		switch(color){
			case OLED_Black:
				OLEDPicture_total.OLEDPicture_64[x+i]&=~((uint64_t)OLED_F8x16[CH - ' '][i]<<y);
				OLEDPicture_total.OLEDPicture_64[x+i]&=~((uint64_t)OLED_F8x16[CH - ' '][i+8]<<(y+8));
			break;
			case OLED_White:
				OLEDPicture_total.OLEDPicture_64[x+i]|= ((uint64_t)OLED_F8x16[CH - ' '][i]<<y);
				OLEDPicture_total.OLEDPicture_64[x+i]|= ((uint64_t)OLED_F8x16[CH - ' '][i+8]<<(y+8));
			break;
			case OLED_Contr:
				OLEDPicture_total.OLEDPicture_64[x+i]^= ((uint64_t)OLED_F8x16[CH - ' '][i]<<y);
				OLEDPicture_total.OLEDPicture_64[x+i]^= ((uint64_t)OLED_F8x16[CH - ' '][i+8]<<(y+8));
			break;
		}
	}
}

/**
  * @brief  显示字符串
	* @param  x、y坐标，CH字符，color颜色
  * @retval 无
  */

void OLED_DrawAString(int x,int y,char * CH,Color color){
	while(*CH){
		OLED_DrawAWord(x,y,*CH,color);
		CH++;
		x+=8;
	}
}

/**
  * @brief  显示数字
	* @param  x、y坐标，CH字符，color颜色
  * @retval 无
  */

void OLED_DrawANum(int x,int y,int num,int length,Color color){
	char CH[15]={0};
	for(int i=length-1;i>=0;i--)
	{
		CH[i]='0'+num%10;
		num/=10;
	}
	CH[length] = '\0';
	OLED_DrawAString(x,y,CH,color);
}

/**
	* @brief  显示放大字符(最大8倍)
	* @param  x、y坐标，CH字符，color颜色，num放大倍数
  * @retval 无
  */
//原来的时间复杂度是O（8*8*num*num），空间复杂度是O（8*8*8）；现在时间复杂度为O（8*8*num），空间复杂度为O（0）
void OLED_AMPDrawAWord(int x,int y,char CH,Color color,uint8_t num){
	uint64_t block = (1ULL << num) - 1; // 生成 num 个连续 1
  int idx = CH - ' ';
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			for(int k=0;k<num;k++){
				if(OLED_F8x16[idx][i]&(1ull<<j)){
					switch(color){
						case OLED_Black:OLEDPicture_total.OLEDPicture_64[num*i+k+x]&=~(block<<(num*j+y));break;
						case OLED_White:OLEDPicture_total.OLEDPicture_64[num*i+k+x]|= (block<<(num*j+y));break;
						case OLED_Contr:OLEDPicture_total.OLEDPicture_64[num*i+k+x]^= (block<<(num*j+y));break;
					}
				}
				if(OLED_F8x16[idx][i+8]&(1ull<<j)){
					switch(color){
						case OLED_Black:OLEDPicture_total.OLEDPicture_64[num*i+k+x]&=~(block<<(num*j+y+8*num));break;
						case OLED_White:OLEDPicture_total.OLEDPicture_64[num*i+k+x]|= (block<<(num*j+y+8*num));break;
						case OLED_Contr:OLEDPicture_total.OLEDPicture_64[num*i+k+x]^= (block<<(num*j+y+8*num));break;
					}
				}
			}
		}
	}
}

/**
	* @brief  显示放大mini字符(最大8倍)
	* @param  x、y坐标，CH字符，color颜色，num放大倍数
  * @retval 无
  */
void OLEDMin_AMPDrawAWord(int x,int y,char CH,Color color,uint8_t num){
	uint64_t block = (1ULL << num) - 1; // 生成 num 个连续 1
  int idx = CH - ' ';
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			for(int k=0;k<num;k++){
				if(OLEDMin_F8x8[idx][i]&(1ull<<j)){
					switch(color){
						case OLED_Black:OLEDPicture_total.OLEDPicture_64[num*i+k+x]&=~(block<<(num*j+y));break;
						case OLED_White:OLEDPicture_total.OLEDPicture_64[num*i+k+x]|= (block<<(num*j+y));break;
						case OLED_Contr:OLEDPicture_total.OLEDPicture_64[num*i+k+x]^= (block<<(num*j+y));break;
					}
				}
			}
		}
	}
}

/**
  * @brief  显示字符(mini)
	* @param  x、y坐标，CH字符，color颜色
  * @retval 无
  */

void OLEDMin_DrawAWord(int x,int y,char CH,Color color)
{
	for(int i=0;i<8;i++){
		switch(color){
			case OLED_Black:OLEDPicture_total.OLEDPicture_64[x+i]&=~((uint64_t)OLEDMin_F8x8[CH - ' '][i]<<y);break;
			case OLED_White:OLEDPicture_total.OLEDPicture_64[x+i]|= ((uint64_t)OLEDMin_F8x8[CH - ' '][i]<<y);break;
			case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x+i]^= ((uint64_t)OLEDMin_F8x8[CH - ' '][i]<<y);break;
		}
	}
}

void OLEDMin_DrawAString(int x,int y,char * CH,Color color){
	while(*CH){
		OLEDMin_DrawAWord(x,y,*CH,color);
		CH++;
		x+=8;
	}
}

/**
  * @brief  求向量叉乘绝对值
	* @param  向量坐标
  * @retval 向量叉乘绝对值
  */

int area2(int x1, int y1,
					int x2, int y2,
					int x3, int y3)
{
    int val = (x2 - x1)*(y3 - y1) - (y2 - y1)*(x3 - x1);
    return val < 0 ? -val : val;  // 绝对值
}
/**
  * @brief  判断点(px,py) 是否在三角形内
	* @param  向量坐标
	* @retval 布尔值
  */
int pointInTriangle(int px, int py,
                    int x0, int y0,
                    int x1, int y1,
                    int x2, int y2)
{
    int A  = area2(x0,y0, x1,y1, x2,y2);
    int A1 = area2(px,py, x1,y1, x2,y2);
    int A2 = area2(x0,y0, px,py, x2,y2);
    int A3 = area2(x0,y0, x1,y1, px,py);
    return (A == A1 + A2 + A3);
}

/**
  * @brief  画一个三角形
	* @param  x、y坐标，三个点，color颜色
  * @retval 无
  */
void OLED_DrawAThriangle(	int x1,int y1,
													int x2,int y2,
													int x3,int y3,Color color)
{
	int x,y;
	int x_max,y_max,x_min,y_min;
	x_min = (x1 < x2) ? ((x1 < x3) ? x1 : x3) : ((x2 < x3) ? x2 : x3);
	x_max = (x1 > x2) ? ((x1 > x3) ? x1 : x3) : ((x2 > x3) ? x2 : x3);
	y_min = (y1 < y2) ? ((y1 < y3) ? y1 : y3) : ((y2 < y3) ? y2 : y3);
	y_max = (y1 > y2) ? ((y1 > y3) ? y1 : y3) : ((y2 > y3) ? y2 : y3);
	for(x=x_min;x<=x_max;x++){
		for(y=y_min;y<=y_max;y++){
			if(x < 0 || x >= 128 || y <0 || y >=64) continue;
			if(pointInTriangle(x,y,x3,y3,x1,y1,x2,y2)){
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x]&=~(1ULL<<y);break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x]|= (1ULL<<y);break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x]^= (1ULL<<y);break;
				}
			}
		}
	}
}

/**
  * @brief  画一个四边形
	* @param  x、y坐标，四个点，color颜色
  * @retval 无
  */
void OLED_DrawAquard(	int x1,int y1,
											int x2,int y2,
											int x3,int y3,
											int x4,int y4,Color color)
{
	OLED_DrawAThriangle(x1,y1,x2,y2,x3,y3,color);
	OLED_DrawAThriangle(x1,y1,x3,y3,x4,y4,color);
}
/**
  * @brief  画一个五边形
	* @param  x、y坐标，五个点，color颜色
  * @retval 无
  */
void OLED_DrawAPentagon(int x1,int y1,
												int x2,int x3,
												int y3,int y2,
												int x4,int y4,
												int x5,int y5,Color color)
{
	OLED_DrawAThriangle(x1,y1,x2,y2,x3,y3,color);
	OLED_DrawAThriangle(x1,y1,x3,y3,x4,y4,color);
	OLED_DrawAThriangle(x1,y1,x4,y4,x5,y5,color);
}

/**
  * @brief  画一个正多边形
	* @param  x、y坐标，color颜色
  * @retval 无
  */
#define pi 3.1415926
void OLED_DrawAPolygon(int x,int y,int side, int lenght , float angle,Color color){
	float angle1[side];
	float rad[side];
	for(int i=0;i<side;i++){
		angle1[i]=360*i/side+angle;
		rad[i]=angle1[i]*pi/180;
	}
	OLED_DrawAThriangle(x,y,x+sin(rad[0])*lenght,y+cos(rad[0])*lenght,x+sin(rad[side-1])*lenght,y+cos(rad[side-1])*lenght,OLED_White);
	for(int i=0;i<side-1;i++){
		OLED_DrawAThriangle(x,y,x+sin(rad[i])*lenght,y+cos(rad[i])*lenght,x+sin(rad[i+1])*lenght,y+cos(rad[i+1])*lenght,OLED_White);
	}
}

/**
  * @brief  画一格调
	* @param  x、y坐标，两个向量，color颜色
  * @retval 无
  */
void OLED_DrawAGeDiao(void){
	int a=32;
	int a_a=1;
  while (1){
		memset(OLEDPicture_total.OLEDPicture_8[0],0x00,1024);
		OLED_DrawARound(a,40,8,0,OLED_White);
		OLED_DrawARound(96,48,8,0,OLED_White);
		OLED_DrawARound(96,32,8,0,OLED_White);
		OLED_DrawAquard(a,32,96,32,a,48,96,48,OLED_White);
		OLED_ShowPicture();
		if(a>=64||a<=32){
			a=32;
			a_a=-a_a;
		}
		a+=a_a;
		HAL_Delay(50);
  }
}
/**
  * @brief  求旋转坐标
	* @param  x、y坐标
  * @retval 无
  */
void rotate_point(float 	cx, float   cy, float angle, 
                  float		 x, float    y, 
                  int *out_x, int *out_y)
{
		float rad =(pi/180)*angle;
    float s = sin(rad);
    float c = cos(rad);

    float dx = x - cx;
    float dy = y - cy;

    *out_x = (int)round(cx + dx * c - dy * s);
    *out_y = (int)round(cy + dx * s + dy * c);
}

/**
  * @brief  显示旋转字符
	* @param  x、y坐标，CH字符，color颜色
  * @retval 无
  */
void OLED_DrawARatateWord(int x,int y,float angle,char CH,Color color){
	int x_r,y_r;
	int idx = CH - ' ';
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			if(OLED_F8x16[idx][i]&(1ull<<j)){
				rotate_point(x+4,y+8,angle,x+i,y+j,&x_r,&y_r);
				if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
				}
			}
			if(OLED_F8x16[idx][i+8]&(1ull<<j)){
				rotate_point(x+4,y+8,angle,x+i,y+j+8,&x_r,&y_r);
				if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
				}
			}
		}
	}
}
/**
  * @brief  显示旋转字符(Mini)
	* @param  x、y坐标，角度，CH字符，color颜色
  * @retval 无
  */
void OLEDMin_DrawARatateWord(int x,int y,float angle,char CH,Color color){
	int x_r,y_r;
	int idx = CH - ' ';
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			if(OLEDMin_F8x8[idx][i]&(1ull<<j)){
				rotate_point(x+4,y+4,angle,x+i,y+j,&x_r,&y_r);
				if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
				}
			}
		}
	}
}

/**
  * @brief  显示旋转字符串
	* @param  x、y坐标，角度，CH字符，color颜色
  * @retval 无
  */
void OLED_DrawARatateString(int x,int y,float angle,char* CH,Color color){
	int stringlen =x+4*strlen(CH);
	while(*CH){
		int x_r,y_r;
		int idx = *CH - ' ';
		for(int i=0;i<8;i++){
			for(int j=0;j<8;j++){
				if(OLED_F8x16[idx][i]&(1ull<<j)){
					rotate_point(stringlen,y+8,angle,x+i,y+j,&x_r,&y_r);
					if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
					switch(color){
						case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
						case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
						case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
					}
				}
				if(OLED_F8x16[idx][i+8]&(1ull<<j)){
					rotate_point(stringlen,y+8,angle,x+i,y+j+8,&x_r,&y_r);
					if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
					switch(color){
						case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
						case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
						case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
					}
				}
			}
		}
		CH++;
		x+=8;
	}
}

/**
  * @brief  显示旋转字符串(Mini)
	* @param  x、y坐标，角度，CH字符，color颜色
  * @retval 无
  */
void OLEDMin_DrawARatateString(int x,int y,float angle,char * CH,Color color){
	int stringlen =x+4*strlen(CH);
	while(*CH){
		int x_r,y_r;
		int idx = *CH - ' ';
		for(int i=0;i<8;i++){
			for(int j=0;j<8;j++){
				if(OLEDMin_F8x8[idx][i]&(1ull<<j)){
					rotate_point(stringlen,y+4,angle,x+i,y+j,&x_r,&y_r);
					if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
					switch(color){
						case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
						case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
						case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
					}
				}
			}
		}
		CH++;
		x+=10;
	}
}

/**
  * @brief  显示旋转数字
	* @param  x、y坐标，角度，CH字符，color颜色
  * @retval 无
  */
void OLED_DrawARatateNum(int x,int y,float angle,int num,int length,Color color){
	char CH[15]={0};
	for(int i=length-1;i>=0;i--)
	{
		CH[i]='0'+num%10;
		num/=10;
	}
	CH[length] = '\0';
	OLED_DrawARatateString(x,y,angle,CH,color);
}

/**
  * @brief  显示旋转数字(min)
	* @param  x、y坐标，角度，CH字符，color颜色
  * @retval 无
  */
void OLEDMin_DrawARatateNum(int x,int y,float angle,int num,int length,Color color){
	char CH[15]={0};
	for(int i=length-1;i>=0;i--)
	{
		CH[i]='0'+num%10;
		num/=10;
	}
	CH[length] = '\0';
	OLEDMin_DrawARatateString(x,y,angle,CH,color);
}

/**
  * @brief  显示旋转放大字符
* @param  x、y坐标，角度，CH字符，color颜色，放大倍数
  * @retval 无
  */
void OLED_AMPDrawARatateWord(int x,int y,float angle ,char CH,Color color,uint8_t num){
	uint64_t block = (1ULL << num) - 1; // 生成 num 个连续 1
	static uint64_t amp[32]={0};
	memset(amp,0,256);
	int x_r,y_r;
  int idx = CH - ' ';
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			for(int k=0;k<num;k++){
				if(OLED_F8x16[idx][i]&(1ull<<j))
					amp[num*i+k]|=(block<<(num*j));
				if(OLED_F8x16[idx][i+8]&(1ull<<j))
					amp[num*i+k]|=(block<<(num*(j+8)));
			}
		}
	}
	for(int i=0;i<(8*num);i++){
		for(int j=0;j<(16*num);j++){
			if(amp[i]&(1ull<<j)){
				rotate_point(x+4*num,y+8*num,angle,x+i,y+j,&x_r,&y_r);
				if(x_r < 0 || x_r >= 128 || y_r <0 || y_r >=64) continue;
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_r]&=~(1ull<<(y_r));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_r]|= (1ull<<(y_r));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_r]^= (1ull<<(y_r));break;
				}
			}
		}
	}
}

void OLED_DrawAAixs(int16_t num,uint16_t t,int AMP)
{
	if(AMP==1)
	{
		for(int i=0;i<128;i++)
		{
			OLEDPicture_total.OLEDPicture_64[i]|=(1ull<<(64-OLED_Sample_value[(t+i+128)%256][num]/64));
			OLED_DrawALine_For_Axis(i-1,(64-OLED_Sample_value[(t+i+127)%256][num]/64),i,(64-OLED_Sample_value[(t+i+128)%256][num]/64),1);
		}
	}
	if(AMP==2)
	{
		for(int i=0;i<128;i++)
		{
			OLEDPicture_total.OLEDPicture_64[i]|=(1ull<<(64-(OLED_Sample_value[(t+2*i)%256][num]+OLED_Sample_value[(t+2*i+((t%2)?1:-1))%256][num])/128));
		}
	}
	
}

void OLED_DrawALine(int x,int y,int x1,int y1,Color color)
{
	int dx=x1-x;
	int dy=y1-y;
	int x_a=(x1>x)?1:-1;
	int y_a=(y1>y)?1:-1;
	int x_n=x,y_n=y;
	int length_x = (dx>0)?dx:-dx;
	int length_y = (dy>0)?dy:-dy;
	if(length_x>=length_y)
	{
		for(int i=0;i<length_x;i++)
		{
			y_n=((x_n-x)*dy/dx)+y;
			if(x_n > 0 && x_n <= 128 && y_n >0 && y_n <=64)
			{
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_n]&=~(1ull<<(y_n));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_n]|= (1ull<<(y_n));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_n]^= (1ull<<(y_n));break;
				}
			}
			x_n+=x_a;
		}
	}
	else
	{
		for(int i=0;i<length_y;i++)
		{
			x_n=((y_n-y)*dx/dy)+x;
			if(x_n > 0 && x_n <= 128 && y_n >0 && y_n <=64)
			{
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_n]&=~(1ull<<(y_n));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_n]|= (1ull<<(y_n));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_n]^= (1ull<<(y_n));break;
				}
			}
			y_n+=y_a;
		}
	}
}

void OLED_DrawALine_For_Axis(int x,int y,int x1,int y1,Color color)
{
	int dx=x1-x;
	int dy=y1-y;
	int x_a=(x1>x)?1:-1;
	int y_a=(y1>y)?1:-1;
	int x_n=x,y_n=y;
	int length_x = (dx>0)?dx:-dx;
	int length_y = (dy>0)?dy:-dy;
	if(length_x<length_y)
	{
		for(int i=0;i<length_y;i++)
		{
			x_n=((y_n-y)*dx/dy)+x;
			if(x_n > 0 && x_n <= 128 && y_n >0 && y_n <=64)
			{
				switch(color){
					case OLED_Black:OLEDPicture_total.OLEDPicture_64[x_n]&=~(1ull<<(y_n));break;
					case OLED_White:OLEDPicture_total.OLEDPicture_64[x_n]|= (1ull<<(y_n));break;
					case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x_n]^= (1ull<<(y_n));break;
				}
			}
			y_n+=y_a;
		}
	}
}

void OLED_DrawAClock(int h,int min,int scend)
{
	int h_x,h_y;
	int m_x,m_y;
	int s_x,s_y;
	h_x=sin((float)-(h*pi/6+pi+min*pi/720))*10+64;
	h_y=cos((float)-(h*pi/6+pi+min*pi/720))*10+32;
	m_x=sin((float)-(min*pi/30+pi+scend*pi/3600))*20+64;
	m_y=cos((float)-(min*pi/30+pi+scend*pi/3600))*20+32;
	s_x=sin((float)-scend*pi/30+pi)*25+64;
	s_y=cos((float)-scend*pi/30+pi)*25+32;
	OLED_DrawARound(64,32,30,29,OLED_White);
	OLED_DrawALine(64,32,h_x,h_y,OLED_White);
	OLED_DrawALine(64,32,m_x,m_y,OLED_White);
	OLED_DrawALine(64,32,s_x,s_y,OLED_White);
	OLEDMin_DrawARatateWord(60,4,90,'0',OLED_White);
	OLEDMin_DrawARatateWord(36,28,90,'9',OLED_White);
	OLEDMin_DrawARatateWord(84,28,90,'3',OLED_White);
	OLEDMin_DrawARatateWord(60,52,90,'6',OLED_White);
}
