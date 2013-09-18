/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define acosf  acos
#endif

typedef t_sic t_acos;
static t_class *acos_class;

static t_int *acos_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = acosf(f);  /* CHECKED no protection against NaNs */
    }
    return (w + 4);
}

static void acos_dsp(t_acos *x, t_signal **sp)
{
    dsp_add(acos_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *acos_new(void)
{
    t_acos *x = (t_acos *)pd_new(acos_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void acos_tilde_setup(void)
{
    acos_class = class_new(gensym("acos~"),
			   (t_newmethod)acos_new, 0,
			   sizeof(t_acos), 0, 0);
    sic_setup(acos_class, acos_dsp, SIC_FLOATTOSIGNAL);
}
