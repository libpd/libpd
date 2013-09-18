////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_subtract.h"

CPPEXTERN_NEW(pix_subtract);

/////////////////////////////////////////////////////////
//
// pix_subtract
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_subtract :: pix_subtract()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_subtract :: ~pix_subtract()
{ }

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_subtract :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  register int datasize = (image.xsize * image.ysize)>>3;
  register unsigned char *leftPix = image.data;
  register unsigned char *rightPix = right.data;

    while (datasize--) {
      SUB8_NOALPHA(leftPix,rightPix);
      leftPix+=8;rightPix+=8;
      SUB8_NOALPHA(leftPix,rightPix);
      leftPix+=8;rightPix+=8;
      SUB8_NOALPHA(leftPix,rightPix);
      leftPix+=8;rightPix+=8;
      SUB8_NOALPHA(leftPix,rightPix);
      leftPix+=8;rightPix+=8;
    }
}

/////////////////////////////////////////////////////////
// processRightGray
//
/////////////////////////////////////////////////////////
void pix_subtract :: processRGBA_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  
  while(datasize--)    {
    register int alpha = *rightPix++;
    leftPix[chRed]   = CLAMP_LOW(static_cast<int>(leftPix[chRed])   - alpha);
    leftPix[chGreen] = CLAMP_LOW(static_cast<int>(leftPix[chGreen]) - alpha);
    leftPix[chBlue]  = CLAMP_LOW(static_cast<int>(leftPix[chBlue])  - alpha);
    leftPix += 4;
    }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////

void pix_subtract :: processYUV_YUV(imageStruct &image, imageStruct &right)
{
   long src,h,w;
   int	y1,y2;
   int u,v;
   src =0;
   //format is U Y V Y
   for (h=0; h<image.ysize; h++){
    for(w=0; w<image.xsize/2; w++){
        
        u = image.data[src] - ((2*right.data[src]) - 255);
        image.data[src] = CLAMP(u);

        y1 =image.data[src+1] - right.data[src+1];
        image.data[src+1] = CLAMP(y1);
        v = image.data[src+2] - ((2*right.data[src+2]) - 255);
        image.data[src+2] = CLAMP(v);

        y2 = image.data[src+3] - right.data[src+3];
        image.data[src+3] = CLAMP(y2);
       
        src+=4;
    }
   }
}

#ifdef __MMX__
void pix_subtract :: processRGBA_MMX(imageStruct &image, imageStruct &right){
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 l, r;
  while (datasize--) {
    l=leftPix[datasize];
    r=rightPix[datasize];

    leftPix[datasize]=_mm_subs_pu8(l,r);
  }
  _mm_empty();
}
void pix_subtract :: processYUV_MMX (imageStruct &image, imageStruct &right){
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);
  __m64 null64 = _mm_setzero_si64();
  __m64 offset = _mm_setr_pi16(0x80, 0x00, 0x80, 0x00);
  __m64 l0, l1, r0, r1;
  while (datasize--) {
    l1=leftPix[datasize];
    r1=rightPix[datasize];

    l0=_mm_unpacklo_pi8 (l1, null64);
    r0=_mm_unpacklo_pi8 (r1, null64);
    l1=_mm_unpackhi_pi8 (l1, null64);
    r1=_mm_unpackhi_pi8 (r1, null64);

    l0=_mm_adds_pu16(l0, offset);
    l1=_mm_adds_pu16(l1, offset);

    l0=_mm_subs_pu16(l0, r0);
    l1=_mm_subs_pu16(l1, r1);

    leftPix[datasize]=_mm_packs_pu16(l0, l1);
  }
  _mm_empty();
}
void pix_subtract :: processGray_MMX(imageStruct &image, imageStruct &right){
  processRGBA_MMX(image, right);
}
#endif

#ifdef __VEC__
void pix_subtract :: processRGBA_Altivec(imageStruct &image, imageStruct &right)
{
 int h,w,width;
   width = image.xsize/4;


    vector unsigned char *inData = (vector unsigned char*) image.data;
    vector unsigned char *rightData = (vector unsigned char*) right.data;
   
        #ifndef PPC970
   	UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
	vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        #endif
    for ( h=0; h<image.ysize; h++){
        for (w=0; w<width; w++)
        {
        #ifndef PPC970
	vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        #endif
            
            inData[0] = vec_subs(inData[0], rightData[0]);
        
            inData++;
            rightData++;
        }
        #ifndef PPC970
        vec_dss( 0 );
        vec_dss( 1 );
        #endif
    }  /*end of working altivec function */
}

