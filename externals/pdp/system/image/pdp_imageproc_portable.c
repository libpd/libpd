/*
 *   Pure Data Packet. portable image processing routines.
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



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "pdp_imageproc.h"

/* pdp memory alloc/dealloc prototype */
void *pdp_alloc(int size);
void pdp_dealloc(void *);


// utility stuff
inline static s32 float2fixed(float f)
{
    if (f > 1) f = 1;
    if (f < -1) f = -1;
    f *= 0x7fff;
    return (s32)f;
}



#define CLAMP16(x) (((x) > 0x7fff) ? 0x7fff : (((x) < -0x7fff) ? -0x7fff : (x)))

// add two images
void pdp_imageproc_add_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    int a, b;
    unsigned int i;
    for (i=0; i<width*height; i++){
	a = (int)image[i];
	b = (int)image2[i];
	image[i] = (s16)(CLAMP16(a+b));
    }
    
}

// mul two images
void pdp_imageproc_mul_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    int a, b;
    unsigned int i;
    for (i=0; i<width*height; i++){
	a = (int)image[i];
	b = (int)image2[i];
	image[i] = (s16)((a*b)>>15);
    }
    
}

// mix 2 images
void *pdp_imageproc_mix_new(void){return pdp_alloc(2*sizeof(s32));}
int  pdp_imageproc_mix_nb_stackwords(void) {return PDP_IMAGEPROC_NB_STACKWORDS(2*sizeof(s32));}
void pdp_imageproc_mix_delete(void *x) {pdp_dealloc (x);}
void pdp_imageproc_mix_setleftgain(void *x, float gain)
{
    s32 *d = (s32 *)x;
    d[0] = float2fixed(gain);
}
void pdp_imageproc_mix_setrightgain(void *x, float gain)
{
    s32 *d = (s32 *)x;
    d[1] = float2fixed(gain);
}
void pdp_imageproc_mix_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    s32 *d = (s32 *)x;
    u32 i;
    s32 a,b;


    if (*d == 0)
      for(i=0; i<width*height; i++){
	image[i] = image2[i];
    }
    else
      for(i=0; i<width*height; i++){
	a = (s32)image[i];
	b = (s32)image2[i];
	a = (a*d[0] + b*d[1]) >> 15;
	image[i] = (s16)CLAMP16(a);
      }
}


// random mix 2 images
void *pdp_imageproc_randmix_new(void){return pdp_alloc(2*sizeof(s32));;}
int  pdp_imageproc_randmix_nb_stackwords(void) {return PDP_IMAGEPROC_NB_STACKWORDS(2*sizeof(s32));}
void pdp_imageproc_randmix_delete(void *x) {pdp_dealloc(x);}
void pdp_imageproc_randmix_setthreshold(void *x, float threshold)
{
    s32 *d = (s32 *)x;
    if (threshold > 1.0f) threshold = 1.0f;
    if (threshold < 0.0f) threshold = 0.0f;
    d[0] = float2fixed(threshold);
}
void pdp_imageproc_randmix_setseed(void *x, float seed)
{
    s32 *d = (s32 *)x;
    d[1] = float2fixed(seed);
}
void pdp_imageproc_randmix_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    s32 *d = (s32 *)x;
    u32 i;
    s16 r;
    srandom((u32)d[1]);


    for(i=0; i<width*height; i++){
	// get a random val between 0 and 0x7fff
	r = (s16)(random() & 0x7fff);
	if (r < d[0]) image[i] = image2[i];
    }
}


// 3x1 or 1x3 in place convolution
// orientation
void *pdp_imageproc_conv_new(void){return(pdp_alloc(6*sizeof(s32)));}
void pdp_imageproc_conv_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_conv_setmin1(void *x, float val)
{
    s32 *d = (s32 *)x;
    d[0] = float2fixed(val);
}
void pdp_imageproc_conv_setzero(void *x, float val)
{
    s32 *d = (s32 *)x;
    d[1] = float2fixed(val);
}
void pdp_imageproc_conv_setplus1(void *x, float val)
{
    s32 *d = (s32 *)x;
    d[2] = float2fixed(val);
}
void pdp_imageproc_conv_setbordercolor(void *x, float val)
{
    s32 *d = (s32 *)x;
    d[3] = float2fixed(val);
}
void pdp_imageproc_conv_setorientation(void *x, u32 val){((u32 *)x)[4] = val;}
void pdp_imageproc_conv_setnbpasses(void *x, u32 val){((u32 *)x)[5] = val;}

