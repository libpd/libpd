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
#include "pdp_imagebase.h"


typedef struct pdp_constant_struct
{
    t_pdp_imagebase x_base;

    t_outlet *x_outlet0;

    int x_packet0;

    t_symbol *x_type;

    unsigned int x_width;
    unsigned int x_height;

    void *x_constant;
 
} t_pdp_constant;



void pdp_constant_type(t_pdp_constant *x, t_symbol *s)
{
    x->x_type = s;
}


void pdp_constant_value(t_pdp_constant *x, t_floatarg f)
{
    if (f>1.0f) f = 1.0f;
    if (f<-1.0f) f = -1.0f;

    x->x_constant = (void *)((sptr)(0x7fff * f));
}



static void pdp_constant_process(t_pdp_constant *x)
{
    /* get channel mask */
    u32 mask = pdp_imagebase_get_chanmask(x);

    /* create new packet */
    if (x->x_type == gensym("yv12")){x->x_packet0 = pdp_packet_new_image_YCrCb(x->x_width, x->x_height);}
    else if (x->x_type == gensym("grey")){x->x_packet0 = pdp_packet_new_image_grey(x->x_width, x->x_height);}
    else return;

    /* this processes the packets using a pdp image processor */
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_constant_process, x->x_constant, mask, x->x_packet0);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_constant_process, 0, ~mask, x->x_packet0);

    return;
}

static void pdp_constant_postproc(t_pdp_constant *x)
{
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_constant_bang(t_pdp_constant *x)
{
    pdp_base_bang(x);
}

static void pdp_constant_dim(t_pdp_constant *x, t_floatarg w, t_floatarg h)
{
    x->x_width  = pdp_imageproc_legalwidth((int)w);
    x->x_height = pdp_imageproc_legalheight((int)h);
    //post("dims %d %d",  x->x_width, x->x_height);
}


static void pdp_constant_free(t_pdp_constant *x)
{
    pdp_imagebase_free(x);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_constant_class;



void *pdp_constant_new(void)
{
    int i;
    t_pdp_constant *x = (t_pdp_constant *)pd_new(pdp_constant_class);

    /* super init */
    pdp_imagebase_init(x);

    /* in/out*/
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("value"));
    x->x_outlet0 = pdp_base_add_pdp_outlet(x);

    /* base callbacks */
    pdp_base_disable_active_inlet(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_constant_process);
    pdp_base_set_postproc_method(x, (t_pdp_method)pdp_constant_postproc);

    /* data init */
    x->x_packet0 = -1;
    pdp_constant_dim(x, 320, 240);
    pdp_constant_value(x, 0.0f);
    pdp_constant_type(x, gensym("yv12"));


    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_constant_setup(void)
{


    pdp_constant_class = class_new(gensym("pdp_constant"), (t_newmethod)pdp_constant_new,
    	(t_method)pdp_constant_free, sizeof(t_pdp_constant), 0, A_NULL);

    pdp_imagebase_setup(pdp_constant_class);

    class_addmethod(pdp_constant_class, (t_method)pdp_constant_value, gensym("value"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_constant_class, (t_method)pdp_constant_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_constant_class, (t_method)pdp_constant_dim, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_constant_class, (t_method)pdp_constant_bang, gensym("bang"), A_NULL);

}

#ifdef __cplusplus
}
#endif
