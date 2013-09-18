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


typedef struct pdp_gain_struct
{
    t_pdp_imagebase x_base;
    void *x_gain;

} t_pdp_gain;




static void pdp_gain_process(t_pdp_gain *x)
{
    int p = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);

    pdp_packet_image_set_chanmask(p, mask);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_gain_process, x->x_gain, 0, p);
    
}


static void pdp_gain_gain(t_pdp_gain *x, t_floatarg f)
{
    pdp_imageproc_gain_setgain(x->x_gain, f);
}



t_class *pdp_gain_class;



void pdp_gain_free(t_pdp_gain *x)
{
    pdp_imagebase_free(x);
    pdp_imageproc_gain_delete(x->x_gain);
}

void *pdp_gain_new(t_floatarg f)
{
    t_pdp_gain *x = (t_pdp_gain *)pd_new(pdp_gain_class);

    /* super init */
    pdp_imagebase_init(x);

    /* no arg, or zero -> gain = 1 */
    if (f==0.0f) f = 1.0f;


    /* io */
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("gain"));
    pdp_base_add_pdp_outlet(x);

    /* callbacks */
    pdp_base_set_process_method(x, (t_pdp_method)pdp_gain_process);

    x->x_gain = pdp_imageproc_gain_new();
    pdp_gain_gain(x, f);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_gain_setup(void)
{


    pdp_gain_class = class_new(gensym("pdp_gain"), (t_newmethod)pdp_gain_new,
    	(t_method)pdp_gain_free, sizeof(t_pdp_gain), 0, A_DEFFLOAT, A_NULL);

    pdp_imagebase_setup(pdp_gain_class);

    class_addmethod(pdp_gain_class, (t_method)pdp_gain_gain, gensym("gain"),  A_DEFFLOAT, A_NULL);   

}

#ifdef __cplusplus
}
#endif