static inline void pdp_imageproc_conv_scanline(void *x, s16 *data, u32 count, s32 stride)
{
    s32 *d = (s32 *)x;
    s32 a,b,c,r;
    u32 i;

    a = d[3]; //border
    b = data[0];
    c = data[stride];

    for(i = 0; i < count-2; i++){
	r = a*d[0] + b*d[1] + c*d[2];
	a = data[0];
	b = data[stride];
	c = data[stride<<1];
	data[0] = (s16)CLAMP16(r>>15);
	data += stride;
    }
    r = a*d[0] + b*d[1] + c*d[2];
    a = data[0];
    b = data[stride];
    c = d[3]; //border
    data[0] = (s16)CLAMP16(r>>15);
    r = a*d[0] + b*d[1] + c*d[2];
    data[stride] = (s16)CLAMP16(r>>15);

}

void pdp_imageproc_conv_process(void *x, u32 width, u32 height, s16 *image)
{
    s32 *d = (s32 *)x;
    u32 i, j;
    u32 orientation = d[4];
    u32 nbp = d[5];
    if (orientation == PDP_IMAGEPROC_CONV_HORIZONTAL){
	for(i=0; i<width*height; i+=width)
	    for(j=0; j<nbp; j++)
		pdp_imageproc_conv_scanline(x, image+i, width, 1);

    }

    if (orientation == PDP_IMAGEPROC_CONV_VERTICAL){
	for(i=0; i<width; i++)
	    for(j=0; j<nbp; j++)
		pdp_imageproc_conv_scanline(x, image+i, height, width);

    }



	
}

// apply a gain to an image
void *pdp_imageproc_gain_new(void){return(pdp_alloc(2*sizeof(s32)));}
void pdp_imageproc_gain_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_gain_setgain(void *x, float gain)
{
    /* convert float to s16 + shift */
    s32 *d = (s32 *)x;
    s32 g;
    int i;
    float sign;
    s32 shift = 0;
    
    sign = (gain < 0) ? -1 : 1;
    gain *= sign;

    /* max shift = 16 */
    for(i=0; i<=16; i++){
	if (gain < 0x4000){
	    gain *= 2;
	    shift++;
	}
	else break;
    }

    gain *= sign;
    g = (s32) gain;

    //g = 0x4000;
    //shift = 14;

    d[0]=g;
    d[1]=shift;
}
void pdp_imageproc_gain_process(void *x, u32 width, u32 height, s16 *image)
{
    s32 *d = (s32 *)x;
    s32 a;
    u32 i;
    for (i=0; i<width*height; i++){
	a = (s32)image[i];
	image[i] = (s16)(CLAMP16((a * d[0]) >> d[1]));
    }
}

// colour rotation for 2 colour planes
void *pdp_imageproc_crot2d_new(void){return pdp_alloc(4*sizeof(s32));}
void pdp_imageproc_crot2d_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_crot2d_setmatrix(void *x, float *matrix)
{
    s32 *d = (s32 *)x;
    d[0] = float2fixed(matrix[0]);
    d[1] = float2fixed(matrix[1]);
    d[2] = float2fixed(matrix[2]);
    d[3] = float2fixed(matrix[3]);

}
void pdp_imageproc_crot2d_process(void *x, s16 *image, u32 width, u32 height)
{
    s32 *d = (s32 *)x;
    u32 i,j;
    s32 a1,a2,c1,c2;

    for(i=0, j=width*height; i<width*height; i++, j++){
	c1 = (s32)image[i];
	c2 = (s32)image[j];
	
	a1 = d[0] * c1;
	a2 = d[1] * c1;
	a1+= d[2] * c2;
	a2+= d[3] * c2;

	a1 >>= 15;
	a2 >>= 15;

	image[i] = (s16)CLAMP16(a1);
	image[j] = (s16)CLAMP16(a2);
    }
}

