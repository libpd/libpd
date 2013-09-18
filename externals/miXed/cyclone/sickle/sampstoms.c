/* Copyright (c) 2003 krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _sampstoms
{
    t_sic      x_sic;
    float      x_rcpksr;
    t_outlet  *x_floatout;
} t_sampstoms;

static t_class *sampstoms_class;

static void sampstoms_float(t_sampstoms *x, t_float f)
{
    outlet_float(x->x_floatout, f * x->x_rcpksr);
}

static t_int *sampstoms_perform(t_int *w)
{
    t_sampstoms *x = (t_sampstoms *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    float rcpksr = x->x_rcpksr;
    while (nblock--) *out++ = *in++ * rcpksr;
    return (w + 5);
}

static void sampstoms_dsp(t_sampstoms *x, t_signal **sp)
{
    x->x_rcpksr = 1000. / sp[0]->s_sr;
    dsp_add(sampstoms_perform, 4, x,
	    sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *sampstoms_new(void)
{
    t_sampstoms *x = (t_sampstoms *)pd_new(sampstoms_class);
    x->x_rcpksr = 1000. / sys_getsr();  /* LATER rethink */
    outlet_new((t_object *)x, &s_signal);
    x->x_floatout = outlet_new((t_object *)x, &s_float);
    return (x);
}

void sampstoms_tilde_setup(void)
{
    sampstoms_class = class_new(gensym("sampstoms~"),
				(t_newmethod)sampstoms_new, 0,
				sizeof(t_sampstoms), 0, 0);
    sic_setup(sampstoms_class, sampstoms_dsp, sampstoms_float);
}
