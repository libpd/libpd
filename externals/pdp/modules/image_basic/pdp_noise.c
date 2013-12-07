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


typedef struct pdp_noise_struct
{
    t_pdp_imagebase x_base;

    int x_packet0;
    t_outlet *x_outlet0;
    void *x_noisegen;
    
    t_symbol *x_type;

    unsigned int x_width;
    unsigned int x_height;
 
} t_pdp_noise;



void pdp_noise_type(t_pdp_noise *x, t_symbol *s)
{
    x->x_type = s;
}


void pdp_noise_random(t_pdp_noise *x, t_floatarg seed)
{
    if (seed == 0.0f) seed = (float)random();
    pdp_imageproc_random_setseed(x->x_noisegen, seed);

}

/* called inside pdp thread */
static void pdp_noise_process(t_pdp_noise *x)
{
    /* seed the 16 bit rng with a new random number from the clib */
    pdp_noise_random(x, 0.0f);

    /* create new packet */
    if      (x->x_type == gensym("grey")) {
	x->x_packet0 = pdp_packet_new_image_grey(x->x_width, x->x_height);
    }
    else if (x->x_type == gensym("yv12")) {
	x->x_packet0 = pdp_packet_new_image_YCrCb(x->x_width, x->x_height);
    }
    else return;

    /* call the image processor */
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_random_process, x->x_noisegen, 
				-1, x->x_packet0);
}

/* called inside pd thread: involves an outlet */
static void pdp_noise_postproc(t_pdp_noise *x)
{
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_noise_bang(t_pdp_noise *x)
{
    pdp_base_bang(x);
}

static void pdp_noise_dim(t_pdp_noise *x, t_floatarg w, t_floatarg h)
{
    x->x_width  = pdp_imageproc_legalwidth((int)w);
    x->x_height = pdp_imageproc_legalheight((int)h);
    //post("dims %d %d",  x->x_width, x->x_height);
}


static void pdp_noise_free(t_pdp_noise *x)
{
    pdp_imagebase_free(x);

    /* tidy up */
    pdp_packet_mark_unused(x->x_packet0);
    pdp_imageproc_random_delete(x->x_noisegen);
}

t_class *pdp_noise_class;


void *pdp_noise_new(void)
{
    int i;

    t_pdp_noise *x = (t_pdp_noise *)pd_new(pdp_noise_class);

    pdp_imagebase_init(x);
    pdp_base_disable_active_inlet(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_noise_process);
    pdp_base_set_postproc_method(x, (t_pdp_method)pdp_noise_postproc);

    x->x_outlet0 = pdp_base_add_pdp_outlet(x);
    x->x_packet0 = -1;

    x->x_width = 320;
    x->x_height = 240;

    x->x_noisegen = pdp_imageproc_random_new();

    pdp_noise_random(x, 0.0f);
    pdp_noise_type(x, gensym("yv12"));

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_noise_setup(void)
{


    pdp_noise_class = class_new(gensym("pdp_noise"), (t_newmethod)pdp_noise_new,
    	(t_method)pdp_noise_free, sizeof(t_pdp_noise), 0, A_NULL);

    pdp_imagebase_setup(pdp_noise_class);

    class_addmethod(pdp_noise_class, (t_method)pdp_noise_random, gensym("seed"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_noise_class, (t_method)pdp_noise_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_noise_class, (t_method)pdp_noise_dim, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_noise_class, (t_method)pdp_noise_bang, gensym("bang"), A_NULL);

}

#ifdef __cplusplus
}
#endif
