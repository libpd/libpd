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



#include "pdp.h"
#include "pdp_base.h"
#include "pdp_opengl.h"


typedef struct pdp_3d_context_struct
{
    t_pdp_base x_base;

    t_outlet *x_outlet0;

    int x_packet0;

    t_symbol *x_type;

    unsigned int x_width;
    unsigned int x_height;

    void *x_constant;
 
} t_pdp_3d_context;





static void pdp_3d_context_preproc(t_pdp_3d_context *x)
{
    int p;
    int i;

    /* create new packet */
    p = pdp_packet_new_pbuf(x->x_width, x->x_height, 0);
    x->x_packet0 = p;

    if (-1 == p) return;
    
    pdp_pbuf_set_rendering_context(p);
    pdp_pbuf_setup_3d_context(p);

    /* clear buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);


    /* disable everything that is enabled in other modules */
    glDisable(GL_LIGHTING);
    for (i=0; i<8; i++) glDisable(GL_LIGHT0 + i);
    glDisable(GL_COLOR_MATERIAL);


}

static void pdp_3d_context_process(t_pdp_3d_context *x)
{
}

static void pdp_3d_context_postproc(t_pdp_3d_context *x)
{
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_3d_context_bang(t_pdp_3d_context *x)
{
    pdp_base_bang(x);
}

static void pdp_3d_context_dim(t_pdp_3d_context *x, t_floatarg w, t_floatarg h)
{
    x->x_width  = pdp_imageproc_legalwidth((int)w);
    x->x_height = pdp_imageproc_legalheight((int)h);
    //post("dims %d %d",  x->x_width, x->x_height);
}


static void pdp_3d_context_free(t_pdp_3d_context *x)
{
    pdp_base_free(x);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_3d_context_class;



void *pdp_3d_context_new(void)
{
    int i;
    t_pdp_3d_context *x = (t_pdp_3d_context *)pd_new(pdp_3d_context_class);

    /* super init */
    pdp_base_init(x);

    /* in/out*/
    x->x_outlet0 = pdp_base_add_pdp_outlet(x);

    /* base callbacks */
    pdp_base_disable_active_inlet(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_3d_context_process);
    pdp_base_set_preproc_method(x, (t_pdp_method)pdp_3d_context_preproc);
    pdp_base_set_postproc_method(x, (t_pdp_method)pdp_3d_context_postproc);

    /* data init */
    x->x_packet0 = -1;
    pdp_3d_context_dim(x, 320, 240);


    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_3d_context_setup(void)
{


    pdp_3d_context_class = class_new(gensym("pdp_3d_context"), (t_newmethod)pdp_3d_context_new,
    	(t_method)pdp_3d_context_free, sizeof(t_pdp_3d_context), 0, A_NULL);
    class_addcreator((t_newmethod)pdp_3d_context_new, gensym("3dp_context"), A_NULL);

    pdp_base_setup(pdp_3d_context_class);

    class_addmethod(pdp_3d_context_class, (t_method)pdp_3d_context_dim, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_3d_context_class, (t_method)pdp_3d_context_bang, gensym("bang"), A_NULL);

}

#ifdef __cplusplus
}
#endif
