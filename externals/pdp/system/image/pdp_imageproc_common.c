/*
 *   Pure Data Packet. common image processing routines.
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


/*
  This file contains common code for (portable) low level image processing objects
  pdp_imageproc_* methods
  The rest is int pdp_imageproc_<platform>.c

  There are also highlevel dispatcher methods that operate on packets:
  pdp_imageproc_dispatch_* methods

*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pdp_imageproc.h"
#include "pdp_image.h"
#include "pdp_mem.h"
#include "pdp_packet.h"

#define CLAMP16(x) (((x) > 0x7fff) ? 0x7fff : (((x) < -0x7fff) ? -0x7fff : (x)))

u32 pdp_imageproc_legalwidth(int i)
{
    // sevy : we don't see no need for that limitation
    // has been tested on linux with 1280x1024 without problems
    // if (i>1024) return 1024;
    if (i>0) return  ((((i-1)>>3)+1)<<3);
    return 8;
    
}

u32 pdp_imageproc_legalheight(int i)
{
    // if (i>1024) return 1024;
    if (i>0) return  ((((i-1)>>3)+1)<<3);
    return 8;
}
u32 pdp_imageproc_legalwidth_round_down(int i)
{
    // if (i>1024) return 1024;
    if (i>8) return  ((i>>3)<<3);
    return 8;
    
}

u32 pdp_imageproc_legalheight_round_down(int i)
{
    // if (i>1024) return 1024;
    if (i>8) return  ((i>>3)<<3);
    return 8;
}


/* check if two packets are allocated and of the same type */
int pdp_packet_compat(int packet0, int packet1)
{

    t_pdp *header0 = pdp_packet_header(packet0);
    t_pdp *header1 = pdp_packet_header(packet1);
    if (!(header1)) return 0;
    if (!(header0)) return 0;
    if (header0->type != header1->type)	return 0;
    return 1;
}

/* some operations */

/* logic operators */

void pdp_imageproc_xor_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    u32 *plane = (u32 *)image;
    u32 *plane2 = (u32 *)image2;
    int count = (width * height) >> 1;
    int i;

    for (i=0; i<count; i++){
	plane[i] ^= plane2[i];
    }
}

void pdp_imageproc_and_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    u32 *plane = (u32 *)image;
    u32 *plane2 = (u32 *)image2;
    int count = (width * height) >> 1;
    int i;

    for (i=0; i<count; i++){
	plane[i] &= plane2[i];
    }
}

void pdp_imageproc_or_process(void *x, u32 width, u32 height, s16 *image, s16 *image2)
{
    u32 *plane = (u32 *)image;
    u32 *plane2 = (u32 *)image2;
    int count = (width * height) >> 1;
    int i;

    for (i=0; i<count; i++){
	plane[i] |= plane2[i];
    }
}

void pdp_imageproc_not_process(void *x, u32 width, u32 height, s16 *image)
{
    u32 *plane = (u32 *)image;
    int count = (width * height) >> 1;
    int i;

    for (i=0; i<count; i++){
	plane[i] ^= 0xffffffff;
    }
}

void pdp_imageproc_mask_process(void *x, u32 width, u32 height, s16 *image)
{
    uptr mask = (uptr)x;
    u32 *plane = (u32 *)image;
    int count = (width * height) >> 1;
    int i;

    mask = (mask & 0xffff) | (mask << 16);

    for (i=0; i<count; i++){
	plane[i] &= mask;
    }
}

// produce a plasma image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)

typedef struct
{
    u32 seed;
    s32 scale;
} t_plasma;

static inline s16 _rand_s16(void)
{
  return (s16)(random()<<0);
}

static inline s16 _new_color(s32 one, s32 two, s32 scale)
{
  return CLAMP16((one >> 1) + (two >> 1) + ((scale * _rand_s16()) >> 16));
  //return (one >> 1) + (two >> 1);
}

void *pdp_imageproc_plasma_new(void){return pdp_alloc(sizeof(t_plasma));}
void pdp_imageproc_plasma_delete(void *x){pdp_dealloc(x);}
void pdp_imageproc_plasma_setseed(void *x, float seed)
{
    *((float *)x) = seed;
}
void pdp_imageproc_plasma_setturbulence(void *x, float f)
{
    ((t_plasma *)x)->scale = CLAMP16(f * ((float)0x7fff));
}

