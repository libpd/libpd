/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

/* CHECKME (the refman): if the buffer~ has more channels, channels are mixed */

typedef t_arsic t_play;
static t_class *play_class;

static void play_set(t_play *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}

/* LATER optimize */
static t_int *play_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    int nch = sic->s_nchannels;
    t_int *outp = w + 4;
    if (sic->s_playable)
    {	
	t_play *x = (t_play *)sic;
	t_float *xin = (t_float *)(w[3]);
	int vecsize = sic->s_vecsize;
	t_float **vectable = sic->s_vectors;
	float ksr = sic->s_ksr;
	int nointerp = 0;
	int maxindex = (nointerp ? vecsize - 1 : vecsize - 3);
	int iblock;

	for (iblock = 0; iblock < nblock; iblock++)
	{
	    float phase = *xin++ * ksr;
	    int ndx;
	    int ch = nch;
	    float frac,  a,  b,  c,  d, cminusb;
	    if (phase < 0 || phase > maxindex)
		phase = 0;  /* CHECKED: a value 0, not ndx 0 */
	    ndx = (int)phase;
	    /* CHECKME: what kind of interpolation? (CHECKED: multi-point) */
	    if (ndx < 1)
		ndx = 1, frac = 0;
	    else if (ndx > maxindex)
		ndx = maxindex, frac = 1;
	    else frac = phase - ndx;
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

static void play_dsp(t_play *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, play_perform, 1);
}

static void play_free(t_play *x)
{
    arsic_free((t_arsic *)x);
}

static void *play_new(t_symbol *s, t_floatarg f)
{
    /* one auxiliary signal:  position input */
    t_play *x = (t_play *)arsic_new(play_class, s, (int)f, 0, 1);
    if (x)
    {
	int nch = arsic_getnchannels((t_arsic *)x);
	while (nch--)
	    outlet_new((t_object *)x, &s_signal);
    }
    return (x);
}

void play_tilde_setup(void)
{
    play_class = class_new(gensym("play~"),
			   (t_newmethod)play_new,
			   (t_method)play_free,
			   sizeof(t_play), 0,
			   A_DEFSYM, A_DEFFLOAT, 0);
    arsic_setup(play_class, play_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(play_class, (t_method)play_set,
		    gensym("set"), A_SYMBOL, 0);
}
