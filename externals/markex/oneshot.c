/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- oneshot ------------------------------ */

/* instance structure */
static t_class *oneshot_class;

typedef struct _oneshot
{
	t_object    x_obj;	        /* obligatory object header */
	int	        t_armed;	    /* if willing to pass data */
	t_outlet    *t_out1;	    /* the outlet */
} t_oneshot;

static void oneshot_float(t_oneshot *x, t_floatarg n)
{
    if (x->t_armed)
    {
	    outlet_float(x->t_out1, n);
	    x->t_armed = 0;
    }
}

static void oneshot_bang(t_oneshot *x)
{
    if (x->t_armed)
    {
	    outlet_bang(x->t_out1);
	    x->t_armed = 0;
    }
}

static void oneshot_clear(t_oneshot *x)
{
    x->t_armed = 1;
}

static void *oneshot_new(t_symbol *s) /* init vals in struc */
{
    t_oneshot *x = (t_oneshot *)pd_new(oneshot_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    x->t_armed = 1;
    return (x);
}

void oneshot_setup(void)
{
    oneshot_class = class_new(gensym("oneshot"), (t_newmethod)oneshot_new, 0,
    	    	    	sizeof(t_oneshot), 0, A_NULL);
    class_addfloat(oneshot_class, (t_method)oneshot_float);
    class_addbang(oneshot_class, (t_method)oneshot_bang);
    class_addmethod(oneshot_class, (t_method)oneshot_clear, gensym("clear"), A_NULL);
#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(oneshot_class, gensym("oneshot-help.pd"));
#endif
}

