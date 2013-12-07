/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

/* by definition, this is just an interface to the -lm call
   (do not use costable) */

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define sinf  sin
#endif

typedef t_sic t_sinx;
static t_class *sinx_class;

static t_int *sinx_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = sinf(f);
    }
    return (w + 4);
}

static void sinx_dsp(t_sinx *x, t_signal **sp)
{
    dsp_add(sinx_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *sinx_new(void)
{
    t_sinx *x = (t_sinx *)pd_new(sinx_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void sinx_tilde_setup(void)
{
    sinx_class = class_new(gensym("sinx~"),
			   (t_newmethod)sinx_new, 0,
			   sizeof(t_sinx), 0, 0);
    sic_setup(sinx_class, sinx_dsp, SIC_FLOATTOSIGNAL);
}
