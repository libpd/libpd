/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

/* CHECKME for a fixed minimum deltime, if any (5ms for c74's metro) */

typedef struct _snapshot
{
    t_sic     x_sic;
    t_float   x_value;
    int       x_rqoffset;  /* requested */
    int       x_offset;    /* effective (truncated) */
    int       x_stopped;
    int       x_on;        /* !stopped && deltime > 0 */
    float     x_deltime;
    int       x_npoints;
    int       x_nleft;
    int       x_nblock;
    float     x_ksr;
    t_clock  *x_clock;
} t_snapshot;

static t_class *snapshot_class;

static void snapshot_tick(t_snapshot *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void snapshot_bang(t_snapshot *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void snapshot_correct(t_snapshot *x)
{
    int wason = x->x_on;
    x->x_offset =
	(x->x_rqoffset < x->x_nblock ? x->x_rqoffset : x->x_nblock - 1);
    x->x_npoints = x->x_deltime * x->x_ksr - x->x_nblock + x->x_offset;
    if (x->x_on = (!x->x_stopped && x->x_deltime > 0.))
    {
	if (!wason) x->x_nleft = x->x_offset;  /* CHECKME */
    }
    else if (wason) clock_unset(x->x_clock);
}

static void snapshot_start(t_snapshot *x)
{
    x->x_stopped = 0;
    if (!x->x_on && x->x_deltime > 0.)  /* CHECKED no default */
    {
	x->x_nleft = x->x_offset;  /* CHECKME */
	x->x_on = 1;
    }
}

static void snapshot_stop(t_snapshot *x)
{
    x->x_stopped = 1;
    if (x->x_on)
    {
	clock_unset(x->x_clock);
	x->x_on = 0;
    }
}

static void snapshot_float(t_snapshot *x, t_float f)
{
    /* CHECKED nonzero/zero, CHECKED incompatible: int only (float ignored) */
    if (f != 0.)
	snapshot_start(x);
    else
	snapshot_stop(x);
}

static void snapshot_ft1(t_snapshot *x, t_floatarg f)
{
    x->x_deltime = (f > 0. ? f : 0.);  /* CHECKED */
    /* CHECKED setting deltime to a positive value starts the clock
       only if it was stopped by setting deltime to zero */
    snapshot_correct(x);
}

static void snapshot_offset(t_snapshot *x, t_floatarg f)
{
    int i = (int)f;  /* CHECKME */
    x->x_rqoffset = (i >= 0 ? i : 0);  /* CHECKME */
    /* CHECKME if the change has an effect prior to next dsp_add call */
    snapshot_correct(x);
}

static t_int *snapshot_perform(t_int *w)
{
    t_snapshot *x = (t_snapshot *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    x->x_value = in[x->x_offset];
    if (x->x_on)
    {
	/* CHECKME nleft vs offset */
	if (x->x_nleft < x->x_nblock)
	{
	    clock_delay(x->x_clock, 0);
	    x->x_nleft = x->x_npoints;
	}
	else x->x_nleft -= x->x_nblock;
    }
    return (w + 3);
}

static void snapshot_dsp(t_snapshot *x, t_signal **sp)
{
    x->x_nblock = sp[0]->s_n;
    x->x_ksr = sp[0]->s_sr * 0.001;
    snapshot_correct(x);
    x->x_nleft = x->x_offset;  /* CHECKME */
    dsp_add(snapshot_perform, 2, x, sp[0]->s_vec);
}

static void snapshot_free(t_snapshot *x)
{
    if (x->x_clock) clock_free(x->x_clock);
}

static void *snapshot_new(t_floatarg f1, t_floatarg f2)
{
    t_snapshot *x = (t_snapshot *)pd_new(snapshot_class);
    x->x_stopped = 0;  /* CHECKED */
    x->x_on = 0;
    x->x_value = 0;
    x->x_nblock = 64;  /* redundant */
    x->x_ksr = 44.1;  /* redundant */
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_clock = clock_new(x, (t_method)snapshot_tick);
    snapshot_offset(x, f2);  /* CHECKME (this is fixed at nblock-1 in Pd) */
    snapshot_ft1(x, f1);
    return (x);
}

void Snapshot_tilde_setup(void)
{
    snapshot_class = class_new(gensym("Snapshot~"),
			       (t_newmethod)snapshot_new,
			       (t_method)snapshot_free,
			       sizeof(t_snapshot), 0,
			       A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)snapshot_new, gensym("snapshot~"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)snapshot_new, gensym("cyclone/snapshot~"), A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(snapshot_class, snapshot_dsp, snapshot_float);
    class_addbang(snapshot_class, snapshot_bang);
    class_addmethod(snapshot_class, (t_method)snapshot_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(snapshot_class, (t_method)snapshot_offset,
		    gensym("offset"), A_FLOAT, 0);
    class_addmethod(snapshot_class, (t_method)snapshot_start,
		    gensym("start"), 0);
    class_addmethod(snapshot_class, (t_method)snapshot_stop,
		    gensym("stop"), 0);
}

void snapshot_tilde_setup(void)
{
    Snapshot_tilde_setup();
}
