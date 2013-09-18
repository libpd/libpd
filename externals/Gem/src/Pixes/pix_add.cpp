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

#include "pix_add.h"

CPPEXTERN_NEW(pix_add);

/////////////////////////////////////////////////////////
//
// pix_add
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_add :: pix_add()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_add :: ~pix_add()
{ }

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_add :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  register int datasize = (image.xsize * image.ysize)>>3;
  register unsigned char *leftPix = image.data;
  register unsigned char *rightPix = right.data;

  while (datasize--) {
    ADD8_NOALPHA(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    ADD8_NOALPHA(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    ADD8_NOALPHA(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    ADD8_NOALPHA(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
  }
}

/////////////////////////////////////////////////////////
// processRightGray
//
/////////////////////////////////////////////////////////
void pix_add :: processRGBA_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  
  while(datasize--)    {
    register int alpha = *rightPix++;
    leftPix[chRed]   = CLAMP_HIGH(leftPix[chRed]   + alpha);
    leftPix[chGreen] = CLAMP_HIGH(leftPix[chGreen] + alpha);
    leftPix[chBlue]  = CLAMP_HIGH(leftPix[chBlue]  + alpha);
    leftPix += 4;
    }
}
 
/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_add :: processYUV_YUV(imageStruct &image, imageStruct &right)
{
   long src,h,w;
   int	y0,y1;
   int u,v;
   src =0;
   //format is U Y V Y

   for (h=0; h<image.ysize; h++){
     for(w=0; w<image.xsize/2; w++){   
        u = image.data [src+chU ] + (2*right.data[src+chU ]) - 255;
        image.data[src] = CLAMP(u);
        y0 =image.data [src+chY0] + right.data   [src+chY0];
        image.data[src+1] = CLAMP(y0);
        v = image.data [src+chV ] + (2*right.data[src+chV ]) - 255;
        image.data[src+2] = CLAMP(v);
        y1 = image.data[src+chY1] + right.data   [src+chY1];
        image.data[src+3] = CLAMP(y1);
       
        src+=4;
     }
   }
}

#ifdef __MMX__
void pix_add :: processRGBA_MMX(imageStruct &image, imageStruct &right){
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  reinterpret_cast<__m64*>(image.data);
  __m64*rightPix = reinterpret_cast<__m64*>(right.data);

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 l, r;
  while (datasize--) {
    l=leftPix[datasize];
    r=rightPix[datasize];
    leftPix[datasize]=_mm_adds_pu8(l,r);
  }
  _mm_empty();
}
void pix_add :: processYUV_MMX (imageStruct &image, imageStruct &right){
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  reinterpret_cast<__m64*>(image.data);
  __m64*rightPix = reinterpret_cast<__m64*>(right.data);

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);
  __m64 mask = _mm_setr_pi8(0x40, 0x00, 0x40, 0x00,
			    0x40, 0x00, 0x40, 0x00);
  __m64 l, r;
  while (datasize--) {
    l=leftPix[datasize];
    r=rightPix[datasize];
    l=_mm_subs_pu8(l, mask);
    r=_mm_subs_pu8(r, mask);

    l=_mm_adds_pu8(l,r);

    leftPix[datasize]=l;
  }
  _mm_empty();
}
void pix_add :: processGray_MMX(imageStruct &image, imageStruct &right){
  processRGBA_MMX(image, right);
}
#endif

#ifdef __VEC__
void pix_add :: processYUV_Altivec(imageStruct &image, imageStruct &right)
{
 int h,w,width;
   width = image.xsize/8;
   //format is U Y V Y
    union
    {
        //unsigned int	i;
        short	elements[8];
        //vector signed char v;
        vector	signed short v;
    }shortBuffer;
    
        union
    {
        //unsigned int	i;
        unsigned char	elements[16];
        //vector signed char v;
        vector	unsigned char v;
    }charBuffer;
     
    //vector unsigned char c;
    register vector signed short d, hiImage, loImage, YRight, UVRight, YImage, UVImage, UVTemp, YTemp;
   // vector unsigned char zero = vec_splat_u8(0);
    register vector unsigned char c,one;
  //  vector signed short zshort = vec_splat_s16(0);
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
    d = static_cast<vector signed short>(vec_splat(static_cast<vector signed short>(d),0));
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
            
	  //vec_mule UV * 2 to short vector U V U V shorts
	  UVImage = static_cast<vector signed short>(vec_mule(one,inData[0]));
	  UVRight = static_cast<vector signed short>(vec_mule(c,rightData[0]));
            
	  //vec_mulo Y * 1 to short vector Y Y Y Y shorts
	  YImage = static_cast<vector signed short>(vec_mulo(c,inData[0]));
	  YRight = static_cast<vector signed short>(vec_mulo(c,rightData[0]));

	  //vel_subs UV - 255
	  UVRight = static_cast<vector signed short>(vec_subs(UVRight, d));
            
	  //vec_adds UV
	  UVTemp = vec_adds(UVImage,UVRight);
            
	  //vec_adds Y
	  YTemp = vec_adds(YImage,YRight);
            
	  hiImage = vec_mergeh(UVTemp,YTemp);
	  loImage = vec_mergel(UVTemp,YTemp);
            
	  //vec_mergel + vec_mergeh Y and UV
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
void pix_add :: processRGBA_Altivec(imageStruct &image, imageStruct &right)
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
            
            inData[0] = vec_adds(inData[0], rightData[0]);
        
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

void pix_add :: processDualImage(imageStruct &image, imageStruct &right){
  if (image.format!=right.format){
    error("no method to combine (0x%X) and (0x%X)", image.format, right.format);
    return;
  }
  int datasize = (image.xsize * image.ysize * image.csize)>>5;
  int restsize = image.xsize * image.ysize * image.csize - datasize;
  register unsigned char *leftPix  = image.data;
  register unsigned char *rightPix = right.data;

  while (datasize--) {
    ADD8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    ADD8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    ADD8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
    ADD8(leftPix,rightPix);
    leftPix+=8;rightPix+=8;
  }
  while(restsize--){
    *leftPix = CLAMP_HIGH(static_cast<int>(*leftPix + *rightPix));
    leftPix++; rightPix++;
  }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_add :: obj_setupCallback(t_class *)
{ }
