
/*
 *   Pure Data Packet. Header file for image processing routines (used in modules).
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* this is a c wrapper around platform specific (mmx) code */

#include "pdp_types.h"

#ifndef PDP_IMAGEPROC_H
#define PDP_IMAGEPROC_H


#ifdef __cplusplus
extern "C"
{
#endif

/* the basic allocation unit, for stack alignment */
typedef long long t_pdp_imageproc_stackword;

/* convert byte size to nb of stack words */
#define PDP_IMAGEPROC_NB_STACKWORDS(x) (((x-1)/sizeof(t_pdp_imageproc_stackword))+1)


/* the packet types should be the same for the dispatchers. packet0 is the dominant packet */

/* image processing dispatchers */
void pdp_imageproc_dispatch_1buf(void (*process_routine)(void*, u32, u32, s16*), void *x, u32 chanmask, int packet0);
void pdp_imageproc_dispatch_2buf(void (*process_routine)(void*, u32, u32, s16*, s16 *), void *x, u32 chanmask, int packet0, int packet1);
void pdp_imageproc_dispatch_3buf(void (*process_routine)(void*, u32, u32, s16*, s16 *, s16*), void *x, u32 chanmask, int packet0, int packet1, int packet2);




/* get legal image dimensions */
/* this is a fix for the dimension problem */
/* some imageproc implementations require the dims to be a multiple of some square */
u32 pdp_imageproc_legalwidth(int i);
u32 pdp_imageproc_legalheight(int i);
u32 pdp_imageproc_legalwidth_round_down(int i);
u32 pdp_imageproc_legalheight_round_down(int i);



/****************************** 16 bit signed (pixel) routines ***************************************/


// mix 2 images
void *pdp_imageproc_mix_new(void);
int  pdp_imageproc_mix_nb_stackwords(void);
void pdp_imageproc_mix_delete(void *x);
void pdp_imageproc_mix_setleftgain(void *x, float gain);
void pdp_imageproc_mix_setrightgain(void *x, float gain);
void pdp_imageproc_mix_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);

// random mix 2 images
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
// threshold = 0 -> left image
// threshold = 1 -> right image

void *pdp_imageproc_randmix_new(void);
int  pdp_imageproc_randmix_nb_stackwords(void);
void pdp_imageproc_randmix_delete(void *x);
void pdp_imageproc_randmix_setthreshold(void *x, float threshold);
void pdp_imageproc_randmix_setseed(void *x, float seed);
void pdp_imageproc_randmix_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);


// produce a random image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
void *pdp_imageproc_random_new(void);
void pdp_imageproc_random_delete(void *x);
void pdp_imageproc_random_setseed(void *x, float seed);
void pdp_imageproc_random_process(void *x, u32 width, u32 height, s16 *image);


// produce a plasma image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
void *pdp_imageproc_plasma_new(void);
void pdp_imageproc_plasma_delete(void *x);
void pdp_imageproc_plasma_setseed(void *x, float seed);
void pdp_imageproc_plasma_setturbulence(void *x, float seed);
void pdp_imageproc_plasma_process(void *x, u32 width, u32 height, s16 *image);


// apply a gain to an image
void *pdp_imageproc_gain_new(void);
void pdp_imageproc_gain_delete(void *x);
void pdp_imageproc_gain_setgain(void *x, float gain);
void pdp_imageproc_gain_process(void *x, u32 width, u32 height, s16 *image);



// add two images
void pdp_imageproc_add_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);

// mul two images
void pdp_imageproc_mul_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);


// 3x1 or 1x3 in place convolution
// orientation
#define PDP_IMAGEPROC_CONV_HORIZONTAL 0
#define PDP_IMAGEPROC_CONV_VERTICAL   1
void *pdp_imageproc_conv_new(void);
void pdp_imageproc_conv_delete(void *x);
void pdp_imageproc_conv_setmin1(void *x, float val);
void pdp_imageproc_conv_setzero(void *x, float val);
void pdp_imageproc_conv_setplus1(void *x, float val);
void pdp_imageproc_conv_setbordercolor(void *x, float intensity);
void pdp_imageproc_conv_setorientation(void *x, u32 val);
void pdp_imageproc_conv_setnbpasses(void *x, u32 val);
void pdp_imageproc_conv_process(void *x, u32 width, u32 height, s16 *image);


