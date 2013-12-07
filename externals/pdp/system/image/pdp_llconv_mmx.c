
/*
 *   Pure Data Packet system implementation. : wrapper for mmx low level format conversion code
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


#include "pdp_mmx.h"



/* convert greyscale 8 bit unsigned to 16bit signed */
void llconv_grey_s16u8(short int *src, unsigned char *dst, unsigned int nbpixels)
{
    pixel_pack_s16u8_y(src, dst, nbpixels>>3);
}

/* convert yvu planar 411 16 bit signed to 8 bit unsigned */
void llconv_yvu_planar_s16u8(short int *src, unsigned char *dst, unsigned int nbpixels)
{
    pixel_pack_s16u8_y(src, dst, nbpixels>>3);
    pixel_pack_s16u8_uv(src + nbpixels, dst + nbpixels, nbpixels>>4);
}


/* convert yvu planar 411 8 bit unsigned to yv12 planar 16bit signed */
void llconv_yvu_planar_u8s16(unsigned char* source, short int *dest, int nbpixels)
{
    pixel_unpack_u8s16_y(source, dest, nbpixels>>3);
    pixel_unpack_u8s16_uv(&source[nbpixels], &dest[nbpixels], nbpixels>>4);
}

/* convert yuv planar 411 8 bit unsigned to yv12 planar 16bit signed */
void llconv_yuv_planar_u8s16(unsigned char* source, short int *dest, int nbpixels)
{
    pixel_unpack_u8s16_y(source, dest, nbpixels>>3);
    pixel_unpack_u8s16_uv(&source[nbpixels], &dest[nbpixels + (nbpixels>>2)], nbpixels>>5);
    pixel_unpack_u8s16_uv(&source[nbpixels + (nbpixels>>2)], &dest[nbpixels], nbpixels>>5);
}

