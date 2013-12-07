/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* FIXME find a way of setting a 32-bit mask in an argument */

#include "m_pd.h"
#include "unstable/forky.h"
#include "sickle/sic.h"

typedef struct _bitor
{
    t_sic     x_sic;
    t_glist  *x_glist;
    t_int     x_mask;  /* set by a 'bits' message or a creation argument */
    int       x_mode;
    int       x_convert1;
} t_bitor;

static t_class *bitor_class;

static t_int *bitor_perform(t_int *w)
{
    t_bitor *x = (t_bitor *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_int mask = x->x_mask;
    switch (x->x_mode)
    {
	/* LATER think about performance */
    case 0:
	/* CHECKED */
	while (nblock--)
	{
	    t_int i = ((*(t_int *)(t_float *)in1++) |
		       (*(t_int *)(t_float *)in2++));
	    *out++ = *(t_float *)&i;
	}
	break;
    case 1:
	/* CHECKED */
	while (nblock--)
	{
	    t_int i = (((t_int)*in1++) |
		       ((t_int)*in2++));
	    *out++ = (t_float)i;
	}
	break;
    case 2:
	/* CHECKED */
	while (nblock--)
	{
	    t_int i = (*(t_int *)(t_float *)in1++) | ((t_int)*in2++);
	    *out++ = *(t_float *)&i;
	}
	break;
    case 3:
	/* CHECKED */
	while (nblock--)
	{
	    t_int i = ((t_int)*in1++) | (*(t_int *)(t_float *)in2++);
	    *out++ = (t_float)i;
	}
	break;
    }
    return (w + 6);
}

static t_int *bitor_perform_noin2(t_int *w)
{
    t_bitor *x = (t_bitor *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_int mask = x->x_mask;
    /* LATER think about performance */
    if (x->x_convert1) while (nblock--)
    {
	/* CHECKED */
	t_int i = ((t_int)*in++) | mask;
	*out++ = (t_float)i;
    }
    else while (nblock--)
    {
	/* CHECKED */
	t_int i = (*(t_int *)(t_float *)in++) | mask;
	*out++ = *(t_float *)&i;
    }
    return (w + 5);
}

static void bitor_dsp(t_bitor *x, t_signal **sp)
{
    if (forky_hasfeeders((t_object *)x, x->x_glist, 1, 0))
	/* use the mask set by a second inlet's signal or float,
	   CHECKED (incompatible) second inlet's int is persistent */
	dsp_add(bitor_perform, 5, x, sp[0]->s_n, sp[0]->s_vec,
		sp[1]->s_vec, sp[2]->s_vec);
    else  /* use the mask set by a 'bits' message or a creation argument */
	dsp_add(bitor_perform_noin2, 4, x, sp[0]->s_n, sp[0]->s_vec,
		sp[1]->s_vec);
}

static void bitor_bits(t_bitor *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_mask = forky_getbitmask(ac, av);
}

static void bitor_mode(t_bitor *x, t_floatarg f)
{
    int i = (int)f;
    if (i < 0)
	i = 0;  /* CHECKED */
    else if (i > 3)
	i = 3;  /* CHECKED */
    x->x_mode = i;
    x->x_convert1 = (x->x_mode == 1 || x->x_mode == 3);
}

static void *bitor_new(t_floatarg f1, t_floatarg f2)
{
    t_bitor *x = (t_bitor *)pd_new(bitor_class);
    x->x_glist = canvas_getcurrent();
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    x->x_mask = (t_int)f1;  /* FIXME (how?) */
    bitor_mode(x, f2);
    return (x);
}

void bitor_tilde_setup(void)
{
    bitor_class = class_new(gensym("bitor~"),
			    (t_newmethod)bitor_new, 0,
			    sizeof(t_bitor), 0,
			    A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(bitor_class, bitor_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(bitor_class, (t_method)bitor_bits,
		    gensym("bits"), A_GIMME, 0);
    class_addmethod(bitor_class, (t_method)bitor_mode,
		    gensym("mode"), A_FLOAT, 0);
}
