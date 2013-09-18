/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define sinhf  sinh
#endif

typedef t_sic t_sinh;
static t_class *sinh_class;

static t_int *sinh_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = sinhf(f);  /* CHECKME no protection against overflow */
    }
    return (w + 4);
}

static void sinh_dsp(t_sinh *x, t_signal **sp)
{
    dsp_add(sinh_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *sinh_new(void)
{
    t_sinh *x = (t_sinh *)pd_new(sinh_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void sinh_tilde_setup(void)
{
    sinh_class = class_new(gensym("sinh~"),
			   (t_newmethod)sinh_new, 0,
			   sizeof(t_sinh), 0, 0);
    sic_setup(sinh_class, sinh_dsp, SIC_FLOATTOSIGNAL);
}
