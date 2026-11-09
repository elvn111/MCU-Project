#include "OLED.h"
#include "OLED_Font.h"
#include "I2C.h"

int Vedio_Normal_Vector[3] = { 0,0,0 };

//灭点距离
unsigned int WINDOWS_LENGTH = 128;
unsigned int WINDOWS_HIGH = 64;

int Vedio_X = 0;
int Vedio_Y = 0;
int Vedio_Z = 0;

float Vedio_AngleX = 0;
float Vedio_AngleY = 0;
float Vedio_AngleZ = 0;

float Vedio_Vector[3] = { 0, 10, 0 };

int VanishingPointLength = 700;


int VanishingPointLength_z = 32;
int VanishingPointLength_x = 64;
int VanishingPointLength_y =7000;

union {
	 uint8_t OLEDPicture_8[128][8];
	 uint64_t OLEDPicture_64[128];
 }OLEDPicture_total;


/**
  * @brief  引脚初始化
  * @param  无
  * @retval 无
  */
void OLED_I2C_Init(void){
	I2C_Init();   /* I2C2: PB10=SCL, PB11=SDA; power PB0=GND, PB1=VCC */
}




/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command){
	uint8_t buf[2] = {0x00, Command};
	I2C_Transmit(I2C_DEV_ADDR, buf, 2);
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data){
	uint8_t buf[2] = {0x40, Data};
	I2C_Transmit(I2C_DEV_ADDR, buf, 2);
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X){
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
void OLED_Clear(void){
	static uint8_t zeros[128];
	uint8_t p;
	for (p = 0; p < 8; p++){
		OLED_SetCursor(p, 0);
		I2C_WriteDataBlock(zeros, 128);
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char){      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++){
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++){
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

void OLED_ShowMinChar(uint8_t Line, uint8_t Column, char Char){      	
	uint8_t trans[8]={0};
	OLED_SetCursor((Line - 1), (Column - 1) * 8);		//设置光标位置在上半部分
	for (uint8_t z = 0; z < 8; z++){
		OLED_WriteData(OLEDMin_F8x8[Char - ' '][z]);			//显示上半部分内容
	}
}

/**
  * @brief  OLED显示一帧画面
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowPicture(int x0, int y0, int x1, int y1)
{
	static uint8_t line[128];
	uint8_t p;
	int x, n;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > WINDOWS_LENGTH - 1) x1 = WINDOWS_LENGTH - 1;
	if (y1 > WINDOWS_HIGH - 1) y1 = WINDOWS_HIGH - 1;
	if (x0 > x1 || y0 > y1) return;

	for (p = (uint8_t)(y0 / 8); p <= (uint8_t)(y1 / 8); p++){
		n = x1 - x0 + 1;
		for (x = 0; x < n; x++)
			line[x] = OLEDPicture_total.OLEDPicture_8[x0 + x][p];
		OLED_SetCursor(p, (uint8_t)x0);
		I2C_WriteDataBlock(line, (uint16_t)n);
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String){
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++){
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
void OLED_ShowMinString(uint8_t Line, uint8_t Column, char *String){
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++){
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
void OLED_ShowRowString(uint8_t Line, uint8_t Column, char *String){
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2,0);		//设置光标位置在上半部分
	for (i = 0; i < 128; i++){
		OLED_WriteData(0x00);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2+1,0);		//设置光标位置在上半部分
	for (i = 0; i < 128; i++){
		OLED_WriteData(0x00);			//显示上半部分内容
	}
	for (i = 0; String[i] != '\0'&&i<15; i++){
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y){
	uint32_t Result = 1;
	while (Y--){
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
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length){
	uint8_t i;
	for (i = 0; i < Length; i++){
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
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length){
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0){
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}else{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++){
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
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length){
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10){
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}else{
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
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length){
	uint8_t i;
	for (i = 0; i < Length; i++){
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

void OLED_Init(void)
{
	uint32_t i, j;
	//上电延时
	for (i = 0; i < 1000; i++){
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	/* 供电稳定：模块上电后需几毫秒才应答 I2C */
	for (i = 0; i < 2000; i++){ for (j = 0; j < 1000; j++); }
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0x20);	// 内存地址模式
	OLED_WriteCommand(0x00);	// 水平模式（行+列自增）
	
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
  * @brief  在缓冲区上打一个点
  * @param  x，横坐标
  * @param  y，纵坐标
  * @param  color 颜色
  * @retval 无
  */

void OLED_DrawAPoint(int x, int y, Color color)
{
	switch(color)
	{
		case OLED_Black:OLEDPicture_total.OLEDPicture_64[x]&=~(1ULL<<y);break;
		case OLED_White:OLEDPicture_total.OLEDPicture_64[x]|= (1ULL<<y);break;
		case OLED_Contr:OLEDPicture_total.OLEDPicture_64[x]^= (1ULL<<y);break;
	}
}

/**
  * @brief  在缓冲区上打一排点
  * @param  x，横坐标
  * @param  y，纵坐标
  * @param  color 颜色
  * @retval 无
  */
void OLED_AMPDrawAPoint(int x, int y, int num, Color color)
{
	for (int i = 0;i < num; i++)
	{
		OLED_DrawAPoint(x, y + i, color);
	}
}

/**
  * @brief  在缓冲区上打一个Byte
  * @param  x，横坐标
  * @param  y，纵坐标
  * @param Byte，byte
  * @param  color 颜色
  * @retval 无
  */
void OLED_DrawAByte(int x, int y, int Byte, Color color)
{
	for (int i = 0;i < 8;i++)
	{
		if (Byte & (1 << i))
			OLED_DrawAPoint(x, y + i, color);
	}
}


void OLED_DrawClear(int x0, int y0, int x1, int y1)
{
	int x, y;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > WINDOWS_LENGTH - 1) x1 = WINDOWS_LENGTH - 1;
	if (y1 > WINDOWS_HIGH - 1) y1 = WINDOWS_HIGH - 1;
	if (x0 > x1 || y0 > y1) return;

	for (x = x0; x <= x1; x++)
		for (y = y0; y <= y1; y++)
			OLEDPicture_total.OLEDPicture_64[x] &= ~(1ULL << y);
}


/**
  * @brief  画一个圆
  * @param  x_a 圆心x坐标，范围：0~127
  * @param  y_a 圆心y坐标，范围：0~63
  * @param  r 圆半径，范围：0~WINDOWS_HIGH
  * @param  r_a 内圆半径，范围：0~WINDOWS_HIGH
  * @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
  * @retval 无
  */
void OLED_DrawARound(int x_a, int y_a, int r, int r_a, Color color) {
	int x, y;
	int x_max, y_max, x_min, y_min;
	x_max = x_a + r;
	x_min = x_a - r;
	y_max = y_a + r;
	y_min = y_a - r;
	for (x = x_min;x <= x_max;x++) {
		for (y = y_min;y <= y_max;y++) {
			if (x < 0 || x >= WINDOWS_LENGTH || y < 0 || y >= WINDOWS_HIGH) continue;
			if (((x - x_a) * (x - x_a) + (y - y_a) * (y - y_a) >= r_a * r_a) && ((x - x_a) * (x - x_a) + (y - y_a) * (y - y_a) <= r * r) && r > 0) {
				OLED_DrawAPoint(x, y, color);
			}
		}
	}
}

/**
  * @brief  显示字符
  * @param  x x坐标，范围：0~127
  * @param  y y坐标，范围：0~63
  * @param  CH 字符
  * @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
  * @retval 无
  */
void OLED_DrawAWord(int x, int y, char CH, Color color) {
	for (int i = 0;i < 8;i++) {
		OLED_DrawAByte(x + i, y, OLED_F8x16[CH - ' '][i], color);
		OLED_DrawAByte(x + i, y + 8, OLED_F8x16[CH - ' '][i + 8], color);
	}
}

/**
  * @brief  显示字符串
  * @param  x x坐标，范围：0~127
  * @param  y y坐标，范围：0~63
  * @param  CH 字符串
  * @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
  * @retval 无
  */
void OLED_DrawAString(int x, int y, char* CH, Color color) {
	while (*CH) {
		OLED_DrawAWord(x, y, *CH, color);
		CH++;
		x += 8;
	}
}

/**
  * @brief  显示数字
  * @param  x x坐标，范围：0~127
  * @param  y y坐标，范围：0~63
  * @param  num 要显示的数字，范围：0~4294967295
  * @param  length 要显示数字的长度，范围：1~10
  * @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
  * @retval 无
  */
void OLED_DrawANum(int x, int y, int num, int length, Color color) {
	char CH[50] = { 0 };
	if (num >= 0){
		for (int i = length - 1;i >= 0;i--){
			CH[i] = '0' + num % 10;
			num /= 10;
		}
		CH[length] = '\0';
		OLED_DrawAString(x, y, CH, color);
	}
	else{
		CH[0] = '-';
		for (int i = length - 2;i >= 0;i--){
			CH[i + 1] = '0' + (-num % 10);
			num /= 10;
		}
		CH[length] = '\0';
		OLED_DrawAString(x, y, CH, color);
	}
}

//原来的时间复杂度是O（8*8*num*num），空间复杂度是O（8*8*8）；现在时间复杂度为O（8*8*num），空间复杂度为O（0）
/**
	* @brief  显示放大字符(最大8倍)
	* @param  x x坐标，范围：0-127
	* @param  y y坐标，范围：0-63
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @param  num 放大倍数，范围：1~8
	* @retval 无
	*/
void OLED_AMPDrawAWord(int x, int y, char CH, Color color, uint8_t num) {
	int idx = CH - ' ';
	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 8;j++) {
			for (int k = 0;k < num;k++) {
				if (OLED_F8x16[idx][i] & (1ull << j)) {
					OLED_AMPDrawAPoint(num * i + k + x, num * j + y, num, color);
				}
				if (OLED_F8x16[idx][i + 8] & (1ull << j)) {
					OLED_AMPDrawAPoint(num * i + k + x, num * j + y + 8 * num, num, color);
				}
			}
		}
	}
}

