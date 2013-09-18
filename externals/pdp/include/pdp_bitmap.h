/*
 *   Pure Data Packet system implementation. 8 bit image packet interface
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

#ifndef PDP_BITMAP_H
#define PDP_BITMAP_H



/* bitmap  data packet */   
typedef struct _bitmap
{
    /* standard images */
    unsigned int encoding;    /* image encoding (fourcc data format) */
    unsigned int width;       /* image width in pixels */
    unsigned int height;      /* image height in pixels */
    unsigned int bpp;         /* bits per pixel (0 == standard) */

} t_bitmap;



/* supported encodings (fourcc) */

/* special */
#define PDP_BITMAP_RGB  0x32424752
#define PDP_BITMAP_RGBA 0x41424752
#define PDP_BITMAP_GREY 0x59455247

/* packet yuv */
#define PDP_BITMAP_YUY2 0x32595559
#define PDP_BITMAP_UYVY 0x59565955

/* planar yuv */
#define PDP_BITMAP_I420 0x30323449
#define PDP_BITMAP_YV12 0x32315659


/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif



/* bitmap constructors*/
int pdp_packet_new_bitmap_yv12(u32 width, u32 height);
int pdp_packet_new_bitmap_grey(u32 width, u32 height);
int pdp_packet_new_bitmap_rgb(u32 width, u32 height);
int pdp_packet_new_bitmap_rgba(u32 width, u32 height);
int pdp_packet_new_bitmap(int type, u32 width, u32 height);

/* utility methids */
void pdp_packet_bitmap_flip_top_bottom(int packet);


/* get description */
t_pdp_symbol *pdp_packet_bitmap_get_description(int packet);

/* get subheader */
t_bitmap *pdp_packet_bitmap_info(int packet);

int pdp_packet_bitmap_isvalid(int packet);

#ifdef __cplusplus
}
#endif

#endif
