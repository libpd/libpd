/*
 *   Pure Data Packet header file. ascii packet type.
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


#ifndef PDP_H
#define PDP_H

/* ascii data packet */   
typedef struct
{
    unsigned int encoding;  /* image encoding (data format) */
    unsigned int width;     /* image width in pixels */
    unsigned int height;    /* image height in pixels */
} t_ascii;


/* ascii encodings */
#define PDP_ASCII_BW     1  /* 8 bit per character black and white.*/
#define PDP_ASCII_IBM    2  /* 16 bit per character colour (8 bit character, 8 bit colour, like good old text framebuffers.*/
#define PDP_ASCII_RGB    3  /* 64 bit per character colour (8 bit character, 3x8 bit RGB */

#endif
