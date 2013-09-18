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

#include "pix_composite.h"

CPPEXTERN_NEW(pix_composite);

/////////////////////////////////////////////////////////
//
// pix_composite
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_composite :: pix_composite()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_composite :: ~pix_composite()
{}

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_composite :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned int alpha;

  // The src1, src2, dst is a little bit backwards.  This
  //	is because we want the image on the left inlet to be
  //	on top of the image on the right inlet.
  unsigned char *dst = image.data;
  unsigned char *src1 = right.data;
  unsigned char *src2 = image.data;
  
  while(datasize--)    {
    if ( (alpha = src2[chAlpha]) )      {
      if (alpha == 255)	{
	dst[chRed]   = src2[chRed];
	dst[chGreen] = src2[chGreen];
	dst[chBlue]  = src2[chBlue];
      }	else {
	dst[chRed]   = INT_LERP(src1[chRed], src2[chRed], alpha);
	dst[chGreen] = INT_LERP(src1[chGreen], src2[chGreen], alpha);
	dst[chBlue]  = INT_LERP(src1[chBlue], src2[chBlue], alpha);
      }
    } else {
      dst[chRed]	 = src1[chRed];
      dst[chGreen]	 = src1[chGreen];
      dst[chBlue]	 = src1[chBlue];
    }
    src1 += 4;
    src2 += 4;
    dst += 4;
  }
}
void pix_composite :: processRGBA_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  //  unsigned int alpha;

  // The src1, src2, dst is a little bit backwards.  This
  //	is because we want the image on the left inlet to be
  //	on top of the image on the right inlet.
  unsigned char *dst = image.data;
  unsigned char *src1 = right.data;
  unsigned char *src2 = image.data;
  
  while(datasize--)    {
    int rightPix = *src1++;
    if ( unsigned int alpha = src2[chAlpha] )      {
      if (alpha == 255)	{
	dst[chRed]   = src2[chRed];
	dst[chGreen] = src2[chGreen];
	dst[chBlue]  = src2[chBlue];
      }	else {
	dst[chRed]   = INT_LERP(src1[chRed],   rightPix, alpha);
	dst[chGreen] = INT_LERP(src1[chGreen], rightPix, alpha);
	dst[chBlue]  = INT_LERP(src1[chBlue],  rightPix, alpha);
      }
    } else {
      dst[chRed]	 = rightPix;
      dst[chGreen]	 = rightPix;
      dst[chBlue]	 = rightPix;
    }
    src2 += 4;
    dst += 4;
  }
}

#ifdef __MMX__
void pix_composite :: processRGBA_MMX(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize * image.csize;
  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  // The src1, src2, dst is a little bit backwards.  This
  //	is because we want the image on the left inlet to be
  //	on top of the image on the right inlet.
  __m64*dst  = (__m64*)image.data;
  __m64*src1 = (__m64*)right.data;
  __m64*src2 = (__m64*)image.data;

  const __m64 maskA= _mm_setr_pi8((unsigned char)0x00,
				  (unsigned char)0x00,
				  (unsigned char)0x00,
				  (unsigned char)0xFF,
				  (unsigned char)0x00,
				  (unsigned char)0x00,
				  (unsigned char)0x00,
				  (unsigned char)0xFF);
  const __m64 null64= _mm_setzero_si64();
  const __m64 one= _mm_set_pi16  ((short)0xFF,
				  (short)0xFF,
				  (short)0xFF,
				  (short)0xFF);

  __m64 r0, r1, l0, l1, a, b, a0;
  
  while(datasize--)    {
    r0=src1[datasize];
    l0=src2[datasize];

    a=_mm_and_si64 (l0, maskA);
    b=_mm_srli_pi32(a, 16);
    a=_mm_or_si64  (a, b);
    b=_mm_srli_pi16(a, 8);
    a=_mm_or_si64  (a, b);
    a=_mm_or_si64  (a, maskA);

    a0=_mm_xor_si64(a, one);

    l1=_mm_unpackhi_pi8(l0, null64);
    r1=_mm_unpackhi_pi8(r0, null64);
    l0=_mm_unpacklo_pi8(l0, null64);
    r0=_mm_unpacklo_pi8(r0, null64);
    b =_mm_unpackhi_pi8(a , null64);
    a =_mm_unpacklo_pi8(a , null64);

    l0=_mm_mullo_pi16  (l0, a);
    l1=_mm_mullo_pi16  (l1, b);

    a =_mm_subs_pu16    (one, a);
    b =_mm_subs_pu16    (one, b);
    
    r0=_mm_mullo_pi16  (r0, a);
    r1=_mm_mullo_pi16  (r1, b);

    l0=_mm_adds_pu16   (l0, r0);
    l1=_mm_adds_pu16   (l1, r1);

    l0=_mm_adds_pu16   (l0, one);
    l1=_mm_adds_pu16   (l1, one);

    l0 = _mm_srli_pi16(l0, 8);
    l1 = _mm_srli_pi16(l1, 8);

    dst[datasize]=_mm_packs_pu16(l0, l1);
  }
  _mm_empty();
}
#endif
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_composite :: obj_setupCallback(t_class *)
{ }