/**
	* @brief  显示放大mini字符(最大8倍)
	* @param  x x坐标，范围：0-127
	* @param  y y坐标，范围：0-63
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @param  num 放大倍数，范围：1~8
	* @retval 无
	*/
void OLEDMin_AMPDrawAWord(int x, int y, char CH, Color color, uint8_t num) {
	int idx = CH - ' ';
	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 8;j++) {
			for (int k = 0;k < num;k++) {
				if (OLEDMin_F8x8[idx][i] & (1ull << j)) {
					OLED_AMPDrawAPoint(num * i + k + x, num * j + y, num, color);
				}
			}
		}
	}
}

/**
	* @brief  显示字符(mini)
	* @param  x x坐标，范围：0-127
	* @param  y y坐标，范围：0-63
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLEDMin_DrawAWord(int x, int y, char CH, Color color)
{
	int ch1 = CH - ' ';
	for (int i = 0;i < 8;i++) {
		OLED_DrawAByte(x + i, y, OLEDMin_F8x8[ch1][i], color);
	}
}

/**
	* @brief  显示字符串(mini)
	* @param  x x坐标，范围：0-127
	* @param  y y坐标，范围：0-63
	* @param  CH 字符串
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLEDMin_DrawAString(int x, int y, char* CH, Color color) {
	while (*CH) {
		OLEDMin_DrawAWord(x, y, *CH, color);
		CH++;
		x += 8;
	}
}

/**
	* @brief  求向量叉乘绝对值
	* @param  x1 y1 向量1坐标
	* @param  x2 y2 向量2坐标
	* @param  x3 y3 向量3坐标
	* @retval 向量叉乘绝对值
	*/

int area2(int x1, int y1,
	int x2, int y2,
	int x3, int y3) {
	int val = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
	return val < 0 ? -val : val;  // 绝对值
}

/**
	* @brief  判断点(px,py) 是否在三角形内
	* @param  px py 点坐标
	* @param  x0 y0 三角形顶点坐标
	* @param  x1 y1 三角形顶点坐标
	* @param  x2 y2 三角形顶点坐标
	* @retval 布尔值
	*/
int pointInTriangle(int px, int py,
	int x0, int y0,
	int x1, int y1,
	int x2, int y2) {
	int A = area2(x0, y0, x1, y1, x2, y2);
	int A1 = area2(px, py, x1, y1, x2, y2);
	int A2 = area2(x0, y0, px, py, x2, y2);
	int A3 = area2(x0, y0, x1, y1, px, py);
	return (A == A1 + A2 + A3);
}

