/*
 * EffecTV - Realtime Digital Video Effector
 * Copyright (C) 2001-2002 FUKUCHI Kentaro
 *
 * yuv.c: YUV(YCbCr) color system utilities
 *
 */

#include <math.h>
#include "m_pd.h"

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

float YtoRGB[256];
float VtoR[256], VtoG[256];
float UtoG[256], UtoB[256];
float RtoY[256], RtoU[256], RtoV[256];
float GtoY[256], GtoU[256], GtoV[256];
float BtoY[256],            BtoV[256];

static int yuvinit=-1;

int yuv_init(void)
{
  int i;

    if(yuvinit==-1) {
      for(i=0; i<256; i++) {
		YtoRGB[i] =  1.164*(i-16);
		VtoR[i] =  1.596*(i-128);
		VtoG[i] = -0.813*(i-128);
		UtoG[i] = -0.391*(i-128);
		UtoB[i] =  2.018*(i-128);
		RtoY[i] =  0.257*i;
		RtoU[i] = -0.148*i;
		RtoV[i] =  0.439*i;
		GtoY[i] =  0.504*i;
		GtoU[i] = -0.291*i;
		GtoV[i] = -0.368*i;
		BtoY[i] =  0.098*i;
		BtoV[i] = -0.071*i;
      }
      yuvinit=1;
    }

    return 0;
}

unsigned char yuv_RGBtoY(int rgb)
{
  int i;

    if ( yuvinit == -1 ) { yuv_init(); }
    i = RtoY[(rgb>>16)&0xff];
    i += GtoY[(rgb>>8)&0xff];
    i += BtoY[rgb&0xff];
    i += 16;

    return i;
}

unsigned char yuv_RGBtoU(int rgb)
{
  int i;

    if ( yuvinit == -1 ) { yuv_init(); }
    i = RtoU[(rgb>>16)&0xff];
    i += GtoU[(rgb>>8)&0xff];
    i += RtoV[rgb&0xff];/* BtoU == RtoV */
    i += 128;

    return i;
}

unsigned char yuv_RGBtoV(int rgb)
{
  int i;
	
    if ( yuvinit == -1 ) { yuv_init(); }
    i = RtoV[(rgb>>16)&0xff];
    i += GtoV[(rgb>>8)&0xff];
    i += BtoV[rgb&0xff];
    i += 128;

    return i;
}

unsigned char yuv_YUVtoR(unsigned char y, unsigned char u, unsigned char v)
{
  int r;

    if ( yuvinit == -1 ) { yuv_init(); }
    r = YtoRGB[(int)y] + VtoR[(int)v];
    if ( r>255 ) r=255;
    if ( r<0 ) r=0;
    return r;
}

unsigned char yuv_YUVtoG(unsigned char y, unsigned char u, unsigned char v)
{
  int g;
    
    if ( yuvinit == -1 ) { yuv_init(); }
    g = YtoRGB[(int)y] + UtoG[(int)u] + VtoG[(int)v];
    if ( g>255 ) g=255;
    if ( g<0 ) g=0;
    return g;
}

unsigned char yuv_YUVtoB(unsigned char y, unsigned char u, unsigned char v)
{
  int b;

    if ( yuvinit == -1 ) { yuv_init(); }
    b = YtoRGB[(int)y] + UtoB[(int)u];
    if ( b>255 ) b=255;
    if ( b<0 ) b=0;
    return b;
}

int yuv_YUVtoRGB(unsigned char y, unsigned char u, unsigned char v)
{
    if ( yuvinit == -1 ) { yuv_init(); }
    return ( (yuv_YUVtoR(y,u,v)<<16) + (yuv_YUVtoG(y,u,v)<<8) + (yuv_YUVtoB(y,u,v)) ); 
}

void yuv_Y122RGB( short int* packet, unsigned int *rgb, int width, int height )
{
  unsigned char y=0,u=0,v=0;
  int X,Y;
  int uoffset = width*height;
  int maxoffset = width*height+((width*height)>>1);
  int voffset = width*height+((width*height)>>2);

  if ( !packet || !rgb ) 
  {
     post( "yuv_Y122RGB : pointers are NULL !!!" );
     return;
  }
  if ( yuvinit == -1 ) { yuv_init(); }
  for(Y=0; Y < height; Y++){
     for(X=0; X < width; X++){
        // post( "yuv_Y122RGB : X=%d Y=%d", X, Y );
        if ( (Y*width+X) < maxoffset ) 
            y=(packet[Y*width+X]>>7);
        if( (uoffset+((Y>>1)*width+(X>>1))) < maxoffset )
            u=(packet[uoffset+((Y>>1)*width+(X>>1))]>>8)+128;
        if( (voffset+((Y>>1)*width+(X>>1))) < maxoffset )
            v=(packet[voffset+((Y>>1)*width+(X>>1))]>>8)+128;

        rgb[Y*width+X] = yuv_YUVtoRGB( y, u, v );
     }
  }

}

void yuv_RGB2Y12( unsigned int *rgb, short int* packet, int width, int height )
{
  short int y,u,v,iu=0,iv=0;
  int X,Y;
  int uoffset = width*height;
  int maxoffset = width*height+((width*height)>>1);
  int voffset = width*height+((width*height)>>2);

  if ( !packet || !rgb ) 
  {
     post( "yuv_RGB2Y12 : pointers are NULL !!!" );
     return;
  }
  if ( yuvinit == -1 ) { yuv_init(); };
  for(Y=0; Y < height; Y++)
     for(X=0; X < width; X++){
     {
        // post( "yuv_RGB2Y12 : X=%d Y=%d", X, Y );
        y=yuv_RGBtoY( rgb[Y*width+X] );
        u=yuv_RGBtoU( rgb[Y*width+X] );
        v=yuv_RGBtoV( rgb[Y*width+X] );

        if ( (Y*width+X) < maxoffset ) 
            packet[Y*width+X]=(y<<7); 
          if( (uoffset+iu) < maxoffset )
            packet[uoffset+iu]=(u-128<<8);
          if( (voffset+iv) < maxoffset )
            packet[voffset+iv]=(v-128<<8);
        if ( ( X%2 == 0) && ( Y%2 == 0) )
        {
          iu++; iv++;
        }
     }
  }

}
