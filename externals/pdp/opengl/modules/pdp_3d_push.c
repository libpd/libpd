/*
 *   Pure Data Packet module.
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



#include <GL/gl.h>
#include "pdp_opengl.h"
#include "pdp_3dp_base.h"

typedef struct pdp_3d_push_struct
{
    t_pdp_3dp_base x_base;
    GLenum x_matrix;
    int x_change_mode;

} t_pdp_3d_push;



static void pdp_3d_push_process_right(t_pdp_3d_push *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* push one of the matrices */
	glMatrixMode(x->x_matrix);
	glPushMatrix();

	/* set default matrix to modelview */
	if (!x->x_change_mode) glMatrixMode(GL_MODELVIEW);

    }
}
static void pdp_3d_push_process_left(t_pdp_3d_push *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* restore the saved matrix */
	glMatrixMode(x->x_matrix);
	glPopMatrix();

	/* set default matrix back to modelview */
	glMatrixMode(GL_MODELVIEW);

    }

}


static void pdp_3d_mode_process_right(t_pdp_3d_push *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* change matrix mode */
	glMatrixMode(x->x_matrix);

    }
}

static void pdp_3d_mode_process_left(t_pdp_3d_push *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* restore default matrix to modelview */
	glMatrixMode(GL_MODELVIEW);

    }
}


static void pdp_3d_push_setmatrix(t_pdp_3d_push *x, t_symbol *s)
{
    GLenum m;

    /* find out which matrix to push */
    if (s == gensym("projection")) m = GL_PROJECTION;
    else if (s == gensym("modelview")) m = GL_MODELVIEW;
    else if (s == gensym("texture")) m = GL_TEXTURE;
    else if (s == gensym("color")) m = GL_COLOR;

    /* default is modelview */
    else m = GL_MODELVIEW;

    x->x_matrix = m;
}


t_class *pdp_3d_push_class;



void pdp_3d_push_free(t_pdp_3d_push *x)
{
    pdp_3dp_base_free(x);
}

void *pdp_3d_push_mode_new(t_symbol *s)
{
    t_pdp_3d_push *x = (t_pdp_3d_push *)pd_new(pdp_3d_push_class);

    /* super init */
    pdp_3dp_base_init(x);

    /* setup which matrix we are talking about */
    pdp_3d_push_setmatrix(x, s);

    x->x_change_mode = 0;

    return (void *)x;
}

void *pdp_3d_push_new(t_symbol *s, t_floatarg f)
{
    t_pdp_3d_push *x = (t_pdp_3d_push *)pdp_3d_push_mode_new(s);

    /* create dpd outlets */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_push_process_left, 0);
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_push_process_right, 0);

    x->x_change_mode = (f != 0.0f);
    
    return (void *)x;
}


void *pdp_3d_mode_new(t_symbol *s)
{
    t_pdp_3d_push *x = (t_pdp_3d_push *)pdp_3d_push_mode_new(s);

    /* create dpd outlets */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_mode_process_left, 0);
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_mode_process_right, 0);


    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_push_setup(void)
{


    pdp_3d_push_class = class_new(gensym("3dp_push"), (t_newmethod)pdp_3d_push_new,
    	(t_method)pdp_3d_push_free, sizeof(t_pdp_3d_push), 0, A_DEFSYMBOL, A_NULL);

    class_addcreator((t_newmethod)pdp_3d_mode_new, gensym("3dp_mode"), A_DEFSYMBOL, A_NULL);

    pdp_3dp_base_setup(pdp_3d_push_class);

}

#ifdef __cplusplus
}
#endif
