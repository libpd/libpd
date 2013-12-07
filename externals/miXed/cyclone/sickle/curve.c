/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "common/grow.h"
#include "common/loud.h"
#include "common/clc.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define CURVE_DEBUG
#endif

/* CHECKED apparently c74's formula has not been carefully tuned (yet?).
   It has 5% deviation from the straight line for ccinput = 0 at half-domain,
   range 1, and generates nans for ccinput > .995 (cf comment in clc.h). */

#define CURVE_INISIZE  64  /* LATER rethink */
#define CURVE_MAXSIZE  64

typedef struct _curveseg
{
    float   s_target;
    float   s_delta;
    int     s_nhops;
    float   s_ccinput;
    double  s_bb;
    double  s_mm;
} t_curveseg;

typedef struct _curve
{
    t_sic        x_sic;
    float        x_value;
    float        x_ccinput;
    float        x_target;
    float        x_delta;
    int          x_deltaset;
    double       x_vv;
    double       x_bb;
    double       x_mm;
    float        x_y0;
    float        x_dy;
    float        x_ksr;
    int          x_nleft;
    int          x_retarget;
    int          x_size;   /* as allocated */
    int          x_nsegs;  /* as used */
    t_curveseg  *x_curseg;
    t_curveseg  *x_segs;
    t_curveseg   x_segini[CURVE_INISIZE];
    t_clock     *x_clock;
    t_outlet    *x_bangout;
#ifdef CURVE_DEBUG
    int          dbg_nretargets;
    int          dbg_exitpoint;
    int          dbg_nhops;
#endif
} t_curve;

static t_class *curve_class;
static double curve_coef;

static void curve_cc(t_curve *x, t_curveseg *segp, float f)
{
    int nhops = segp->s_delta * x->x_ksr + 0.5;  /* LATER rethink */
    segp->s_ccinput = f;
    segp->s_nhops = (nhops > 0 ? nhops : 0);
    clccurve_coefs(segp->s_nhops, (double)f, &segp->s_bb, &segp->s_mm);
#ifdef CURVE_DEBUG
    loudbug_post("%g %g %g %g",
		 segp->s_target, segp->s_delta, segp->s_bb, segp->s_mm);
#endif
}

static void curve_tick(t_curve *x)
{
    outlet_bang(x->x_bangout);
#ifdef CURVE_DEBUG
    loudbug_post("exit point %d, after %d retarget calls",
		 x->dbg_exitpoint, x->dbg_nretargets);
    loudbug_post("at value %g, after last %d nhops, with bb %g, mm %g",
		 x->x_value, x->dbg_nhops, x->x_bb, x->x_mm);
    x->dbg_nretargets = x->dbg_exitpoint = x->dbg_nhops = 0;
#endif
}