static void _plasma_subdiv(u32 w, u32 h, u32 s, s16 *image, int calc_left, int calc_top, s32 scale)
{
  int w0 = ((w-1)>>1);  // width of left segments
  int h0 = ((h-1)>>1);  // heigth of top segments
  int w1 = w - w0;
  int h1 = h - h0;

  /* conditions: w0 <= w1, h0 <= h1 */

  /* original coordinates */
  int topleft = 0;
  int topright = w-1;
  int bottomleft = s * (h-1);
  int bottomright = bottomleft + topright;

  /* new subdivision coordinates */
  int top = w0;
  int left = s * h0;
  int bottom = bottomleft + w0;
  int right = topright + left;
  int center = left + top;

  if (w0 && h0){ /* left-right and top-bottom subdivide */

    /* calculate corner pixel colours */
    if (calc_top)  image[top]    = _new_color(image[topleft], image[topright], scale);
    if (calc_left) image[left]   = _new_color(image[topleft], image[bottomleft], scale);
    image[right]  = _new_color(image[topright], image[bottomright], scale);
    image[bottom] = _new_color(image[bottomleft], image[bottomright], scale);
    image[center] = (_new_color(image[top], image[bottom], scale) >> 1)
	+(_new_color(image[left], image[right], scale) >> 1);


    /* subdivide (with overlap) */
    _plasma_subdiv(w0+1, h0+1, s, &image[topleft], 1, 1, scale);
    _plasma_subdiv(w1, h0+1, s, &image[top], 0, 1, scale);
    _plasma_subdiv(w0+1, h1, s, &image[left], 1, 0, scale);
    _plasma_subdiv(w1, h1, s, &image[center], 0, 0, scale);
    
  }
  

  else if(h0) { /* top-bottom subdivide */

      //post("h:%d", h);

    /* calculate corner pixel colours */
    if(calc_left) image[left]   = _new_color(image[topleft], image[bottomleft], scale);
    image[right]  = _new_color(image[topright], image[bottomright], scale);

    /* subdivide (without overlap) */
    _plasma_subdiv(w, h0+1, s, &image[topleft], 1, 0, scale);
    _plasma_subdiv(w, h1, s, &image[left], 1, 0, scale);
    
  }
  
  else if (w0){ /* left-right subdivide */

    /* calculate corner pixel colours */
    if (calc_top) image[top]    = _new_color(image[topleft], image[topright], scale);
    image[bottom] = _new_color(image[bottomleft], image[bottomright],scale);

    /* subdivide with overlap */
    _plasma_subdiv(w0+1, h, s, &image[topleft], 0, 1, scale);
    _plasma_subdiv(w1, h, s, &image[top], 0, 1, scale);

  }

}

void pdp_imageproc_plasma_process(void *x, u32 width, u32 height, s16 *image)
{
    s32 scale = (((t_plasma *)x)->scale);
    srandom (((t_plasma *)x)->seed);

    /* set initial border colours */
    image[0]                  = _rand_s16();
    image[width-1]            = _rand_s16();
    image[width * (height-1)] = _rand_s16();
    image[width * height - 1] = _rand_s16();

    /* subdivide */
    _plasma_subdiv(width, height, width, image, 1, 1, scale);

    ((t_plasma *)x)->seed = random();

}
  


void pdp_imageproc_zero_process(void *x, u32 width, u32 height, s16 *image)
{
    int bytesize = (width * height) << 1;
    memset(image, 0, bytesize);
}

void pdp_imageproc_constant_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    uptr value = (uptr)x;
    u32 *plane = (u32 *)image;
    int wordsize = (width * height) >> 1;
    value = (value & 0xffff) | (value << 16);
    for (i=0; i<wordsize; i++){
	plane[i] = value;
    }
}


/* other stateless operators */

/* some 2x16bit vector ops */

/* some bit shuffling to ensure 32 bit accesses 
   get the sign bit extended as a mask: - : 0xffff +: 0x0000 */
static inline u32 _sign(s32 invec)
{
    s32 mask_top = invec;
    s32 mask_bot = invec;

    mask_top &= 0x80000000; /* isolate top sign bit */
    mask_bot <<= 16;        /* shift bottom word to top word */
    mask_bot &= 0x80000000; /* isolate bottom sign bit */
    mask_top >>= 15;        /* shift sign bit into top word */
    mask_bot >>= 15;
    mask_bot = (s32)(((u32)mask_bot) >> 16);  /* shift top word into bottom word */
    return mask_top |mask_bot;    
}

/* clear the least significant bit of the top word
   to ensure a decoupled vector add */
static inline void _decouple(s32 *invec)
{
    *invec &= 0xfffeffff;
}

