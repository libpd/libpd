/*
 *   Pure Data Packet header file. - image resampling prototypes
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

#ifndef PDP_RESAMPLE_H
#define PDP_RESAMPLE_H

#include "pdp_types.h"


/* image resampling methods */
void pdp_resample_scale_bilin(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h, s32 dst_w, s32 dst_h);
void pdp_resample_scale_nn(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h, s32 dst_w, s32 dst_h);

/* USE pdp_imageproc_resample_affinemap
void pdp_resample_zoom_tiled_bilin(s16 *src_image, s16 *dst_image, s32 w, s32 h, 
				   float zoom_x, float zoom_y, float center_x_relative, float center_y_relative);
*/

//void pdp_resample_zoom_tiled_nn(s16 *src_image, s16 *dst_image, s32 w, s32 h, float zoom_x, float zoom_y);



/* power of 2 resamplers */
void pdp_resample_halve(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h);
void pdp_resample_double(s16 *src_image, s16 *dst_image, s32 src_w, s32 src_h);



/* core routines */
//s32 pdp_resample_bilin(s16 *image, s32 width, s32 height, s32 virt_x, s32 virt_y);


#endif
