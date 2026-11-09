#include "OLED.h"

/* 低模汽车：34 四边形 + 4 八边形轮
 * 顶点统一为循环顺序（从面外侧看逆时针），法向量朝外 */
static const int16_t car_quad[34][12] = {
    { -10, -25, 4, -10, 25, 4, 10, 25, 4, 10, -25, 4 },
    { -10, -25, 4, -10, -25, 12, -10, 25, 12, -10, 25, 4 },
    { 10, -25, 4, 10, 25, 4, 10, 25, 12, 10, -25, 12 },
    { -10, -25, 4, 10, -25, 4, 10, -25, 12, -10, -25, 12 },
    { -10, 18, 5, -9, 25, 8, 9, 25, 8, 10, 18, 5 },
    { -10, 18, 5, -10, 18, 12, -9, 25, 11, -9, 25, 8 },
    { 10, 18, 5, 9, 25, 8, 9, 25, 11, 10, 18, 12 },
    { -9, 25, 8, -9, 25, 11, 9, 25, 11, 9, 25, 8 },
    { -9, 7, 12, 9, 7, 12, 8, 18, 11, -8, 18, 11 },
    { -9, 7, 12, -8, 18, 11, -10, 18, 11, -10, 7, 12 },
    { 9, 7, 12, 10, 7, 12, 10, 18, 11, 8, 18, 11 },
    { -6, -6, 18, 6, -6, 18, 6, 6, 18, -6, 6, 18 },
    { -9, -7, 12, -6, -6, 18, -6, 6, 18, -9, 7, 12 },
    { 9, -7, 12, 9, 7, 12, 6, 6, 18, 6, -6, 18 },
    { -6, 6, 18, 6, 6, 18, 9, 7, 12, -9, 7, 12 },
    { -6, -6, 18, -9, -7, 12, 9, -7, 12, 6, -6, 18 },
    { -10, 0, 5, -10, 0, 12, -10, 8, 12, -10, 8, 5 },
    { -10, 1, 12, -10, 1, 16, -10, 6, 16, -10, 7, 12 },
    { -10, -9, 5, -10, -8, 12, -10, 0, 12, -10, 0, 5 },
    { -10, -7, 12, -10, -6, 16, -10, -1, 16, -10, -1, 12 },
    { 10, 0, 5, 10, 8, 5, 10, 8, 12, 10, 0, 12 },
    { 10, 1, 12, 10, 7, 12, 10, 6, 16, 10, 1, 16 },
    { 10, -9, 5, 10, 0, 5, 10, 0, 12, 10, -8, 12 },
    { 10, -7, 12, 10, -1, 12, 10, -1, 16, 10, -6, 16 },
    { -9, 25, 4, -9, 26, 7, 9, 26, 7, 9, 25, 4 },
    { -9, 25, 4, -9, 26, 7, -6, 26, 7, -6, 25, 4 },
    { 9, 25, 4, 6, 25, 4, 6, 26, 7, 9, 26, 7 },
    { -8, 26, 8, -8, 26, 11, -4, 26, 11, -4, 26, 8 },
    { 4, 26, 8, 4, 26, 11, 8, 26, 11, 8, 26, 8 },
    { -3, 26, 5, -3, 26, 7, 3, 26, 7, 3, 26, 5 },
    { -9, -25, 4, 9, -25, 4, 9, -26, 7, -9, -26, 7 },
    { -8, -26, 8, -4, -26, 8, -4, -26, 11, -8, -26, 11 },
    { 4, -26, 8, 8, -26, 8, 8, -26, 11, 4, -26, 11 },
    { -8, -15, 12, -8, -21, 11, 8, -21, 11, 8, -15, 12 },
};

