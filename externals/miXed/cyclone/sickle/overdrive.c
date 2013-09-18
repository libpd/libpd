/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

/* FIXME this is unnecessary in vc > 6.0 and darwin? */
#if defined(MSW) || defined(MACOSX)
#define powf  pow
#endif

typedef struct _overdrive
{
    t_sic  x_sic;
    float  x_drivefactor;
} t_overdrive;

static t_class *overdrive_class;

/* CHECKED this is redundant (a design flaw), LATER fitter-optionally use
   float-to-signal conversion. */
static void overdrive_float(t_overdrive *x, t_float f)
{
    x->x_drivefactor = f;
}

static void overdrive_ft1(t_overdrive *x, t_floatarg f)
{
    x->x_drivefactor = f;
}

/* CHECKED negative parameter values may cause output to go out of bounds */
static t_int *overdrive_perform(t_int *w)
{
    float df = *(t_float *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	float f = *in++;
	if (f >= 1.)  /* CHECKED incompatible (garbage for sig~ 1.) */
	    *out++ = 1.;  /* CHECKED constant for > 1. */
	else if (f > 0.)
	    *out++ = 1. - powf(1. - f, df);  /* CHECKED */
	else if (f > -1.)  /* CHECKED incompatible (garbage for sig~ -1.) */
	    *out++ = powf(1. + f, df) - 1.;  /* CHECKED */
	else
	    *out++ = -1.;  /* CHECKED constant for < -1. */
    }
    return (w + 5);
}

static void overdrive_dsp(t_overdrive *x, t_signal **sp)
{
    dsp_add(overdrive_perform, 4, &x->x_drivefactor,
	    sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *overdrive_new(t_floatarg f)
{
    t_overdrive *x = (t_overdrive *)pd_new(overdrive_class);
    x->x_drivefactor = f;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void overdrive_tilde_setup(void)
{
    overdrive_class = class_new(gensym("overdrive~"),
				(t_newmethod)overdrive_new, 0,
				sizeof(t_overdrive), 0, A_DEFFLOAT, 0);
    /* CHECKED no float-to-signal conversion */
    sic_setup(overdrive_class, overdrive_dsp, overdrive_float);
    class_addmethod(overdrive_class, (t_method)overdrive_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}