// colour rotation for 2 colour planes ($$$TODO: change interface)
// matrix is column encoded
void *pdp_imageproc_crot2d_new(void);
void pdp_imageproc_crot2d_delete(void *x);
void pdp_imageproc_crot2d_setmatrix(void *x, float *matrix);
void pdp_imageproc_crot2d_process(void *x, s16 *image, u32 width, u32 height);

// colour rotation for 3 colour planes ($$$TODO: change interface)
void *pdp_imageproc_crot3d_new(void);
void pdp_imageproc_crot3d_delete(void *x);
void pdp_imageproc_crot3d_setmatrix(void *x, float *matrix);
void pdp_imageproc_crot3d_process(void *x, s16 *image, u32 width, u32 height);




// biquad space

// directions
#define PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM (1<<0)
#define PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP (1<<1)
#define PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT (1<<2)
#define PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT (1<<3)
void *pdp_imageproc_bq_new(void);
void pdp_imageproc_bq_delete(void *x);
void pdp_imageproc_bq_setcoef(void *x, float *coef); // a0,a1,a2,b0,b1,b2
void pdp_imageproc_bq_setnbpasses(void *x, u32 nbpasses);
void pdp_imageproc_bq_setdirection(void *x, u32 direction);
void pdp_imageproc_bq_process(void *x, u32 width, u32 height, s16* image);


// biquad time
//void *pdp_imageproc_bqt_new(void);
//void pdp_imageproc_bqt_delete(void *x);
//void pdp_imageproc_bqt_setcoef(void *x, float *coef); // a0,a1,a2,b0,b1,b2
void pdp_imageproc_bqt_process(void *x, u32 width, u32 height, s16 *image, s16 *state0, s16 *state1);



// zoom object
void *pdp_imageproc_resample_affinemap_new(void);
void pdp_imageproc_resample_affinemap_delete(void *x);
void pdp_imageproc_resample_affinemap_setcenterx(void *x, float f);
void pdp_imageproc_resample_affinemap_setcentery(void *x, float f);
void pdp_imageproc_resample_affinemap_setzoomx(void *x, float f);
void pdp_imageproc_resample_affinemap_setzoomy(void *x, float f);
void pdp_imageproc_resample_affinemap_setangle(void *x, float f);
void pdp_imageproc_resample_affinemap_process(void *x, u32 width, u32 height, s16 *srcimage, s16 *dstimage);



//chebyshev poly
void *pdp_imageproc_cheby_new(int order);
void pdp_imageproc_cheby_delete(void *x);
void pdp_imageproc_cheby_setcoef(void *x, u32 n, float f);
void pdp_imageproc_cheby_setnbpasses(void *x, u32 n);
void pdp_imageproc_cheby_process(void *x, u32 width, u32 height, s16 *image);


//logic ops
void pdp_imageproc_xor_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);
void pdp_imageproc_or_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);
void pdp_imageproc_and_process(void *x, u32 width, u32 height, s16 *image, s16 *image2);
void pdp_imageproc_mask_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_not_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_hardthresh_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_softthresh_process(void *x, u32 width, u32 height, s16 *image);


//other stateles operators
void pdp_imageproc_abs_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_zthresh_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_plasma_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_ispositive_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_sign_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_flip_lr_process(void *dummy, u32 width, u32 height, s16 *image);
void pdp_imageproc_flip_tb_process(void *dummy, u32 width, u32 height, s16 *image);

//set to zero
void pdp_imageproc_zero_process(void *x, u32 width, u32 height, s16 *image);
void pdp_imageproc_constant_process(void *x, u32 width, u32 height, s16 *image);



#ifdef __cplusplus
}
#endif



#endif //PDP_IMAGEPROC_H
