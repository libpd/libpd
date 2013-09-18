/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER think again about avoiding memory allocation overhead in run-time.
   One would need to use a creation argument greater than any future right
   inlet value.  But this is incompatible (max uses a `static', max-size
   array), and should be put somewhere in the docs... */

#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/fitter.h"
#include "common/rand.h"

#define URN_INISIZE      128  /* LATER rethink */
#define URN_C74MAXSIZE  4096  /* CHECKED */
#define URN_MAXSIZE    65536  /* LATER use USHRT_MAX */

typedef struct _urn
{
    t_object         x_ob;
    int              x_count;
    int              x_size;   /* as allocated (in bytes) */
    int              x_range;  /* as used */
    unsigned short  *x_urn;
    unsigned short   x_urnini[URN_INISIZE];
    unsigned int     x_seed;
    t_outlet        *x_bangout;
} t_urn;

static t_class *urn_class;

static int urn_resize(t_urn *x, t_float f, int init)
{
    int maxmax = URN_C74MAXSIZE;
    int range = (int)f;  /* CHECKED silent truncation */
    if (init)
    {
	maxmax--;  /* CHECKED: max 4095 here (a bug, sort of) */
	/* CHECKED in the constructor this is silent
	   (also > maxmax clipped without complaining) */
	if (range < 1)
	    range = 1;
    }
    else if (range < 1)
    {
	/* CHECKED (the same for > maxmax) */
	loud_error((t_pd *)x, "illegal size %d", f);
	return (0);
    }
    if (range > URN_MAXSIZE)
    {
	loud_warning((t_pd *)x, 0,
		     "requested size (%d) clipped -- effective size is %d",
		     range, URN_MAXSIZE);
	range = URN_MAXSIZE;
    }
    if (range > maxmax)
	fittermax_rangewarning(urn_class, maxmax, "elements");
    x->x_range = range;
    if (range > x->x_size)
	x->x_urn = grow_nodata(&x->x_range, &x->x_size, x->x_urn,
			       URN_INISIZE, x->x_urnini,
			       sizeof(*x->x_urn));
    return (1);
}

static void urn_bang(t_urn *x)
{
    if (x->x_count)
    {
	int ndx = rand_int(&x->x_seed, x->x_count);
	unsigned short pick = x->x_urn[ndx];
	x->x_urn[ndx] = x->x_urn[--x->x_count];
	outlet_float(((t_object *)x)->ob_outlet, pick);
    }
    /* CHECKED: start banging when the first bang is input
       into an empty urn (and not when the last value is output).
       CHECKED: keep banging until cleared. */
    else outlet_bang(x->x_bangout);
}

static void urn_clear(t_urn *x)
{
    int i;
    x->x_count = x->x_range;
    for (i = 0; i < x->x_count; i++) x->x_urn[i] = i;
}

static void urn_float(t_urn *x, t_float f)
{
    /* CHECKED: float loudly rejected, int (any value) same as bang */
    int i;
    if (loud_checkint((t_pd *)x, f, &i, &s_float))
	urn_bang(x);
}

static void urn_ft1(t_urn *x, t_floatarg f)
{
    if (urn_resize(x, f, 0))  /* CHECKED cleared only if a legal resize */
	urn_clear(x);
}

static void urn_seed(t_urn *x, t_floatarg f)
{
    int i = (int)f;  /* CHECKED */
    if (i < 0)
	i = 1;  /* CHECKED */
    rand_seed(&x->x_seed, (unsigned int)i);
}

static void urn_free(t_urn *x)
{
    if (x->x_urn != x->x_urnini)
	freebytes(x->x_urn, x->x_size * sizeof(*x->x_urn));
}

static void *urn_new(t_floatarg f1, t_floatarg f2)
{
    t_urn *x = (t_urn *)pd_new(urn_class);
    x->x_size = URN_INISIZE;
    x->x_urn = x->x_urnini;
    urn_resize(x, f1, 1);
    urn_seed(x, f2);  /* CHECKME */
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    urn_clear(x);
    return (x);
}

void urn_setup(void)
{
    urn_class = class_new(gensym("urn"),
			  (t_newmethod)urn_new,
			  (t_method)urn_free,
			  sizeof(t_urn), 0,
			  A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(urn_class, urn_bang);
    class_addfloat(urn_class, urn_float);
    class_addmethod(urn_class, (t_method)urn_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    /* CHECKED list is auto-unfolded */
    class_addmethod(urn_class, (t_method)urn_seed,
		    gensym("seed"), A_FLOAT, 0);  /* CHECKED arg obligatory */
    class_addmethod(urn_class, (t_method)urn_clear,
		    gensym("clear"), 0);
    fitter_setup(urn_class, 0);
}
