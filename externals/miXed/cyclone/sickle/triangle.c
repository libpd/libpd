/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

#define TRIANGLE_DEFPHASE  0.5
#define TRIANGLE_DEFLO    -1.0
#define TRIANGLE_DEFHI     1.0

typedef struct _triangle
{
    t_sic      x_sic;
    float      x_low;
    float      x_range;
} t_triangle;

static t_class *triangle_class;

static void triangle_lo(t_triangle *x, t_floatarg f)
{
    float high = x->x_low + x->x_range;
    x->x_low = f;
    x->x_range = high - x->x_low;
}

static void triangle_hi(t_triangle *x, t_floatarg f)
{
    x->x_range = f - x->x_low;
}

/* LATER optimize */
static t_int *triangle_perform(t_int *w)
{
    t_triangle *x = (t_triangle *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    float low = x->x_low;
    float range = x->x_range;
    while (nblock--)
    {
	float ph = *in1++;
	float peakph = *in2++;
	/* CHECKED ph wrapped */
	if (ph < 0.)
	    ph -= (int)ph - 1.;
	else if (ph > 1.)
	    ph -= (int)ph;
	/* CHECKED peakph clipped  */
	if (peakph < 0.)
	    peakph = 0.;
	else if (peakph > 1.)
	    peakph = 1.;

	if (ph < peakph)
	    ph /= peakph;
	else if (peakph < 1.)
	    ph = (1. - ph) / (1. - peakph);
	else
	    ph = 0.;
	*out++ = low + ph * range;
    }
    return (w + 6);
}

static void triangle_dsp(t_triangle *x, t_signal **sp)
{
    dsp_add(triangle_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *triangle_new(t_symbol *s, int ac, t_atom *av)
{
    t_triangle *x = (t_triangle *)pd_new(triangle_class);
    sic_inlet((t_sic *)x, 1, TRIANGLE_DEFPHASE, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    x->x_low = TRIANGLE_DEFLO;
    x->x_range = (TRIANGLE_DEFHI - TRIANGLE_DEFLO);
    return (x);
}

void triangle_tilde_setup(void)
{
    triangle_class = class_new(gensym("triangle~"),
			       (t_newmethod)triangle_new, 0,
			       sizeof(t_triangle), 0, A_GIMME, 0);
    sic_setup(triangle_class, triangle_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(triangle_class, (t_method)triangle_lo,
		    gensym("lo"), A_DEFFLOAT, 0);  /* CHECKED */
    class_addmethod(triangle_class, (t_method)triangle_hi,
		    gensym("hi"), A_DEFFLOAT, 0);  /* CHECKED */
}
