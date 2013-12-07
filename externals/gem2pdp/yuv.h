/*
 * EffecTV - Realtime Digital Video Effector
 * Copyright (C) 2001-2002 FUKUCHI Kentaro
 *
 * yuv.c: YUV(YCbCr) color system utilities
 *
 */

#include <math.h>

/*
 * conversion from YUV to RGB
 *   r = 1.164*(y-16) + 1.596*(v-128);
 *   g = 1.164*(y-16) - 0.813*(v-128) - 0.391*(u-128);
 *   b = 1.164*(y-16)                 + 2.018*(u-128);
 * conversion from RGB to YUV
 *   y =  0.257*r + 0.504*g + 0.098*b + 16
 *   u = -0.148*r - 0.291*g + 0.439*b + 128
 *   v =  0.439*r - 0.368*g - 0.071*b + 128
 */

extern "C" {
int yuv_init(void);
unsigned char yuv_RGBtoY(int rgb);
unsigned char yuv_RGBtoU(int rgb);
unsigned char yuv_RGBtoV(int rgb);
unsigned char yuv_YUVtoR(unsigned char y, unsigned char u, unsigned char v);
unsigned char yuv_YUVtoG(unsigned char y, unsigned char u, unsigned char v);
unsigned char yuv_YUVtoB(unsigned char y, unsigned char u, unsigned char v);
int yuv_YUVtoRGB(unsigned char y, unsigned char u, unsigned char v);
void yuv_Y122RGB( short int* packet, unsigned int *rgb, int width, int height );
void yuv_RGB2Y12( unsigned int *rgb, short int* packet, int width, int height );
}