/**
	* @brief  画一个三角形
	* @param  x1 y1 三角形顶点坐标
	* @param  x2 y2 三角形顶点坐标
	* @param  x3 y3 三角形顶点坐标
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawAThriangle(int x1, int y1,
	int x2, int y2,
	int x3, int y3, Color color) {
	int x, y;
	int x_max, y_max, x_min, y_min;
	x_min = (x1 < x2) ? ((x1 < x3) ? x1 : x3) : ((x2 < x3) ? x2 : x3);
	x_max = (x1 > x2) ? ((x1 > x3) ? x1 : x3) : ((x2 > x3) ? x2 : x3);
	y_min = (y1 < y2) ? ((y1 < y3) ? y1 : y3) : ((y2 < y3) ? y2 : y3);
	y_max = (y1 > y2) ? ((y1 > y3) ? y1 : y3) : ((y2 > y3) ? y2 : y3);
	if (x_min < 0)x_min = 0;
	if (y_min < 0)y_min = 0;
	if (x_max < 0)x_max = 0;
	if (y_max < 0)y_max = 0;
	if (x_min > WINDOWS_LENGTH_MAX)x_min = WINDOWS_LENGTH_MAX;
	if (y_min > WINDOWS_HIGH_MAX)y_min = WINDOWS_HIGH_MAX;
	if (x_max > WINDOWS_LENGTH_MAX)x_max = WINDOWS_LENGTH_MAX;
	if (y_max > WINDOWS_HIGH_MAX)y_max = WINDOWS_HIGH_MAX;
	for (x = x_min;x <= x_max;x++) {
		for (y = y_min;y <= y_max;y++) {
			if (x < 0 || x >= WINDOWS_LENGTH || y < 0 || y >= WINDOWS_HIGH) continue;
			if (pointInTriangle(x, y, x3, y3, x1, y1, x2, y2)) {
				OLED_DrawAPoint(x, y, color);
			}
		}
	}
}

/**
	* @brief  画一个四边形
	* @param  x1 y1 四边形顶点坐标
	* @param  x2 y2 四边形顶点坐标
	* @param  x3 y3 四边形顶点坐标
	* @param  x4 y4 四边形顶点坐标
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawAquard(int x1, int y1,
	int x2, int y2,
	int x3, int y3,
	int x4, int y4, Color color) {
		OLED_DrawALine(x1,y1,x2,y2,OLED_White);
		OLED_DrawALine(x2,y2,x3,y3,OLED_White);
		OLED_DrawALine(x3,y3,x4,y4,OLED_White);
		OLED_DrawALine(x4,y4,x1,y1,OLED_White);
	// OLED_DrawAThriangle(x1, y1, x2, y2, x3, y3, color);
	// OLED_DrawAThriangle(x1, y1, x3, y3, x4, y4, color);
}

/**
	* @brief  画一个正多边形
	* @param  x y 正多边形中心坐标
	* @param  side 边数，范围：3~8
	* @param  lenght 边长，范围：0~WINDOWS_HIGH
	* @param  angle 旋转角度，范围：0~360
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
#define pi 3.1415926
void OLED_DrawAPolygon(int x, int y, int side, int lenght, float angle, Color color) {
	float angle1[100];
	float rad[100];
	for (int i = 0;i < side;i++) {
		angle1[i] = 360 * i / side + angle;
		rad[i] = angle1[i] * pi / 180;
	}
	OLED_DrawAThriangle(x, y, x + sin(rad[0]) * lenght, y + cos(rad[0]) * lenght, x + sin(rad[side - 1]) * lenght, y + cos(rad[side - 1]) * lenght, OLED_White);
	for (int i = 0;i < side - 1;i++) {
		OLED_DrawAThriangle(x, y, x + sin(rad[i]) * lenght, y + cos(rad[i]) * lenght, x + sin(rad[i + 1]) * lenght, y + cos(rad[i + 1]) * lenght, OLED_White);
	}
}

/**
	* @brief  画一格调 测试显示
	* @retval 无
	 */
void OLED_DrawAGeDiao(void) {
	int a = 32;
	int a_a = 1;
	while (1) {
		OLED_DrawClear(0, 0, 127, 63);
		OLED_DrawARound(a, 40, 8, 0, OLED_White);
		OLED_DrawARound(96, 48, 8, 0, OLED_White);
		OLED_DrawARound(96, 32, 8, 0, OLED_White);
		OLED_DrawAquard(a, 32, 96, 32, 96, 48, a, 48, OLED_White);
		OLED_ShowPicture(0, 0, 127, 63);
		if (a >= WINDOWS_HIGH || a <= 32) {
			a = 32;
			a_a = -a_a;
		}
		a += a_a;
	}
}

/**
	* @brief  求旋转坐标
	* @param  cx cy 旋转中心坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  x y 旋转前坐标
	* @param  out_x out_y 旋转后坐标
	* @retval 无
	*/
void rotate_point(float 	cx, float   cy, float angle,
	float		 x, float    y,
	int* out_x, int* out_y) {
	float rad = (pi / 180) * angle;
	float s = sin(rad);
	float c = cos(rad);

	float dx = x - cx;
	float dy = y - cy;

	*out_x = round(cx + dx * c - dy * s);
	*out_y = round(cy + dx * s + dy * c);
}

void rotate3D_point(
	float 	cx, float   cy, float   cz,
	float anglex, float angley, float anglez,
	float	x, float    y, float    z,
	int* out_x, int* out_y, int* out_z) {

	int x_r = (int)x, y_r = (int)y, z_r = (int)z;
	rotate_point(cx, cz, angley, x_r, z_r, &x_r, &z_r);
	rotate_point(cx, cy, anglez, x_r, y_r, &x_r, &y_r);
	rotate_point(cy, cz, anglex, y_r, z_r, &y_r, &z_r);
	*out_x = x_r;*out_y = y_r;*out_z = z_r;
}

