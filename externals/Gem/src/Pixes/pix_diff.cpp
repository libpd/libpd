////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_diff.h"
#include <stdlib.h>

CPPEXTERN_NEW(pix_diff);

/////////////////////////////////////////////////////////
//
// pix_diff
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_diff :: pix_diff()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_diff :: ~pix_diff()
{ }

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_diff :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
    int datasize = image.xsize * image.ysize;
    unsigned char *leftPix = image.data;
    unsigned char *rightPix = right.data;

    while(datasize--)    {
      leftPix[chRed] =
	abs(leftPix[chRed] - (int)rightPix[chRed]);
      leftPix[chGreen] =
	abs(leftPix[chGreen] - (int)rightPix[chGreen]);
      leftPix[chBlue] =
	abs((int)leftPix[chBlue] - (int)rightPix[chBlue]);
      leftPix += 4;
      rightPix += 4;
    }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////

void pix_diff :: processYUV_YUV(imageStruct &image, imageStruct &right)
{
   long src,h,w;
   int	y1,y2;
   int u,v;
   src =0;
   //format is U Y V Y
   for (h=0; h<image.ysize; h++){
    for(w=0; w<image.xsize/2; w++){
       
        u = (image.data[src] - 128) - (right.data[src] - 128);
        image.data[src] = abs(u + 128);
        y1 =image.data[src+1] - right.data[src+1];
        image.data[src+1] = abs(y1);
        v = (image.data[src+2] - 128) - (right.data[src+2] - 128);
        image.data[src+2] = abs(v+128);
        y2 = image.data[src+3] - right.data[src+3];
        image.data[src+3] = abs(y2);
       
        src+=4;
        }
    }
}

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_diff :: processGray_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  
  while(datasize--)
    {
      leftPix[chGray] =
	abs(leftPix[chGray] - (int)rightPix[chGray]);
      leftPix++;
      rightPix++;
    }
}


#ifdef __MMX__
void pix_diff :: processRGBA_MMX(imageStruct &image, imageStruct &right){
  int datasize = image.xsize * image.ysize * image.csize;
  __m64*leftPix  = (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;
  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 l, r, b;

  while(datasize--){
    l =leftPix [datasize];
    r=rightPix[datasize];

    b  = l;
    b  = _mm_subs_pu8     (b, r);
    r  = _mm_subs_pu8     (r, l);
    b  = _mm_or_si64      (b, r);

    leftPix[datasize]=b;
  }
  _mm_empty();
}
void pix_diff :: processYUV_MMX (imageStruct &image, imageStruct &right){
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);
  __m64 mask = _mm_setr_pi8(0x40, 0x00, 0x40, 0x00,
			    0x40, 0x00, 0x40, 0x00);
  __m64 l, r, b;
  while (datasize--) {
    l=leftPix[datasize];
    r=rightPix[datasize];

    l=_mm_adds_pu8(l, mask);
    r=_mm_subs_pu8(r, mask);

    b  = l;
    b  = _mm_subs_pu8     (b, r);
    r  = _mm_subs_pu8     (r, l);
    b  = _mm_or_si64      (b, r);

    leftPix[datasize]=b;
  }
  _mm_empty();
}
void pix_diff :: processGray_MMX(imageStruct &image, imageStruct &right){
  processRGBA_MMX(image, right);
}

#endif

#ifdef __VEC__
void pix_diff :: processRGBA_Altivec(imageStruct &image, imageStruct &right)
{

    int datasize = image.xsize * image.ysize / 4;
    vector signed short  hiImage, loImage, hiRight, loRight;
    vector unsigned char zero = vec_splat_u8(0);
    vector unsigned char *inData = (vector unsigned char *)image.data;
    vector unsigned char *rightData = (vector unsigned char *)right.data;
    
    #ifndef PPC970
   	UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
	vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        vec_dst( inData+256, prefetchSize, 2 );
        vec_dst( rightData+256, prefetchSize, 3 );
    #endif  
    
    do {
        
        #ifndef PPC970
	vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        vec_dst( inData+256, prefetchSize, 2 );
        vec_dst( rightData+256, prefetchSize, 3 );
        #endif  
        
        hiImage = (vector signed short)vec_mergeh(zero,inData[0]);
        loImage = (vector signed short)vec_mergel(zero,inData[0]);
        hiRight = (vector signed short)vec_mergeh(zero,rightData[0]);
        loRight = (vector signed short)vec_mergel(zero,rightData[0]);
        
        hiImage = vec_subs(hiImage,hiRight);
        loImage = vec_subs(loImage,loRight);
        
        hiImage = vec_abs(hiImage);
        loImage = vec_abs(loImage);
        
        inData[0] = vec_packsu(hiImage,loImage);
        
        inData++;
        rightData++;
    }
    while (--datasize);
    #ifndef PPC970
        vec_dss( 0 );
        vec_dss( 1 );
        vec_dss( 2 );
        vec_dss( 3 );
    #endif
}
void pix_diff :: processYUV_Altivec(imageStruct &image, imageStruct &right)
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
    
    
    vector signed short d, hiImage, loImage,hiRight, loRight;//, YRight, UVRight, YImage, UVImage, UVTemp, YTemp;
    vector unsigned char zero = vec_splat_u8(0);
    vector unsigned char *inData = (vector unsigned char*) image.data;
    vector unsigned char *rightData = (vector unsigned char*) right.data;

     
    shortBuffer.elements[0] = 128;
    shortBuffer.elements[1] = 0;
    shortBuffer.elements[2] = 128;
    shortBuffer.elements[3] = 0;
    shortBuffer.elements[4] = 128;
    shortBuffer.elements[5] = 0;
    shortBuffer.elements[6] = 128;
    shortBuffer.elements[7] = 0;
   
    //Load it into the vector unit
    d = shortBuffer.v;
    
    
    
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
            //interleaved U Y V Y chars
            
            //break out to unsigned shorts
            hiImage = (vector signed short) vec_mergeh( zero, inData[0] );
            loImage = (vector signed short) vec_mergel( zero, inData[0] );
            hiRight = (vector signed short) vec_mergeh( zero, rightData[0] );
            loRight = (vector signed short) vec_mergel( zero, rightData[0] );
            
            //subtract the 128 offset for UV
            hiImage = vec_subs(hiImage,d);
            loImage = vec_subs(loImage,d);
            hiRight = vec_subs(hiRight,d);
            loRight = vec_subs(loRight,d);
            
            hiImage = vec_subs(hiImage,hiRight);
            loImage = vec_subs(loImage,loRight);
            
            hiImage = vec_adds(hiImage,d);
            loImage = vec_adds(loImage,d);
            
            hiImage = vec_abs(hiImage);
            loImage = vec_abs(loImage);            
            
            inData[0] = vec_packsu(hiImage, loImage);
        
            inData++;
            rightData++;
            
        }
        #ifndef PPC970
        vec_dss( 0 );
        vec_dss( 1 );
        #endif
    }  /*end of working altivec function */
}
#endif


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_diff :: obj_setupCallback(t_class *)
{ }