void pdp_imageproc_abs_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    s32 *wimage = (s32 *)image;
    int wsize = (width * height) >> 1;
    for (i=0; i<wsize; i++){
	/* this computes  c = (c >= 0) ? (c) : (~c) */
	/* not is used instead of neg to prevent overflow on 0x8000 */
	/* this maps both 0 and -1 to 0 */

	wimage[i] ^= _sign(wimage[i]);
	
    }
}

void pdp_imageproc_zthresh_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    s32 *wimage = (s32 *)image;
    int wsize = (width * height) >> 1;
    for (i=0; i<wsize; i++){
	/* this computes  c = (c >= 0) ? (c) : (0) */
	wimage[i] &= ~_sign(wimage[i]);
    }
}

/* hard thresholding: x contains a positive unsigned short int */
void pdp_imageproc_hardthresh_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    uptr thresh = (uptr)x;
    s32 sign1, isign2, a;
    s32 *wimage = (s32 *)image;
    int wsize = (width * height) >> 1;
    thresh |= (thresh << 16);
    for (i=0; i<wsize; i++){
	a = wimage[i];
	sign1 = _sign(a);   
	a ^= sign1;           /* take abs */
	_decouple(&a);
	a -= thresh;          /* subtract threshold */
	isign2 = ~ _sign(a);
	a &= isign2;          /* zero thresh */
	_decouple(&a);
	a += thresh & isign2; /* add threshold (if not zero thresholded)*/
	a ^= sign1;
	wimage[i] = a;
    }
}

/* soft thresholding: x contains a positive unsigned short int */
void pdp_imageproc_softthresh_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    sptr thresh = (sptr)x;
    s32 sign1, sign2, a;
    s32 *wimage = (s32 *)image;
    int wsize = (width * height) >> 1;
    thresh |= thresh << 16;
    for (i=0; i<wsize; i++){
	a = wimage[i];
	sign1 = _sign(a);   
	a ^= sign1;           /* take abs */
	_decouple(&a);
	a -= thresh;          /* subtract threshold */
	sign2 = _sign(a);
	a &= ~ sign2;         /* zero thresh */
	_decouple(&a);
	//a += thresh;        /* add threshold */
	a ^= sign1;
	wimage[i] = a;
	
    }

}


/* turns an image into a positive andmask */
void pdp_imageproc_ispositive_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    s32 *wimage = (s32 *)image;
    int wsize = (width * height) >> 1;
    for (i=0; i<wsize; i++){
	wimage[i] = ~_sign(wimage[i]);
    }

}

/* get sign */
void pdp_imageproc_sign_process(void *x, u32 width, u32 height, s16 *image)
{
    int i;
    s32 *wimage = (s32 *)image;
    int wsize = (width * height) >> 1;
    for (i=0; i<wsize; i++){
	wimage[i] = _sign(wimage[i]) ^ 0x7fff7fff;
    }

}

/* flip left <-> right */
void pdp_imageproc_flip_lr_process(void *dummy, u32 width, u32 height, s16 *image)
{
    u32 y;
    s16 tmp, *l, *r;
    for (y=0; y<height; y++){
	l = image;
	r = image + width - 1;
	while (l < r){
	    tmp = *l;
	    *l = *r;
	    *r = tmp;
	    l++;
	    r--;
	}
	image += width;
    }

}

void pdp_llconv_flip_top_bottom(s16 *data, int width, int height, int pixelsize);

void pdp_imageproc_flip_tb_process(void *dummy, u32 width, u32 height, s16 *image)
{
    pdp_llconv_flip_top_bottom(image, width, height, 2);
}


/* image processing dispatcher methods  */
/* if the first packet contains a nonzero channel mask, it will be used instead
   of the one supplied as argument to the dispatcher functions.
   the packet's channel mask will be reset to 0 */

void pdp_imageproc_dispatch_1buf(void (*process_routine)(void*, u32, u32, s16*), void *x, u32 chanmask, int packet0)
{
    t_pdp *header0;
    t_image *image0;
    s16  *idata0;
    unsigned int w,h,d,plane_size,mask;
 
    /* if packet is not a valid image return without doing anything */
    if (!(pdp_packet_image_isvalid(packet0))) return;

    header0 = pdp_packet_header(packet0);
    image0 = pdp_packet_image_info(packet0);
    idata0   = pdp_packet_data  (packet0);

    w = image0->width;
    h = image0->height;
    d = image0->depth;
    plane_size = w*h;

    if (image0->chanmask) chanmask = image0->chanmask;
    image0->chanmask = 0;


    switch(image0->encoding){
    case PDP_IMAGE_GREY:
	if (chanmask & 1) (*process_routine)(x, w, h, idata0);
	break;
    case PDP_IMAGE_YV12:
	if (chanmask & 1) (*process_routine)(x, w, h, idata0);
	idata0 += plane_size;
	plane_size >>= 2;
	w >>= 1;
	h >>= 1;
	if (chanmask & 2) (*process_routine)(x, w, h, idata0);
	idata0 += plane_size;
	if (chanmask & 4) (*process_routine)(x, w, h, idata0);
	break;
    case PDP_IMAGE_MCHP:
	mask = 1;
	while (d--){
	    if (chanmask & mask) (*process_routine)(x, w, h, idata0);
	    idata0 += plane_size;
	    mask <<= 1;
	}
	break;
    default:
	break;
    }
}


