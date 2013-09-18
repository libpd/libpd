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
#define cosf  cos
#endif

typedef t_sic t_cosx;
static t_class *cosx_class;

static t_int *cosx_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = cosf(f);
    }
    return (w + 4);
}

static void cosx_dsp(t_cosx *x, t_signal **sp)
{
    dsp_add(cosx_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *cosx_new(void)
{
    t_cosx *x = (t_cosx *)pd_new(cosx_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void cosx_tilde_setup(void)
{
    cosx_class = class_new(gensym("cosx~"),
			   (t_newmethod)cosx_new, 0,
			   sizeof(t_cosx), 0, 0);
    sic_setup(cosx_class, cosx_dsp, SIC_FLOATTOSIGNAL);
}
