/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKME no reset after changing of a window size? */
/* CHECKME overlap */

#include <math.h>
#include "m_pd.h"
#include "common/loud.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define sqrtf  sqrt
#endif

#define AVERAGE_DEFNPOINTS  100  /* CHECKME */
#define AVERAGE_DEFMODE     AVERAGE_BIPOLAR
enum { AVERAGE_BIPOLAR, AVERAGE_ABSOLUTE, AVERAGE_RMS };

typedef struct _average
{
    t_sic     x_sic;
    int       x_mode;
    float   (*x_sumfn)(t_float*, int, float);
    int       x_phase;
    int       x_npoints;
    float     x_result;
    float     x_accum;
    t_clock  *x_clock;
} t_average;

static t_class *average_class;

static void average_tick(t_average *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_result);
}

static float average_bipolarsum(t_float *in, int nxfer, float accum)
{
    while (nxfer--)
	accum += *in++;
    return (accum);
}

static float average_absolutesum(t_float *in, int nxfer, float accum)
{
    while (nxfer--)
    {
	float f = *in++;
	accum += (f >= 0 ? f : -f);
    }
    return (accum);
}

static float average_rmssum(t_float *in, int nxfer, float accum)
{
    while (nxfer--)
    {
	float f = *in++;
	accum += f * f;
    }
    return (accum);
}

static void average_setmode(t_average *x, int mode)
{
    if (mode == AVERAGE_BIPOLAR)
	x->x_sumfn = average_bipolarsum;
    else if (mode == AVERAGE_ABSOLUTE)
	x->x_sumfn = average_absolutesum;
    else if (mode == AVERAGE_RMS)
	x->x_sumfn = average_rmssum;
    else
    {
	loudbug_bug("average_setmode");
	return;
    }
    x->x_mode = mode;
    x->x_phase = x->x_npoints;
    x->x_accum = 0;
}

static void average_float(t_average *x, t_float f)
{
    int i = (int)f;  /* CHECKME noninteger */
    if (i > 0)  /* CHECKME */
    {
	x->x_npoints = i;
	x->x_phase = x->x_npoints;
	x->x_accum = 0;
    }
}

static void average_bipolar(t_average *x)
{
    average_setmode(x, AVERAGE_BIPOLAR);
}

static void average_absolute(t_average *x)
{
    average_setmode(x, AVERAGE_ABSOLUTE);
}

static void average_rms(t_average *x)
{
    average_setmode(x, AVERAGE_RMS);
}

static t_int *average_perform(t_int *w)
{
    t_average *x = (t_average *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    float (*sumfn)(t_float*, int, float) = x->x_sumfn;
    int phase = x->x_phase;
    if (phase <= nblock)
    {
	float accum = (*sumfn)(in, phase, x->x_accum);
	nblock -= phase;
	if (x->x_mode == AVERAGE_RMS)
	    /* CHECKME scaling and FIXME */
	    x->x_result = sqrtf(accum / x->x_npoints);
	else
	    x->x_result = accum / x->x_npoints;
	clock_delay(x->x_clock, 0);
	x->x_accum = 0;
	if (nblock < x->x_npoints)
	    x->x_phase = x->x_npoints - nblock;
	else
	{
	    x->x_phase = x->x_npoints;
	    return (w + 4);
	}
    }
    else x->x_phase -= nblock;
    x->x_accum = (*sumfn)(in, nblock, x->x_accum);
    return (w + 4);
}

static void average_dsp(t_average *x, t_signal **sp)
{
    dsp_add(average_perform, 3, x, sp[0]->s_n, sp[0]->s_vec);
}

static void average_free(t_average *x)
{
    if (x->x_clock) clock_free(x->x_clock);
}

static void *average_new(t_symbol *s, t_floatarg f)
{
    t_average *x = (t_average *)pd_new(average_class);
    int i = (int)f;  /* CHECKME noninteger */
    int mode;
    /* CHECKED it looks like memory is allocated for the entire window,
       in tune with the refman's note about ``maximum averaging interval'' --
       needed for dynamic control over window size, or what? LATER rethink */
    x->x_npoints = (i > 0 ?  /* CHECKME */
		    i : AVERAGE_DEFNPOINTS);
    if (s == gensym("bipolar"))
	mode = AVERAGE_BIPOLAR;
    else if (s == gensym("absolute"))
	mode = AVERAGE_ABSOLUTE;
    else if (s == gensym("rms"))
	mode = AVERAGE_RMS;
    else
    {
	mode = AVERAGE_DEFMODE;
	/* CHECKME a warning if (s && s != &s_) */
    }
    average_setmode(x, mode);
    /* CHECKME if not x->x_phase = 0 */
    outlet_new((t_object *)x, &s_float);
    x->x_clock = clock_new(x, (t_method)average_tick);
    return (x);
}

void average_tilde_setup(void)
{
    average_class = class_new(gensym("average~"),
			      (t_newmethod)average_new,
			      (t_method)average_free,
			      sizeof(t_average), 0,
			      A_DEFFLOAT, A_DEFSYM, 0);
    sic_setup(average_class, average_dsp, average_float);
    class_addmethod(average_class, (t_method)average_bipolar,
		    gensym("bipolar"), 0);
    class_addmethod(average_class, (t_method)average_absolute,
		    gensym("absolute"), 0);
    class_addmethod(average_class, (t_method)average_rms,
		    gensym("rms"), 0);
}
