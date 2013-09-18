/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- alternate ------------------------------ */

/* instance structure */

static t_class *alternate_class;

typedef struct _alternate
{
	t_object    x_obj;	        /* obligatory object header */
	int	        a_which;	    /* which outlet to go out 0 = left, 1 = right    */
	t_outlet    *t_out1;	    /* the outlet */
	t_outlet    *t_out2;	    /* the other outlet */
} t_alternate;

static void alternate_list(t_alternate *x, t_symbol *s, int argc, t_atom *argv)
{
	static t_symbol *listSym = NULL;

	if (!listSym)
		listSym = gensym("list");

    if (!x->a_which) outlet_list(x->t_out1, listSym, argc, argv);
    else outlet_list(x->t_out2, listSym, argc, argv);
    x->a_which++;
    x->a_which = (x->a_which > 1) ? 0 : x->a_which;
}

static void alternate_float(t_alternate *x, t_floatarg n)
{
    if (!x->a_which) outlet_float(x->t_out1, n);
    else outlet_float(x->t_out2, n);
    x->a_which++;
    x->a_which = (x->a_which > 1) ? 0 : x->a_which;
}

static void alternate_bang(t_alternate *x)
{
	if (!x->a_which) outlet_bang(x->t_out1);
	else outlet_bang(x->t_out2);
	x->a_which++;
	x->a_which = (x->a_which > 1) ? 0 : x->a_which;
}

static void alternate_reset(t_alternate *x)
{
    x->a_which = 0;
}

static void *alternate_new(t_symbol *s) /* init vals in struc */
{
    t_alternate *x = (t_alternate *)pd_new(alternate_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    x->t_out2 = outlet_new(&x->x_obj, 0);
    x->a_which = 0;
    return (x);
}

void alternate_setup(void)
{
    alternate_class = class_new(gensym("alternate"), (t_newmethod)alternate_new, 0,
    	    	    	sizeof(t_alternate), 0, A_NULL);
    class_addfloat(alternate_class, (t_method)alternate_float);
    class_addbang(alternate_class, (t_method)alternate_bang);
    class_addmethod(alternate_class, (t_method)alternate_reset, gensym("reset"), A_NULL);
    class_addmethod(alternate_class, (t_method)alternate_list,
    	    gensym("list"), A_GIMME, A_NULL); 

#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(alternate_class, gensym("alternate-help.pd"));
#endif
}

