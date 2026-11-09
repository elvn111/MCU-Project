#ifndef __OLED_H
#define __OLED_H

#include "gpio.h"
#include <stdint.h>
#include <string.h>
#include "stdio.h"
#include <math.h>

#define WINDOWS_LENGTH_MAX 128
#define WINDOWS_HIGH_MAX 64

typedef enum{
	OLED_Black = 0,
	OLED_White = 1,
	OLED_Contr = 2
}Color;

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_I2C_Init(void);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_SendByte(uint8_t Byte);
void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t Data);
void OLED_SetCursor(uint8_t Y, uint8_t X);
uint32_t OLED_Pow(uint32_t X, uint32_t Y);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_AMPDrawAPoint(int x, int y, int num, Color color);
void OLED_DrawAByte(int x, int y, int Byte, Color color);


void OLED_ShowPicture(int x0, int y0, int x1, int y1);
void OLED_DrawClear(int x0, int y0, int x1, int y1);
void OLED_DrawAPoint(int x, int y, Color color);

void OLED_ShowRowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowMinChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowMinString(uint8_t Line, uint8_t Column, char *String);


void OLEDMin_DrawAWord(int x,int y,char CH,Color color);
void OLEDMin_DrawARatateWord(int x,int y,float angle,char CH,Color color);
void OLEDMin_AMPDrawAWord(int x,int y,char CH,Color color,uint8_t num);

void OLEDMin_DrawARatateString(int x,int y,float angle,char * CH,Color color);

void OLEDMin_DrawARatateNum(int x,int y,float angle,int num,int length,Color color);
void OLEDMin_DrawANum(int x,int y,int num,int length,Color color);
void OLEDMin_DrawAString(int x,int y,char * CH,Color color);


void OLED_DrawAPolygon(int x,int y,int side, int lenght , float angle,Color color);
void OLED_DrawARound(int x_a,int y_a,int r,int r_a,Color color);
void OLED_DrawAThriangle(int x_a,int y_a,int x_1,int y_1,int x_2,int y_2,Color color);
void OLED_DrawAquard(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,Color color);
void OLED_DrawAGeDiao(void);

void OLED_DrawAWord(int x,int y,char CH,Color color);
void OLED_AMPDrawAWord(int x,int y,char CH,Color color,uint8_t num);
void OLED_DrawARatateWord(int x,int y,float angle,char CH,Color color);
void OLED_AMPDrawARatateWord(int x,int y,float angle ,char CH,Color color,uint8_t num);

void OLED_DrawAString(int x,int y,char * CH,Color color);
void OLED_DrawARatateString(int x,int y,float angle,char* CH,Color color);

void OLED_DrawARatateNum(int x,int y,float angle,int num,int length,Color color);
void OLED_DrawANum(int x,int y,int num,int length,Color color);

void rotate_point(float cx, float cy, float angle, float	x, 	float y, int *out_x, int *out_y);
void OLED_DrawAAixs(int x, int y, int wide, int high, int cyc, int* data, int16_t num, uint16_t t, Color color, int AMP, int fangxiang);

void OLED_DrawALine(int x,int y,int x1,int y1,Color color);
void OLED_DrawALine_Wide(int x, int y, int x1, int y1, int wide, Color color);
void OLED_DrawALine_For_Axis(int x,int y,int x1,int y1,Color color);

void OLED_DrawAClock(int h,int min,int scend);

void OLED3D_DrawASquareFace(int x_c, int y_c, int z_c, int x_1, int y_1, int z_1, int x_2, int y_2, int z_2, int x_3, int y_3, int z_3, int x_4, int y_4, int z_4, Color color);
void OLED3D_DrawARotateSquareFace(int x_c, int y_c, int z_c, int x_1, int y_1, int z_1, int x_2, int y_2, int z_2, int x_3, int y_3, int z_3, int x_4, int y_4, int z_4, int angle_x, int angle_y, int angle_z, Color color);
void OLED3D_DrawATriangleFace(int x_c, int y_c, int z_c, int x_1, int y_1, int z_1, int x_2, int y_2, int z_2, int x_3, int y_3, int z_3, Color color);
void OLED3D_DrawARotateTriangleFace(int x_c, int y_c, int z_c, int x_1, int y_1, int z_1, int x_2, int y_2, int z_2, int x_3, int y_3, int z_3, int angle_x, int angle_y, int angle_z, Color color);
void OLED3D_AMPDrawARatateWord(int x, int y, int z, int x_c, int y_c, int z_c, int  anglex, int  angley, int  anglez, char CH, Color color, int num);
void OLED3DTKN_AMPDrawARatateWord(int x, int y, int z, int x_c, int y_c, int z_c, int TKN, int  anglex, int  angley, int  anglez, char CH, Color color, int num);
void OLED3DTKN_AMPDrawARatateString(int x, int y, int z, int x_c, int y_c, int z_c, int TKN, int  anglex, int  angley, int  anglez, char* CH, Color color, int num);
void OLED3D_AMPDrawARatateString(int x, int y, int z, int x_c, int y_c, int z_c, int  anglex, int  angley, int  anglez, char* CH, Color color, int num);
void OLED3D_DrawARound(int x_a, int y_a, int z_a, int r, int  anglex, int  angley, int  anglez, Color color);
void OLED3D_DrawACube(int x_c, int y_c, int z_c, int half,int angle_x, int angle_y, int angle_z, Color color);

void OLED_DrawAPen(int x, int y, int wide, Color color);

void Vedio_Updata(int x, int y, int z, float anglex, float angley, float anglez);
int area2(int x1, int y1, int x2, int y2, int x3, int y3);
int pointInTriangle(int px, int py, int x0, int y0, int x1, int y1, int x2, int y2);
void quick_sort(int arr[], int lo, int hi);
void Normal_Vector(int x_1, int y_1, int z_1, int x_2, int y_2, int z_2, int x_3, int y_3, int z_3, float* x, float* y, float* z);
void rotate3D_point(float cx, float cy, float cz, float anglex, float angley, float anglez, float x, float y, float z, int* out_x, int* out_y, int* out_z);
#endif
