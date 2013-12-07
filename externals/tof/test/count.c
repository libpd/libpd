/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _count
{
    t_sic  x_sic;
    int    x_min;
    int    x_max;
    int    x_limit;
    int    x_on;
    int    x_autoreset;
    int    x_count;  /* MAYBE use 64 bits (if 13.5 hours is not enough...) */
} t_count;

static t_class *count_class;

static void count_min(t_count *x, t_floatarg f)
{
    x->x_min = (int)f;
}

static void count_max(t_count *x, t_floatarg f)
{
    x->x_max = (int)f;
    /* MAYBE use 64 bits */
    x->x_limit = (x->x_max == 0 ? 0x7fffffff
		  : x->x_max - 1);  /* CHECKED */
}

static void count_autoreset(t_count *x, t_floatarg f)
{
    x->x_autoreset = (f != 0);
}

static void count_bang(t_count *x)
{
    x->x_count = x->x_min;
    x->x_on = 1;
}

static void count_float(t_count *x, t_floatarg f)
{
    x->x_count = x->x_min = (int)f;
    x->x_on = 1;
}

static void count_list(t_count *x, t_symbol *s, int ac, t_atom *av)
{
    int i;
    if (ac > 4) ac = 4;
    for (i = 0; i < ac; i++)
	if (av[i].a_type != A_FLOAT) break;
    switch (i)
    {
    case 4:
	count_autoreset(x, av[3].a_w.w_float);
    case 3:
	x->x_on = (av[2].a_w.w_float != 0);
    case 2:
	count_max(x, av[1].a_w.w_float);
    case 1:
	count_min(x, av[0].a_w.w_float);
    default:
	x->x_count = x->x_min;
    }
}

static void count_set(t_count *x, t_symbol s, int ac, t_atom *av)
{
    if (av[0].a_type == A_FLOAT) count_min(x, av[0].a_w.w_float);
    if (av[1].a_type == A_FLOAT) count_max(x, av[1].a_w.w_float);
}

static void count_stop(t_count *x)
{
    x->x_count = x->x_min;
    x->x_on = 0;
}

static t_int *count_perform(t_int *w)
{
    t_count *x = (t_count *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int count = x->x_count;
    int limit = x->x_limit;
    if (x->x_on)
    {
	while (nblock--)
	{
	    if (count > limit) count = x->x_min;
	    *out++ = (t_float)count++;
	}
    }
    else
	while (nblock--) *out++ = count;
    x->x_count = count;
    return (w + 4);
}

static void count_dsp(t_count *x, t_signal **sp)
{
    if (x->x_autoreset) count_bang(x);
    dsp_add(count_perform, 3, x, sp[0]->s_n, sp[0]->s_vec);
}

static void *count_new(t_floatarg minval, t_floatarg maxval,
		       t_floatarg onflag, t_floatarg autoflag)
{
    t_count *x = (t_count *)pd_new(count_class);
    count_min(x, minval);
    count_max(x, maxval);
    x->x_on = (onflag != 0);
    count_autoreset(x, autoflag);
    x->x_count = x->x_min;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void count_tilde_setup(void)
{
    count_class = class_new(gensym("count~"),
			    (t_newmethod)count_new, 0,
			    sizeof(t_count), 0,
			    A_DEFFLOAT, A_DEFFLOAT,
			    A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(count_class, count_dsp, SIC_NOMAINSIGNALIN);
    class_addbang(count_class, count_bang);
    class_addfloat(count_class, count_float);
    class_addlist(count_class, count_list);
    class_addmethod(count_class, (t_method)count_max,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(count_class, (t_method)count_autoreset,
		    gensym("autoreset"), A_FLOAT, 0);
    class_addmethod(count_class, (t_method)count_min,
		    gensym("min"), A_FLOAT, 0);
    class_addmethod(count_class, (t_method)count_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(count_class, (t_method)count_stop, gensym("stop"), 0);
}
