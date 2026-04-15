#ifndef __OLED_H
#define __OLED_H


 typedef enum{
	OLED_Black = 0,
	OLED_White = 1,
	OLED_Contr = 2
}Color;

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowPicture(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

void OLED_Horizon(int X,int Y,int Z);
void OLED_ShowRowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowMinChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowMinString(uint8_t Line, uint8_t Column, char *String);

void OLED_DrawARound(int x_a,int y_a,int r,int r_a,Color color);
void OLED_DrawAWord(int x,int y,char CH,Color color);
void OLED_AMPDrawAWord(int x,int y,char CH,Color color,uint8_t num);
void OLEDMin_AMPDrawAWord(int x,int y,char CH,Color color,uint8_t num);
void OLED_DrawAThriangle(int x_a,int y_a,int x_1,int y_1,int x_2,int y_2,Color color);
void OLED_DrawAquard(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,Color color);
void OLED_DrawAPentagon(int x1,int y1,int x2,int x3,int y3,int y2,int x4,int y4,int x5,int y5,Color color);
void OLED_DrawAGeDiao(void);
void OLED_DrawAString(int x,int y,char * CH,Color color);
void OLED_DrawARatateWord(int x,int y,float angle,char CH,Color color);
void OLEDMin_DrawARatateWord(int x,int y,float angle,char CH,Color color);
void OLED_DrawARatateString(int x,int y,float angle,char* CH,Color color);
void OLED_AMPDrawARatateWord(int x,int y,float angle ,char CH,Color color,uint8_t num);
void OLED_DrawAPolygon(int x,int y,int side, int lenght , float angle,Color color);
void OLED_DrawARatateNum(int x,int y,float angle,int num,int length,Color color);
void OLEDMin_DrawARatateString(int x,int y,float angle,char * CH,Color color);
void OLEDMin_DrawARatateNum(int x,int y,float angle,int num,int length,Color color);
void rotate_point(float cx, float cy, float angle, float	x, 	float y, int *out_x, int *out_y);

#endif
