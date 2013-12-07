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

/* a for loop for 3dp packets
   this can later be adapted to a for loop for dpd packets. */

#include "pdp_opengl.h"
#include "pdp_internals.h"


typedef struct pdp_3d_for_struct
{
    t_object x_obj;

    t_int x_count;

    t_outlet *x_outlet_dpd;
    t_outlet *x_outlet_float;

} t_pdp_3d_for;



static void pdp_3d_for_input_0(t_pdp_3d_for *x, t_symbol *s, t_floatarg f)
{
    int i;

    /* trigger on "accumulate" */

    if (s == gensym("accumulate")){
	for (i=0; i<x->x_count; i++){
	    outlet_float(x->x_outlet_float, (float)i);
	    outlet_dpd(x->x_outlet_dpd, (int)f);
	}
    }
}

static void pdp_3d_for_count(t_pdp_3d_for *x, t_floatarg f)
{
    int count = (int)f;
    if (count >= 0) x->x_count = count;
}


static void pdp_3d_for_free(t_pdp_3d_for *x)
{
}

t_class *pdp_3d_for_class;



void *pdp_3d_for_new(t_floatarg f)
{
    int count = (int)f;

    t_pdp_3d_for *x = (t_pdp_3d_for *)pd_new(pdp_3d_for_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("count"));

    x->x_outlet_dpd = outlet_new(&x->x_obj, &s_anything); 
    x->x_outlet_float = outlet_new(&x->x_obj, &s_float); 
    x->x_count = (count > 0) ? count : 1;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_for_setup(void)
{


    pdp_3d_for_class = class_new(gensym("3dp_for"), (t_newmethod)pdp_3d_for_new,
    	(t_method)pdp_3d_for_free, sizeof(t_pdp_3d_for), 0, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_3d_for_class, (t_method)pdp_3d_for_input_0, gensym("dpd"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_3d_for_class, (t_method)pdp_3d_for_count, gensym("count"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
