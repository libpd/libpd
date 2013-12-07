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

#include "pix_invert.h"

CPPEXTERN_NEW(pix_invert);

/////////////////////////////////////////////////////////
//
// pix_invert
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_invert :: pix_invert()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_invert :: ~pix_invert()
{ }
 
/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_invert :: processRGBAImage(imageStruct &image)
{
  int i = image.xsize * image.ysize;
    
  unsigned char *base = image.data;
  while (i) {
    i--;
    unsigned char alpha = base[chAlpha];
    *((unsigned long *)base) = ~*((unsigned long *)base);
    base[chAlpha] = alpha;
    base += 4;
  }
}

#ifdef __MMX__
void pix_invert :: processRGBAMMX(imageStruct &image)
{
  int i = (image.xsize * image.ysize) / 2; // 2 pixels at a time
  vector64i offset;
  vector64i *input = (vector64i*)image.data;

  offset.c[0+chRed]=255;
  offset.c[0+chGreen]=255;
  offset.c[0+chBlue]=255;
  offset.c[0+chAlpha]=0;

  offset.c[4+chRed]=255;
  offset.c[4+chGreen]=255;
  offset.c[4+chBlue]=255;
  offset.c[4+chAlpha]=0;

  while (i--) {
    //*((unsigned long *)base) = ~*((unsigned long *)base);
    input[0].v= _mm_xor_si64(input[0].v, offset.v);
    input++;
  }
  _mm_empty();
}
void pix_invert :: processGrayMMX(imageStruct &image)
{
  int i = (image.xsize * image.ysize) / 8; // 8 pixels at a time
  vector64i offset;
  vector64i *input = (vector64i*)image.data;

  offset.c[0]=255;
  offset.c[1]=255;
  offset.c[2]=255;
  offset.c[3]=255;
  offset.c[4]=255;
  offset.c[5]=255;
  offset.c[6]=255;
  offset.c[7]=255;

  while (i--) {
    //*((unsigned long *)base) = ~*((unsigned long *)base);
    input[0].v= _mm_xor_si64(input[0].v, offset.v);
    input++;
  }
  _mm_empty();
}
void pix_invert :: processYUVMMX(imageStruct &image)
{
  int i = (image.xsize * image.ysize) / 4; // 4 pixels at a time
  vector64i offset;
  vector64i *input = (vector64i*)image.data;

  // 1st 2 pixels
  offset.c[0]=255;
  offset.c[1]=255;
  offset.c[2]=255;
  offset.c[3]=255;
  // 2nd 2 pixels
  offset.c[4]=255;
  offset.c[5]=255;
  offset.c[6]=255;
  offset.c[7]=255;

  while (i--) {
    //*((unsigned long *)base) = ~*((unsigned long *)base);
    input[0].v= _mm_xor_si64(input[0].v, offset.v);
    input++;
  }
  _mm_empty();
}
#endif

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_invert :: processGrayImage(imageStruct &image)
{
  int i = image.xsize * image.ysize;
    
  unsigned char *base = image.data;
  while (i--) {
    base[chGray] = 255 - base[chGray];
    base++;
  }    
}

/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_invert :: processYUVImage(imageStruct &image)
{
  int h,w;
  long src;

  src = 0;

  //format is U Y V Y

  for (h=0; h<image.ysize; h++){
    for(w=0; w<image.xsize/2; w++){
      image.data[src] = 255 - image.data[src];
      image.data[src+1] = 255 - image.data[src+1];
      image.data[src+2] = 255 - image.data[src+2];
      image.data[src+3] = 255 - image.data[src+3];
      src+=4;
    }
  }
}

/////////////////////////////////////////////////////////
// processYUVAltivec  -- good stuff apply liberally
//
/////////////////////////////////////////////////////////
#ifdef __VEC__
void pix_invert :: processYUVAltivec(imageStruct &image)
{
int h,w,width;
   width = image.xsize/8;

    union{
        unsigned char c[16];
        vector unsigned char v;
    }charBuffer;

    vector unsigned char offset;
    vector unsigned char *inData = (vector unsigned char*) image.data;
    
    charBuffer.c[0] = 255;
    offset = charBuffer.v;
    offset = (vector unsigned char) vec_splat(offset,0);
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
        inData[0]=vec_subs(offset,inData[0]);
        inData++;

         }
         #ifndef PPC970
        vec_dss( 0 );
        #endif
    }  /*end of working altivec function */
}
#endif // ALTIVEC


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_invert :: obj_setupCallback(t_class *classPtr)
{}
