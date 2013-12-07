/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _change
{
    t_sic    x_sic;
    t_float  x_last;
} t_change;

static t_class *change_class;

static t_int *change_perform(t_int *w)
{
    t_change *x = (t_change *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float last = x->x_last;
    while (nblock--)
    {
	t_float f = *in++;
    	*out++ = (f > last ? 1. : (f < last ? -1. : 0.));
	last = f;
    }
    x->x_last = last;
    return (w + 5);
}

static void change_dsp(t_change *x, t_signal **sp)
{
    dsp_add(change_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *change_new(void)
{
    t_change *x = (t_change *)pd_new(change_class);
    outlet_new((t_object *)x, &s_signal);
    x->x_last = 0;  /* CHECKME startup conditions */
    return (x);
}

void change_tilde_setup(void)
{
    change_class = class_new(gensym("change~"),
			     (t_newmethod)change_new, 0,
			     sizeof(t_change), 0, 0);
    sic_setup(change_class, change_dsp, SIC_FLOATTOSIGNAL);
}