static const int16_t car_wheel[4][24] = {
    { -11, 12, 9, -11, 14, 8, -11, 15, 5, -11, 14, 2, -11, 12, 1, -11, 9, 2, -11, 8, 5, -11, 9, 8 },
    { 11, 12, 9, 11, 9, 8, 11, 8, 5, 11, 9, 2, 11, 12, 1, 11, 14, 2, 11, 15, 5, 11, 14, 8 },
    { -11, -13, 9, -11, -10, 8, -11, -9, 5, -11, -10, 2, -11, -13, 1, -11, -16, 2, -11, -17, 5, -11, -16, 8 },
    { 11, -13, 9, 11, -16, 8, 11, -17, 5, 11, -16, 2, 11, -13, 1, 11, -10, 2, 11, -9, 5, 11, -10, 8 },
};

void OLED3D_DrawACar(int x_c, int y_c, int z_c, int scale,
                     int anglex, int angley, int anglez, Color color)
{
	int i;

	for (i = 0; i < 34; i++){
		OLED3D_DrawARotateSquareFace(x_c, y_c, z_c,
			car_quad[i][0]*scale, car_quad[i][1]*scale, -car_quad[i][2]*scale,
			car_quad[i][3]*scale, car_quad[i][4]*scale, -car_quad[i][5]*scale,
			car_quad[i][6]*scale, car_quad[i][7]*scale, -car_quad[i][8]*scale,
			car_quad[i][9]*scale, car_quad[i][10]*scale, -car_quad[i][11]*scale,
			anglex, angley, anglez, color);
	}

	for (i = 0; i < 4; i++){
		OLED3D_DrawARotateTriangleFace(x_c, y_c, z_c,
			car_wheel[i][0]*scale,  car_wheel[i][1]*scale,  -car_wheel[i][2]*scale,
			car_wheel[i][3]*scale,  car_wheel[i][4]*scale,  -car_wheel[i][5]*scale,
			car_wheel[i][6]*scale,  car_wheel[i][7]*scale,  -car_wheel[i][8]*scale,
			anglex, angley, anglez, color);
		OLED3D_DrawARotateTriangleFace(x_c, y_c, z_c,
			car_wheel[i][0]*scale,  car_wheel[i][1]*scale,  -car_wheel[i][2]*scale,
			car_wheel[i][6]*scale,  car_wheel[i][7]*scale,  -car_wheel[i][8]*scale,
			car_wheel[i][9]*scale,  car_wheel[i][10]*scale, -car_wheel[i][11]*scale,
			anglex, angley, anglez, color);
		OLED3D_DrawARotateTriangleFace(x_c, y_c, z_c,
			car_wheel[i][0]*scale,  car_wheel[i][1]*scale,  -car_wheel[i][2]*scale,
			car_wheel[i][9]*scale,  car_wheel[i][10]*scale, -car_wheel[i][11]*scale,
			car_wheel[i][12]*scale, car_wheel[i][13]*scale, -car_wheel[i][14]*scale,
			anglex, angley, anglez, color);
		OLED3D_DrawARotateTriangleFace(x_c, y_c, z_c,
			car_wheel[i][0]*scale,  car_wheel[i][1]*scale,  -car_wheel[i][2]*scale,
			car_wheel[i][12]*scale, car_wheel[i][13]*scale, -car_wheel[i][14]*scale,
			car_wheel[i][15]*scale, car_wheel[i][16]*scale, -car_wheel[i][17]*scale,
			anglex, angley, anglez, color);
		OLED3D_DrawARotateTriangleFace(x_c, y_c, z_c,
			car_wheel[i][0]*scale,  car_wheel[i][1]*scale,  -car_wheel[i][2]*scale,
			car_wheel[i][15]*scale, car_wheel[i][16]*scale, -car_wheel[i][17]*scale,
			car_wheel[i][18]*scale, car_wheel[i][19]*scale, -car_wheel[i][20]*scale,
			anglex, angley, anglez, color);
		OLED3D_DrawARotateTriangleFace(x_c, y_c, z_c,
			car_wheel[i][0]*scale,  car_wheel[i][1]*scale,  -car_wheel[i][2]*scale,
			car_wheel[i][18]*scale, car_wheel[i][19]*scale, -car_wheel[i][20]*scale,
			car_wheel[i][21]*scale, car_wheel[i][22]*scale, -car_wheel[i][23]*scale,
			anglex, angley, anglez, color);
	}
}