/**
	* @brief  显示旋转字符
	* @param  x y 旋转中心坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawARatateWord(int x, int y, float angle, char CH, Color color) {
	int x_r, y_r;
	int idx = CH - ' ';
	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 8;j++) {
			if (OLED_F8x16[idx][i] & (1ull << j)) {
				rotate_point(x + 4, y + 8, angle, x + i, y + j, &x_r, &y_r);
				if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
				OLED_DrawAPoint(x_r, y_r, color);
			}
			if (OLED_F8x16[idx][i + 8] & (1ull << j)) {
				rotate_point(x + 4, y + 8, angle, x + i, y + j + 8, &x_r, &y_r);
				if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
				OLED_DrawAPoint(x_r, y_r, color);
			}
		}
	}
}

/**
	* @brief  显示旋转字符(Mini)
	* @param  x y 旋转中心坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLEDMin_DrawARatateWord(int x, int y, float angle, char CH, Color color) {
	int x_r, y_r;
	int idx = CH - ' ';
	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 8;j++) {
			if (OLEDMin_F8x8[idx][i] & (1ull << j)) {
				rotate_point(x + 4, y + 4, angle, x + i, y + j, &x_r, &y_r);
				if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
				OLED_DrawAPoint(x_r, y_r, color);
			}
		}
	}
}

/**
	* @brief  显示旋转字符串
	* @param  x y 旋转中心坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符串
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawARatateString(int x, int y, float angle, char* CH, Color color) {
	int stringlen = x + 4 * strlen(CH);
	while (*CH) {
		int x_r, y_r;
		int idx = *CH - ' ';
		for (int i = 0;i < 8;i++) {
			for (int j = 0;j < 8;j++) {
				if (OLED_F8x16[idx][i] & (1ull << j)) {
					rotate_point(stringlen, y + 8, angle, x + i, y + j, &x_r, &y_r);
					if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
					OLED_DrawAPoint(x_r, y_r, color);
				}
				if (OLED_F8x16[idx][i + 8] & (1ull << j)) {
					rotate_point(stringlen, y + 8, angle, x + i, y + j + 8, &x_r, &y_r);
					if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
					OLED_DrawAPoint(x_r, y_r, color);
				}
			}
		}
		CH++;
		x += 8;
	}
}

/**
	* @brief  显示旋转字符串(Mini)
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符串
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLEDMin_DrawARatateString(int x, int y, float angle, char* CH, Color color) {
	int stringlen = x + 4 * strlen(CH);
	while (*CH) {
		int x_r, y_r;
		int idx = *CH - ' ';
		for (int i = 0;i < 8;i++) {
			for (int j = 0;j < 8;j++) {
				if (OLEDMin_F8x8[idx][i] & (1ull << j)) {
					rotate_point(stringlen, y + 4, angle, x + i, y + j, &x_r, &y_r);
					if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
					OLED_DrawAPoint(x_r, y_r, color);
				}
			}
		}
		CH++;
		x += 10;
	}
}

/**
	* @brief  显示旋转数字
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  num 要显示的数字，范围：0~4294967295
	* @param  length 要显示数字的长度，范围：1~10
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawARatateNum(int x, int y, float angle, int num, int length, Color color) {
	char CH[15] = { 0 };
	for (int i = length - 1;i >= 0;i--) {
		CH[i] = '0' + num % 10;
		num /= 10;
	}
	CH[length] = '\0';
	OLED_DrawARatateString(x, y, angle, CH, color);
}

/**
	* @brief  显示旋转数字(min)
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  num 要显示的数字，范围：0~4294967295
	* @param  length 要显示数字的长度，范围：1~10
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLEDMin_DrawARatateNum(int x, int y, float angle, int num, int length, Color color) {
	char CH[15] = { 0 };
	for (int i = length - 1;i >= 0;i--) {
		CH[i] = '0' + num % 10;
		num /= 10;
	}
	CH[length] = '\0';
	OLEDMin_DrawARatateString(x, y, angle, CH, color);
}

/**
	* @brief  显示旋转数字(min)
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  num 要显示的数字，范围：0~4294967295
	* @param  length 要显示数字的长度，范围：1~10
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLEDMin_DrawANum(int x, int y, int num, int length, Color color) {
	char CH[15] = { 0 };
	for (int i = length - 1;i >= 0;i--) {
		CH[i] = '0' + num % 10;
		num /= 10;
	}
	CH[length] = '\0';
	OLEDMin_DrawAString(x, y, CH, color);
}

/**
	* @brief  显示旋转放大字符
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @param  num 放大倍数，范围：1~8
	* @retval 无
	*/
void OLED_AMPDrawARatateWord(int x, int y, float angle, char CH, Color color, uint8_t num) {
	uint64_t block = (1ULL << num) - 1; // 生成 num 个连续 1

	static uint64_t amp[2][64] = { 0 };
	memset(amp, 0, sizeof(amp));
	if (num > 8)
		num = 8;
	int x_r, y_r;
	int idx = CH - ' ';
	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 8;j++) {
			for (int k = 0;k < num;k++) {
				if (OLED_F8x16[idx][i] & (1ull << j))
					amp[0][num * i + k] |= (block << (num * j));
				if (OLED_F8x16[idx][i + 8] & (1ull << j))
					amp[1][num * i + k] |= (block << (num * j));
			}
		}
	}
	for (int i = 0;i < (8 * num);i++) {
		for (int j = 0;j < (8 * num);j++) {
			if (amp[0][i] & (1ull << j)) {
				rotate_point(x + 4 * num, y + 8 * num, angle, x + i, y + j, &x_r, &y_r);
				if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
				OLED_DrawAPoint(x_r, y_r, color);
			}
			if (amp[1][i] & (1ull << j)) {
				rotate_point(x + 4 * num, y + 8 * num, angle, x + i, y + j + 8 * num, &x_r, &y_r);
				if (x_r < 0 || x_r >= WINDOWS_LENGTH || y_r < 0 || y_r >= WINDOWS_HIGH) continue;
				OLED_DrawAPoint(x_r, y_r, color);
			}
		}
	}
}

/**
	* @brief  显示ADC波形
	* @param  num 通道
	* @param  t 时间
	* @param  AMP 是否放大，范围：1、2
	* @retval 无
	*/
