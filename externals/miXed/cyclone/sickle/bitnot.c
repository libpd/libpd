/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _bitnot
{
    t_sic  x_sic;
    int    x_convert1;
} t_bitnot;

static t_class *bitnot_class;

static t_int *bitnot_perform(t_int *w)
{
    t_bitnot *x = (t_bitnot *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    /* LATER think about performance */
    if (x->x_convert1) while (nblock--)
    {
	/* CHECKME */
	t_int i = ~((t_int)*in++);
	*out++ = (t_float)i;
    }
    else while (nblock--)
    {
	/* CHECKME */
	t_int i = ~(*(t_int *)(t_float *)in++);
	*out++ = *(t_float *)&i;
    }
    return (w + 5);
}

static void bitnot_dsp(t_bitnot *x, t_signal **sp)
{
    dsp_add(bitnot_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void bitnot_mode(t_bitnot *x, t_floatarg f)
{
    int i = (int)f;
    x->x_convert1 = (i > 0);  /* CHECKME */
}

static void *bitnot_new(t_floatarg f)
{
    t_bitnot *x = (t_bitnot *)pd_new(bitnot_class);
    outlet_new((t_object *)x, &s_signal);
    bitnot_mode(x, f);
    return (x);
}

void bitnot_tilde_setup(void)
{
    bitnot_class = class_new(gensym("bitnot~"),
			     (t_newmethod)bitnot_new, 0,
			     sizeof(t_bitnot), 0,
			     A_DEFFLOAT, 0);
    sic_setup(bitnot_class, bitnot_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(bitnot_class, (t_method)bitnot_mode,
		    gensym("mode"), A_FLOAT, 0);
}
