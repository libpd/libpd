/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

/* LATER ask about osx */
#if defined(NT) || defined(MACOSX)
#define acoshf(x)  (log(x + sqrt(x * x - 1)))
#endif

typedef t_sic t_acosh;
static t_class *acosh_class;

static t_int *acosh_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = acoshf(f);  /* CHECKME no protection against NaNs */
    }
    return (w + 4);
}

static void acosh_dsp(t_acosh *x, t_signal **sp)
{
    dsp_add(acosh_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *acosh_new(void)
{
    t_acosh *x = (t_acosh *)pd_new(acosh_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void acosh_tilde_setup(void)
{
    acosh_class = class_new(gensym("acosh~"),
			    (t_newmethod)acosh_new, 0,
			    sizeof(t_acosh), 0, 0);
    sic_setup(acosh_class, acosh_dsp, SIC_FLOATTOSIGNAL);
}