void OLED_DrawAAixs(int x, int y, int wide, int high, int cyc, int* data, int16_t num, uint16_t t, Color color, int AMP, int fangxiang) {
	if (fangxiang){
		for (int i = 0;i < wide;i++){
			if (i * AMP > wide){

			}
			else{
				OLED_DrawAPoint(i * AMP + x, y + high / 2 - data[(t + i) % (cyc)], color);
				OLED_DrawALine((i - 1) * AMP + x, y + (high / 2 - data[(t + i - 1) % (cyc)]), i * AMP + x, y + (high / 2 - data[(t + i) % (cyc)]), color);
			}
		}
	}
	else{
		for (int i = 0;i < wide;i++){
			if (i * AMP > wide){

			}
			else{
				OLED_DrawAPoint(wide - i * AMP + x, y + high / 2 - data[(t + i) % (cyc)], color);
				OLED_DrawALine((wide - (i - 1) * AMP) + x, y + (high / 2 - data[(t + i - 1) % (cyc)]), (wide - i * AMP) + x, y + (high / 2 - data[(t + i) % (cyc)]), color);
			}
		}
	}
}

/**
	* @brief  画一条线
	* @param  x、y坐标
	* @param  x1、y1坐标
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawALine(int x, int y, int x1, int y1, Color color) {
	int dx = x1 - x;
	int dy = y1 - y;
	int x_a = (x1 > x) ? 1 : -1;
	int y_a = (y1 > y) ? 1 : -1;
	int x_n = x, y_n = y;
	int length_x = (dx > 0) ? dx : -dx;
	int length_y = (dy > 0) ? dy : -dy;
	if (length_x >= length_y) {
		for (int i = 0;i < length_x;i++) {
			y_n = ((x_n - x) * dy / dx) + y;
			if (x_n > 0 && x_n <= WINDOWS_LENGTH && y_n > 0 && y_n <= WINDOWS_HIGH) {
				OLED_DrawAPoint(x_n, y_n, color);
			}
			x_n += x_a;
		}
	}
	else {
		for (int i = 0;i < length_y;i++) {
			x_n = ((y_n - y) * dx / dy) + x;
			if (x_n > 0 && x_n <= WINDOWS_LENGTH && y_n > 0 && y_n <= WINDOWS_HIGH) {
				OLED_DrawAPoint(x_n, y_n, color);
			}
			y_n += y_a;
		}
	}
}

/**
	* @brief  画一条有宽度的线
	* @param  x、y坐标
	* @param  x1、y1坐标
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawALine_Wide(int x, int y, int x1, int y1, int wide, Color color) {
	int dx = x1 - x;
	int dy = y1 - y;
	int x_a = (x1 > x) ? 1 : -1;
	int y_a = (y1 > y) ? 1 : -1;
	int x_n = x, y_n = y;
	int x_o, y_o;
	int length_x = (dx > 0) ? dx : -dx;
	int length_y = (dy > 0) ? dy : -dy;
	if (length_x >= length_y) {
		for (int i = 0;i < length_x;i++) {
			y_n = ((x_n - x) * dy / dx) + y;
			x_o = x_n - wide / 2;
			y_o = y_n - wide / 2;
			for (int k = 0;k < wide;k++){
				for (int j = 0;j < wide;j++){
					OLED_DrawAPoint(x_o + k, y_o + j, color);
				}
			}
			x_n += x_a;
		}
	}
	else {
		for (int i = 0;i < length_y;i++) {
			x_n = ((y_n - y) * dx / dy) + x;
			x_o = x_n - wide / 2;
			y_o = y_n - wide / 2;
			for (int k = 0;k < wide;k++){
				for (int j = 0;j < wide;j++){
					OLED_DrawAPoint(x_o + k, y_o + j, color);
				}
			}
			y_n += y_a;
		}
	}
}


/**
	* @brief  Axis专用画线函数
	* @param  x、y坐标
	* @param  x1、y1坐标
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @retval 无
	*/
void OLED_DrawALine_For_Axis(int x, int y, int x1, int y1, Color color) {
	int dx = x1 - x;
	int dy = y1 - y;
	int x_a = (x1 > x) ? 1 : -1;
	int y_a = (y1 > y) ? 1 : -1;
	int x_n = x, y_n = y;
	int length_x = (dx > 0) ? dx : -dx;
	int length_y = (dy > 0) ? dy : -dy;
	if (length_x < length_y) {
		for (int i = 0;i < ((WINDOWS_HIGH_MAX < length_y) ? WINDOWS_HIGH_MAX : length_y);i++) {
			x_n = ((y_n - y) * dx / dy) + x;
			if (x_n > 0 && x_n <= WINDOWS_LENGTH && y_n > 0 && y_n <= WINDOWS_HIGH) {
				OLED_DrawAPoint(x_n, y_n, color);
			}
			y_n += y_a;
		}
	}
}

/**
	* @brief  显示旋转字符
	* @param  h 小时，范围：0~11
	* @param  min 分钟，范围：0~59
	* @param  second 秒钟，范围：0~59
	* @retval 无
	*/
void OLED_DrawAClock(int h, int min, int second) {
	int h_x, h_y;
	int m_x, m_y;
	int s_x, s_y;
	h_x = sin((float)-(h * pi / 6 + pi + min * pi / 720)) * 10 + WINDOWS_HIGH;
	h_y = cos((float)-(h * pi / 6 + pi + min * pi / 720)) * 10 + 32;
	m_x = sin((float)-(min * pi / 30 + pi + second * pi / 3600)) * 20 + WINDOWS_HIGH;
	m_y = cos((float)-(min * pi / 30 + pi + second * pi / 3600)) * 20 + 32;
	s_x = sin((float)-second * pi / 30 + pi) * 25 + WINDOWS_HIGH;
	s_y = cos((float)-second * pi / 30 + pi) * 25 + 32;
	OLED_DrawARound(WINDOWS_HIGH, 32, 30, 29, OLED_White);
	OLED_DrawALine(WINDOWS_HIGH, 32, h_x, h_y, OLED_White);
	OLED_DrawALine(WINDOWS_HIGH, 32, m_x, m_y, OLED_White);
	OLED_DrawALine(WINDOWS_HIGH, 32, s_x, s_y, OLED_White);
	OLEDMin_DrawAWord(60, 4, '0', OLED_White);
	OLEDMin_DrawAWord(36, 28, '9', OLED_White);
	OLEDMin_DrawAWord(84, 28, '3', OLED_White);
	OLEDMin_DrawAWord(60, 52, '6', OLED_White);
}

