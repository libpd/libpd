/*
 *  pix_contrast.cpp
 *  GEM_darwin
 *
 *  Created by lincoln on 8/23/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_contrast.h"

CPPEXTERN_NEW(pix_contrast);


pix_contrast :: pix_contrast():
  m_contrast(1.f), m_saturation(1.f)    
{
  m_inCon=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("contrast"));
  m_inSat=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("saturation"));
}

pix_contrast :: ~pix_contrast()
{
  inlet_free(m_inSat);
  inlet_free(m_inCon);
}

#ifdef __VEC__
// this will be called automatically _if and only if_
// this function is called processYUVAltivec
// (e.g. not processYUV_Altivec)
void pix_contrast :: processYUVAltivec(imageStruct &image)
{
  union
  {
    short	elements[8];
    vector	signed short v;
  }shortBuffer;

  short s_contrast=static_cast<short>(256.*m_contrast);
  short s_saturation=static_cast<short>(256. * m_saturation);

  if(256==s_contrast && 256==s_saturation)return;

	vector unsigned char *inData = (vector unsigned char*) image.data;
	//vector unsigned char load1,store1;
	vector signed short v128;
	vector unsigned int bitshift;
	vector signed short Y, UV,hiImage, loImage;
	vector signed short con, sat,szero;
	vector signed short clampLO,clampHI;
	register vector signed int UVhi,UVlo,Yhi,Ylo;
	
	vector unsigned char permuteY,permuteUV, permuteYUV, zero;
	
	union{
		unsigned char c[16];
		vector unsigned char v;
	}cBuf;


	//Y permute - odd bytes
	cBuf.c[0] = 16;
	cBuf.c[1] = 1;
	cBuf.c[2] = 16;
	cBuf.c[3] = 3;
	
	cBuf.c[4] = 16;
	cBuf.c[5] = 5;
	cBuf.c[6] = 16;
	cBuf.c[7] = 7;
	
	cBuf.c[8] = 16;
	cBuf.c[9] = 9;
	cBuf.c[10] = 16;
	cBuf.c[11] = 11;
	
	cBuf.c[12] = 16;
	cBuf.c[13] = 13;
	cBuf.c[14] = 16;
	cBuf.c[15] = 15;
	
	permuteY = cBuf.v;
	
	//UV permute - even bytes
	cBuf.c[0] = 16;
	cBuf.c[1] = 0;
	cBuf.c[2] = 16;
	cBuf.c[3] = 2;
	
	cBuf.c[4] = 16;
	cBuf.c[5] = 4;
	cBuf.c[6] = 16;
	cBuf.c[7] = 6;
	
	cBuf.c[8] = 16;
	cBuf.c[9] = 8;
	cBuf.c[10] = 16;
	cBuf.c[11] = 10;
	
	cBuf.c[12] = 16;
	cBuf.c[13] = 12;
	cBuf.c[14] = 16;
	cBuf.c[15] = 14;
	
	permuteUV = cBuf.v;
	
	
	//for final permute to recombine Y UV
	//UV first
	cBuf.c[0] = 1;
	cBuf.c[1] = 17;
	cBuf.c[2] = 3;
	cBuf.c[3] = 19;
	
	cBuf.c[4] = 5;
	cBuf.c[5] = 21;
	cBuf.c[6] = 7;
	cBuf.c[7] = 23;
	
	cBuf.c[8] = 9;
	cBuf.c[9] = 25;
	cBuf.c[10] = 11;
	cBuf.c[11] = 27;
	
	cBuf.c[12] = 13;
	cBuf.c[13] = 29;
	cBuf.c[14] = 15;
	cBuf.c[15] = 31;
	
	permuteYUV = cBuf.v;
	
	zero = vec_splat_u8(0);
	szero = vec_splat_s16(0);
	
	//vector of 128 to subtract and add to pixels
	shortBuffer.elements[0] = 128;
	v128 = shortBuffer.v;
	v128 = vec_splat(v128,0);
	
	//bitshift value
	//shortBuffer.elements[0] = 8;
	//bitshift = shortBuffer.v;
	bitshift = vec_splat_u32(8);
	
	shortBuffer.elements[0] = 255;
	clampHI = shortBuffer.v;
	clampHI = vec_splat(clampHI,0);
	
	clampLO = vec_splat_s16(0);
	
	//contrast value
	shortBuffer.elements[0] = s_contrast;
	con = shortBuffer.v;
	con = vec_splat(con,0);
	
	shortBuffer.elements[0] = s_saturation;
	sat = shortBuffer.v;
	sat = vec_splat(sat,0);
	
	int datasize = (image.xsize/8) * image.ysize;
	while(datasize--){
		
		//expand the UInt8's to short's
            hiImage = (vector signed short) vec_mergeh( zero, inData[0] );
            loImage = (vector signed short) vec_mergel( zero, inData[0] );
            
            //vec_subs -128
            hiImage = (vector signed short) vec_sub( hiImage, v128 );
            loImage = (vector signed short) vec_sub( loImage, v128 );   
            
            //now vec_mule the UV into two vector ints
            UVhi = vec_mule(sat,hiImage);
            UVlo = vec_mule(sat,loImage);
            
            //now vec_mulo the Y into two vector ints
            Yhi = vec_mulo(con,hiImage);
            Ylo = vec_mulo(con,loImage);
            
            //this is where to do the bitshift/divide due to the resolution
            UVhi = vec_sra(UVhi,bitshift);
            UVlo = vec_sra(UVlo,bitshift);
            Yhi = vec_sra(Yhi,bitshift);
            Ylo = vec_sra(Ylo,bitshift);
            
            //pack the UV into a single short vector
            UV = vec_packs(UVhi,UVlo);
            
            //pack the Y into a single short vector
            Y = vec_packs(Yhi,Ylo);
                                            
            
            //vec_adds +128 to U V U V short
            UV = vec_adds(UV,v128);
			Y = vec_adds(Y,v128);
            
            //vec_mergel + vec_mergeh Y and UV
            hiImage =  vec_mergeh(UV,Y);
            loImage =  vec_mergel(UV,Y);
            
            //pack back to 16 chars
            inData[0] = vec_packsu(hiImage, loImage);
            
          
            inData++;
		
		
	}

	
}
#endif //__VEC__

void pix_contrast :: processYUVImage(imageStruct &image)
{
  // no need to call the altivec code from here
  // it will be called automatically from the parent-class

	int datasize = (image.xsize/2) * image.ysize;
	unsigned char *pixels = image.data;
	unsigned short c,s;
	
	int y0, y1,u,v;
	
	c = static_cast<short>(256. * m_contrast);
	s = static_cast<short>(256. * m_saturation);
        if(256==s && 256==c)return;

	while(datasize--){
	
		u = pixels[chU] -128;
		pixels[chU] = CLAMP(((u * s) >> 8) + 128);
		
		y0 = (pixels[chY0] - 128);
		pixels[chY0] = CLAMP(((y0 * c) >> 8) + 128);
		
		v = pixels[chV] -128;
		pixels[chV] = CLAMP(((v * s) >> 8) + 128);
		
		y1 = pixels[chY1] - 128;
		pixels[chY1] = CLAMP(((y1 * c) >> 8) + 128);
		
		
		pixels+=4;
	}
}


void pix_contrast :: processRGBAImage(imageStruct &image)
{
int datasize = (image.xsize) * image.ysize;
	unsigned char *pixels = image.data;
	unsigned short c,s;
	
	int y,u,v;
	
	c = static_cast<short>(256. * m_contrast);
	s = static_cast<short>(256. * m_saturation);

        if(256==s && 256==c)return;

	while(datasize--){
	

		u = (((pixels[chRed] * -38) +  (pixels[chGreen] * -74 ) + (pixels[chBlue] * 112))>>8)-0;
		//u = CLAMP(((u * s) >> 8) + 0);
		u = (((u * s) >> 8) + 0);

		y = (((pixels[chRed] * 66) +  (pixels[chGreen] * 129 ) + (pixels[chBlue] * 25))>>8)-128;
		//y = CLAMP(((y * c) >> 8) + 128);
		y = (((y * c) >> 8) + 128);

		v = (((pixels[chRed] * 112) +  (pixels[chGreen] * -94 ) + (pixels[chBlue] * -18))>>8)-0;
		//v = CLAMP(((v * s) >> 8) + 0);
		v = (((v * s) >> 8) + 0);
		
		pixels[chRed] = CLAMP(((y * 298) + (u * 1) + (v * 409))>>8);
		pixels[chGreen] = CLAMP(((y * 298) + (u * -100) + (v * -208))>>8);
		pixels[chBlue] = CLAMP(((y * 298) + (u * 516) + (v * 0))>>8);
		
		pixels+=4; 
	}

}

void pix_contrast :: processGrayImage(imageStruct &image)
{

  int datasize = image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  // we scale by 256, since "(x*256)>>8=x"
  short c = static_cast<short>(256. * m_contrast);
  int g;

  if(256==c)return; // the effect would produce the same result...

  while(datasize--){
    g=(((*pixels - 128) * c) >> 8) + 128;
    *pixels++ = CLAMP(g);
  }
}

void pix_contrast :: contrastMess(float contrast)
{
	m_contrast = contrast;
	setPixModified();
}

void pix_contrast :: saturationMess(float saturation)
{
	m_saturation = saturation;
	setPixModified();
}

void pix_contrast :: obj_setupCallback(t_class *classPtr)
{

	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_contrast::contrastMessCallback), gensym("contrast"),A_FLOAT,A_NULL);
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_contrast::saturationMessCallback), gensym("saturation"),A_FLOAT,A_NULL);

}

void pix_contrast :: contrastMessCallback(void *data, t_floatarg contrast)
{
	GetMyClass(data)->contrastMess(contrast);
}

void pix_contrast :: saturationMessCallback(void *data, t_floatarg saturation)
{
	GetMyClass(data)->saturationMess(saturation);
}
