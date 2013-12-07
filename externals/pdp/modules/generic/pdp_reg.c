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
#include "pdp_png.h"
#include "pdp_internals.h"


typedef struct pdp_reg_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;

} t_pdp_reg;


static void pdp_reg_load_png(t_pdp_reg *x, t_symbol *s)
{
    int packet;
    //post("sym: %s", s->s_name);
    packet = pdp_packet_bitmap_from_png_file(s->s_name);
    if (-1 == packet){
	post("pdp_reg: error loading png file %s", s->s_name);
    }
    else{
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = packet;
    }

}

static void pdp_reg_save_png(t_pdp_reg *x, t_symbol *s)
{
    int newpacket = pdp_packet_convert_ro(x->x_packet0, pdp_gensym("bitmap/*/*"));

    if (-1 == newpacket){
	post("pdp_reg: nothing to save");
	return;
    }

    if (!(pdp_packet_bitmap_save_png_file(newpacket, s->s_name))){
	post("pdp_reg: error saving png file %s", s->s_name);
    }

    pdp_packet_mark_unused(newpacket);
					  
}


static void pdp_reg_bang(t_pdp_reg *x)
{

    if (-1 != x->x_packet0) outlet_pdp(x->x_outlet0, x->x_packet0);

}



static void pdp_reg_input_0(t_pdp_reg *x, t_symbol *s, t_floatarg f)
{

    /* if this is a register_ro message or register_rw message, register with packet factory */

    /* if this is a process message, start the processing + propagate stuff to outputs */

    if (s == gensym("register_ro")){
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_copy_ro((int)f);
	//post("in0ro: requested %d, got %d", (int)f, x->x_packet0);
    }
    else if (s == gensym("process")){
	pdp_reg_bang(x);

    }


}


static void pdp_reg_input_1(t_pdp_reg *x, t_symbol *s, t_floatarg f)
{

    /* if this is a register_ro message or register_rw message, register with packet factory */

    /* if this is a process message, start the processing + propagate stuff to outputs */

    if (s == gensym("register_ro")){
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_copy_ro((int)f);
	//post("in0ro: requested %d, got %d", (int)f, x->x_packet0);
    }

}



static void pdp_reg_free(t_pdp_reg *x)
{
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_reg_class;



void *pdp_reg_new(void)
{
    t_pdp_reg *x = (t_pdp_reg *)pd_new(pdp_reg_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pdp"), gensym("pdp1"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_reg_setup(void)
{


    pdp_reg_class = class_new(gensym("pdp_reg"), (t_newmethod)pdp_reg_new,
    	(t_method)pdp_reg_free, sizeof(t_pdp_reg), 0, A_NULL);


    class_addmethod(pdp_reg_class, (t_method)pdp_reg_bang, gensym("bang"), A_NULL);
    
    class_addmethod(pdp_reg_class, (t_method)pdp_reg_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_reg_class, (t_method)pdp_reg_input_1, gensym("pdp1"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_reg_class, (t_method)pdp_reg_save_png, gensym("save_png"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_reg_class, (t_method)pdp_reg_load_png, gensym("load_png"), A_SYMBOL, A_NULL);

}

#ifdef __cplusplus
}
#endif
