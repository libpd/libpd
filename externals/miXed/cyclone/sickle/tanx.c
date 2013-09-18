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
#define tanf  tan
#endif

typedef t_sic t_tanx;
static t_class *tanx_class;

static t_int *tanx_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = tanf(f);
    }
    return (w + 4);
}

static void tanx_dsp(t_tanx *x, t_signal **sp)
{
    dsp_add(tanx_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *tanx_new(void)
{
    t_tanx *x = (t_tanx *)pd_new(tanx_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void tanx_tilde_setup(void)
{
    tanx_class = class_new(gensym("tanx~"),
			   (t_newmethod)tanx_new, 0,
			   sizeof(t_tanx), 0, 0);
    sic_setup(tanx_class, tanx_dsp, SIC_FLOATTOSIGNAL);
}
