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

typedef struct pdp_mix_struct
{
    t_pdp_imagebase x_base;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;

    void *x_mixer;
 
    int x_extrapolate;
    
} t_pdp_mix;


static void pdp_mix_process(t_pdp_mix *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_mix_process, x->x_mixer, mask, p0, p1);
}


static void pdp_mix_mix(t_pdp_mix *x, t_floatarg f)
{
    float f2;
    if (!x->x_extrapolate){
	if (f < 0.0f) f = 0.0f;
	if (f > 1.0f) f = 1.0f;
    }

    f2 = (1.0f - f);
    pdp_imageproc_mix_setleftgain(x->x_mixer, f2);
    pdp_imageproc_mix_setrightgain(x->x_mixer, f);

}

static void pdp_mix_mix1(t_pdp_mix *x, t_floatarg f)
{
    pdp_imageproc_mix_setleftgain(x->x_mixer, f);

}
static void pdp_mix_mix2(t_pdp_mix *x, t_floatarg f2)
{
    pdp_imageproc_mix_setrightgain(x->x_mixer, f2);
}

static void pdp_mix_extrapolate(t_pdp_mix *x, t_floatarg f)
{
    if (f == 0.0f) x->x_extrapolate = 0;
    if (f == 1.0f) x->x_extrapolate = 1;
}


static void pdp_mix_free(t_pdp_mix *x)
{
    pdp_imagebase_free(x);
    pdp_imageproc_mix_delete(x->x_mixer);
}

t_class *pdp_mix_class;
t_class *pdp_mix2_class;


void *pdp_mix_common_init(t_pdp_mix *x)
{
    int i;

    pdp_imagebase_init(x);
    pdp_base_add_pdp_inlet(x);
    pdp_base_add_pdp_outlet(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_mix_process);

    x->x_extrapolate = 0;
    x->x_mixer = pdp_imageproc_mix_new();
    pdp_mix_mix(x, 0.0f);

    return (void *)x;
}


void *pdp_mix_new(t_floatarg mix)
{
    t_pdp_mix *x = (t_pdp_mix *)pd_new(pdp_mix_class);
    pdp_mix_common_init(x);
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("mix"));

    if (mix == 0.0f) mix = 0.5f;
    pdp_mix_mix(x, mix);
    return (void *)x;
}

void *pdp_mix2_new(t_floatarg mix1, t_floatarg mix2)
{
    t_pdp_mix *x = (t_pdp_mix *)pd_new(pdp_mix2_class);
    pdp_mix_common_init(x);

    pdp_base_add_gen_inlet(x, gensym("float"), gensym("mix1"));
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("mix2"));

    if ((mix1 == 0.0f) && (mix2 == 0.0f)) mix1 = mix2 = 0.5f;
    pdp_mix_mix1(x, mix1);
    pdp_mix_mix2(x, mix2);
    return (void *)x;
}

#ifdef __cplusplus
extern "C"
{
#endif



void pdp_mix_setup(void)
{


    pdp_mix_class = class_new(gensym("pdp_mix"), (t_newmethod)pdp_mix_new,
    	(t_method)pdp_mix_free, sizeof(t_pdp_mix), 0, A_DEFFLOAT, A_NULL);

    pdp_imagebase_setup(pdp_mix_class);
    class_addmethod(pdp_mix_class, (t_method)pdp_mix_mix, gensym("mix"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix_class, (t_method)pdp_mix_extrapolate, gensym("extrapolate"), A_DEFFLOAT, A_NULL);




    pdp_mix2_class = class_new(gensym("pdp_mix2"), (t_newmethod)pdp_mix2_new,
    	(t_method)pdp_mix_free, sizeof(t_pdp_mix), 0, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    pdp_imagebase_setup(pdp_mix2_class);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_mix1, gensym("mix1"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_mix2, gensym("mix2"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_extrapolate, gensym("extrapolate"), A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
