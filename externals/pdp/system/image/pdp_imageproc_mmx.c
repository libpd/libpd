/*
 *   Pure Data Packet. c wrapper for mmx image processing routines.
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
#include <stdlib.h>
#include <math.h>
#include "pdp_mmx.h"
#include "pdp_imageproc.h"

/* pdp memory alloc/dealloc prototype */
void *pdp_alloc(int size);
void pdp_dealloc(void *);

// utility stuff
inline static s16 float2fixed(float f)
{
    if (f > 1) f = 1;
    if (f < -1) f = -1;
    f *= 0x7fff;
    return (s16)f;
}

inline static void setvec(s16 *v, float f)
{
    s16 a = float2fixed(f);
    v[0] = a;
    v[1] = a;
    v[2] = a;
    v[3] = a;
}



// add two images
void pdp_imageproc_add_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    unsigned int totalnbpixels = width * height;
    pixel_add_s16(image, image2, totalnbpixels>>2);
}

// mul two images
void pdp_imageproc_mul_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    unsigned int totalnbpixels = width * height;
    pixel_mul_s16(image, image2, totalnbpixels>>2);
}


// mix 2 images
#define MIX_SIZE 8*sizeof(s16)
void *pdp_imageproc_mix_new(void){return pdp_alloc(MIX_SIZE);}
int pdp_imageproc_mix_nb_stackwords(void){return PDP_IMAGEPROC_NB_STACKWORDS(MIX_SIZE);}
void pdp_imageproc_mix_delete(void *x) {pdp_dealloc (x);}
void pdp_imageproc_mix_setleftgain(void *x, float gain){setvec((s16 *)x, gain);}
void pdp_imageproc_mix_setrightgain(void *x, float gain){setvec((s16 *)x + 4, gain);}
void pdp_imageproc_mix_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    s16 *d = (s16 *)x;
    u32 i;

    if (*d == 0)
      for(i=0; i<width*height; i++)
	image[i] = image2[i];
    else {
      unsigned int totalnbpixels = width * height;
      pixel_mix_s16(image, image2, totalnbpixels>>2, d, d+4);
    }
}


// random mix 2 images
#define RANDMIX_SIZE 8*sizeof(s16)
void *pdp_imageproc_randmix_new(void){return pdp_alloc(RANDMIX_SIZE);}
int  pdp_imageproc_randmix_nb_stackwords(void) {return  PDP_IMAGEPROC_NB_STACKWORDS(RANDMIX_SIZE);}
void pdp_imageproc_randmix_delete(void *x) {pdp_dealloc (x);}
void pdp_imageproc_randmix_setthreshold(void *x, float threshold){setvec((s16 *)x, 2*threshold-1);}
void pdp_imageproc_randmix_setseed(void *x, float seed)
{
    s16 *d = (s16 *)x;
    srandom((u32)seed);
    d[4] = (s16)random();
    d[5] = (s16)random();
    d[6] = (s16)random();
    d[7] = (s16)random();
    
}
void pdp_imageproc_randmix_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    s16 *d = (s16 *)x;
    unsigned int totalnbpixels = width * height;
    pixel_randmix_s16(image, image2, totalnbpixels>>2, d+4, d);
}


// 3x1 or 1x3 in place convolution
// orientation
typedef struct
{
    s16 min1[4];
    s16 zero[4];
    s16 plus1[4];
    s16 border[4];
    u32 orientation;
    u32 nbpasses;
} t_conv;
void *pdp_imageproc_conv_new(void){return(pdp_alloc(sizeof(t_conv)));}
void pdp_imageproc_conv_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_conv_setmin1(void *x, float val){setvec(((t_conv *)x)->min1, val);}
void pdp_imageproc_conv_setzero(void *x, float val){setvec(((t_conv *)x)->zero, val);}
void pdp_imageproc_conv_setplus1(void *x, float val){setvec(((t_conv *)x)->plus1, val);}
void pdp_imageproc_conv_setbordercolor(void *x, float val){setvec(((t_conv *)x)->border, val);}
void pdp_imageproc_conv_setorientation(void *x, u32 val){((t_conv *)x)->orientation = val;}
void pdp_imageproc_conv_setnbpasses(void *x, u32 val){((t_conv *)x)->nbpasses = val;}
void pdp_imageproc_conv_process(void *x, u32 width, u32 height, s16 *image)

