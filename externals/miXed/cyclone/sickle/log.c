/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define logf  log
#endif

#define LOG_MININPUT  1e-10  /* CHECKED */

typedef struct _log
{
    t_sic    x_sic;
    t_float  x_rcplogbase;  /* LATER consider using double (and log()) */
} t_log;

static t_class *log_class;

static void log_ft1(t_log *x, t_floatarg f)
{
    x->x_rcplogbase = (f == 0. ? 1. :   /* CHECKED no protection against NaNs */
		       (f == 1. ? 0. :  /* CHECKED */
			1. / logf(f)));
}

static t_int *log_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    t_float rcplogbase = *(t_float *)(w[4]);
    if (rcplogbase != 0.)
    {
	while (nblock--)
	{
	    float f = *in++;
	    if (f < LOG_MININPUT)
		f = LOG_MININPUT;  /* CHECKED */
	    *out++ = logf(f) * rcplogbase;
	}
    }
    else while (nblock--) *out++ = 0.;
    return (w + 5);
}

static void log_dsp(t_log *x, t_signal **sp)
{
    dsp_add(log_perform, 4, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec,
	    &x->x_rcplogbase);
}

static void *log_new(t_floatarg f)
{
    t_log *x = (t_log *)pd_new(log_class);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_signal);
    log_ft1(x, f);
    return (x);
}

void log_tilde_setup(void)
{
    log_class = class_new(gensym("log~"),
			  (t_newmethod)log_new, 0,
			  sizeof(t_log), 0, A_DEFFLOAT, 0);
    sic_setup(log_class, log_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(log_class, (t_method)log_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}
