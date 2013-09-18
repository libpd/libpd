/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

/* CHECKME (the refman): the extra channels are not played */

typedef struct _wave
{
    t_arsic  x_arsic;
    int      x_nointerp;
} t_wave;

static t_class *wave_class;

static void wave_interp(t_wave *x, t_floatarg f)
{
    x->x_nointerp = (f == 0);
    arsic_setminsize((t_arsic *)x, (x->x_nointerp ? 1 : 4));
    arsic_check((t_arsic *)x);
}

static void wave_set(t_wave *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}

static t_int *wave_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    int nch = sic->s_nchannels;
    t_int *outp = w + 6;
    if (sic->s_playable)
    {	
	t_wave *x = (t_wave *)sic;
	t_float *xin = (t_float *)(w[3]);
	t_float *sin = (t_float *)(w[4]);
	t_float *ein = (t_float *)(w[5]);
	int vecsize = sic->s_vecsize;
	t_float **vectable = sic->s_vectors;
	float ksr = sic->s_ksr;
	int nointerp = x->x_nointerp;
	int maxindex = (nointerp ? vecsize - 1 : vecsize - 3);
	int iblock;

	for (iblock = 0; iblock < nblock; iblock++)
	{
	    float spos = *sin++ * ksr;
	    float xpos = *ein++ * ksr;
	    /* msp seems to be buggy here, but CHECKME again */
	    int siz = (int)((xpos > 0 ? xpos : maxindex) - spos);
	    float phase = *xin++;
	    int ndx;
	    int ch = nch;
	    /* CHECKED: phase is clipped, not wrapped */
	    if (phase < 0) phase = 0;
	    else if (phase > 1.0) phase = 1.0;
	    xpos = (siz > 0 ? spos + siz * phase : spos);
	    ndx = (int)xpos;
	    if (nointerp)
	    {
		if (ndx < 0) ndx = 0;
		else if (ndx > maxindex) ndx = maxindex;
		while (ch--)
		{
		    t_float *vp = vectable[ch];
		    t_float *out = (t_float *)(outp[ch]);
		    out[iblock] = (vp ? vp[ndx] : 0);
		}
	    }
	    else
	    {
		float frac,  a,  b,  c,  d, cminusb;
		if (ndx < 1)
		    ndx = 1, frac = 0;
		else if (ndx > maxindex)
		    ndx = maxindex, frac = 1;
		else frac = xpos - ndx;
		while (ch--)
		{
		    t_float *vp = vectable[ch];
		    t_float *out = (t_float *)(outp[ch]);
		    if (vp)
		    {
			vp += ndx;
			a = vp[-1];
			b = vp[0];
			c = vp[1];
			d = vp[2];
			cminusb = c-b;
			out[iblock] = b + frac * (
			    cminusb - 0.1666667f * (1. - frac) * (
				(d - a - 3.0f * cminusb) * frac
				+ (d + 2.0f * a - 3.0f * b)
			    )
			);
		    }
		    else out[iblock] = 0;
		}
	    }
	}
    }
    else
    {
	int ch = nch;
	while (ch--)
	{
	    t_float *out = (t_float *)outp[ch];
	    int n = nblock;
	    while (n--) *out++ = 0;
	}
    }
    return (w + sic->s_nperfargs + 1);
}

static void wave_dsp(t_wave *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, wave_perform, 1);
}

static void wave_free(t_wave *x)
{
    arsic_free((t_arsic *)x);
}

static void *wave_new(t_symbol *s, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
    /* three auxiliary signals:  phase, clipstart, and clipend inputs */
    t_wave *x = (t_wave *)arsic_new(wave_class, s, (int)f3, 0, 3);
    if (x)
    {
	int nch = arsic_getnchannels((t_arsic *)x);
	if (f1 < 0) f1 = 0;
	if (f2 < 0) f2 = 0;
	sic_newinlet((t_sic *)x, f1);
	sic_newinlet((t_sic *)x, f2);
	while (nch--)
	    outlet_new((t_object *)x, &s_signal);
	wave_interp(x, 1);
    }
    return (x);
}

void wave_tilde_setup(void)
{
    wave_class = class_new(gensym("wave~"),
			   (t_newmethod)wave_new,
			   (t_method)wave_free,
			   sizeof(t_wave), 0,
			   A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    arsic_setup(wave_class, wave_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(wave_class, (t_method)wave_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(wave_class, (t_method)wave_interp,
		    gensym("interp"), A_FLOAT, 0);
}
