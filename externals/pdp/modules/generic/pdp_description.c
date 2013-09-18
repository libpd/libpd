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



typedef struct pdp_description_struct
{
    t_object x_obj;
    t_outlet *x_outlet;

} t_pdp_description;




static void pdp_description_input_pdp(t_pdp_description *x, t_symbol *s, t_floatarg f)
{
    int p = (int)f;
    t_symbol *rro = S_REGISTER_RO;

    if (rro == s){
	outlet_symbol(x->x_outlet, gensym(pdp_packet_get_description(p)->s_name));
    }
}

static void pdp_description_input_dpd(t_pdp_description *x, t_symbol *s, t_floatarg f)
{
    int p = (int)f;
    t_symbol *ins = S_INSPECT;

    if (ins == s){
	outlet_symbol(x->x_outlet, gensym(pdp_packet_get_description(p)->s_name));
    }
}


static void pdp_description_free(t_pdp_description *x)
{

}

t_class *pdp_description_class;



static void *pdp_description_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pdp_description *x = (t_pdp_description *)pd_new(pdp_description_class);

    x->x_outlet = outlet_new(&x->x_obj, &s_symbol);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_description_setup(void)
{


    pdp_description_class = class_new(gensym("pdp_description"), (t_newmethod)pdp_description_new,
    	(t_method)pdp_description_free, sizeof(t_pdp_description), 0, A_NULL);

    class_addmethod(pdp_description_class, (t_method)pdp_description_input_pdp, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_description_class, (t_method)pdp_description_input_dpd, gensym("dpd"),  A_SYMBOL, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
