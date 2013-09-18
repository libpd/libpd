
/*
 *   OpenGL Extension Module for pdp - pbuffer packet implementation
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
  this code uses glx. i don't know if it is worth to take into
  account portabiliy. since it will take a while until pdp runs
  on anything else than linux. but in any case, providing a windows/osx
  implementation here should not be too difficult..
*/

#include "pdp_3Dcontext.h"
#include <GL/gl.h>
#include <GL/glu.h>

#define D if (0)

/* constructor */

/* pbuf operators */

u32 pdp_packet_3Dcontext_width(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c) return c->width;
    else return 0;
}

u32 pdp_packet_3Dcontext_height(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c) return c->height;
    else return 0;
}

u32 pdp_packet_3Dcontext_subwidth(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c) return c->sub_width;
    else return 0;
}


u32 pdp_packet_3Dcontext_subheight(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c) return c->sub_height;
    else return 0;
}


void  pdp_packet_3Dcontext_set_subwidth(int packet, u32 w)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c) c->sub_width = w;
}


void pdp_packet_3Dcontext_set_subheight(int packet, u32 h)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c)  c->sub_height = h;
}


float pdp_packet_3Dcontext_subaspect(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (c) return (float)c->sub_width/c->sub_height;
    else return 0;
}

int pdp_packet_3Dcontext_isvalid(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);

    if (!header) return 0;
    if (!c) return 0;
    if (PDP_3DCONTEXT != header->type) return 0;
    return 1;
}

t_3Dcontext *pdp_packet_3Dcontext_info(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    if (!header) return 0;
    if (PDP_3DCONTEXT != header->type) return 0;
    return (t_3Dcontext *)&header->info.raw;
}



void pdp_llconv_flip_top_bottom(char *data, int width, int height, int pixelsize);

int pdp_packet_3Dcontext_snap_to_bitmap(int packet, int w, int h)
{
    int x, y, new_p, i;
    char *data = 0;
    // char r;
    // int extra = 5;

    if (!pdp_packet_3Dcontext_isvalid(packet)) goto error;

    x = pdp_packet_3Dcontext_subwidth(packet);
    y = pdp_packet_3Dcontext_subheight(packet);

    x = (x - w) >> 1;
    y = (y - h) >> 1;
    x = (x < 0 ) ? 0 : x;
    y = (y < 0 ) ? 0 : y;

    new_p = pdp_packet_new_bitmap_rgb(w, h);
    data = (char *)pdp_packet_data(new_p);
    if (-1 == new_p || !data) goto error;
    pdp_packet_3Dcontext_set_rendering_context(packet);

    // D post("BEGIN READPIXELS %d %d %d %d %x", w, h, x, y, data);
    
    //for (i=0; i<w*h; i++){
    //	data[3*i] = 255;
    //	data[3*i+1] = 255;
    //	data[3*i+2] = 0;
    //}
    // r = random();
    // data[w*h*3] = r;

    /* seems nvidia drivers 4191 have a bug
       when w % 4 is not zero */
    glReadPixels(x,y, w ,h,GL_RGB,GL_UNSIGNED_BYTE, data);

    /* inplace swap top to bottom (textures and buffers have
       another coordinate system than standard images)
       instead of fixing this by using a different texture coordinate
       system, a memory swap is performed. this is more expensive
       but eliminates hassle when converting between buffers, textures
       and bitmaps */

    pdp_llconv_flip_top_bottom(data, w, h, 3);

    // if (r != data[w*h*3]) post("PANIC");

    // post("END READPIXELS %d %d", w, h);


    return new_p;

 error:
    return -1;
    
}




/* move these to the pdp_3d_context object: they're too specific */

/* setup for 2d operation from pbuf dimensions */
void pdp_packet_3Dcontext_setup_2d_context(int p)
{
    u32 w;
    u32 h;
    float asp;
    if (!pdp_packet_3Dcontext_isvalid(p)) return;
    w = pdp_packet_3Dcontext_subwidth(p);
    h = pdp_packet_3Dcontext_subheight(p);
    asp = pdp_packet_3Dcontext_subaspect(p);


    /* set the viewport to the size of the sub frame */
    glViewport(0, 0, w, h);

    /* set orthogonal projection, with a relative frame size of (2asp x 2) */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 2*asp, 0, 2);

    /* set the center of view */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(asp, 1, 0);
    glScalef(1,-1,1);


}

/* setup for 3d operation from pbuf dimensions */
void pdp_packet_3Dcontext_setup_3d_context(int p)
{
    u32 w;
    u32 h;
    int i;
    float asp;
    float m_perspect[] = {-1.f, /* left */
			  1.f,  /* right */
			  -1.f, /* bottom */
			  1.f,  /* top */
			  1.f,  /* front */
			  20.f};/* back */

    if (!pdp_packet_3Dcontext_isvalid(p)) return;
    w = pdp_packet_3Dcontext_subwidth(p);
    h = pdp_packet_3Dcontext_subheight(p);
    asp = pdp_packet_3Dcontext_subaspect(p);


    /* set the viewport to the size of the sub frame */
    glViewport(0, 0, w, h);

    /* set orthogonal projection, with a relative frame size of (2asp x 2) */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(m_perspect[0] * asp, m_perspect[1] * asp,       // left, right
	      m_perspect[2], m_perspect[3],                   // bottom, top
	      m_perspect[4], m_perspect[5]);                  // front, back
    
    /* reset texture matrix */
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();


    /* set the center of view */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
    //glTranslatef(asp, 1, 0);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    //glShadeModel(GL_FLAT);


    /* disable everything that is enabled in other modules
     this resets the ogl state to its initial conditions */
    glDisable(GL_LIGHTING);
    for (i=0; i<8; i++) glDisable(GL_LIGHT0 + i);
    glDisable(GL_COLOR_MATERIAL);


}


void pdp_3Dcontext_common_setup(void)
{
}
