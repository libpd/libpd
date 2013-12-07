/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define atanf  atan
#endif

typedef t_sic t_atan;
static t_class *atan_class;

static t_int *atan_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
	*out++ = atanf(f);  /* CHECKME no protection against NaNs */
    }
    return (w + 4);
}

static void atan_dsp(t_atan *x, t_signal **sp)
{
    dsp_add(atan_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *atan_new(void)
{
    t_atan *x = (t_atan *)pd_new(atan_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void atan_tilde_setup(void)
{
    atan_class = class_new(gensym("atan~"),
			   (t_newmethod)atan_new, 0,
			   sizeof(t_atan), 0, 0);
    sic_setup(atan_class, atan_dsp, SIC_FLOATTOSIGNAL);
}