{
    t_conv *d = (t_conv *)x;
    
    u32 orientation = d->orientation;
    u32 nbp = d->nbpasses;
    u32 i,j;

    if (orientation == PDP_IMAGEPROC_CONV_HORIZONTAL)
    {
	for(i=0; i<width*height; i+=width)
	    for (j=0; j<nbp; j++)
		pixel_conv_hor_s16(image+i, width>>2, d->border, d->min1);
    }

    else
    {
	for (j=0; j<nbp; j++)
	    for(i=0; i<width; i +=4) pixel_conv_ver_s16(image+i,  height, width, d->border, d->min1);
    }

	
	
}

// apply a gain to an image
void *pdp_imageproc_gain_new(void){return(pdp_alloc(8*sizeof(s16)));}
void pdp_imageproc_gain_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_gain_setgain(void *x, float gain)
{
    /* convert float to s16 + shift */
    s16 *d = (s16 *)x;
    s16 g;
    int i;
    float sign;
    int shift = 0;
    
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
    g = (s16) gain;

    //g = 0x4000;
    //shift = 14;

    d[0]=g;
    d[1]=g;
    d[2]=g;
    d[3]=g;
    d[4]=(s16)shift;
    d[5]=0;
    d[6]=0;
    d[7]=0;
}
void pdp_imageproc_gain_process(void *x, u32 width, u32 height, s16 *image)
{
    s16 *d = (s16 *)x;
    pixel_gain_s16(image, (width*height)>>2, d, (u64 *)(d+4));
}

// colour rotation for 2 colour planes
void *pdp_imageproc_crot2d_new(void){return pdp_alloc(16*sizeof(s16));}
void pdp_imageproc_crot2d_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_crot2d_setmatrix(void *x, float *matrix)
{
    s16 *d = (s16 *)x;
    setvec(d, matrix[0]);
    setvec(d+4, matrix[1]);
    setvec(d+8, matrix[2]);
    setvec(d+12, matrix[3]);
}
void pdp_imageproc_crot2d_process(void *x, s16 *image, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    pixel_crot2d_s16(image, width*height >> 2, d);
}

// biquad and biquad time
typedef struct
{
    s16 ma1[4];
    s16 ma2[4];
    s16 b0[4];
    s16 b1[4];
    s16 b2[4];
    s16 u0[4];
    s16 u1[4];
    s16 u0_save[4];
    s16 u1_save[4];
    u32 nbpasses;
    u32 direction;
} t_bq;

void *pdp_imageproc_bq_new(void){return pdp_alloc(sizeof(t_bq));}
void pdp_imageproc_bq_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_bq_setcoef(void *x, float *coef) // a0,-a1,-a2,b0,b1,b2,u0,u1
{
    t_bq *d = (t_bq *)x;
    float ia0 = 1.0f / coef[0];

    /* all coefs are s1.14 fixed point */
    /* representing values -2 < x < 2  */
    /* so scale down before using the ordinary s0.15 float->fixed routine */

    ia0 *= 0.5f;

    // coef
    setvec(d->ma1, ia0*coef[1]);
    setvec(d->ma2, ia0*coef[2]);
    setvec(d->b0, ia0*coef[3]);
    setvec(d->b1, ia0*coef[4]);
    setvec(d->b2, ia0*coef[5]);

    // state to reset too
    setvec(d->u0_save, coef[6]);
    setvec(d->u1_save, coef[7]);

}
void pdp_imageproc_bq_setnbpasses(void *x, u32 nbpasses){((t_bq *)x)->nbpasses = nbpasses;}
void pdp_imageproc_bq_setdirection(void *x, u32 direction){((t_bq *)x)->direction = direction;}
void pdp_imageproc_bq_process(void *x, u32 width, u32 height, s16* image);


void pdp_imageproc_bqt_process(void *x, u32 width, u32 height, s16 *image, s16 *state0, s16 *state1)
{
    s16 *d = (s16 *)x;
    pixel_biquad_time_s16(image, state0, state1, d, (width*height)>>2);
}

