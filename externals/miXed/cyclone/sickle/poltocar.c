/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "unstable/fragile.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define sinf  sin
#define cosf  cos
#endif

typedef struct _poltocar
{
    t_sic      x_sic;
    t_outlet  *x_out2;
} t_poltocar;

static t_class *poltocar_class;

static t_int *poltocar_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    t_float *out2 = (t_float *)(w[5]);
    while (nblock--)
    {
	float am = *in1++, ph = *in2++;
	*out1++ = am * cosf(ph);
	*out2++ = -am * sinf(ph);  /* CHECKED */
    }
    return (w + 6);
}

static t_int *poltocar_perform_noimag(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out1 = (t_float *)(w[4]);
    while (nblock--)
    {
	float am = *in1++, ph = *in2++;
	*out1++ = am * cosf(ph);
    }
    return (w + 5);
}

static void poltocar_dsp(t_poltocar *x, t_signal **sp)
{
    if (fragile_outlet_connections(x->x_out2))
	dsp_add(poltocar_perform, 5, sp[0]->s_n, sp[0]->s_vec,
		sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
    else
	dsp_add(poltocar_perform_noimag, 4, sp[0]->s_n, sp[0]->s_vec,
		sp[1]->s_vec, sp[2]->s_vec);
}

static void *poltocar_new(void)
{
    t_poltocar *x = (t_poltocar *)pd_new(poltocar_class);
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_out2 = outlet_new((t_object *)x, &s_signal);
    return (x);
}

void poltocar_tilde_setup(void)
{
    poltocar_class = class_new(gensym("poltocar~"),
			       (t_newmethod)poltocar_new, 0,
			       sizeof(t_poltocar), 0, 0);
    sic_setup(poltocar_class, poltocar_dsp, SIC_FLOATTOSIGNAL);
}
