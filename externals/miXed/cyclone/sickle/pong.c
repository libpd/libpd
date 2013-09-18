/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKED whatever args, there are always 3 inlets (refman's rubbish) */

#include "m_pd.h"
#include "common/loud.h"
#include "unstable/forky.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define PONG_DEBUG
#endif

#define PONG_DEFLO  0.
#define PONG_DEFHI  1.

typedef struct _pong
{
    t_sic     x_sic;
    t_glist  *x_glist;
    int       x_mode;
} t_pong;

static t_class *pong_class;

static t_int *pong_perform(t_int *w)
{
    t_pong *x = (t_pong *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    if (x->x_mode) while (nblock--)
    {
	float f = *in1++;
	float lo = *in2++;
	float hi = *in3++;
	float range;
	if (lo < hi) range = hi - lo;
	else if (lo > hi)
	{
	    range = lo;
	    lo = hi;
	    hi = range;
	    range -= lo;
	}
	else
	{
	    *out++ = lo;
	    continue;
	}
	if (f < lo)
	{
	    if (f < lo - range)
	    {
		double dnorm = (f - lo) / range;
		dnorm -= (int)dnorm;
		*out++ = hi + dnorm * range;
	    }
	    else *out++ = f + range;
	}
	else if (f > hi)
	{
	    if (f > hi + range)
	    {
		double dnorm = (f - lo) / range;
		dnorm -= (int)dnorm;
		*out++ = lo + dnorm * range;
	    }
	    else *out++ = f - range;
	}
	else *out++ = f;
    }
    else while (nblock--)
    {
	float f = *in1++;
	float lo = *in2++;
	float hi = *in3++;
	float range;
	if (lo < hi) range = hi - lo;
	else if (lo > hi)
	{
	    range = lo;
	    lo = hi;
	    hi = range;
	    range -= lo;
	}
	else
	{
	    *out++ = lo;
	    continue;
	}
	if (f < lo)
	{
	    f = lo - f;
	    if (f <= range)
	    {
		*out++ = lo + f;
		continue;
	    }
	}
	else if (f > hi) f -= lo;
	else
	{
	    *out++ = f;
	    continue;
	}
	if (f > range + range)
	{
	    double dnorm = f / range;
	    int inorm = (int)dnorm;
	    if (inorm % 2)
		*out++ = lo + ((inorm + 1) - dnorm) * range;
	    else
		*out++ = lo + (dnorm - inorm) * range;
	}
	else *out++ = hi + range - f;
    }
    return (w + 7);
}

static t_int *pong_perform_nofeeders(t_int *w)
{
    t_pong *x = (t_pong *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    float lo = *in2;
    float hi = *in3;
    float range;
    double coef;
    if (lo < hi) range = hi - lo;
    else if (lo > hi)
    {
	range = lo;
	lo = hi;
	hi = range;
	range -= lo;
    }
    else
    {
	while (nblock--) *out++ = lo;
	goto done;
    }
    coef = 1. / range;
    if (x->x_mode) while (nblock--)
    {
	float f = *in1++;
	if (f < lo)
	{
	    if (f < lo - range)
	    {
		double dnorm = (f - lo) * coef;
		dnorm -= (int)dnorm;
		*out++ = hi + dnorm * range;
	    }
	    else *out++ = f + range;
	}
	else if (f > hi)
	{
	    if (f > hi + range)
	    {
		double dnorm = (f - lo) * coef;
		dnorm -= (int)dnorm;
		*out++ = lo + dnorm * range;
	    }
	    else *out++ = f - range;
	}
	else *out++ = f;
    }
    else while (nblock--)
    {
	float f = *in1++;
	if (f < lo)
	{
	    f = lo - f;
	    if (f <= range)
	    {
		*out++ = lo + f;
		continue;
	    }
	}
	else if (f > hi) f -= lo;
	else
	{
	    *out++ = f;
	    continue;
	}
	if (f > range + range)
	{
	    double dnorm = f * coef;
	    int inorm = (int)dnorm;
	    if (inorm % 2)
		*out++ = lo + ((inorm + 1) - dnorm) * range;
	    else
		*out++ = lo + (dnorm - inorm) * range;
	}
	else *out++ = hi + range - f;
    }
done:
    return (w + 7);
}

static void pong_dsp(t_pong *x, t_signal **sp)
{
    if (forky_hasfeeders((t_object *)x, x->x_glist, 1, &s_signal) ||
	forky_hasfeeders((t_object *)x, x->x_glist, 2, &s_signal))
	dsp_add(pong_perform, 6, x, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
    else
    {
#ifdef PONG_DEBUG
	loudbug_post("using pong_perform_nofeeders");
#endif
	dsp_add(pong_perform_nofeeders, 6, x, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
    }
}

static void pong_mode(t_pong *x, t_floatarg f)
{
    x->x_mode = ((int)f != 0);
}

static void *pong_new(t_symbol *s, int ac, t_atom *av)
{
    t_pong *x = (t_pong *)pd_new(pong_class);
    x->x_glist = canvas_getcurrent();
    x->x_mode = (ac && av->a_type == A_FLOAT
		 && (int)av->a_w.w_float != 0);
    sic_inlet((t_sic *)x, 1, PONG_DEFLO, 1, ac, av);
    sic_inlet((t_sic *)x, 2, PONG_DEFHI, 2, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void pong_tilde_setup(void)
{
    pong_class = class_new(gensym("pong~"),
			   (t_newmethod)pong_new, 0,
			   sizeof(t_pong), 0, A_GIMME, 0);
    sic_setup(pong_class, pong_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(pong_class, (t_method)pong_mode,
		    gensym("mode"), A_DEFFLOAT, 0);  /* CHECKED default */
}
