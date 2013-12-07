/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"

#define SPRAY_MINOUTS  1
/* CHECKED: no upper limit */
#define SPRAY_DEFOUTS  2

typedef struct _spray
{
    t_object    x_ob;
    int         x_offset;
    int         x_nouts;
    t_outlet  **x_outs;
} t_spray;

static t_class *spray_class;

static void spray_float(t_spray *x, t_float f)
{
    /* CHECKED: floats ignored (LATER rethink), ints loudly rejected */
    if (f == (int)f) loud_error((t_pd *)x, "requires list");
}

/* LATER decide, whether float in first atom is to be truncated,
   or causing a list to be ignored as in max (CHECKED) */
static void spray_list(t_spray *x, t_symbol *s, int ac, t_atom *av)
{
    int ndx;
    if (ac >= 2 && av->a_type == A_FLOAT
	/* CHECKED: lists with negative effective ndx are ignored */
	&& (ndx = (int)av->a_w.w_float - x->x_offset) >= 0
	&& ndx < x->x_nouts)
    {
	/* CHECKED: ignored atoms (symbols and floats) are counted */
	/* CHECKED: we must spray in right-to-left order */
	t_atom *argp;
	t_outlet **outp;
	int last = ac - 1 + ndx;  /* ndx of last outlet filled (first is 1) */
	if (last > x->x_nouts)
	{
	    argp = av + 1 + x->x_nouts - ndx;
	    outp = x->x_outs + x->x_nouts;
	}
	else
	{
	    argp = av + ac;
	    outp = x->x_outs + last;
	}
	/* argp/outp now point to one after the first atom/outlet to deliver */
	for (argp--, outp--; argp > av; argp--, outp--)
	    if (argp->a_type == A_FLOAT)
		outlet_float(*outp, argp->a_w.w_float);
    }
}

static void spray_free(t_spray *x)
{
    if (x->x_outs)
	freebytes(x->x_outs, x->x_nouts * sizeof(*x->x_outs));
}

static void *spray_new(t_floatarg f1, t_floatarg f2)
{
    t_spray *x;
    int i, nouts = (int)f1;
    t_outlet **outs;
    if (nouts < SPRAY_MINOUTS)
        nouts = SPRAY_DEFOUTS;
    if (!(outs = (t_outlet **)getbytes(nouts * sizeof(*outs))))
	return (0);
    x = (t_spray *)pd_new(spray_class);
    x->x_nouts = nouts;
    x->x_outs = outs;
    x->x_offset = (int)f2;
    for (i = 0; i < nouts; i++)
        x->x_outs[i] = outlet_new((t_object *)x, &s_float);
    return (x);
}

void spray_setup(void)
{
    spray_class = class_new(gensym("spray"),
			    (t_newmethod)spray_new,
			    (t_method)spray_free,
			    sizeof(t_spray), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    /* CHECKED: bang, symbol, anything -- ``doesn't understand'' */
    class_addfloat(spray_class, spray_float);
    class_addlist(spray_class, spray_list);
}
