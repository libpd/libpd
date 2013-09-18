/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

/* LATER ask about osx */
#if defined(NT) || defined(MACOSX)
#define asinhf(x)  (log(x + sqrt(x * x + 1)))
#endif

typedef t_sic t_asinh;
static t_class *asinh_class;

static t_int *asinh_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = asinhf(f);  /* CHECKME no protection against NaNs */
    }
    return (w + 4);
}

static void asinh_dsp(t_asinh *x, t_signal **sp)
{
    dsp_add(asinh_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *asinh_new(void)
{
    t_asinh *x = (t_asinh *)pd_new(asinh_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void asinh_tilde_setup(void)
{
    asinh_class = class_new(gensym("asinh~"),
			    (t_newmethod)asinh_new, 0,
			    sizeof(t_asinh), 0, 0);
    sic_setup(asinh_class, asinh_dsp, SIC_FLOATTOSIGNAL);
}
