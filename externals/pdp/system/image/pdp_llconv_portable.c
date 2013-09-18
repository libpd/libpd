
/*
 *   Pure Data Packet system implementation. : portable low level format conversion code
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

#define CLAMP(x) (((x)<0) ? 0 : ((x>255)? 255 : (x)))
#define FP(x) ((int)(((float)(x)) * 256.0f))

void pixel_unpack_portable_u8s16_y(unsigned char *src ,short int *dst, unsigned int nbpixels)
{
    unsigned int i;
    for (i=0; i<nbpixels; i++) dst[i] = ((short int)(src[i])) << 7;
}

void pixel_unpack_portable_u8s16_uv(unsigned char *src ,short int *dst, unsigned int nbpixels)
{
    unsigned int i;
    for (i=0; i<nbpixels; i++) dst[i] = (((short int)(src[i])) << 8) ^ 0x8000;
}


void pixel_pack_portable_s16u8_y(short int *src, unsigned char *dst, unsigned int nbpixels)
{
    unsigned int i;
    for (i=0; i<nbpixels; i++) dst[i] = (unsigned char)(CLAMP(src[i]>>7));
}

void pixel_pack_portable_s16u8_uv(short int *src, unsigned char *dst, unsigned int nbpixels)
{
    unsigned int i;
    unsigned short *usrc = (unsigned short *)src;
    for (i=0; i<nbpixels; i++) dst[i] = ((usrc[i]^0x8000)>>8);
}


/* convert greyscale 8 bit unsigned to 16bit signed */
void llconv_grey_s16u8(short int *src, unsigned char *dst, unsigned int nbpixels)
{
    pixel_pack_portable_s16u8_y(src, dst, nbpixels);
}

/* convert yvu planar 411 16 bit signed to 8 bit unsigned */
void llconv_yvu_planar_s16u8(short int *src, unsigned char *dst, unsigned int nbpixels)
{
    pixel_pack_portable_s16u8_y(src, dst, nbpixels);
    pixel_pack_portable_s16u8_uv(src + nbpixels, dst + nbpixels, nbpixels>>1);

}


/* convert yvu planar 411 8 bit unsigned to yv12 planar 16bit signed */
void llconv_yvu_planar_u8s16(unsigned char* source, short int *dest, int nbpixels)
{
    pixel_unpack_portable_u8s16_y(source, dest, nbpixels);
    pixel_unpack_portable_u8s16_uv(&source[nbpixels], &dest[nbpixels], nbpixels>>1);
}

/* convert yuv planar 411 8 bit unsigned to yv12 planar 16bit signed */
void llconv_yuv_planar_u8s16(unsigned char* source, short int *dest, int nbpixels)
{
    pixel_unpack_portable_u8s16_y(source, dest, nbpixels);
    pixel_unpack_portable_u8s16_uv(&source[nbpixels], &dest[nbpixels + (nbpixels>>2)], nbpixels>>2);
    pixel_unpack_portable_u8s16_uv(&source[nbpixels + (nbpixels>>2)], &dest[nbpixels], nbpixels>>2);
}


