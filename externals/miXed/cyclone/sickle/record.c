/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

#define RECORD_REDRAWPAUSE  1000.  /* refractory period */

typedef struct _record
{
    t_arsic   x_arsic;
    float     x_startpoint;  /* the inputs */
    float     x_endpoint;
    int       x_appendmode;
    int       x_loopmode;
    int       x_startindex;
    int       x_endindex;    /* (one past last record position) */
    int       x_pauseindex;
    int       x_phase;       /* writing head */
    float     x_sync;
    float     x_syncincr;
    int       x_isrunning;   /* to know if sync should be 0.0 or 1.0 */
    t_clock  *x_clock;
    double    x_clocklasttick;
} t_record;

static t_class *record_class;

static void record_tick(t_record *x)
{
    double timesince = clock_gettimesince(x->x_clocklasttick);
    if (timesince >= RECORD_REDRAWPAUSE)
    {
	arsic_redraw((t_arsic *)x);
	x->x_clocklasttick = clock_getlogicaltime();
    }
    else clock_delay(x->x_clock, RECORD_REDRAWPAUSE - timesince);
}

static void record_setsync(t_record *x)
{
    /* CHECKED: clipped to array size -- using indices, not points */
    float range = (float)(x->x_endindex - x->x_startindex);
    int phase = x->x_phase;
    if (phase == SHARED_INT_MAX || range < 1.)
    {
	x->x_sync = (x->x_isrunning ? 1. : 0.);  /* CHECKED */
	x->x_syncincr = 0.;
    }
    else
    {
	x->x_sync = (float)(phase - x->x_startindex) / range;
	x->x_syncincr = 1. / range;
    }
}

static void record_mstoindex(t_record *x)
{
    t_arsic *sic = (t_arsic *)x;
    x->x_startindex = (int)(x->x_startpoint * sic->s_ksr);
    if (x->x_startindex < 0)
	x->x_startindex = 0;  /* CHECKED */
    x->x_endindex = (int)(x->x_endpoint * sic->s_ksr);
    if (x->x_endindex > sic->s_vecsize
	|| x->x_endindex <= 0)
	x->x_endindex = sic->s_vecsize;  /* CHECKED (both ways) */
    record_setsync(x);
}

static void record_set(t_record *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
    record_mstoindex(x);
}

static void record_reset(t_record *x)
{
    x->x_startpoint = x->x_endpoint = 0.;
    x->x_pauseindex = SHARED_INT_MAX;
    x->x_phase = SHARED_INT_MAX;
    x->x_isrunning = 0;
    record_mstoindex(x);
}

static void record_startpoint(t_record *x, t_floatarg f)
{
    x->x_startpoint = f;
    record_mstoindex(x);
}

static void record_endpoint(t_record *x, t_floatarg f)
{
    x->x_endpoint = f;
    record_mstoindex(x);
}

static void record_float(t_record *x, t_float f)
{
    if (x->x_isrunning = (f != 0))
    {
	/* CHECKED: no (re)start in append mode */
	/* LATER consider restart if x->x_pauseindex == SHARED_INT_MAX */
	x->x_phase = x->x_appendmode ? x->x_pauseindex : x->x_startindex;
	if (x->x_phase >= x->x_endindex) x->x_phase = SHARED_INT_MAX;
    }
    else if (x->x_phase != SHARED_INT_MAX)  /* CHECKED: no rewind */
    {
	clock_delay(x->x_clock, 10.);
	x->x_pauseindex = x->x_phase;
	x->x_phase = SHARED_INT_MAX;
    }
    record_setsync(x);
}

static void record_append(t_record *x, t_floatarg f)
{
    if (f != 0)
    {
	x->x_appendmode = 1;  /* CHECKED: always allow appending */
    }
    else x->x_appendmode = 0;
}

static void record_loop(t_record *x, t_floatarg f)
{
    x->x_loopmode = (f != 0);
}

static t_int *record_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    int nch = sic->s_nchannels;
    t_float *out = (t_float *)(w[3 + nch]);
    t_record *x = (t_record *)sic;
    int phase = x->x_phase;
    int endphase = x->x_endindex;
    float sync = x->x_sync;
    if (sic->s_playable && endphase > phase)
    {
	int vecsize = sic->s_vecsize;
	float syncincr = x->x_syncincr;
	int ch, over, i, nxfer, ndone = 0;
loopover:
	if ((nxfer = endphase - phase) > nblock)
	{
	    nxfer = nblock;
	    over = 0;
	}
	else over = 1;
	ch = nch;
	while (ch--)
	{
	    t_float *vp = sic->s_vectors[ch];
	    if (vp)
	    {
		t_float *ip = (t_float *)(w[3 + ch]) + ndone;
		vp += phase;
		i = nxfer;
		/* LATER consider handling under and overflows */
		while (i--) *vp++ = *ip++;
	    }
	}
	i = nxfer;

	sync = phase;
	syncincr = 1.;

	while (i--)
	{
	    *out++ = sync;
	    sync += syncincr;
	}
	if (over)
	{
	    clock_delay(x->x_clock, 0);
	    nblock -= nxfer;
	    if (x->x_loopmode
		&& (phase = x->x_startindex) < endphase)
	    {
		x->x_phase = phase;
		x->x_sync = sync = 0;
		if (nblock > 0)
		{
		    ndone += nxfer;
		    goto loopover;
		}
		goto alldone;
	    }
	    /* CHECKED: no restart in append mode */
	    x->x_pauseindex = SHARED_INT_MAX;
	    x->x_phase = SHARED_INT_MAX;
	    x->x_sync = 1.;
	    x->x_syncincr = 0.;
	}
	else
	{
	    x->x_phase += nxfer;
	    x->x_sync = sync;
	    goto alldone;
	}
    }
    while (nblock--) *out++ = -1; //sync;
alldone:
    return (w + sic->s_nperfargs + 1);
}

static void record_dsp(t_record *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, record_perform, 1);
    record_mstoindex(x);
}

static void record_free(t_record *x)
{
    arsic_free((t_arsic *)x);
    if (x->x_clock) clock_free(x->x_clock);
}

static void *record_new(t_symbol *s, t_floatarg f)
{
    /* one auxiliary signal:  sync output */
    t_record *x = (t_record *)arsic_new(record_class, s, (int)f, 0, 1);
    if (x)
    {
	int nch = arsic_getnchannels((t_arsic *)x);
	arsic_setminsize((t_arsic *)x, 2);
	x->x_appendmode = 0;
	x->x_loopmode = 0;
	record_reset(x);
	x->x_clock = clock_new(x, (t_method)record_tick);
	x->x_clocklasttick = clock_getlogicaltime();
	while (--nch)
	    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft-2"));
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft-1"));
	outlet_new((t_object *)x, &s_signal);
    }
    return (x);
}

void record_tilde_setup(void)
{
    record_class = class_new(gensym("record~"),
			     (t_newmethod)record_new,
			     (t_method)record_free,
			     sizeof(t_record), 0,
			     A_DEFSYM, A_DEFFLOAT, 0);
    arsic_setup(record_class, record_dsp, record_float);
    class_addmethod(record_class, (t_method)record_startpoint,
		    gensym("ft-2"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_endpoint,
		    gensym("ft-1"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_append,
		    gensym("append"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_loop,
		    gensym("loop"), A_FLOAT, 0);
    class_addmethod(record_class, (t_method)record_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(record_class, (t_method)record_reset,
		    gensym("reset"), 0);
}
