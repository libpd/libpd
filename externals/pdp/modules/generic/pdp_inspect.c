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

/* adapted from the pd trigger object */

#define TR_BANG 0
#define TR_FLOAT 1
#define TR_SYMBOL 2
#define TR_POINTER 3
#define TR_LIST 4
#define TR_ANYTHING 5
#define TR_PDP 6

/*

$$$TODO: emplement so that it behaves like the standard trigger object

i.e. [trigger bang pdp pdp bang pdp]

register_ro and register_rw messages pass right trough,
since they're not action events, only configure events.
a bang is made equivalent to a process event.

*/

typedef struct pdp_inspect_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet;

} t_pdp_inspect;




static void pdp_inspect_input_0(t_pdp_inspect *x, t_symbol *s, t_floatarg f)
{
    t_atom atom[2];
    t_symbol *pdp = gensym("pdp");
    t_symbol *prc = gensym("process");
    t_symbol *rro = gensym("register_ro");
    int i;


    /* if there is a reg_ro, shortcut the right outlet */
    if (s == rro){
	SETSYMBOL(atom+0, s);
	SETFLOAT(atom+1, f);
	outlet_anything(x->x_outlet, pdp, 2, atom);
	SETSYMBOL(atom+0, prc);
	outlet_anything(x->x_outlet, pdp, 1, atom);
    }

    
}



static void pdp_inspect_free(t_pdp_inspect *x)
{

}

t_class *pdp_inspect_class;



static void *pdp_inspect_new(void)
{
    t_pdp_inspect *x = (t_pdp_inspect *)pd_new(pdp_inspect_class);

    
    x->x_outlet = outlet_new(&x->x_obj, &s_anything);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_inspect_setup(void)
{


    pdp_inspect_class = class_new(gensym("pdp_inspect_ro"), (t_newmethod)pdp_inspect_new,
    	(t_method)pdp_inspect_free, sizeof(t_pdp_inspect), 0, A_GIMME, A_NULL);

    class_addcreator((t_newmethod)pdp_inspect_new, gensym("pdp_t"), A_GIMME, 0);
    
    class_addmethod(pdp_inspect_class, (t_method)pdp_inspect_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
