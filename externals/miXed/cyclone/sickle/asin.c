/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define asinf  asin
#endif

typedef t_sic t_asin;
static t_class *asin_class;

static t_int *asin_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = asinf(f);  /* CHECKME no protection against NaNs */
    }
    return (w + 4);
}

static void asin_dsp(t_asin *x, t_signal **sp)
{
    dsp_add(asin_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *asin_new(void)
{
    t_asin *x = (t_asin *)pd_new(asin_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void asin_tilde_setup(void)
{
    asin_class = class_new(gensym("asin~"),
			   (t_newmethod)asin_new, 0,
			   sizeof(t_asin), 0, 0);
    sic_setup(asin_class, asin_dsp, SIC_FLOATTOSIGNAL);
}