void pix_subtract :: processYUV_Altivec(imageStruct &image, imageStruct &right)
{
  long h,w,width;

   width = image.xsize/8;
   //format is U Y V Y
    union
    {
        //unsigned int	i;
        short	elements[8];
        //vector signed char v;
        vector	short v;
    }shortBuffer;
    
        union
    {
        //unsigned int	i;
        unsigned char	elements[16];
        //vector signed char v;
        vector	unsigned char v;
    }charBuffer;
    
    //vector unsigned char c;
    vector signed short d, hiImage, loImage, YRight, UVRight, YImage, UVImage, UVTemp, YTemp;
  //  vector unsigned char zero = vec_splat_u8(0);
    vector unsigned char c,one;
   // vector signed short zshort = vec_splat_s16(0);
    vector unsigned char *inData = (vector unsigned char*) image.data;
    vector unsigned char *rightData = (vector unsigned char*) right.data;

    //Write the pixel (pair) to the transfer buffer
    charBuffer.elements[0] = 2;
    charBuffer.elements[1] = 1;
    charBuffer.elements[2] = 2;
    charBuffer.elements[3] = 1;
    charBuffer.elements[4] = 2;
    charBuffer.elements[5] = 1;
    charBuffer.elements[6] = 2;
    charBuffer.elements[7] = 1;
    charBuffer.elements[8] = 2;
    charBuffer.elements[9] = 1;
    charBuffer.elements[10] = 2;
    charBuffer.elements[11] = 1;
    charBuffer.elements[12] = 2;
    charBuffer.elements[13] = 1;
    charBuffer.elements[14] = 2;
    charBuffer.elements[15] = 1;

    //Load it into the vector unit
    c = charBuffer.v;
        
    one =  vec_splat_u8( 1 );
     
    shortBuffer.elements[0] = 255;
   
    //Load it into the vector unit
    d = shortBuffer.v;
    d = (vector signed short)vec_splat((vector signed short)d,0);
#ifndef PPC970
   	UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
	vec_dst( inData, prefetchSize, 0 );
    #endif    
    for ( h=0; h<image.ysize; h++){
        for (w=0; w<width; w++)
        {
        #ifndef PPC970
	vec_dst( inData, prefetchSize, 0 );
           #endif 
            //interleaved U Y V Y chars
            
            //vec_mule UV * 2 to short vector U V U V shorts
            UVImage = (vector signed short)vec_mule(one,inData[0]);
            UVRight = (vector signed short)vec_mule(c,rightData[0]);
            
            //vec_mulo Y * 1 to short vector Y Y Y Y shorts
            YImage = (vector signed short)vec_mulo(c,inData[0]);
            YRight = (vector signed short)vec_mulo(c,rightData[0]);
            
            //vel_subs UV - 255
            UVRight = (vector signed short)vec_subs(UVRight, d);
            
            //vec_adds UV
            UVTemp = vec_subs(UVImage,UVRight);
            
            //vec_adds Y
            YTemp = vec_subs(YImage,YRight);
            
            hiImage = vec_mergeh(UVTemp,YTemp);
            loImage = vec_mergel(UVTemp,YTemp);
            
            //vec_mergel + vec_mergeh Y and UV
            inData[0] = vec_packsu(hiImage, loImage);
        
            inData++;
            rightData++;
        }
        #ifndef PPC970
        vec_dss( 0 );
        #endif
    }  /*end of working altivec function */
}
#endif

void pix_subtract :: processDualImage(imageStruct &image, imageStruct &right){
  if (image.format!=right.format){
    error("pix_add: no method to combine (0x%X) and (0x%X)", image.format, right.format);
    return;
  }
  int datasize = (image.xsize * image.ysize * image.csize)>>5;
  int restsize = image.xsize * image.ysize * image.csize - datasize;
  register unsigned char *leftPix  = image.data;
  register unsigned char *rightPix = right.data;

  while (datasize--) {
    SUB8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    SUB8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    SUB8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    SUB8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
  }
  while(restsize--){
    *leftPix = CLAMP_LOW(static_cast<int>(*leftPix - *rightPix));
    leftPix++; rightPix++;
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_subtract :: obj_setupCallback(t_class *)
{ }
