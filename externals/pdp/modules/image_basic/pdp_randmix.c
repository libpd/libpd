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

typedef struct pdp_randmix_struct
{
    t_pdp_imagebase x_base;
    void *x_randmixer;
 
} t_pdp_randmix;


void pdp_randmix_random(t_pdp_randmix *x, t_floatarg seed)
{
    pdp_imageproc_randmix_setseed(x->x_randmixer, seed);
}


static void pdp_randmix_process(t_pdp_randmix *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_randmix_process, x->x_randmixer, mask, p0, p1);

}


static void pdp_randmix_threshold(t_pdp_randmix *x, t_floatarg f)
{
    pdp_imageproc_randmix_setthreshold(x->x_randmixer, f);

}



static void pdp_randmix_free(t_pdp_randmix *x)
{
    pdp_imagebase_free(x);
    pdp_imageproc_randmix_delete(x->x_randmixer);
}

t_class *pdp_randmix_class;


void *pdp_randmix_new(void)
{
    int i;

    t_pdp_randmix *x = (t_pdp_randmix *)pd_new(pdp_randmix_class);

    pdp_imagebase_init(x);
    pdp_base_add_pdp_inlet(x);
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("threshold"));

    pdp_base_set_process_method(x, (t_pdp_method)pdp_randmix_process);
    
    pdp_base_add_pdp_outlet(x); 
    x->x_randmixer = pdp_imageproc_randmix_new();

    pdp_randmix_threshold(x, 0.5f);
    pdp_randmix_random(x, 0.0f);

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_randmix_setup(void)
{


    pdp_randmix_class = class_new(gensym("pdp_randmix"), (t_newmethod)pdp_randmix_new,
    	(t_method)pdp_randmix_free, sizeof(t_pdp_randmix), 0, A_NULL);

    pdp_imagebase_setup(pdp_randmix_class);

    class_addmethod(pdp_randmix_class, (t_method)pdp_randmix_threshold, gensym("threshold"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_randmix_class, (t_method)pdp_randmix_random, gensym("seed"), A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