/// @brief 透视投影: VP越大透视越弱, VP越小透视越强
void OLED3D_DrawASquareFace(int x_c, int y_c, int z_c,
	int x_1, int y_1, int z_1,
	int x_2, int y_2, int z_2,
	int x_3, int y_3, int z_3,
	int x_4, int y_4, int z_4, Color color) {
	x_1 += 64;x_2 += 64;x_3 += 64;x_4 += 64;

	z_1 += 32;z_2 += 32;z_3 += 32;z_4 += 32;
	// 叠加物体中心
	float yw1 = (float)(y_c + y_1);
	float yw2 = (float)(y_c + y_2);
	float yw3 = (float)(y_c + y_3);
	float yw4 = (float)(y_c + y_4);
	if (yw1 < 0 || yw2 < 0 || yw3 < 0 || yw4 < 0)
		return;

	float VP = (float)VanishingPointLength;
	// 除法透视: k = VP / (VP + yw)
	float k1 = (VP) / (VP + yw1);
	float k2 = (VP) / (VP + yw2);
	float k3 = (VP) / (VP + yw3);
	float k4 = (VP) / (VP + yw4);

	float cx = 64;
	float cz = 32;

	int sx1 = (int)(((x_1 + x_c - cx) * k1) + cx);
	int sx2 = (int)(((x_2 + x_c - cx) * k2) + cx);
	int sx3 = (int)(((x_3 + x_c - cx) * k3) + cx);
	int sx4 = (int)(((x_4 + x_c - cx) * k4) + cx);

	int sz1 = (int)(((z_1 + z_c - cz) * k1) + cz);
	int sz2 = (int)(((z_2 + z_c - cz) * k2) + cz);
	int sz3 = (int)(((z_3 + z_c - cz) * k3) + cz);
	int sz4 = (int)(((z_4 + z_c - cz) * k4) + cz);

	OLED_DrawAquard(sx1, sz1, sx2, sz2, sx3, sz3, sx4, sz4, color);
}


void Normal_Vector(
	int x_1, int y_1, int z_1,
	int x_2, int y_2, int z_2,
	int x_3, int y_3, int z_3,
	float* x, float* y, float* z) {

	float ax = x_2 - x_1, ay = y_2 - y_1, az = z_2 - z_1;
	float bx = x_3 - x_1, by = y_3 - y_1, bz = z_3 - z_1;
	*x = ay * bz - az * by;
	*y = az * bx - ax * bz;
	*z = ax * by - ay * bx;
}

void quick_sort(int arr[], int lo, int hi) {
	if (lo < hi) {
		int pivot = arr[hi];
		int i = lo - 1;
		for (int j = lo; j < hi; j++) {
			if (arr[j] <= pivot) {
				i++;
				int tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
			}
		}
		int tmp = arr[i + 1];
		arr[i + 1] = arr[hi];
		arr[hi] = tmp;

		int di = i + 1;
		quick_sort(arr, lo, di - 1);
		quick_sort(arr, di + 1, hi);
	}
}




/**
	* @brief  显示一个旋转三维面
	* @param  对应坐标
	* @param  对应旋转角
	* @retval 无
	*/
void OLED3D_DrawARotateSquareFace(int x_c, int y_c, int z_c,
	int x_1, int y_1, int z_1,
	int x_2, int y_2, int z_2,
	int x_3, int y_3, int z_3,
	int x_4, int y_4, int z_4,
	int angle_x, int angle_y, int angle_z, Color color)
{
	x_c -= Vedio_X;
	y_c -= Vedio_Y;
	z_c -= Vedio_Z;

	float x,y,z;

	rotate_point(0, 0, angle_y, x_1, z_1, &x_1, &z_1);
	rotate_point(0, 0, angle_y, x_2, z_2, &x_2, &z_2);
	rotate_point(0, 0, angle_y, x_3, z_3, &x_3, &z_3);
	rotate_point(0, 0, angle_y, x_4, z_4, &x_4, &z_4);

	rotate_point(0, 0, angle_z, x_1, y_1, &x_1, &y_1);
	rotate_point(0, 0, angle_z, x_2, y_2, &x_2, &y_2);
	rotate_point(0, 0, angle_z, x_3, y_3, &x_3, &y_3);
	rotate_point(0, 0, angle_z, x_4, y_4, &x_4, &y_4);

	rotate_point(0, 0, angle_x, y_1, z_1, &y_1, &z_1);
	rotate_point(0, 0, angle_x, y_2, z_2, &y_2, &z_2);
	rotate_point(0, 0, angle_x, y_3, z_3, &y_3, &z_3);
	rotate_point(0, 0, angle_x, y_4, z_4, &y_4, &z_4);

	Normal_Vector(x_1, y_1, z_1, x_2, y_2, z_2, x_3, y_3, z_3, &x, &y, &z);

	x_1 += x_c;x_2 += x_c;x_3 += x_c;x_4 += x_c;
	
	y_1 += y_c;y_2 += y_c;y_3 += y_c;y_4 += y_c;

	z_1 += z_c;z_2 += z_c;z_3 += z_c;z_4 += z_c;

	// rotate_point(0, 0,  - Vedio_AngleY, x_1, z_1, &x_1, &z_1);
	// rotate_point(0, 0,  - Vedio_AngleY, x_2, z_2, &x_2, &z_2);
	// rotate_point(0, 0,  - Vedio_AngleY, x_3, z_3, &x_3, &z_3);
	// rotate_point(0, 0,  - Vedio_AngleY, x_4, z_4, &x_4, &z_4);

	// rotate_point(0, 0,  - Vedio_AngleZ, x_1, y_1, &x_1, &y_1);
	// rotate_point(0, 0,  - Vedio_AngleZ, x_2, y_2, &x_2, &y_2);
	// rotate_point(0, 0,  - Vedio_AngleZ, x_3, y_3, &x_3, &y_3);
	// rotate_point(0, 0,  - Vedio_AngleZ, x_4, y_4, &x_4, &y_4);

	// rotate_point(0, 0,  - Vedio_AngleX, y_1, z_1, &y_1, &z_1);
	// rotate_point(0, 0,  - Vedio_AngleX, y_2, z_2, &y_2, &z_2);
	// rotate_point(0, 0,  - Vedio_AngleX, y_3, z_3, &y_3, &z_3);
	// rotate_point(0, 0,  - Vedio_AngleX, y_4, z_4, &y_4, &z_4);

	//if (x * Vedio_Normal_Vector[0] + y * Vedio_Normal_Vector[1] + z * Vedio_Normal_Vector[2]<=0)
	OLED3D_DrawASquareFace(0, 0, 0, x_1, y_1, z_1, x_2, y_2, z_2, x_3, y_3, z_3, x_4, y_4, z_4, color);
}
void OLED3D_DrawATriangleFace(int x_c, int y_c, int z_c,
	int x_1, int y_1, int z_1,
	int x_2, int y_2, int z_2,
	int x_3, int y_3, int z_3, Color color) {
	x_1 += 64;x_2 += 64;x_3 += 64;
	z_1 += 32;z_2 += 32;z_3 += 32;

	double yw1 = (double)(y_c + y_1);
	double yw2 = (double)(y_c + y_2);
	double yw3 = (double)(y_c + y_3);
	if (yw1 < 0 || yw2 < 0 || yw3 < 0)
		return;

	double VP = (double)VanishingPointLength;
	double k1 = (VP) / (VP + yw1);
	double k2 = (VP) / (VP + yw2);
	double k3 = (VP) / (VP + yw3);

	double cx = 64;
	double cz = 32;

	int sx1 = (int)(((x_1 + x_c - cx) * k1) + cx);
	int sx2 = (int)(((x_2 + x_c - cx) * k2) + cx);
	int sx3 = (int)(((x_3 + x_c - cx) * k3) + cx);

	int sz1 = (int)(((z_1 + z_c - cz) * k1) + cz);
	int sz2 = (int)(((z_2 + z_c - cz) * k2) + cz);
	int sz3 = (int)(((z_3 + z_c - cz) * k3) + cz);

	OLED_DrawAThriangle(sx1, sz1, sx2, sz2, sx3, sz3, color);
}

