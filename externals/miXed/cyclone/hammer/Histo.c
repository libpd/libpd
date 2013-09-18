/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is an entirely rewritten version of Joseph A. Sarlo's code.
   The most important changes are listed in "pd-lib-notes.txt" file.  */

#include "m_pd.h"
#include "common/loud.h"

#define HISTO_DEFSIZE  128

typedef struct _Histo
{
    t_object   x_ob;
    int        x_size;
    unsigned  *x_hist;  /* LATER consider using 64 bits */
    int        x_lastinput;
    t_outlet  *x_countout;
} t_Histo;

static t_class *Histo_class;

static void Histo_clear(t_Histo *x)
{
    int i = x->x_size;
    while (i--) x->x_hist[i] = 0;
    /* CHECKED: last input is kept */
}

static void Histo_doit(t_Histo *x, int val, int doincr)
{
    if (val >= 0 && val < x->x_size)
    {
	if (doincr)
	{
	    /* CHECKED: only in-range numbers are stored */
	    x->x_lastinput = val;
	    x->x_hist[val]++;
	}
	outlet_float(x->x_countout, x->x_hist[val]);
	/* CHECKED: out-of-range numbers are never passed thru */
	outlet_float(((t_object *)x)->ob_outlet, val);
    }
}

static void Histo_bang(t_Histo *x)
{
    Histo_doit(x, x->x_lastinput, 0);
}

static void Histo_float(t_Histo *x, t_floatarg f)
{
    int i;
    if (loud_checkint((t_pd *)x, f, &i, &s_float))  /* CHECKED */
	Histo_doit(x, i, 1);
}

static void Histo_ft1(t_Histo *x, t_floatarg f)
{
    /* CHECKED: floats are accepted in second inlet (truncated) */
    Histo_doit(x, (int)f, 0);
}

static void Histo_free(t_Histo *x)
{
    if (x->x_hist)
	freebytes(x->x_hist, x->x_size * sizeof(*x->x_hist));
}

static void *Histo_new(t_floatarg f)
{
    t_Histo *x;
    int size = (int)f;
    unsigned *hist;
    if (size < 1)  /* CHECKED: 1 is allowed */
	size = HISTO_DEFSIZE;
    if (!(hist = (unsigned *)getbytes(size * sizeof(*hist))))
	return (0);
    x = (t_Histo *)pd_new(Histo_class);
    x->x_size = size;
    x->x_hist = hist;
    x->x_lastinput = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_countout = outlet_new((t_object *)x, &s_float);
    Histo_clear(x);
    return (x);
}

void Histo_setup(void)
{
    Histo_class = class_new(gensym("Histo"),
			    (t_newmethod)Histo_new,
			    (t_method)Histo_free,
			    sizeof(t_Histo), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Histo_new, gensym("histo"), A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Histo_new, gensym("cyclone/histo"), A_DEFFLOAT, 0);
    class_addbang(Histo_class, Histo_bang);
    class_addfloat(Histo_class, Histo_float);
    class_addmethod(Histo_class, (t_method)Histo_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(Histo_class, (t_method)Histo_clear,
		    gensym("clear"), 0);
}

void histo_setup(void)
{
    Histo_setup();
}
