/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"

typedef struct _decide
{
    t_object      x_ob;
    unsigned int  x_seed;
} t_decide;

static t_class *decide_class;

/* the random bit algo is taken from NR (method II in 7.4) -- CHECKED */
#define RBIT1      1
#define RBIT2      2
#define RBIT5      16
#define RBIT18     131072
#define RBIT_MASK  (RBIT1 + RBIT2 + RBIT5)

static void decide_bang(t_decide *x)
{
    if (x->x_seed & RBIT18)
    {
	x->x_seed = ((x->x_seed ^ RBIT_MASK) << 1) | RBIT1;
	outlet_float(((t_object *)x)->ob_outlet, 1);
    }
    else
    {
	x->x_seed <<= 1;
	outlet_float(((t_object *)x)->ob_outlet, 0);
    }
}

static void decide_float(t_decide *x, t_float f)
{
    /* CHECKED: float loudly rejected, int (any value) same as bang */
    int i;
    if (loud_checkint((t_pd *)x, f, &i, &s_float))
	decide_bang(x);
}

static void decide_ft1(t_decide *x, t_floatarg f)
{
    int i = (int)f;  /* CHECKED */
    if (i)  /* CHECKED: negative numbers are accepted */
	x->x_seed = i;
    else
	x->x_seed = 123456789;  /* FIXME */
}

static void *decide_new(t_floatarg f)
{
    t_decide *x = (t_decide *)pd_new(decide_class);
    x->x_seed = 123456789;  /* FIXME */
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void decide_setup(void)
{
    decide_class = class_new(gensym("decide"),
			    (t_newmethod)decide_new, 0,
			    sizeof(t_decide), 0,
			    A_DEFFLOAT, 0);
    class_addbang(decide_class, decide_bang);
    class_addfloat(decide_class, decide_float);
    class_addmethod(decide_class, (t_method)decide_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    /* CHECKED list is auto-unfolded */
    /* CHECKED doesn't understand "seed" */
}