// biquad and biquad time
typedef struct
{
    s32 ma1;
    s32 ma2;
    s32 b0;
    s32 b1;
    s32 b2;

    s32 u0;
    s32 u1;

    s32 u0_save;
    s32 u1_save;

    u32 nbpasses;
    u32 direction;
} t_bq;
void *pdp_imageproc_bq_new(void){return pdp_alloc(sizeof(t_bq));}
void pdp_imageproc_bq_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_bq_setnbpasses(void *x, u32 i){((t_bq *)x)->nbpasses = i;}
void pdp_imageproc_bq_setdirection(void *x, u32 i){((t_bq *)x)->direction = i;}
void pdp_imageproc_bq_setcoef(void *x, float *coef) // a0,-a1,-a2,b0,b1,b2,u0,u1
{
    s32 *d = (s32 *)x;
    float ia0 = 1.0f / coef[0];

    /* all coefs are s1.14 fixed point */
    /* representing values -2 < x < 2  */
    /* so scale down before using the ordinary s0.15 float->fixed routine */

    ia0 *= 0.5f;

    // coef
    d[0] = float2fixed(ia0*coef[1]); // -a1
    d[1] = float2fixed(ia0*coef[2]); // -a2
    d[2] = float2fixed(ia0*coef[3]); // b0
    d[3] = float2fixed(ia0*coef[4]); // b1
    d[4] = float2fixed(ia0*coef[5]); // b2


    // state to reset too
    d[5] = float2fixed(coef[6]);
    d[6] = float2fixed(coef[7]);

}

#define A1 d[0]
#define A2 d[1]
#define B0 d[2]
#define B1 d[3]
#define B2 d[4]
/*
 	# DIRECT FORM II BIQUAD (from pixel_biquad_s16.s)
 	#
 	# y[k]  = b0 * x[k] + u1[k-1]
 	# u1[k] = b1 * x[k] + u2[k-1] - a1 * y[k]
	# u2[k] = b2 * x[k]           - a2 * y[k]
*/

/* remark A1 and A2 are already negated) */


static inline void pdp_imageproc_bq_scanline(void *x, s16 *data, u32 count, s32 stride)
{

    s32 *d = (s32 *)x;
    s32 u1,u2, xx, yy;

    u32 i;

    u1 = d[7];
    u2 = d[8];

    for(i = 0; i < count; i++){

	xx = (s32)data[0];

	yy = ((B0 * xx)>>14) + u1;
	u1 = ((B1 * xx)>>14) + u2 + ((A1 * yy)>>14);
	u2 = ((B2 * xx)>>14)      + ((A2 * yy)>>14);

	data[0] = (s16)CLAMP16(yy);

	data += stride;

    }

    d[7] = u1;
    d[8] = u2;

}

void pdp_imageproc_bqt_process(void *x, u32 width, u32 height, s16 *image, s16 *state1, s16 *state2)
{
    s32 *d = (s32 *)x;
    u32 i;
    s32 u1, u2, xx, yy;

    for (i=0; i<width*height; i++){

	xx = (s32)image[i];
	u1 = (s32)state1[i];
	u2 = (s32)state2[i];

	yy = ((B0 * xx)>>14) + u1;
	u1 = ((B1 * xx)>>14) + u2 + ((A1 * yy)>>14);
	u2 = ((B2 * xx)>>14)      + ((A2 * yy)>>14);

	image[i] = (s16)CLAMP16(yy);
	state1[i] = (s16)CLAMP16(u1);
	state2[i] = (s16)CLAMP16(u2);
    }
	
	
}

