/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "common/fitter.h"

#define MAXIMUM_C74MAXITEMS  256

typedef struct _maximum
{
    t_object  x_ob;
    t_float   x_last;
    t_float   x_test;
} t_maximum;

static t_class *maximum_class;

static void maximum_bang(t_maximum *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_last);
}

static void maximum_float(t_maximum *x, t_float f)
{
    outlet_float(((t_object *)x)->ob_outlet,
		 x->x_last = (f > x->x_test ? f : x->x_test));
}

static void maximum_list(t_maximum *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac > MAXIMUM_C74MAXITEMS)
	fittermax_rangewarning(*(t_pd *)x, MAXIMUM_C74MAXITEMS, "items");
    while (ac && av->a_type != A_FLOAT) ac--, av++;  /* CHECKME (a warning?) */
    if (ac)
    {
	t_float fpick = av->a_w.w_float;
	ac--; av++;
	while (ac && av->a_type != A_FLOAT) ac--, av++;  /* CHECKME */
	if (ac)
	{
	    t_float fnext, f = av->a_w.w_float;
	    if (f > fpick)
	    {
		fnext = fpick;
		fpick = f;
	    }
	    else fnext = f;
	    ac--; av++;
	    while (ac--)
	    {
		if (av->a_type == A_FLOAT)
		{
		    f = av->a_w.w_float;
		    if (f > fpick)
		    {
			fnext = fpick;
			fpick = f;
		    }
		    else if (f > fnext) fnext = f;
		}
		/* CHECKME else */
		av++;
	    }
	    x->x_test = fnext;
	    outlet_float(((t_object *)x)->ob_outlet, x->x_last = fpick);
	}
	else maximum_float(x, fpick);  /* CHECKME */
    }
    /* CHECKME else */
}

static void *maximum_new(t_floatarg f)
{
    t_maximum *x = (t_maximum *)pd_new(maximum_class);
    x->x_last = 0;  /* CHECKME */
    x->x_test = f;
    floatinlet_new((t_object *)x, &x->x_test);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void maximum_setup(void)
{
    maximum_class = class_new(gensym("maximum"),
			      (t_newmethod)maximum_new, 0,
			      sizeof(t_maximum), 0, A_DEFFLOAT, 0);
    class_addbang(maximum_class, maximum_bang);
    class_addfloat(maximum_class, maximum_float);
    class_addlist(maximum_class, maximum_list);
    fitter_setup(maximum_class, 0);
}