void OLED3D_DrawARotateTriangleFace(int x_c, int y_c, int z_c,
	int x_1, int y_1, int z_1,
	int x_2, int y_2, int z_2,
	int x_3, int y_3, int z_3,
	int angle_x, int angle_y, int angle_z, Color color)
{
	x_c -= Vedio_X;
	y_c -= Vedio_Y;
	z_c -= Vedio_Z;

	float x,y,z;

	rotate_point(0, 0, angle_y, x_1, z_1, &x_1, &z_1);
	rotate_point(0, 0, angle_y, x_2, z_2, &x_2, &z_2);
	rotate_point(0, 0, angle_y, x_3, z_3, &x_3, &z_3);

	rotate_point(0, 0, angle_z, x_1, y_1, &x_1, &y_1);
	rotate_point(0, 0, angle_z, x_2, y_2, &x_2, &y_2);
	rotate_point(0, 0, angle_z, x_3, y_3, &x_3, &y_3);

	rotate_point(0, 0, angle_x, y_1, z_1, &y_1, &z_1);
	rotate_point(0, 0, angle_x, y_2, z_2, &y_2, &z_2);
	rotate_point(0, 0, angle_x, y_3, z_3, &y_3, &z_3);

	Normal_Vector(x_1, y_1, z_1, x_2, y_2, z_2, x_3, y_3, z_3, &x, &y, &z);

	x_1 += x_c;x_2 += x_c;x_3 += x_c;
	y_1 += y_c;y_2 += y_c;y_3 += y_c;
	z_1 += z_c;z_2 += z_c;z_3 += z_c;

	rotate_point(0, 0,  - Vedio_AngleY, x_1, z_1, &x_1, &z_1);
	rotate_point(0, 0,  - Vedio_AngleY, x_2, z_2, &x_2, &z_2);
	rotate_point(0, 0,  - Vedio_AngleY, x_3, z_3, &x_3, &z_3);

	rotate_point(0, 0,  - Vedio_AngleZ, x_1, y_1, &x_1, &y_1);
	rotate_point(0, 0,  - Vedio_AngleZ, x_2, y_2, &x_2, &y_2);
	rotate_point(0, 0,  - Vedio_AngleZ, x_3, y_3, &x_3, &y_3);

	rotate_point(0, 0,  - Vedio_AngleX, y_1, z_1, &y_1, &z_1);
	rotate_point(0, 0,  - Vedio_AngleX, y_2, z_2, &y_2, &z_2);
	rotate_point(0, 0,  - Vedio_AngleX, y_3, z_3, &y_3, &z_3);

	OLED3D_DrawATriangleFace(0, 0, 0, x_1, y_1, z_1, x_2, y_2, z_2, x_3, y_3, z_3, color);
}

/**
 * @brief  Draw an axis-aligned cube with 6 square faces
 * @param  x_c y_c z_c : cube center
 * @param  half        : half side length (offset from center)
 * @param  color       : face color
 */
void OLED3D_DrawACube(int x_c, int y_c, int z_c, int half,int angle_x, int angle_y, int angle_z, Color color)
{
	int a = half;

	/* face y = +a */
	OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
		-a,  a, -a,   a,  a, -a,   a,  a,  a,  -a,  a,  a,angle_x, angle_y, angle_z, color);
	/* face y = -a */
	OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
		-a, -a,  a,   a, -a,  a,   a, -a, -a,  -a, -a, -a,angle_x, angle_y, angle_z, color);
	/* face x = +a */
	OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
		 a, -a, -a,   a,  a, -a,   a,  a,  a,   a, -a,  a,angle_x, angle_y, angle_z, color);
	/* face x = -a */
	OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
		-a, -a,  a,  -a,  a,  a,  -a,  a, -a,  -a, -a, -a,angle_x, angle_y, angle_z, color);
	/* face z = +a */
	OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
		-a, -a,  a,   a, -a,  a,   a,  a,  a,  -a,  a,  a,angle_x, angle_y, angle_z, color);
	/* face z = -a */
	OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
		-a,  a, -a,   a,  a, -a,   a, -a, -a,  -a, -a, -a, angle_x, angle_y, angle_z,color);
}


/**
	* @brief  显示旋转放大字符
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @param  num 放大倍数，范围：1~8
	* @retval 无
	*/
