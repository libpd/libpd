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
#include "pdp_internals.h"

typedef struct pdp_snap_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    bool x_snapnext;

} t_pdp_snap;


static void pdp_snap_bang(t_pdp_snap *x)
{

  if (-1 != x->x_packet0)
    outlet_pdp(x->x_outlet0, x->x_packet0);

}




static void pdp_snap_input_1(t_pdp_snap *x, t_symbol *s, t_floatarg f)
{

    /* if this is a register_ro message or register_rw message, register with packet factory */

    /* if this is a process message, start the processing + propagate stuff to outputs */

    if (s == gensym("register_ro")){
      if(x->x_snapnext) {
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_copy_ro((int)f);
	x->x_snapnext = false;
      }
    }

}

static void pdp_snap_snap(t_pdp_snap *x)
{
  x->x_snapnext = true;
}

static void pdp_snap_free(t_pdp_snap *x)
{
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_snap_class;



void *pdp_snap_new(void)
{
    t_pdp_snap *x = (t_pdp_snap *)pd_new(pdp_snap_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pdp"), gensym("pdp1"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_snapnext = false;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_snap_setup(void)
{


    pdp_snap_class = class_new(gensym("pdp_snap"), (t_newmethod)pdp_snap_new,
    	(t_method)pdp_snap_free, sizeof(t_pdp_snap), 0, A_NULL);


    class_addmethod(pdp_snap_class, (t_method)pdp_snap_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_snap_class, (t_method)pdp_snap_snap, gensym("snap"), A_NULL);
    
    class_addmethod(pdp_snap_class, (t_method)pdp_snap_input_1, gensym("pdp1"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
