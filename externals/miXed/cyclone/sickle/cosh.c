/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define coshf  cosh
#endif

typedef t_sic t_cosh;
static t_class *cosh_class;

static t_int *cosh_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = coshf(f);  /* CHECKME no protection against overflow */
    }
    return (w + 4);
}

static void cosh_dsp(t_cosh *x, t_signal **sp)
{
    dsp_add(cosh_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *cosh_new(void)
{
    t_cosh *x = (t_cosh *)pd_new(cosh_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void cosh_tilde_setup(void)
{
    cosh_class = class_new(gensym("cosh~"),
			   (t_newmethod)cosh_new, 0,
			   sizeof(t_cosh), 0, 0);
    sic_setup(cosh_class, cosh_dsp, SIC_FLOATTOSIGNAL);
}
