/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKED:
    bang for a float at > (refman error: >=)
    bang for a list if all >= (refman page says the same)
    bang for a list if any is >, even if the rest (but not previous) is <
    well...
*/

#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/fitter.h"

#define PAST_C74MAXSIZE  8  /* CHECKED */

typedef struct past
{
    t_object  x_ob;
    int       x_low;
    int       x_size;     /* as allocated */
    int       x_nthresh;  /* as used */
    t_atom   *x_thresh;
    t_atom    x_thrini[PAST_C74MAXSIZE];
} t_past;

static t_class *past_class;

static int past_compare(t_past *x, t_float f, t_atom *ap)
{
    if (ap->a_type == A_FLOAT)
    {
	if (f > ap->a_w.w_float)
	    return (1);
	else if (f == ap->a_w.w_float)
	    return (0);
	else
	    return (-1);
    }
    else  /* CHECKED */
    {
	if (f > 0.)
	    return (1);
	else if (f == 0.)
	    return (0);
	else
	    return (-1);
    }
}

static void past_float(t_past *x, t_float f)
{
    if (x->x_nthresh == 1)
    {
	if (past_compare(x, f, x->x_thresh) > 0)  /* CHECKED: equal is low */
	{
	    if (x->x_low)
	    {
		x->x_low = 0;
		outlet_bang(((t_object *)x)->ob_outlet);
	    }
	}
	else x->x_low = 1;
    }
    else if (past_compare(x, f, x->x_thresh) < 0) x->x_low = 1;
}

/* CHECKME: x_low handling */
static void past_list(t_past *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && ac <= x->x_nthresh)
    {
	int result;
	t_atom *vp = x->x_thresh;
	if (av->a_type == A_FLOAT
	    && (result = past_compare(x, av->a_w.w_float, vp)) >= 0)
	{
	    if (!result)
	    {
		for (ac--, av++, vp++; ac; ac--, av++, vp++)
		{
		    if (av->a_type != A_FLOAT
			|| (result =
			    past_compare(x, av->a_w.w_float, vp++)) < 0)
		    {
			x->x_low = 1;
			return;
		    }
		    if (result) break;
		}
	    }
	    if (x->x_low)
	    {
		x->x_low = 0;
		outlet_bang(((t_object *)x)->ob_outlet);
	    }
	}
	else x->x_low = 1;
    }
}

static void past_clear(t_past *x)
{
    x->x_low = 1;
}

static void past_set(t_past *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_atom *vp = x->x_thresh;
	if (ac > x->x_size)
	{
	    fittermax_rangewarning(past_class, PAST_C74MAXSIZE, "guard points");
	    x->x_thresh = grow_nodata(&ac, &x->x_size, x->x_thresh,
				      PAST_C74MAXSIZE, x->x_thrini,
				      sizeof(*x->x_thresh));
	}
	x->x_nthresh = ac;
	while (ac--) *vp++ = *av++;
	/* CHECKED: x_low is not set here */
    }
}

static void past_free(t_past *x)
{
    if (x->x_thresh != x->x_thrini)
	freebytes(x->x_thresh, x->x_size * sizeof(*x->x_thresh));
}

static void *past_new(t_symbol *s, int ac, t_atom *av)
{
    t_past *x = (t_past *)pd_new(past_class);
    x->x_low = 1;
    x->x_nthresh = 0;
    x->x_size = PAST_C74MAXSIZE;
    x->x_thresh = x->x_thrini;
    outlet_new((t_object *)x, &s_bang);
    past_set(x, 0, ac, av);
    return (x);
}			

void past_setup(void)
{
    past_class = class_new(gensym("past"),
			   (t_newmethod)past_new,
			   (t_method)past_free,
			   sizeof(t_past), 0, A_GIMME, 0);
    class_addfloat(past_class, past_float);
    class_addlist(past_class, past_list);
    class_addmethod(past_class, (t_method)past_clear, gensym("clear"), 0);
    class_addmethod(past_class, (t_method)past_set, gensym("set"), A_GIMME, 0);
    fitter_setup(past_class, 0);
}
