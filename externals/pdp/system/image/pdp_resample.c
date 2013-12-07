/*
 *   Pure Data Packet system file. - image resampling routines
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


#include <string.h>
#include "pdp_resample.h"


/*

efficient bilinear resampling ??
performance: how to eliminate divides? -> virtual coordinates 2^k x 2^k (conf. opengl)

i.e. 16 bit virtual coordinates: easy modular addressing

*/


/* code in this file should go out to be replaced by code in pdp_imageproc */

static s32 pdp_resample_bilin(s16 *image, s32 width, s32 height, s32 virt_x, s32 virt_y)
{

    s32 fp_x, fp_y, frac_x, frac_y, f, offset, r_1, r_2;

    virt_x &= 0xffff;
    virt_y &= 0xffff;

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


void pdp_resample_scale_bilin(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h, s32 dst_w, s32 dst_h)
{
    s32 i,j;
    s32 virt_x=0;
    s32 virt_y=0; /* virtual coordinates in 30 bit */
    s32 scale_x = 0x40000000 / dst_w;
    s32 scale_y = 0x40000000 / dst_h;

    for (j=0; j<dst_h; j++){
	for (i=0; i<dst_w; i++){
	    *dst_image++ = pdp_resample_bilin(src_image, src_w, src_h, virt_x>>14, virt_y>>14);
	    virt_x += scale_x;
	}
	virt_x = 0;
	virt_y += scale_y;
    }

}

void pdp_resample_scale_nn(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h, s32 dst_w, s32 dst_h)
{
    s32 i,j;
    s32 x=0;
    s32 y=0;
    s32 frac_x=0;
    s32 frac_y=0;
    s32 scale_x = (src_w << 20 ) / dst_w;
    s32 scale_y = (src_h << 20 ) / dst_h;

    for (j=0; j<dst_h; j++){
	for (i=0; i<dst_w; i++){
	    *dst_image++ = src_image[x+y];
	    frac_x += scale_x;
	    x = frac_x >> 20;
	}
	x = 0;
	frac_x = 0;
	frac_y += scale_y;
	y = (frac_y >> 20) * src_w;
    }

}

/* USE pdp_resample_affinemap
void pdp_resample_zoom_tiled_bilin(s16 *src_image, s16 *dst_image, s32 w, s32 h, 
				   float zoom_x, float zoom_y, float center_x_relative, float center_y_relative)
{
    float izx = 1.0f / zoom_x;
    float izy = 1.0f / zoom_y;
    s32 scale_x = (s32)((float)0x100000 * izx / (float)w);
    s32 scale_y = (s32)((float)0x100000 * izy / (float)h);

    s32 top_virt_x = (s32)((1.0f - izx) * (float)0x100000 * center_x_relative);
    s32 top_virt_y = (s32)((1.0f - izy) * (float)0x100000 * center_y_relative);

    s32 virt_x = top_virt_x;
    s32 virt_y = top_virt_y; 

    s32 i,j;

    for (j=0; j<h; j++){
	for (i=0; i<w; i++){
	    *dst_image++ = pdp_resample_bilin(src_image, w, h, virt_x>>4, virt_y>>4);
	    virt_x += scale_x;
	}
	virt_x = top_virt_x;
	virt_y += scale_y;
    }

}
*/

void pdp_resample_halve(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h)
{

    int dst_x,dst_y;
    int src_x = 0;
    int src_y = 0;
    int dst_w = src_w >> 1;
    int dst_h = src_h >> 1;
    s32 tmp1,tmp2,tmp3,tmp4;

    //post("%x %x %d %d\n", src_image, dst_image, src_w, src_h);

    for(dst_y = 0; dst_y < dst_h * dst_w; dst_y += dst_w){
	for (dst_x = 0; dst_x < dst_w; dst_x++){

	    tmp1 = (s32)src_image[src_y + src_x];
	    tmp2 = (s32)src_image[src_y + src_x + 1];
	    tmp3 = (s32)src_image[src_y + src_x + src_w];
	    tmp4 = (s32)src_image[src_y + src_x + src_w + 1];

	    tmp1 += tmp2;
	    tmp3 += tmp4;

	    src_x += 2;

	    dst_image[dst_x+dst_y] = (s16)((tmp1 + tmp3)>>2);
	}
	src_y += src_w << 1;
	src_x = 0;
    }
}

void pdp_resample_double(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h)
{
    int src_x = 0;
    int src_y = 0;
    int dst = 0;
    int dst_w = src_w << 1;

    s16 tmp;

    for(src_y = 0; src_y < src_h * src_w; src_y += src_w){
	for (src_x = 0; src_x < src_w; src_x++){

	    tmp = *src_image++;
      	    dst = (src_y << 2) + (src_x << 1);
	    dst_image[dst]   = tmp;
	    dst_image[dst+1] = tmp;
	    dst+=dst_w;
	    dst_image[dst]   = tmp;
	    dst_image[dst+1] = tmp;
	}
    }
}

/* $$$TODO: finish this */
void pdp_resample_padcrop(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h, s32 dst_w, s32 dst_h)
{

    int shift_x = (dst_w - src_w) / 2;
    int shift_y = (dst_h - src_h) / 2;
}