void pdp_imageproc_bq_process(void *x, u32 width, u32 height, s16 *image)
{
    t_bq *d = (t_bq *)x;
    s16 *c = d->ma1; /* coefs */
    s16 *s = d->u0;  /* state */
    u32 direction = d->direction;
    u32 nbp = d->nbpasses;
    unsigned int i,j;



    /* VERTICAL */

    if ((direction & PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM)
	&& (direction &  PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP)){

	for(i=0; i<width; i +=4){
	    for (j=0; j<nbp; j++){
		pixel_biquad_vertb_s16(image+i,    height>>2, width, c, s);
		pixel_biquad_verbt_s16(image+i,    height>>2, width, c, s);
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM){
	for(i=0; i<width; i +=4){
	    for (j=0; j<nbp; j++){
		pixel_biquad_vertb_s16(image+i,    height>>2, width, c, s);
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP){
	for(i=0; i<width; i +=4){
	    for (j=0; j<nbp; j++){
		pixel_biquad_verbt_s16(image+i,    height>>2, width, c, s);
	    }
	}
    }

    /* HORIZONTAL */

    if ((direction & PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT)
	&& (direction & PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT)){

	for(i=0; i<(width*height); i +=(width<<2)){
	    for (j=0; j<nbp; j++){
		pixel_biquad_horlr_s16(image+i,    width>>2, width, c, s);
		pixel_biquad_horrl_s16(image+i,    width>>2, width, c, s);
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT){
	for(i=0; i<(width*height); i +=(width<<2)){
	    for (j=0; j<nbp; j++){
		pixel_biquad_horlr_s16(image+i,    width>>2, width, c, s);
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT){
	for(i=0; i<(width*height); i +=(width<<2)){
	    for (j=0; j<nbp; j++){
		pixel_biquad_horrl_s16(image+i,    width>>2, width, c, s);
	    }
	}
    }

}

// produce a random image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
void *pdp_imageproc_random_new(void){return pdp_alloc(4*sizeof(s16));}
void pdp_imageproc_random_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_random_setseed(void *x, float seed)
{
    s16 *d = (s16 *)x;
    srandom((u32)seed);
    d[0] = (s16)random();
    d[1] = (s16)random();
    d[2] = (s16)random();
    d[3] = (s16)random();
    
}
void pdp_imageproc_random_process(void *x, u32 width, u32 height, s16 *image)
{
    s16 *d = (s16 *)x;
    unsigned int totalnbpixels = width * height;
    pixel_rand_s16(image, totalnbpixels>>2, d);
}


/* resampling stuff
   this is quite a zoo of data structures
   the major point is this: the resampler mmx code is shared for all resampling code
   it uses data specified in t_resample_cbrd (Cooked Bilinear Resampler Data)

   then the there are several feeder algorithms. one is the linear mapper. it's
   data is specified in t_resample_clrd (Cooked Linear Remapper Data)

   for each feeder algorithm, there are several high level algorithms. like zoom,
   rotate, ... 
*/

typedef struct
{
    u32 lineoffset;
    s16 *image;
    u32 width;
    u32 height;
    
} t_resample_id; // Image Data

/* initialize image meta data (dimensions + location) */
static void pdp_imageproc_resample_init_id(t_resample_id *x, u32 offset, s16* image, u32 w, u32 h)
{
    x->lineoffset = offset;
    x->image = image;
    x->width = w;
    x->height = h;
}

// mmx resampling source image resampling data + coefs
typedef struct
{
    // vector data for resampling routine (resampling computation)
    u8  reserved[0x60];  //internal data
    s16 *address[2];     //64 bit splatted offset address
    s16 twowidthm1[4];   //64 bit splatted 2*(width-1)
    s16 twoheightm1[4];  //64 bit splatted 2*(height-1)
    s16 lineoffset[4];   //64 bit splatted line offset in pixels

} t_resample_cid; // Cooked Image Data

/* convert image meta data into a cooked format used by the resampler routine */
static void pdp_imageproc_resample_init_cid(t_resample_cid *r, t_resample_id *i)
{
    u32 twowm1 = (i->width-1)<<1;
    u32 twohm1 = (i->height-1)<<1;
    r->address[0] = i->image;
    r->address[1] = i->image;
    r->twowidthm1[0] = twowm1;
    r->twowidthm1[1] = twowm1;
    r->twowidthm1[2] = twowm1;
    r->twowidthm1[3] = twowm1;
    r->twoheightm1[0] = twohm1;
    r->twoheightm1[1] = twohm1;
    r->twoheightm1[2] = twohm1;
    r->twoheightm1[3] = twohm1;
    r->lineoffset[0] = i->lineoffset;
    r->lineoffset[1] = i->lineoffset;
    r->lineoffset[2] = i->lineoffset;
    r->lineoffset[3] = i->lineoffset;
}

// linear mapping data struct (zoom, scale, rotate, shear, ...)
typedef struct
{
    s32 rowstatex[2]; // row state x coord
    s32 rowstatey[2]; // row state y coord
    s32 colstatex[2]; // column state x coord
    s32 colstatey[2]; // column state y coord
    s32 rowincx[2];   // row inc vector x coord
    s32 rowincy[2];   // row inc vector y coord
    s32 colincx[2];   // column inc vector x coord
    s32 colincy[2];   // column inc vector y coord
} t_resample_clmd; // Cooked Linear Mapping Data

/* convert incremental linear remapping vectors to internal cooked format */
static void pdp_imageproc_resample_cookedlinmap_init(t_resample_clmd *l, s32 sx, s32 sy, s32 rix, s32 riy, s32 cix, s32 ciy)
{
    l->colstatex[0] = l->rowstatex[0] = sx;
    l->colstatex[1] = l->rowstatex[1] = sx + rix;
    l->colstatey[0] = l->rowstatey[0] = sy;
    l->colstatey[1] = l->rowstatey[1] = sy + riy;
    l->rowincx[0] = rix << 1;
    l->rowincx[1] = rix << 1;
    l->rowincy[0] = riy << 1;
    l->rowincy[1] = riy << 1;
    l->colincx[0] = cix;
    l->colincx[1] = cix;
    l->colincy[0] = ciy;
    l->colincy[1] = ciy;
}


/* this struct contains all the data necessary for
   bilin interpolation from src -> dst image
   (src can be == dst) */
typedef struct
{
    t_resample_cid csrc;     //cooked src image meta data for bilinear interpolator
    t_resample_id src;       //src image meta
    t_resample_id dst;       //dst image meta
} t_resample_cbrd;            //Bilinear Resampler Data


/* this struct contains high level zoom parameters,
   all image relative */
typedef struct
{
    float centerx;
    float centery;
    float zoomx;
    float zoomy;
    float angle;
} t_resample_zrd;


/* convert floating point center and zoom data to incremental linear remapping vectors */
static void pdp_imageproc_resample_clmd_init_from_id_zrd(t_resample_clmd *l, t_resample_id *i, t_resample_zrd *z)
{
    double izx = 1.0f / (z->zoomx);
    double izy = 1.0f / (z->zoomy);
    double scale = (double)0xffffffff;
    double scalew = scale / ((double)(i->width - 1));
    double scaleh = scale / ((double)(i->height - 1));
    double cx = ((double)z->centerx) * ((double)(i->width - 1));
    double cy = ((double)z->centery) * ((double)(i->height - 1));
    double angle = z->angle * (-M_PI / 180.0);
    double c = cos(angle);
    double s = sin(angle);

    /* affine x, y mappings in screen coordinates */
    double mapx(double x, double y){return cx + izx * ( c * (x-cx) + s * (y-cy));}
    double mapy(double x, double y){return cy + izy * (-s * (x-cx) + c * (y-cy));}

    u32 tl_x = (u32)(scalew * mapx(0,0));
    u32 tl_y = (u32)(scaleh * mapy(0,0));


    u32 row_inc_x = (u32)(scalew * (mapx(1,0)-mapx(0,0)));
    u32 row_inc_y = (u32)(scaleh * (mapy(1,0)-mapy(0,0)));
    u32 col_inc_x = (u32)(scalew * (mapx(0,1)-mapx(0,0)));
    u32 col_inc_y = (u32)(scaleh * (mapy(0,1)-mapy(0,0)));


    pdp_imageproc_resample_cookedlinmap_init(l, tl_x, tl_y, row_inc_x, row_inc_y, col_inc_x, col_inc_y);
}

/* this struct contains all data for the zoom object */
typedef struct
{
    t_resample_cbrd cbrd;      // Bilinear Resampler Data
    t_resample_clmd clmd;      // Cooked Linear Mapping data
    t_resample_zrd   zrd;      // Zoom / Rotate Data
} t_resample_zoom_rotate;

// zoom + rotate
void *pdp_imageproc_resample_affinemap_new(void)
{
    t_resample_zoom_rotate *z = (t_resample_zoom_rotate *)pdp_alloc(sizeof(t_resample_zoom_rotate));
    z->zrd.centerx = 0.5;
    z->zrd.centery = 0.5;
    z->zrd.zoomx = 1.0;
    z->zrd.zoomy = 1.0;
    z->zrd.angle = 0.0f;
    return (void *)z;
}
void pdp_imageproc_resample_affinemap_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_resample_affinemap_setcenterx(void *x, float f){((t_resample_zoom_rotate *)x)->zrd.centerx = f;}
void pdp_imageproc_resample_affinemap_setcentery(void *x, float f){((t_resample_zoom_rotate *)x)->zrd.centery = f;}
void pdp_imageproc_resample_affinemap_setzoomx(void *x, float f){((t_resample_zoom_rotate *)x)->zrd.zoomx = f;}
void pdp_imageproc_resample_affinemap_setzoomy(void *x, float f){((t_resample_zoom_rotate *)x)->zrd.zoomy = f;}
void pdp_imageproc_resample_affinemap_setangle(void *x, float f){((t_resample_zoom_rotate *)x)->zrd.angle = f;}
void pdp_imageproc_resample_affinemap_process(void *x, u32 width, u32 height, s16 *srcimage, s16 *dstimage)
{
    t_resample_zoom_rotate *z = (t_resample_zoom_rotate *)x;

    /* setup resampler image meta data */
    pdp_imageproc_resample_init_id(&(z->cbrd.src), width, srcimage, width, height);
    pdp_imageproc_resample_init_id(&(z->cbrd.dst), width, dstimage, width, height);
    pdp_imageproc_resample_init_cid(&(z->cbrd.csrc),&(z->cbrd.src)); 

    /* setup linmap data from zoom_rotate parameters */
    pdp_imageproc_resample_clmd_init_from_id_zrd(&(z->clmd), &(z->cbrd.src), &(z->zrd));


    /* call assembler routine */
    pixel_resample_linmap_s16(z);   
}



// polynomials


typedef struct
{
    u32 order;
    u32 nbpasses;
    s16 coefs[0];
} t_cheby;

void *pdp_imageproc_cheby_new(int order)
{
    t_cheby *z;
    int i;
    if (order < 2) order = 2;
    z = (t_cheby *)pdp_alloc(sizeof(t_cheby) + (order + 1) * sizeof(s16[4]));
    z->order = order;
    setvec(z->coefs + 0*4, 0);
    setvec(z->coefs + 1*4, 0.25);
    for (i=2; i<=order; i++)  setvec(z->coefs + i*4, 0);

    return z;
}
void pdp_imageproc_cheby_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_cheby_setcoef(void *x, u32 n, float f)
{
    t_cheby *z = (t_cheby *)x;
    if (n <= z->order){
	setvec(z->coefs + n*4, f * 0.25); // coefs are in s2.13 format
    }
}
void pdp_imageproc_cheby_setnbpasses(void *x, u32 n){((t_cheby *)x)->nbpasses = n;}

void pdp_imageproc_cheby_process(void *x, u32 width, u32 height, s16 *image)
{
    t_cheby *z = (t_cheby *)x;
    u32 iterations = z->nbpasses;
    u32 i,j;
    for (j=0; j < (height*width); j += width)
	for (i=0; i<iterations; i++)
	    pixel_cheby_s16_3plus(image+j, width>>2, z->order+1, z->coefs);

    //pixel_cheby_s16_3plus(image, (width*height)>>2, z->order+1, z->coefs);
}