void OLED3D_AMPDrawARatateWord(int x, int y, int z, int x_c, int y_c, int z_c, int  anglex, int  angley, int  anglez, char CH, Color color, int num) {
	float k1 = 0;
	int x_r[4], z_r[4];
	int y_r[4] = { y_c,y_c,y_c,y_c };
	float VP = (float)VanishingPointLength_y;
	int idx = CH - ' ';
	for (int i = 0;i < 8;i++) {
		for (int j = 0;j < 8;j++) {
			if (OLED_F8x16[idx][i] & (1ull << j))
			{
				x_r[0] = (i - 4) * num + x_c;   z_r[0] = (j - 8) * num + z_c;
				x_r[1] = (i - 3) * num + x_c;   z_r[1] = (j - 8) * num + z_c;
				x_r[2] = (i - 3) * num + x_c;   z_r[2] = (j - 7) * num + z_c;
				x_r[3] = (i - 4) * num + x_c;   z_r[3] = (j - 7) * num + z_c;
				OLED3D_DrawARotateSquareFace(x,y,z,x_r[0] , y_r[0], z_r[0], x_r[1] , y_r[1], z_r[1], x_r[2] , y_r[2], z_r[2], x_r[3], y_r[3],z_r[3], anglex, angley, anglez, color);
			}
			if (OLED_F8x16[idx][i + 8] & (1ull << j))
			{
				x_r[0] = (i - 4) * num + x_c;   z_r[0] = (j + 0) * num + z_c;
				x_r[1] = (i - 3) * num + x_c;   z_r[1] = (j + 0) * num + z_c;
				x_r[2] = (i - 3) * num + x_c;   z_r[2] = (j + 1) * num + z_c;
				x_r[3] = (i - 4) * num + x_c;   z_r[3] = (j + 1) * num + z_c;
				OLED3D_DrawARotateSquareFace(x, y, z, x_r[0], y_r[0], z_r[0], x_r[1], y_r[1], z_r[1], x_r[2], y_r[2], z_r[2], x_r[3], y_r[3], z_r[3], anglex, angley, anglez, color);
			}
		}
	}
}

/**
	* @brief  显示旋转放大字符
	* @param  x、y坐标
	* @param  angle 旋转角度，范围：0~360
	* @param  CH 字符
	* @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
	* @param  num 放大倍数，范围：1~8
	* @retval 无
	*/
void OLED3DTKN_AMPDrawARatateWord(int x, int y, int z, int x_c, int y_c, int z_c, int TKN, int  anglex, int  angley, int  anglez, char CH, Color color, int num) {
	
}

void OLED3DTKN_AMPDrawARatateString(int x, int y, int z, int x_c, int y_c, int z_c, int TKN, int  anglex, int  angley, int  anglez, char* CH, Color color, int num) {
	int x_n, y_n, _z_n;
	int stringlength = strlen(CH);
	for (int i = 0;i < stringlength;i++)
	{
		OLED3DTKN_AMPDrawARatateWord(x, y, z, x_c - num * 4 * (stringlength - 2 * i), y_c, z_c, TKN, anglex, angley, anglez, *(CH + i), color, num);
	}
}

void OLED3D_AMPDrawARatateString(int x, int y, int z, int x_c, int y_c, int z_c, int  anglex, int  angley, int  anglez, char* CH, Color color, int num) {
	int x_n, y_n, _z_n;
	int stringlength = strlen(CH);
	for (int i = 0;i < stringlength;i++)
	{
		OLED3D_AMPDrawARatateWord(x, y, z, x_c - num * 4 * (stringlength - 2 * i), y_c, z_c, anglex, angley, anglez, *(CH + i), color, num);
	}
}


/**
  * @brief  画一个3D圆
  * @param  x_a 圆心x坐标，范围：0~127
  * @param  y_a 圆心y坐标，范围：0~63
  * @param  r 圆半径，范围：0~WINDOWS_HIGH
  * @param  r_a 内圆半径，范围：0~WINDOWS_HIGH
  * @param  color 颜色，范围：OLED_Black、OLED_White、OLED_Contr
  * @retval 无
  */
void OLED3D_DrawARound(int x_a, int y_a, int z_a, int r, int  anglex, int  angley, int  anglez, Color color) {
	int x, y, z;
	int x_r, y_r, z_r;
	int x_max, z_max, x_min, z_min;
	float k1;
	int r2 = r * r;
	x_max = x_a + r;
	x_min = x_a - r;
	z_max = z_a + r;
	z_min = z_a - r;

	for (int i = -r;i < r;i++)
	{
		for (int j = -r;j < r;j++)
		{
			if (i * i + j * j <= r2)
			{
				rotate_point(0, 0, angley, i, j, &x_r, &z_r);
				rotate_point(0, 0, anglez, x_r, 0, &x_r, &y_r);
				rotate_point(0, 0, anglex, y_r, z_r, &y_r, &z_r);//A面

				k1 = (float)(VanishingPointLength_y - y_a - y_r) / VanishingPointLength_y;
				x_r = (float)(x_r + x_a - VanishingPointLength_x) * k1 + VanishingPointLength_x;
				z_r = (float)(z_r + x_a - VanishingPointLength_z) * k1 + VanishingPointLength_z;
				OLED_DrawAPoint(x_r, z_r, color);
			}
		}
	}
}

void OLED_DrawAPen(int x, int y, int wide, Color color)
{
	x = x - wide / 2;
	y = y - wide / 2;
	for (int i = 0;i < wide;i++)
	{
		for (int j = 0;j < wide;j++)
		{
			OLED_DrawAPoint(x + i, y + j, color);
		}
	}
}

int n_vector_init[3][3] = {
	   {10,0,0},
	   {0,10,0},
	   {0,0,10}
};

int n_vector[3][3] = {
	   {10,0,0},
	   {0,10,0},
	   {0,0,10}
};

void Vedio_Updata(int x, int y, int z, float anglex, float angley, float anglez){
	Vedio_X = x;Vedio_Y = y;Vedio_Z = z;

	Vedio_AngleX = anglex;Vedio_AngleY = angley;Vedio_AngleZ = anglez;
	
	Vedio_Normal_Vector[0] = 0;Vedio_Normal_Vector[1] = 10;Vedio_Normal_Vector[2] = 0;

	rotate_point(0, 0, angley, Vedio_Normal_Vector[0], Vedio_Normal_Vector[2], &Vedio_Normal_Vector[0], &Vedio_Normal_Vector[2]);
	rotate_point(0, 0, anglez, Vedio_Normal_Vector[0], Vedio_Normal_Vector[1], &Vedio_Normal_Vector[0], &Vedio_Normal_Vector[1]);
	rotate_point(0, 0, anglex, Vedio_Normal_Vector[1], Vedio_Normal_Vector[2], &Vedio_Normal_Vector[1], &Vedio_Normal_Vector[2]);
}
