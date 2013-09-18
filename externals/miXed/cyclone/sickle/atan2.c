/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define atan2f  atan2
#endif

typedef t_sic t_atan2;
static t_class *atan2_class;

static t_int *atan2_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	float f1 = *in1++;
	float f2 = *in2++;
	/* CHECKED arg order, range (radians) */
	*out++ = atan2f(f1, f2);
    }
    return (w + 5);
}

static void atan2_dsp(t_atan2 *x, t_signal **sp)
{
    dsp_add(atan2_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *atan2_new(t_floatarg f)
{
    t_atan2 *x = (t_atan2 *)pd_new(atan2_class);
    sic_newinlet((t_sic *)x, f);  /* CHECKED x-value argument */
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void atan2_tilde_setup(void)
{
    atan2_class = class_new(gensym("atan2~"),
			    (t_newmethod)atan2_new, 0,
			    sizeof(t_atan2), 0, A_DEFFLOAT, 0);
    sic_setup(atan2_class, atan2_dsp, SIC_FLOATTOSIGNAL);
}