void pdp_imageproc_bq_process(void *x, u32 width, u32 height, s16 *data)
{
    s32 *d = (s32 *)x;
    
    u32 nbp = d[9];
    u32 direction = d[10];
    unsigned int i,j, offset;


    /* VERTICAL */
    offset = (height-1)*width;

    if ((direction & PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM)
	&& (direction &  PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP)){

	for(i=0; i<width; i++){
	    for (j=0; j<nbp; j++){
		pdp_imageproc_bq_scanline(x, data+i, height, width); //T->B
		pdp_imageproc_bq_scanline(x, data+offset+i, height, -width); //B->T
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM){
	for(i=0; i<width; i++){
	    for (j=0; j<nbp; j++){
		pdp_imageproc_bq_scanline(x, data+i, height, width); //T->B
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP){
	for(i=0; i<width; i++){
	    for (j=0; j<nbp; j++){
		pdp_imageproc_bq_scanline(x, data+offset+i, height, -width); //B->T
	    }
	}
    }

    /* HORIZONTAL */

    offset = width-1;
    if ((direction & PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT)
	&& (direction & PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT)){

	for(i=0; i<(width*height); i += width){
	    for (j=0; j<nbp; j++){
		pdp_imageproc_bq_scanline(x, data+i, width, 1); //L->R
		pdp_imageproc_bq_scanline(x, data+offset+i, width, -1); //R->L
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT){
	for(i=0; i<(width*height); i += width){
	    for (j=0; j<nbp; j++){
		pdp_imageproc_bq_scanline(x, data+i, width, 1); //L->R
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT){
	for(i=0; i<(width*height); i += width){
	    for (j=0; j<nbp; j++){
		pdp_imageproc_bq_scanline(x, data+offset+i, width, -1); //R->L

	    }
	}
    }

}

// produce a random image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
void *pdp_imageproc_random_new(void){return pdp_alloc(sizeof(s32));}
void pdp_imageproc_random_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_random_setseed(void *x, float seed)
{
    float *f = (float *)x;
    u32 *d = (u32 *)x;
    f[0] = seed;
    srandom(d[0]);
}

void pdp_imageproc_random_process(void *x, u32 width, u32 height, short int *image)
{
    u32 *d = (u32 *)x;
    u32 i;
    s32 r;
    srandom(d[0]);
    for (i=0; i<(width*height); i++) {
	r = random();
	image[i] = r;
    }
    d[0] = random();
}



/* resampling code */
// zoom + rotate

/* bilinear resampling core routine */
/* virtual coordinates are the lowest 16 bits in virt_x and virt_y*/
static inline s32 pdp_resample_bilin(s16 *image, s32 width, s32 height, s32 virt_x, s32 virt_y)
{

    s32 fp_x, fp_y, frac_x, frac_y, f, offset, r_1, r_2;

    //virt_x &= 0xffff;
    //virt_y &= 0xffff;

    fp_x = virt_x * (width - 1);
    fp_y = virt_y * (height - 1);

    frac_x = fp_x & (0xffff);
    frac_y = fp_y & (0xffff);

    offset = (fp_x >> 16) + (fp_y >> 16) * width;
    image += offset;

    f = 0x10000 - frac_x;

    r_1 = ((f * (s32)(image[0])  +  frac_x * (s32)(image[1])))>>16;

    image += width;

    r_2 = ((f * (s32)(image[0])  +  frac_x * (s32)(image[1])))>>16;

    f = 0x10000 - frac_y;

    return ((f * r_1 + frac_y * r_2)>>16);
    
}

typedef struct
{
    float centerx;
    float centery;
    float zoomx;
    float zoomy;
    float angle;
} t_affine_map;


void *pdp_imageproc_resample_affinemap_new(void)
{

    t_affine_map *a  = (t_affine_map *)pdp_alloc(sizeof(t_affine_map));
    a->centerx = 0.5;
    a->centery = 0.5;
    a->zoomx = 1.0;
    a->zoomy = 1.0;
    a->angle = 0.0f;
    return (void *)a;
}

void pdp_imageproc_resample_affinemap_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_resample_affinemap_setcenterx(void *x, float f){((t_affine_map *)x)->centerx = f;}
void pdp_imageproc_resample_affinemap_setcentery(void *x, float f){((t_affine_map *)x)->centery = f;}
void pdp_imageproc_resample_affinemap_setzoomx(void *x, float f){((t_affine_map *)x)->zoomx = f;}
void pdp_imageproc_resample_affinemap_setzoomy(void *x, float f){((t_affine_map *)x)->zoomy = f;}
void pdp_imageproc_resample_affinemap_setangle(void *x, float f){((t_affine_map *)x)->angle = f;}
void pdp_imageproc_resample_affinemap_process(void *x, u32 width, u32 height, s16 *srcimage, s16 *dstimage)
{
    t_affine_map *a = (t_affine_map *)x;
    double izx = 1.0f / (a->zoomx);
    double izy = 1.0f / (a->zoomy);
    double scale = (double)0xffffffff;
    double scalew = scale / ((double)(width - 1));
    double scaleh = scale / ((double)(height - 1));
    double cx = ((double)a->centerx) * ((double)(width - 1));
    double cy = ((double)a->centery) * ((double)(height - 1));
    double angle = a->angle * (-M_PI / 180.0);
    double c = cos(angle);
    double s = sin(angle);

    /* affine x, y mappings in screen coordinates */
    double mapx(double _x, double _y){return cx + izx * ( c * (_x-cx) + s * (_y-cy));}
    double mapy(double _x, double _y){return cy + izy * (-s * (_x-cx) + c * (_y-cy));}

    u32 colstate_x = (u32)(scalew * mapx(0,0));
    u32 colstate_y = (u32)(scaleh * mapy(0,0));
    u32 rowstate_x = colstate_x;
    u32 rowstate_y = colstate_y;

    u32 row_inc_x = (u32)(scalew * (mapx(1,0)-mapx(0,0)));
    u32 row_inc_y = (u32)(scaleh * (mapy(1,0)-mapy(0,0)));
    u32 col_inc_x = (u32)(scalew * (mapx(0,1)-mapx(0,0)));
    u32 col_inc_y = (u32)(scaleh * (mapy(0,1)-mapy(0,0)));

    u32 i,j;

    for (j=0; j<height; j++){
	for (i=0; i<width; i++){
	    *dstimage++ = pdp_resample_bilin(srcimage, width, height, rowstate_x>>16, rowstate_y>>16);
	    rowstate_x += row_inc_x;
	    rowstate_y += row_inc_y;
	}
	colstate_x += col_inc_x;
	colstate_y += col_inc_y;
	rowstate_x = colstate_x;
	rowstate_y = colstate_y;
    }

}





// polynomials




typedef struct
{
    u32 order;
    u32 nbpasses;
    s32 coefs[0];
} t_cheby;

void *pdp_imageproc_cheby_new(int order)
{
    t_cheby *z;
    int i;
    if (order < 2) order = 2;
    z = (t_cheby *)pdp_alloc(sizeof(t_cheby) + (order + 1) * sizeof(s32));
    z->order = order;
    z->coefs[0] = 0;
    z->coefs[1] = 0x7fff;
    for (i=2; i<=order; i++) z->coefs[i] = 0;
    return z;
}
void pdp_imageproc_cheby_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_cheby_setnbpasses(void *x, u32 n){((t_cheby *)x)->nbpasses = n;}
void pdp_imageproc_cheby_setcoef(void *x, u32 n, float f)
{

    t_cheby *z = (t_cheby *)x;
    if (n <= z->order){
	z->coefs[n] = (s32)(f * 32767.0f); // coefs are in s16.15 format
    }

}
void pdp_imageproc_cheby_process(void *x, u32 width, u32 height, s16 *image)
{

    t_cheby *z = (t_cheby *)x;
    u32 iterations = z->nbpasses;
    u32 i,j,k;
    s32 *c = z->coefs;
    for (j=0; j < (height*width); j++){
	s32 acc = (s32)image[j];
	for (i=0; i<iterations; i++){
	    s32 T2 = 0x7fff; /* 1 */
	    s32 T1 = acc;
	    s32 t;
	    s32 in = acc;
	    acc = c[0] + ((in*c[1])>>15);
	    for (k=2; k<=z->order; k++){
		t = ((T1*in)>>14) - T2; /* T_n = 2 x T_n-1 - T_n-2 */
		T2 = T1;
		T1 = t;
		acc += ((c[k] * t)>>15);
	    }
	}
	image[j] = (s16)(CLAMP16(acc));
    }
}
