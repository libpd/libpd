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


typedef struct pdp_convert_struct
{
    t_object x_obj;
    t_symbol *x_type_mask;
    t_outlet *x_outlet0;
    int x_packet0;

} t_pdp_convert;



static void pdp_convert_type_mask(t_pdp_convert *x, t_symbol *s)
{
    x->x_type_mask = s;
}

static void pdp_convert_input_0(t_pdp_convert *x, t_symbol *s, t_floatarg f)
{
    int p = (int)f;
    int passes, i;

    if (s== gensym("register_ro")){
	pdp_packet_mark_unused(x->x_packet0);
	if (x->x_type_mask->s_name[0])
	    x->x_packet0 = pdp_packet_convert_ro(p, pdp_gensym(x->x_type_mask->s_name));
	else
	    x->x_packet0 = pdp_packet_copy_ro(p);
    }


    if ((s == gensym("process")) && (-1 != x->x_packet0)){
	pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);
    }
}


t_class *pdp_convert_class;



void pdp_convert_free(t_pdp_convert *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}

void *pdp_convert_new(t_symbol *s)
{
    t_pdp_convert *x = (t_pdp_convert *)pd_new(pdp_convert_class);

    x->x_type_mask = s;
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet0 = -1;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_convert_setup(void)
{


    pdp_convert_class = class_new(gensym("pdp_convert"), (t_newmethod)pdp_convert_new,
    	(t_method)pdp_convert_free, sizeof(t_pdp_convert), 0, A_DEFSYMBOL, A_NULL);


    class_addmethod(pdp_convert_class, (t_method)pdp_convert_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addsymbol(pdp_convert_class, (t_method)pdp_convert_type_mask);

}

#ifdef __cplusplus
}
#endif
