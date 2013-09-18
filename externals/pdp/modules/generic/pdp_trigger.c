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

typedef struct triggerout
{
    int u_type;         /* outlet type from above */
    t_outlet *u_outlet;
} t_triggerout;


typedef struct pdp_trigger_struct
{
    t_object x_obj;
    t_float x_f;

    int x_n;
    t_triggerout *x_vec;

} t_pdp_trigger;




static void pdp_trigger_input_pdp(t_pdp_trigger *x, t_symbol *s, t_floatarg f)
{
    t_atom atom[2];
    t_symbol *pdp = S_PDP;
    t_symbol *prc = S_PROCESS;
    t_triggerout *u;
    int i;

    for (i = x->x_n, u = x->x_vec + i; u--, i--;){
	/* trigger bang outlet only when a process event is recieved */
	if ((u->u_type == TR_BANG) && (s == prc)){
	    outlet_bang(u->u_outlet);
	}
	/* just pass the message if it is a pdp outlet */
	if ((u->u_type) == TR_PDP){
	    SETSYMBOL(atom+0, s);
	    SETFLOAT(atom+1, f);
	    if (s == prc) outlet_anything(u->u_outlet, pdp, 1, atom);
	    else outlet_anything(u->u_outlet, pdp, 2, atom);
	    
	}
    }
    
}

static void pdp_trigger_input_dpd(t_pdp_trigger *x, t_symbol *s, t_floatarg f)
{
    t_atom atom[2];
    t_symbol *dpd = S_DPD;
    t_symbol *acc = S_ACCUMULATE;
    t_triggerout *u;
    int i;
    int p = (int)f;

    for (i = x->x_n, u = x->x_vec + i; u--, i--;){
	/* trigger outlet only when an accumulate event is recieved */
	if (s == acc){

	    /* output bang */
	    if (u->u_type == TR_BANG) outlet_bang(u->u_outlet);

	    /* output a complete dpd message if it is a pdp outlet */
	    if ((u->u_type) == TR_PDP){
		outlet_dpd(u->u_outlet, p);
	    }
	}
    }
    
}


static void pdp_trigger_free(t_pdp_trigger *x)
{
    pdp_dealloc(x->x_vec);
}

t_class *pdp_trigger_class;



static void *pdp_trigger_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pdp_trigger *x = (t_pdp_trigger *)pd_new(pdp_trigger_class);
    t_atom defarg[2], *ap;
    t_triggerout *u;
    int i;


    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SETSYMBOL(&defarg[0], gensym("pdp"));
        SETSYMBOL(&defarg[1], gensym("bang"));
    }

    x->x_n = argc;
    x->x_vec = pdp_alloc(argc * sizeof(*x->x_vec));

    for (i = 0, ap = argv, u = x->x_vec; i < argc; u++, ap++, i++)
    {
        t_atomtype thistype = ap->a_type;
        char c;
        if (thistype == TR_SYMBOL) c = ap->a_w.w_symbol->s_name[0];
        else c = 0;
        if (c == 'p')
            u->u_type = TR_PDP,
                u->u_outlet = outlet_new(&x->x_obj, &s_anything);
        else if (c == 'b')
            u->u_type = TR_BANG, u->u_outlet = outlet_new(&x->x_obj, &s_bang);
        else
        {
            pd_error(x, "pdp_trigger: %s: bad type", ap->a_w.w_symbol->s_name);
            u->u_type = TR_BANG, u->u_outlet = outlet_new(&x->x_obj, &s_bang);
        }
    }

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_trigger_setup(void)
{


    pdp_trigger_class = class_new(gensym("pdp_trigger"), (t_newmethod)pdp_trigger_new,
    	(t_method)pdp_trigger_free, sizeof(t_pdp_trigger), 0, A_GIMME, A_NULL);

    class_addcreator((t_newmethod)pdp_trigger_new, gensym("pdp_t"), A_GIMME, 0);
    
    class_addmethod(pdp_trigger_class, (t_method)pdp_trigger_input_pdp, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_trigger_class, (t_method)pdp_trigger_input_dpd, gensym("dpd"),  A_SYMBOL, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
