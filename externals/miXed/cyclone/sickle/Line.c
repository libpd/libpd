/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "shared.h"
#include "common/grow.h"
#include "common/loud.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define LINE_DEBUG
#endif

#define LINE_INISIZE  64  /* LATER rethink */
#define LINE_MAXSIZE  64

typedef struct _lineseg
{
    float  s_target;
    float  s_delta;
} t_lineseg;

typedef struct _line
{
    t_sic       x_sic;
    float       x_value;
    float       x_target;
    float       x_delta;
    int         x_deltaset;
    float       x_inc;
    float       x_biginc;
    float       x_ksr;
    int         x_nleft;
    int         x_retarget;
    int         x_size;   /* as allocated */
    int         x_nsegs;  /* as used */
    t_lineseg  *x_curseg;
    t_lineseg  *x_segs;
    t_lineseg   x_segini[LINE_INISIZE];
    t_clock    *x_clock;
    t_outlet   *x_bangout;
#ifdef LINE_DEBUG
    int         dbg_nretargets;
    int         dbg_exitpoint;
    int         dbg_npoints;
#endif
} t_line;

static t_class *line_class;

static void line_tick(t_line *x)
{
    outlet_bang(x->x_bangout);
#ifdef LINE_DEBUG
    loudbug_post("exit point %d, after %d retarget calls",
		 x->dbg_exitpoint, x->dbg_nretargets);
    loudbug_post("at value %g, after last %d npoints, with inc %g, biginc %g",
		 x->x_value, x->dbg_npoints, x->x_inc, x->x_biginc);
    x->dbg_nretargets = x->dbg_exitpoint = x->dbg_npoints = 0;
#endif
}

static t_int *line_perform(t_int *w)
{
    t_line *x = (t_line *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int nblock = (int)(w[3]);
    int nxfer = x->x_nleft;
    float curval = x->x_value;
    float inc = x->x_inc;
    float biginc = x->x_biginc;
    if (PD_BIGORSMALL(curval))  /* LATER rethink */
	curval = x->x_value = 0;
retarget:
    if (x->x_retarget)
    {
	float target = x->x_curseg->s_target;
	float delta = x->x_curseg->s_delta;
    	int npoints = delta * x->x_ksr + 0.5;  /* LATER rethink */
#ifdef LINE_DEBUG
	x->dbg_nretargets++;
#endif
	x->x_nsegs--;
	x->x_curseg++;
    	while (npoints <= 0)
	{
	    curval = x->x_value = target;
	    if (x->x_nsegs)
	    {
		target = x->x_curseg->s_target;
		delta = x->x_curseg->s_delta;
		npoints = delta * x->x_ksr + 0.5;  /* LATER rethink */
		x->x_nsegs--;
		x->x_curseg++;
	    }
	    else
	    {
		while (nblock--) *out++ = curval;
		x->x_nleft = 0;
#ifdef LINE_DEBUG
		x->dbg_exitpoint = 1;
#endif
		clock_delay(x->x_clock, 0);
		x->x_retarget = 0;
		return (w + 4);
	    }
	}
    	nxfer = x->x_nleft = npoints;
    	inc = x->x_inc = (target - x->x_value) / (float)npoints;
	x->x_biginc = (int)(w[3]) * inc;
	biginc = nblock * inc;
	x->x_target = target;
    	x->x_retarget = 0;
#ifdef LINE_DEBUG
	x->dbg_npoints = npoints;
#endif
    }
    if (nxfer >= nblock)
    {
	if ((x->x_nleft -= nblock) == 0)
	{
	    if (x->x_nsegs) x->x_retarget = 1;
	    else
	    {
#ifdef LINE_DEBUG
		x->dbg_exitpoint = 2;
#endif
		clock_delay(x->x_clock, 0);
	    }
	    x->x_value = x->x_target;
	}
	else x->x_value += biginc;
    	while (nblock--)
	    *out++ = curval, curval += inc;
    }
    else if (nxfer > 0)
    {
	nblock -= nxfer;
	do
	    *out++ = curval, curval += inc;
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
#ifdef LINE_DEBUG
	    x->dbg_exitpoint = 3;
#endif
	    clock_delay(x->x_clock, 0);
	}
    }
    else while (nblock--) *out++ = curval;
    return (w + 4);
}

