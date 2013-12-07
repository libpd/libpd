/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

#define TRAPEZOID_DEFUP  0.1  /* a bug? */
#define TRAPEZOID_DEFDN  0.9  /* a bug? */
#define TRAPEZOID_DEFLO  0.0
#define TRAPEZOID_DEFHI  1.0

typedef struct _trapezoid
{
    t_sic      x_sic;
    float      x_low;
    float      x_range;
} t_trapezoid;

static t_class *trapezoid_class;

static void trapezoid_lo(t_trapezoid *x, t_floatarg f)
{
    float high = x->x_low + x->x_range;
    x->x_low = f;
    x->x_range = high - x->x_low;
}

static void trapezoid_hi(t_trapezoid *x, t_floatarg f)
{
    x->x_range = f - x->x_low;
}

/* LATER optimize */
static t_int *trapezoid_perform(t_int *w)
{
    t_trapezoid *x = (t_trapezoid *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    float low = x->x_low;
    float range = x->x_range;
    while (nblock--)
    {
	float ph = *in1++;
	float upph = *in2++;
	float dnph = *in3++;
	/* CHECKED ph wrapped */
	if (ph < 0.)
	    ph -= (int)ph - 1.;
	else if (ph > 1.)
	    ph -= (int)ph;
	/* CHECKED upph, dnph clipped  */
	if (upph < 0.)
	    upph = 0.;
	else if (upph > 1.)  /* CHECKME */
	    upph = 1.;
	if (dnph < upph)
	    dnph = upph;
	else if (dnph > 1.)
	    dnph = 1.;

	if (ph < upph)
	    ph /= upph;
	else if (ph < dnph)
	    ph = 1.;
	else if (dnph < 1.)
	    ph = (1. - ph) / (1. - dnph);
	else
	    ph = 0.;
	*out++ = low + ph * range;
    }
    return (w + 7);
}

static void trapezoid_dsp(t_trapezoid *x, t_signal **sp)
{
    dsp_add(trapezoid_perform, 6, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *trapezoid_new(t_symbol *s, int ac, t_atom *av)
{
    t_trapezoid *x = (t_trapezoid *)pd_new(trapezoid_class);
    sic_inlet((t_sic *)x, 1, TRAPEZOID_DEFUP, 0, ac, av);
    sic_inlet((t_sic *)x, 2, TRAPEZOID_DEFDN, 1, ac, av);
    outlet_new((t_object *)x, &s_signal);
    x->x_low = TRAPEZOID_DEFLO;
    x->x_range = (TRAPEZOID_DEFHI - TRAPEZOID_DEFLO);
    return (x);
}

void trapezoid_tilde_setup(void)
{
    trapezoid_class = class_new(gensym("trapezoid~"),
				(t_newmethod)trapezoid_new, 0,
				sizeof(t_trapezoid), 0, A_GIMME, 0);
    sic_setup(trapezoid_class, trapezoid_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(trapezoid_class, (t_method)trapezoid_lo,
		    gensym("lo"), A_DEFFLOAT, 0);  /* CHECKME */
    class_addmethod(trapezoid_class, (t_method)trapezoid_hi,
		    gensym("hi"), A_DEFFLOAT, 0);  /* CHECKME */
}
