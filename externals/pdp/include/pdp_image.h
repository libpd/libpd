/*
 *   Pure Data Packet system implementation. Image packet interface
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
  This file contains methods for the image packets
  pdp_packet_new_* methods are several image packet constructors

  It also contains some pdp_type_ methods, for type checking
  and conversion.

*/

#ifndef PDP_IMAGE_H
#define PDP_IMAGE_H

#include "pdp_symbol.h"
#include "pdp_types.h"

/* image subheader */   
typedef struct _image
{
    /* standard images */
    unsigned int encoding;    /* image encoding (data format) */
    unsigned int width;       /* image width in pixels */
    unsigned int height;      /* image height in pixels */
    unsigned int depth;       /* number of colour planes if PDP_IMAGE_MCHP */
    unsigned int chanmask;    /* channel bitmask to mask out inactive channels (0 == not used) */

} t_image;


/* image encodings */
#define PDP_IMAGE_YV12   1  /* 24bbp: 16 bit Y plane followed by 16 bit 2x2 subsampled V and U planes.*/
#define PDP_IMAGE_GREY   2  /* 16bbp: 16 bit Y plane */
#define PDP_IMAGE_MCHP   4  /* generic 16bit multi channel planar (16 bit 3D tensor) */

/* slice synchro information */
#define PDP_IMAGE_SLICE_FIRST (1<<0)
#define PDP_IMAGE_SLICE_LAST  (1<<1)
#define PDP_IMAGE_SLICE_BODY  (1<<2)



/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif




/* validate and compat check */
int pdp_packet_image_isvalid(int packet);
int pdp_packet_image_compat(int packet0, int packet1);

/* short cuts to create specific packets */
int pdp_packet_new_image(u32 encoding, u32 width, u32 height);
int pdp_packet_new_image_YCrCb(u32 width, u32 height);
int pdp_packet_new_image_grey(u32 width, u32 height);
int pdp_packet_new_image_mchp(u32 width, u32 height, u32 depth);

#define pdp_packet_new_image_multi pdp_packet_new_image_mchp

/* get info */
t_pdp_symbol *pdp_packet_image_get_description(int packet);
t_image *pdp_packet_image_info(int packet);

/* set props */
void pdp_packet_image_set_chanmask(int packet, unsigned int chanmask);


#ifdef __cplusplus
}
#endif

#endif
