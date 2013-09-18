/*
 *   pdp system module - texture packet type
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

#ifndef PDP_TEXTURE_H
#define PDP_TEXTURE_H

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glx.h>
//#include <GL/glut.h>
#include "pdp.h"





/* TEXTURE PACKET */

typedef struct
{
    u32 tex_obj;           /* gl texture object */
    s32 format;            /* texture format */
    u32 width;             /* dims */
    u32 height;

    u32 sub_width;         /* portion of texture used */
    u32 sub_height;

} t_texture;

#define PDP_TEXTURE         4  /* opengl texture object */


/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif


/* check if valid texture packet. all other methods assume packet is valid */
int pdp_packet_texture_isvalid(int packet);

/* returns a pointer to the packet subheader whem the packet contains a texture */
/* try not to use the header directly, use clone and copy methods instead */
t_texture *pdp_packet_texture_info(int packet);

/* texture constructors */
int pdp_packet_new_texture(u32 width, u32 height, s32 format);      /* create a texture packet */


/* texture operators */
void pdp_packet_texture_make_current(int packet);               /* make a texture the current texture context */
u32 pdp_packet_texture_total_width(int packet);                 /* width of texture */
u32 pdp_packet_texture_total_height(int packet);                /* get heigth of texture */
u32 pdp_packet_texture_sub_width(int packet);                   /* width of subtexture */
u32 pdp_packet_texture_sub_height(int packet);                  /* heigth of subtexture */
float pdp_packet_texture_fracx(int packet);                     /* x fraction */
float pdp_packet_texture_fracy(int packet);                     /* y fraction */
float pdp_packet_texture_sub_aspect(int packet);

/* some utility methods */
void pdp_packet_texture_make_current_enable(int packet);        /* make current & enable with default texture settings (for the lazy)*/
void pdp_packet_texture_setup_2d_context(int packet);           /* set up 2d context (viewport, projection, modelview) from texture dims */







#ifdef __cplusplus
}
#endif

#endif //PDP_TEXTURE_H
