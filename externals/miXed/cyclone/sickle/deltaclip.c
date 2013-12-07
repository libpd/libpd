/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#define DELTACLIP_DEFLO  0.
#define DELTACLIP_DEFHI  0.

typedef struct _deltaclip
{
    t_sic    x_sic;
    t_float  x_last;
} t_deltaclip;

static t_class *deltaclip_class;

static t_int *deltaclip_perform(t_int *w)
{
    t_deltaclip *x = (t_deltaclip *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    t_float last = x->x_last;
    while (nblock--)
    {
    	float f = *in1++;
	float delta = f - last;
    	float lo = *in2++;
    	float hi = *in3++;
    	if (delta < lo)
	    f = last + lo;
    	else if (delta > hi)
	    f = last + hi;
	*out++ = last = f;
    }
    x->x_last = last;
    return (w + 7);
}

static void deltaclip_dsp(t_deltaclip *x, t_signal **sp)
{
    dsp_add(deltaclip_perform, 6, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *deltaclip_new(t_symbol *s, int ac, t_atom *av)
{
    t_deltaclip *x = (t_deltaclip *)pd_new(deltaclip_class);
    sic_inlet((t_sic *)x, 1, DELTACLIP_DEFLO, 0, ac, av);
    sic_inlet((t_sic *)x, 2, DELTACLIP_DEFHI, 1, ac, av);
    outlet_new((t_object *)x, &s_signal);
    x->x_last = 0;
    return (x);
}

void deltaclip_tilde_setup(void)
{
    deltaclip_class = class_new(gensym("deltaclip~"),
				(t_newmethod)deltaclip_new, 0,
				sizeof(t_deltaclip), 0, A_GIMME, 0);
    sic_setup(deltaclip_class, deltaclip_dsp, SIC_FLOATTOSIGNAL);
}
