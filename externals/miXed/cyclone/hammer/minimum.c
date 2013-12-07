/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "common/fitter.h"

#define MINIMUM_C74MAXITEMS  256

typedef struct _minimum
{
    t_object  x_ob;
    t_float   x_last;
    t_float   x_test;
} t_minimum;

static t_class *minimum_class;

static void minimum_bang(t_minimum *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_last);
}

static void minimum_float(t_minimum *x, t_float f)
{
    outlet_float(((t_object *)x)->ob_outlet,
		 x->x_last = (f < x->x_test ? f : x->x_test));
}

static void minimum_list(t_minimum *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac > MINIMUM_C74MAXITEMS)
	fittermax_rangewarning(*(t_pd *)x, MINIMUM_C74MAXITEMS, "items");
    while (ac && av->a_type != A_FLOAT) ac--, av++;  /* CHECKME (a warning?) */
    if (ac)
    {
	t_float fpick = av->a_w.w_float;
	ac--; av++;
	while (ac && av->a_type != A_FLOAT) ac--, av++;  /* CHECKME */
	if (ac)
	{
	    t_float fnext, f = av->a_w.w_float;
	    if (f < fpick)
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
		    if (f < fpick)
		    {
			fnext = fpick;
			fpick = f;
		    }
		    else if (f < fnext) fnext = f;
		}
		/* CHECKME else */
		av++;
	    }
	    x->x_test = fnext;
	    outlet_float(((t_object *)x)->ob_outlet, x->x_last = fpick);
	}
	else minimum_float(x, fpick);  /* CHECKME */
    }
    /* CHECKME else */
}

static void *minimum_new(t_floatarg f)
{
    t_minimum *x = (t_minimum *)pd_new(minimum_class);
    x->x_last = 0;  /* CHECKME */
    x->x_test = f;
    floatinlet_new((t_object *)x, &x->x_test);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void minimum_setup(void)
{
    minimum_class = class_new(gensym("minimum"),
			      (t_newmethod)minimum_new, 0,
			      sizeof(t_minimum), 0, A_DEFFLOAT, 0);
    class_addbang(minimum_class, minimum_bang);
    class_addfloat(minimum_class, minimum_float);
    class_addlist(minimum_class, minimum_list);
    fitter_setup(minimum_class, 0);
}