static void line_float(t_line *x, t_float f)
{
    if (x->x_deltaset)
    {
    	x->x_deltaset = 0;
    	x->x_target = f;
	x->x_nsegs = 1;
	x->x_curseg = x->x_segs;
	x->x_curseg->s_target = f;
	x->x_curseg->s_delta = x->x_delta;
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

static void line_ft1(t_line *x, t_floatarg f)
{
    x->x_delta = f;
    x->x_deltaset = (f > 0);
}

static void line_list(t_line *x, t_symbol *s, int ac, t_atom *av)
{
    int natoms, nsegs, odd;
    t_atom *ap;
    t_lineseg *segp;
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
    odd = natoms % 2;
    nsegs = natoms / 2;
    if (odd) nsegs++;
    if (nsegs > x->x_size)
    {
	int ns = nsegs;
	x->x_segs = grow_nodata(&ns, &x->x_size, x->x_segs,
				LINE_INISIZE, x->x_segini,
				sizeof(*x->x_segs));
	if (ns < nsegs)
	{
	    natoms = ns * 2;
	    nsegs = ns;
	    odd = 0;
	}
    }
    x->x_nsegs = nsegs;
#ifdef LINE_DEBUG
    loudbug_post("%d segments:", x->x_nsegs);
#endif
    segp = x->x_segs;
    if (odd) nsegs--;
    while (nsegs--)
    {
	segp->s_target = av++->a_w.w_float;
	segp->s_delta = av++->a_w.w_float;
#ifdef LINE_DEBUG
	loudbug_post("%g %g", segp->s_target, segp->s_delta);
#endif
	segp++;
    }
    if (odd)
    {
	segp->s_target = av->a_w.w_float;
	segp->s_delta = 0;
#ifdef LINE_DEBUG
	loudbug_post("%g %g", segp->s_target, segp->s_delta);
#endif
    }
    x->x_deltaset = 0;
    x->x_target = x->x_segs->s_target;
    x->x_curseg = x->x_segs;
    x->x_retarget = 1;
}

/* CHECKED no stop, pity... */
#if 0
static void line_stop(t_line *x)
{
    x->x_target = x->x_value;
    x->x_nleft = 0;
    x->x_retarget = 0;
    x->x_nsegs = 0;
    x->x_curseg = 0;
}
#endif

static void line_dsp(t_line *x, t_signal **sp)
{
    dsp_add(line_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
    x->x_ksr = sp[0]->s_sr * 0.001;
}

static void line_free(t_line *x)
{
    if (x->x_segs != x->x_segini)
	freebytes(x->x_segs, x->x_size * sizeof(*x->x_segs));
    if (x->x_clock) clock_free(x->x_clock);
}

static void *line_new(t_floatarg f)
{
    t_line *x = (t_line *)pd_new(line_class);
    x->x_value = x->x_target = f;
    x->x_deltaset = 0;
    x->x_nleft = 0;
    x->x_retarget = 0;
    x->x_size = LINE_INISIZE;
    x->x_nsegs = 0;
    x->x_segs = x->x_segini;
    x->x_curseg = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_signal);
    x->x_bangout = outlet_new((t_object *)x, &s_bang);
    x->x_clock = clock_new(x, (t_method)line_tick);
    return (x);
}

void Line_tilde_setup(void)
{
    line_class = class_new(gensym("Line~"),
			   (t_newmethod)line_new,
			   (t_method)line_free,
			   sizeof(t_line), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)line_new, gensym("line~"), A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)line_new, gensym("cyclone/line~"), A_DEFFLOAT, 0);
    sic_setup(line_class, line_dsp, SIC_NOMAINSIGNALIN);
    class_addfloat(line_class, line_float);
    class_addlist(line_class, line_list);
    class_addmethod(line_class, (t_method)line_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}

void line_tilde_setup(void)
{
    Line_tilde_setup();
}