void pdp_imageproc_dispatch_2buf(void (*process_routine)(void*, u32, u32, s16*, s16 *), void *x, u32 chanmask, int packet0, int packet1)
{
    t_pdp *header0;
    t_image *image0;
    s16  *idata0, *idata1;
    unsigned int w,h,d,plane_size,mask;
 
    /* if packets are not compatible images, return without doing anything */
    if (!(pdp_packet_image_compat(packet0, packet1))) return;

    header0 = pdp_packet_header(packet0);
    image0 = pdp_packet_image_info(packet0);
    idata0   = pdp_packet_data  (packet0);
    idata1   = pdp_packet_data  (packet1);

    w = image0->width;
    h = image0->height;
    d = image0->depth;
    plane_size = w*h;

    if (image0->chanmask) chanmask = image0->chanmask;
    image0->chanmask = 0;

    switch(image0->encoding){
    case PDP_IMAGE_GREY:
	if (chanmask & 1) (*process_routine)(x, w, h, idata0, idata1);
	break;
    case PDP_IMAGE_YV12:
	if (chanmask & 1) (*process_routine)(x, w, h, idata0, idata1);
	idata0 += plane_size;
	idata1 += plane_size;
	plane_size >>= 2;
	w >>= 1;
	h >>= 1;
	if (chanmask & 2) (*process_routine)(x, w, h, idata0, idata1);
	idata0 += plane_size;
	idata1 += plane_size;
	if (chanmask & 4) (*process_routine)(x, w, h, idata0, idata1);
	break;
    case PDP_IMAGE_MCHP:
	mask = 1;
	while (d--){
	    if (chanmask & mask) (*process_routine)(x, w, h, idata0, idata1);
	    idata0 += plane_size;
	    idata1 += plane_size;
	    mask <<= 1;
	}
	break;
    default:
	break;
    }
}
void pdp_imageproc_dispatch_3buf(void (*process_routine)(void*, u32, u32, s16*, s16 *, s16 *), void *x, u32 chanmask, int packet0, int packet1, int packet2)
{
    t_pdp *header0;
    t_image *image0;
    s16  *idata0, *idata1, *idata2;
    unsigned int w,h,d,plane_size, mask;
 
    /* if packets are not compatible images, return without doing anything */
    if (!((pdp_packet_image_compat(packet0, packet1))
	  &&(pdp_packet_image_compat(packet0, packet1)))) return;

    header0 = pdp_packet_header(packet0);
    image0 = pdp_packet_image_info(packet0);
    idata0   = pdp_packet_data  (packet0);
    idata1   = pdp_packet_data  (packet1);
    idata2   = pdp_packet_data  (packet2);

    w = image0->width;
    h = image0->height;
    d = image0->depth;
    plane_size = w*h;

    if (image0->chanmask) chanmask = image0->chanmask;
    image0->chanmask = 0;

    switch(image0->encoding){
    case PDP_IMAGE_GREY:
	if (chanmask & 1)(*process_routine)(x, w, h, idata0, idata1, idata2);
	break;
    case PDP_IMAGE_YV12:
	if (chanmask & 1)(*process_routine)(x, w, h, idata0, idata1, idata2);
	idata0 += plane_size;
	idata1 += plane_size;
	idata2 += plane_size;
	plane_size >>= 2;
	w >>= 1;
	h >>= 1;
	if (chanmask & 2)(*process_routine)(x, w, h, idata0, idata1, idata2);
	idata0 += plane_size;
	idata1 += plane_size;
	idata2 += plane_size;
	if (chanmask & 4)(*process_routine)(x, w, h, idata0, idata1, idata2);
	break;
    case PDP_IMAGE_MCHP:
	mask = 1;
	while (d--){
	    if (chanmask & mask) (*process_routine)(x, w, h, idata0, idata1, idata2);
	    idata0 += plane_size;
	    idata1 += plane_size;
	    idata2 += plane_size;
	    mask <<= 1;
	}
	break;
    default:
	break;
    }
}
