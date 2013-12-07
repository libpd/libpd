/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    GemPixConvertSIMD.h
       - header-file for color conversion
       - this is mainly for (SIMD-)optimized routines
       - part of GEM

    Copyright (c) 2006-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/


#ifndef _INCLUDE__GEM_GEM_PIXCONVERT_H_
#define _INCLUDE__GEM_GEM_PIXCONVERT_H_

#include "Gem/Image.h"
#include "Utils/SIMD.h"

// use formulae from http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC30
/*
 * [Y]    1    [  65.738 129.075  25.064 ]   [R] [ 16]
 * [U] = --- * [ -37.945 -74.494 112.439 ] * [G]+[128]
 * [V] = 256   [ 112.439 -94.154 -18.285 ]   [B] [128]
 *
 * [R]    1    [ 298.082    0.0    408.583 ]    [Y] [ 16]
 * [G] = --- * [ 298.082 -100.291 -208.120 ] * ([U]-[128])
 * [B] = 256   [ 298.082  516.411    0.0   ]    [V] [128]
 */

#define YUV_POYNTON
// here comes something to be afraid of:
// (probably it would be better to define the matrices as real constant-value matrices)
// (instead of element-wise)

#ifdef YUV_POYNTON
# define Y_OFFSET   16
# define UV_OFFSET 128

// RGB2YUV
// poynton-values rounded
# define RGB2YUV_11  66
# define RGB2YUV_12 129
# define RGB2YUV_13  25
# define RGB2YUV_21 -38
# define RGB2YUV_22 -74
# define RGB2YUV_23 112
# define RGB2YUV_31 112
# define RGB2YUV_32 -94
# define RGB2YUV_33 -18

// YUV2RGB
// (we skip _21 and _31 as they are equal to _11)
#if 0
// poynton-values rounded
# define YUV2RGB_11 298
# define YUV2RGB_12   0
# define YUV2RGB_13 409 
# define YUV2RGB_22 -100
# define YUV2RGB_23 -208
# define YUV2RGB_32 516
# define YUV2RGB_33   0
#else

// this is round(256*inv(rgb2yuv/256))
// so the general error should be smaller
# define YUV2RGB_11  298
# define YUV2RGB_12   -1
# define YUV2RGB_13  409
# define YUV2RGB_22 -100
# define YUV2RGB_23 -210
# define YUV2RGB_32  519
# define YUV2RGB_33    0
#endif

#else
/* the old ones: */
# define Y_OFFSET   0
# define UV_OFFSET 128
// RGB2YUV
# define RGB2YUV_11 77
# define RGB2YUV_12 150
# define RGB2YUV_13 29
# define RGB2YUV_21 -43
# define RGB2YUV_22 -85
# define RGB2YUV_23 128
# define RGB2YUV_31 128
# define RGB2YUV_32 -107
# define RGB2YUV_33 -21
// YUV2RGB
# define YUV2RGB_11 256
# define YUV2RGB_12 0
# define YUV2RGB_13 359
# define YUV2RGB_22 -88
# define YUV2RGB_23 -183
# define YUV2RGB_32 454
# define YUV2RGB_33 0

#endif /* POYNTON */

#if 0
/* yuv-coefficients would also need an offset! */
# define RGB2GRAY_RED  RGB2YUV_11
# define RGB2GRAY_GREEN  RGB2YUV_12
# define RGB2GRAY_BLUE  RGB2YUV_13
# define RGB2GRAY_OFFSET Y_OFFSET
#else
# define RGB2GRAY_RED  77
# define RGB2GRAY_GREEN  150
# define RGB2GRAY_BLUE  29
# define RGB2GRAY_OFFSET 0
#endif

/* AltiVec */
#ifdef __VEC__

/* there are problems on OSX10.3 with older versions of gcc, since the intrinsic code
 * below freely changes between signed and unsigned short vectors
 * newer versions of gcc accept this...
 * LATER: fix the code (GemPixConvertAltivec:750..800)
 */
# ifdef __GNUC__
/* according to hcs it does NOT work with gcc-3.3
 * for simplicity, i disable everything below gcc4
 * JMZ: 20061114
 */
#  if __GNUC__ < 4
#   warning disabling AltiVec for older gcc: please fix me
#   define NO_VECTORINT_TO_VECTORUNSIGNEDINT
#  endif
# endif /* GNUC */


  void RGB_to_YCbCr_altivec(const unsigned char *rgbdata, size_t RGB_size, 
							unsigned char *pixels);
  void RGBA_to_YCbCr_altivec(const unsigned char *rgbadata, size_t RGBA_size, 
							 unsigned char *pixels);
  void BGR_to_YCbCr_altivec(const unsigned char *bgrdata, size_t BGR_size, 
							unsigned char *pixels);
  void BGRA_to_YCbCr_altivec(const unsigned char *bgradata, size_t BGRA_size, 
							 unsigned char *pixels);
  void YUV422_to_BGRA_altivec(const unsigned char *yuvdata, size_t pixelnum,
                              unsigned char *pixels);
  void YV12_to_YUV422_altivec(const short*Y, const short*U, const short*V,
                              unsigned char *data, int xsize, int ysize);
# ifndef NO_VECTORINT_TO_VECTORUNSIGNEDINT
  void YUV422_to_YV12_altivec(short*pY, short*pY2, short*pU, short*pV,
                              const unsigned char *gem_image, int xsize, int ysize);
# endif
#endif /* AltiVec */

/* SSE2 */
#ifdef __SSE2__
void RGBA_to_UYVY_SSE2(const unsigned char *rgbadata, 
                       size_t size, 
                       unsigned char *yuvdata);
void UYVY_to_RGBA_SSE2(const unsigned char *yuvdata, 
                       size_t size, 
                       unsigned char *rgbadata);
void UYVY_to_RGB_SSE2(const unsigned char *yuvdata, 
                      size_t size, 
                      unsigned char *rgbadata);
#endif /* SSE2 */

/* in case somebody has an old machine... */
#ifdef __MMX__

#endif /* MMX */

#endif /* _INCLUDE__GEM_GEM_PIXCONVERT_H_ */
