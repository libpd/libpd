/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER use hasfeeders */

#include "m_pd.h"
#include "sickle/sic.h"

#define MAXIMUM_DEFRHS  0.  /* CHECKED */

typedef t_sic t_maximum;
static t_class *maximum_class;

static t_int *maximum_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
	*out++ = (f1 > f2 ? f1 : f2);
    }
    return (w + 5);
}

static void maximum_dsp(t_maximum *x, t_signal **sp)
{
    dsp_add(maximum_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *maximum_new(t_symbol *s, int ac, t_atom *av)
{
    t_maximum *x = (t_maximum *)pd_new(maximum_class);
    sic_inlet((t_sic *)x, 1, MAXIMUM_DEFRHS, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void maximum_tilde_setup(void)
{
    maximum_class = class_new(gensym("maximum~"),
			      (t_newmethod)maximum_new, 0,
			      sizeof(t_maximum), 0, A_GIMME, 0);
    sic_setup(maximum_class, maximum_dsp, SIC_FLOATTOSIGNAL);
}
