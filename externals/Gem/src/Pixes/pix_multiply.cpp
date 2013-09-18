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

#include "pix_multiply.h"

CPPEXTERN_NEW(pix_multiply);

/////////////////////////////////////////////////////////
//
// pix_multiply
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_multiply :: pix_multiply()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_multiply :: ~pix_multiply()
{ }

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_multiply :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  
  while(datasize--)    {
    leftPix[chRed] = INT_MULT(leftPix[chRed], rightPix[chRed]);
    leftPix[chGreen] = INT_MULT(leftPix[chGreen], rightPix[chGreen]);
    leftPix[chBlue] = INT_MULT(leftPix[chBlue], rightPix[chBlue]);
    leftPix += 4;
    rightPix += 4;
  }
}

/////////////////////////////////////////////////////////
// processRightGray
//
/////////////////////////////////////////////////////////
void pix_multiply :: processRGBA_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;

  while(datasize--)	{
    unsigned int alpha = rightPix[chGray];
    leftPix[chRed] = INT_MULT(leftPix[chRed], alpha);
    leftPix[chGreen] = INT_MULT(leftPix[chGreen], alpha);
    leftPix[chBlue] = INT_MULT(leftPix[chBlue], alpha);
    leftPix += 4;
    rightPix++;
  }
}

/////////////////////////////////////////////////////////
// processDualGray
//
/////////////////////////////////////////////////////////
void pix_multiply :: processGray_Gray(imageStruct &image, imageStruct &right)
{
    int datasize = image.xsize * image.ysize;
    unsigned char *leftPix = image.data;
    unsigned char *rightPix = right.data;

    while(datasize--)	{
      unsigned int alpha = rightPix[chGray];
      leftPix[chGray] = INT_MULT(leftPix[chGray], alpha);
      leftPix++;
      rightPix++;
    }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_multiply :: processYUV_YUV(imageStruct &image, imageStruct &right)
{
   long src,h,w;
   int	y1,y2;
   src =0;
   //format is U Y V Y
   for (h=0; h<image.ysize; h++){
    for(w=0; w<image.xsize/2; w++){
       y1 = (image.data[src+chY0] * right.data[src+chY0]) >> 8;
       image.data[src+chY0] = CLAMP_Y(y1);
       y2 = (image.data[src+chY1] * right.data[src+chY1]) >> 8;
       image.data[src+chY1] = CLAMP_Y(y2);
        
       src+=4;
    }
   }
}

#ifdef __MMX__
void pix_multiply :: processRGBA_MMX(imageStruct &image, imageStruct &right)
{
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 l0, r0, l1, r1;
  __m64 null64 = _mm_setzero_si64();
  while(datasize--)    {
    l1=leftPix [datasize];
    r1=rightPix[datasize];

    l0=_mm_unpacklo_pi8(l1, null64);
    r0=_mm_unpacklo_pi8(r1, null64);
    l1=_mm_unpackhi_pi8(l1, null64);
    r1=_mm_unpackhi_pi8(r1, null64);

    l0=_mm_mullo_pi16  (l0, r0);
    l1=_mm_mullo_pi16  (l1, r1);

    l0=_mm_srli_pi16(l0, 8);
    l1=_mm_srli_pi16(l1, 8);

    leftPix[datasize]=_mm_packs_pu16(l0, l1);
  }
  _mm_empty();
}
void pix_multiply :: processYUV_MMX(imageStruct &image, imageStruct &right)
{
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 l0, r0, l1, r1;
  __m64 mask= _mm_setr_pi8((unsigned char)0xFF,
			   (unsigned char)0x00,
			   (unsigned char)0xFF,
			   (unsigned char)0x00,
			   (unsigned char)0xFF,
			   (unsigned char)0x00,
			   (unsigned char)0xFF,
			   (unsigned char)0x00);
  __m64 yuvclamp0 = _mm_setr_pi8((unsigned char)0x00,
				 (unsigned char)0x10,
				 (unsigned char)0x00,
				 (unsigned char)0x10,
				 (unsigned char)0x00,
				 (unsigned char)0x10,
				 (unsigned char)0x00,
				 (unsigned char)0x10);
  __m64 yuvclamp1 = _mm_setr_pi8((unsigned char)0x00,
				 (unsigned char)0x24,
				 (unsigned char)0x00,
				 (unsigned char)0x24,
				 (unsigned char)0x00,
				 (unsigned char)0x24,
				 (unsigned char)0x00,
				 (unsigned char)0x24);
  __m64 yuvclamp2 = _mm_setr_pi8((unsigned char)0x00,
				 (unsigned char)0x14,
				 (unsigned char)0x00,
				 (unsigned char)0x14,
				 (unsigned char)0x00,
				 (unsigned char)0x14,
				 (unsigned char)0x00,
				 (unsigned char)0x14);

  __m64 null64 = _mm_setzero_si64();
  while(datasize--)    {
    r1=rightPix[datasize];
    l1=leftPix [datasize];

    r1=_mm_or_si64(r1, mask);

    l0=_mm_unpacklo_pi8(l1, null64);
    r0=_mm_unpacklo_pi8(r1, null64);
    l1=_mm_unpackhi_pi8(l1, null64);
    r1=_mm_unpackhi_pi8(r1, null64);

    l0=_mm_mullo_pi16  (l0, r0);
    l1=_mm_mullo_pi16  (l1, r1);

    l0=_mm_srli_pi16(l0, 8);
    l1=_mm_srli_pi16(l1, 8);

    l0=_mm_packs_pu16(l0, l1);

    l0=_mm_subs_pu8(l0, yuvclamp0);
    l0=_mm_adds_pu8(l0, yuvclamp1);
    l0=_mm_subs_pu8(l0, yuvclamp2);


    leftPix[datasize]=l0;
  }
  _mm_empty();
}

void pix_multiply :: processGray_MMX(imageStruct &image, imageStruct &right)
{
  processRGBA_MMX(image, right);
}
#endif

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_multiply :: obj_setupCallback(t_class *)
{ }
