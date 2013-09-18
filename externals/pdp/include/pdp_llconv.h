/*
 *   Pure Data Packet system implementation. : low level format conversion code
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
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

/* this file contains low level conversion code
   it is a wrapper around some machine code routines padded
   with some extra c code */

/* don't rely too much on the calling conventions here
   this is mainly to tuck away "ugly" parts of the code
   that come up in several places */

#ifndef PDP_LLCONV_H
#define PDP_LLCONV_H


/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif



/* raw image formats (RIF) descriptions used for low level conversion routines
   format: RIF_[component names and order]_[data arganization]_[data type] 

   component names: R(red), G(green), B(blue), Y(chroma), V(chroma red), U(chroma blue)
   component type: [S/U][nb bits] ex: S16, U8
   data organization: [P/P[samplefrequency]] ex: P(packed) P411(planar, 2nd and 3rd 2x2 subsampled) 


*/

enum RIF {
	RIF_YVU__P411_U8,
	RIF_YUV__P411_U8,
	RIF_YVU__P411_S16,
	RIF_YVU__P444_S16,
	RIF_UYVY_P____U8,
	RIF_YUYV_P____U8,
        RIF_RGB__P____U8,
	RIF_RGBA_P____U8,
	RIF_RGB__P444_S16,
	RIF_GREY______S16,
	RIF_GREY______U8,
        RIF_BGR__P____U8,
        RIF_BGRA_P____U8
	
};	

/*  pdp_llconv is NOT thread safe !*/
/* gain = 1.0 means maximal */
/* low level convert 2 images */
void pdp_llconv(void *src, int stype, void *dest, int dtype, int w, int h);





#ifdef __cplusplus
}
#endif


#endif
