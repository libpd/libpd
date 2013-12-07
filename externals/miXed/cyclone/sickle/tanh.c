/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define tanhf  tanh
#endif

typedef t_sic t_tanh;
static t_class *tanh_class;

static t_int *tanh_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = tanhf(f);  /* CHECKME no protection against overflow */
    }
    return (w + 4);
}

static void tanh_dsp(t_tanh *x, t_signal **sp)
{
    dsp_add(tanh_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *tanh_new(void)
{
    t_tanh *x = (t_tanh *)pd_new(tanh_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void tanh_tilde_setup(void)
{
    tanh_class = class_new(gensym("tanh~"),
			   (t_newmethod)tanh_new, 0,
			   sizeof(t_tanh), 0, 0);
    sic_setup(tanh_class, tanh_dsp, SIC_FLOATTOSIGNAL);
}
