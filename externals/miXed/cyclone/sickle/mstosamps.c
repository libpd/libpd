/* Copyright (c) 2003 krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _mstosamps
{
    t_sic      x_sic;
    float      x_ksr;
    t_outlet  *x_floatout;
} t_mstosamps;

static t_class *mstosamps_class;

static void mstosamps_float(t_mstosamps *x, t_float f)
{
    outlet_float(x->x_floatout, f * x->x_ksr);
}

static t_int *mstosamps_perform(t_int *w)
{
    t_mstosamps *x = (t_mstosamps *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    float ksr = x->x_ksr;
    while (nblock--) *out++ = *in++ * ksr;
    return (w + 5);
}

static void mstosamps_dsp(t_mstosamps *x, t_signal **sp)
{
    x->x_ksr = sp[0]->s_sr * .001;
    dsp_add(mstosamps_perform, 4, x,
	    sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *mstosamps_new(void)
{
    t_mstosamps *x = (t_mstosamps *)pd_new(mstosamps_class);
    x->x_ksr = sys_getsr() * .001;  /* LATER rethink */
    outlet_new((t_object *)x, &s_signal);
    x->x_floatout = outlet_new((t_object *)x, &s_float);
    return (x);
}

void mstosamps_tilde_setup(void)
{
    mstosamps_class = class_new(gensym("mstosamps~"),
				(t_newmethod)mstosamps_new, 0,
				sizeof(t_mstosamps), 0, 0);
    sic_setup(mstosamps_class, mstosamps_dsp, mstosamps_float);
}