static t_int *curve_perform(t_int *w)
{
    t_curve *x = (t_curve *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int nblock = (int)(w[3]);
    int nxfer = x->x_nleft;
    float curval = x->x_value;
    double vv = x->x_vv;
    double bb = x->x_bb;
    double mm = x->x_mm;
    float dy = x->x_dy;
    float y0 = x->x_y0;
    if (PD_BIGORSMALL(curval))  /* LATER rethink */
	curval = x->x_value = 0;
retarget:
    if (x->x_retarget)
    {
	float target = x->x_curseg->s_target;
	float delta = x->x_curseg->s_delta;
    	int nhops = x->x_curseg->s_nhops;
	bb = x->x_curseg->s_bb;
	mm = x->x_curseg->s_mm;
	if (x->x_curseg->s_ccinput < 0)
	    dy = x->x_value - target;
	else
	    dy = target - x->x_value;
#ifdef CURVE_DEBUG
	x->dbg_nretargets++;
#endif
	x->x_nsegs--;
	x->x_curseg++;
    	while (nhops <= 0)
	{
	    curval = x->x_value = target;
	    if (x->x_nsegs)
	    {
		target = x->x_curseg->s_target;
		delta = x->x_curseg->s_delta;
		nhops = x->x_curseg->s_nhops;
		bb = x->x_curseg->s_bb;
		mm = x->x_curseg->s_mm;
		if (x->x_curseg->s_ccinput < 0)
		    dy = x->x_value - target;
		else
		    dy = target - x->x_value;
		x->x_nsegs--;
		x->x_curseg++;
	    }
	    else
	    {
		while (nblock--) *out++ = curval;
		x->x_nleft = 0;
#ifdef CURVE_DEBUG
		x->dbg_exitpoint = 1;
#endif
		clock_delay(x->x_clock, 0);
		x->x_retarget = 0;
		return (w + 4);
	    }
	}
    	nxfer = x->x_nleft = nhops;
	x->x_vv = vv = bb;
	x->x_bb = bb;
	x->x_mm = mm;
	x->x_dy = dy;
	x->x_y0 = y0 = x->x_value;
	x->x_target = target;
    	x->x_retarget = 0;
#ifdef CURVE_DEBUG
	x->dbg_nhops = nhops;
#endif
    }
    if (nxfer >= nblock)
    {
	int silly = ((x->x_nleft -= nblock) == 0);  /* LATER rethink */
    	while (nblock--)
	{
	    *out++ = curval = (vv - bb) * dy + y0;
	    vv *= mm;
	}
	if (silly)
	{
	    if (x->x_nsegs) x->x_retarget = 1;
	    else
	    {
#ifdef CURVE_DEBUG
		x->dbg_exitpoint = 2;
#endif
		clock_delay(x->x_clock, 0);
	    }
	    x->x_value = x->x_target;
	}
	else
	{
	    x->x_value = curval;
	    x->x_vv = vv;
	}
    }
    else if (nxfer > 0)
    {
	nblock -= nxfer;
	do
	    *out++ = (vv - bb) * dy + y0, vv *= mm;
	while (--nxfer);
	curval = x->x_value = x->x_target;
	if (x->x_nsegs)
	{
	    x->x_retarget = 1;
	    goto retarget;
	}
	else
	{
	    while (nblock--) *out++ = curval;
	    x->x_nleft = 0;
#ifdef CURVE_DEBUG
	    x->dbg_exitpoint = 3;
#endif
	    clock_delay(x->x_clock, 0);
	}
    }
    else while (nblock--) *out++ = curval;
    return (w + 4);
}

static void curve_float(t_curve *x, t_float f)
{
    if (x->x_deltaset)
    {
    	x->x_deltaset = 0;
    	x->x_target = f;
	x->x_nsegs = 1;
	x->x_curseg = x->x_segs;
	x->x_curseg->s_target = f;
	x->x_curseg->s_delta = x->x_delta;
#ifdef CURVE_DEBUG
	loudbug_startpost("single segment: ");
#endif
	curve_cc(x, x->x_curseg, x->x_ccinput);
    	x->x_retarget = 1;
    }
    else
    {
    	x->x_value = x->x_target = f;
	x->x_nsegs = 0;
	x->x_curseg = 0;
    	x->x_nleft = 0;
	x->x_retarget = 0;
    }
}

/* CHECKED delta is not persistent, but ccinput is */
static void curve_ft1(t_curve *x, t_floatarg f)
{
    x->x_delta = f;
    x->x_deltaset = (f > 0);
}

static void curve_list(t_curve *x, t_symbol *s, int ac, t_atom *av)
{
    int natoms, nsegs, odd;
    t_atom *ap;
    t_curveseg *segp;
    for (natoms = 0, ap = av; natoms < ac; natoms++, ap++)
    {
	if (ap->a_type != A_FLOAT)
	{
	    loud_messarg((t_pd *)x, &s_list);  /* CHECKED */
	    return;  /* CHECKED */
	}
    }
    if (!natoms)
	return;  /* CHECKED */
    odd = natoms % 3;
    nsegs = natoms / 3;
    if (odd) nsegs++;
    if (nsegs > x->x_size)
    {
	int ns = nsegs;
	x->x_segs = grow_nodata(&ns, &x->x_size, x->x_segs,
				CURVE_INISIZE, x->x_segini,
				sizeof(*x->x_segs));
	if (ns < nsegs)
	{
	    natoms = ns * 3;
	    nsegs = ns;
	    odd = 0;
	}
    }
    x->x_nsegs = nsegs;
#ifdef CURVE_DEBUG
    loudbug_post("%d segments:", x->x_nsegs);
#endif
    segp = x->x_segs;
    if (odd) nsegs--;
    while (nsegs--)
    {
	segp->s_target = av++->a_w.w_float;
	segp->s_delta = av++->a_w.w_float;
	curve_cc(x, segp, av++->a_w.w_float);
	segp++;
    }
    if (odd)
    {
	segp->s_target = av->a_w.w_float;
	if (odd > 1)
	    segp->s_delta = av[1].a_w.w_float;
	else
	    segp->s_delta = 0;
	curve_cc(x, segp, 0.);
    }
    x->x_deltaset = 0;
    x->x_target = x->x_segs->s_target;
    x->x_curseg = x->x_segs;
    x->x_retarget = 1;
}

/* CHECKED no stop, pity... */
#if 0
static void curve_stop(t_curve *x)
{
    x->x_target = x->x_value;
    x->x_nleft = 0;
    x->x_retarget = 0;
    x->x_nsegs = 0;
    x->x_curseg = 0;
}
#endif

static void curve_dsp(t_curve *x, t_signal **sp)
{
    float ksr = sp[0]->s_sr * 0.001;
    if (ksr != x->x_ksr)
    {
	int nsegs = x->x_nsegs;
	t_curveseg *segp = x->x_segs;
	x->x_ksr = ksr;
	while (nsegs--)
	{
	    curve_cc(x, segp, segp->s_ccinput);
	    segp++;
	}
    }
    dsp_add(curve_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void curve_free(t_curve *x)
{
    if (x->x_segs != x->x_segini)
	freebytes(x->x_segs, x->x_size * sizeof(*x->x_segs));
    if (x->x_clock) clock_free(x->x_clock);
}

static void *curve_new(t_floatarg f1, t_floatarg f2)
{
    t_curve *x = (t_curve *)pd_new(curve_class);
    x->x_value = x->x_target = f1;
    x->x_ccinput = f2;
    x->x_deltaset = 0;
    x->x_ksr = sys_getsr() * 0.001;
    x->x_nleft = 0;
    x->x_retarget = 0;
    x->x_size = CURVE_INISIZE;
    x->x_nsegs = 0;
    x->x_segs = x->x_segini;
    x->x_curseg = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    floatinlet_new((t_object *)x, &x->x_ccinput);
    outlet_new((t_object *)x, &s_signal);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    x->x_clock = clock_new(x, (t_method)curve_tick);
    return (x);
}

void curve_tilde_setup(void)
{
    curve_class = class_new(gensym("curve~"),
			    (t_newmethod)curve_new,
			    (t_method)curve_free,
			    sizeof(t_curve), 0,
			    A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(curve_class, curve_dsp, SIC_NOMAINSIGNALIN);
    class_addfloat(curve_class, curve_float);
    class_addlist(curve_class, curve_list);
    class_addmethod(curve_class, (t_method)curve_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}
