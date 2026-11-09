#ifndef __CAR_H
#define __CAR_H

#include "OLED.h"

/* 低模汽车：34 四边形 + 4 八边形轮，整数坐标
 * 调用示例：OLED3D_DrawACar(64, 100, 51, 2, angle, angle, angle, OLED_White); */
void OLED3D_DrawACar(int x_c, int y_c, int z_c, int scale,
                     int anglex, int angley, int anglez, Color color);

#endif /* __CAR_H */