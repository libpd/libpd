/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _sah
{
    t_sic    x_sic;
    t_float  x_threshold;
    t_float  x_lastin;
    t_float  x_lastout;
} t_sah;

static t_class *sah_class;

static t_int *sah_perform(t_int *w)
{
    t_sah *x = (t_sah *)(w[1]);
    int nblock = (t_int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float threshold = x->x_threshold;
    t_float lastin = x->x_lastin;
    t_float lastout = x->x_lastout;
    while (nblock--)
    {
    	float f = *in2++;
	if (lastin <= threshold && f > threshold)  /* CHECKME <=, > */
	    lastout = *in1;
	in1++;
	lastin = f;
	*out++ = lastout;
    }
    x->x_lastin = lastin;
    x->x_lastout = lastout;
    return (w + 6);
}

static void sah_dsp(t_sah *x, t_signal **sp)
{
    dsp_add(sah_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void sah_float(t_sah *x, t_float f)
{
    x->x_threshold = f;
}

static void *sah_new(t_floatarg f)
{
    t_sah *x = (t_sah *)pd_new(sah_class);
    x->x_threshold = f;
    x->x_lastin = 0;
    x->x_lastout = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void sah_tilde_setup(void)
{
    sah_class = class_new(gensym("sah~"),
			  (t_newmethod)sah_new, 0,
			  sizeof(t_sah), 0, A_DEFFLOAT, 0);
    sic_setup(sah_class, sah_dsp, sah_float);
}
