/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define powf  pow
#endif

typedef t_sic t_pow;
static t_class *pow_class;

static t_int *pow_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	float f1 = *in1++;
	float f2 = *in2++;
	/* CHECKED arg order, no protection against NaNs (negative base),
	   under-, and overflows */
	*out++ = powf(f2, f1);
    }
    return (w + 5);
}

static void pow_dsp(t_pow *x, t_signal **sp)
{
    dsp_add(pow_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *pow_new(t_floatarg f)
{
    t_pow *x = (t_pow *)pd_new(pow_class);
    sic_newinlet((t_sic *)x, f);  /* CHECKED default 0 */
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void pow_tilde_setup(void)
{
    pow_class = class_new(gensym("pow~"),
			  (t_newmethod)pow_new, 0,
			  sizeof(t_pow), 0, A_DEFFLOAT, 0);
    sic_setup(pow_class, pow_dsp, SIC_FLOATTOSIGNAL);
}
