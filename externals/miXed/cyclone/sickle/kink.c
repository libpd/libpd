/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKED negative float in 2nd inlet: "illegal slope value %f",
   but no complaints for signal input -- this is impossible in Pd.
   The only thing we could do (and a bit stupid one) would be to
   clock this exception out from the perf routine. */

#include "m_pd.h"
#include "sickle/sic.h"

#define KINK_DEFSLOPE  1.0

typedef t_sic t_kink;
static t_class *kink_class;

static t_int *kink_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	float iph = *in1++;
	float slope = *in2++;
	float oph = iph * slope;
	if (oph > .5)
	{
	    slope = 1. / (slope + slope);  /* x(y=.5) */
	    if (slope == 1.)
		*out++ = 0;  /* CHECKED */
	    else
		*out++ = (iph - slope) / (2. - (slope + slope)) + .5;
	}
	else *out++ = oph;
    }
    return (w + 5);
}

static void kink_dsp(t_kink *x, t_signal **sp)
{
    dsp_add(kink_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *kink_new(t_symbol *s, int ac, t_atom *av)
{
    t_kink *x = (t_kink *)pd_new(kink_class);
    sic_inlet((t_sic *)x, 1, KINK_DEFSLOPE, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void kink_tilde_setup(void)
{
    kink_class = class_new(gensym("kink~"),
			   (t_newmethod)kink_new, 0,
			   sizeof(t_kink), 0, A_GIMME, 0);
    sic_setup(kink_class, kink_dsp, SIC_FLOATTOSIGNAL);
}
