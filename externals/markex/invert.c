/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- invert ------------------------------ */

/* instance structure */

static t_class *invert_class;

typedef struct _invert
{
	t_object    x_obj;			/* obligatory object header */
	t_outlet    *t_out1;	    /* the outlet */
} t_invert;

static void invert_float(t_invert *x, t_floatarg n)
{
    if (n) outlet_float(x->t_out1, 0.f);
    else outlet_float(x->t_out1, 1.f);
}

static void *invert_new(t_symbol *s) /* init vals in struc */
{
    t_invert *x = (t_invert *)pd_new(invert_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    return(x);
}

void invert_setup(void)
{
    invert_class = class_new(gensym("invert"), (t_newmethod)invert_new, 0,
    	    	    	sizeof(t_invert), 0, A_NULL);
    class_addfloat(invert_class, (t_method)invert_float);

#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(invert_class, gensym("invert-help.pd"));
#endif
}
