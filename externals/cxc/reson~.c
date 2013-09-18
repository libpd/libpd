/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 */

/* Original code by Miller Puckette */
/* a non-interpolating reson filter, not very carefully coded... */
/* 11/29/94 modified to do interpolation - M. Danks */

#include "m_pd.h"

#include <stdlib.h>

#define BUFSIZE 4096

typedef struct resonctl
{
	float	c_freq;
	float	c_samprate;
	float	c_feedback;
	int		c_delayinsamps;
	float	c_fraction;
	int		c_phase;
	float	*c_buf;
} t_resonctl;

typedef struct sigreson
{
	t_object	x_obj;		/* header */
	t_resonctl	*x_ctl;	/* pointer to state */
	t_resonctl	x_cspace;	/* garage for state when not in a chain */
} t_sigreson;

/* the DSP routine -- called for every n samples of input */
static t_int *cu_reson(t_int *w)
{
	t_float *in1 = (t_float *)(w[1]);
	t_float *in2 = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	t_resonctl *x = (t_resonctl *)(w[4]);
	int n = (int)(w[5]);
	long i;
	int writephase = x->c_phase;
	for (i = 0; i < n; i++)
	{
			/* note two tricks: 1. input is read before output
			 * is written,  because the routine might be called
			 * in-place;
			 * 2 - a seed of 1E-20 is thrown in to avoid floating
			 * underflow which slows the calculation down.
			 */
		int readphase, phase, delayinsamps;
		float fraction, f, g, freq, freqtemp;

	    float ftemp;
	
		freq = *in2++;
		freqtemp = (freq < 1 ? 1 : freq);
	
		ftemp = x->c_samprate/freqtemp;
		if (ftemp >= BUFSIZE-1)
		    ftemp = BUFSIZE - 1.f;
		else if (ftemp < 1.0)
		    ftemp = 1.f;
		delayinsamps = (int)ftemp;
		fraction =  ftemp - delayinsamps;

		readphase = writephase - delayinsamps;
		phase = readphase & (BUFSIZE-1);
		f = x->c_buf[phase] + fraction * 
		    (x->c_buf[(phase-1)& (BUFSIZE-1)] - x->c_buf[phase]);
		g = *in1++;
		*out++ = x->c_buf[(writephase++) & (BUFSIZE-1)] =
			g + x->c_feedback * f + 1E-20f;
	}
	x->c_phase = writephase & (BUFSIZE-1);
	return (w+6);
}

/* sets the reson frequency */

void sigreson_float(t_sigreson *x, t_floatarg f)
{
	float ftemp;
	
	x->x_ctl->c_freq = (f < 1 ? 1 : f);
	
	ftemp = x->x_ctl->c_samprate/x->x_ctl->c_freq;
	if (ftemp >= BUFSIZE - 1)
	    ftemp = BUFSIZE - 1.f;
	else if (ftemp < 1.0)
	    ftemp = 1.f;
	x->x_ctl->c_delayinsamps = (int)ftemp;
	x->x_ctl->c_fraction =  ftemp - x->x_ctl->c_delayinsamps;
}

/* routine which FTS calls to put you on the DSP chain or take you off. */

static void sigreson_dsp(t_sigreson *x, t_signal **sp)
{
	x->x_ctl->c_samprate = sp[0]->s_sr;
	sigreson_float(x, x->x_ctl->c_freq);
	dsp_add(cu_reson, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, 
		x->x_ctl, sp[0]->s_n);
}

static void sigreson_ft1(t_sigreson *x, t_floatarg f) /* sets feedback */
{
	if (f > .99999) f = .99999f;
	else if (f < -.99999) f = -.99999f;
	x->x_ctl->c_feedback = (float)f;
}

static void sigreson_ff(t_sigreson *x)		/* cleanup on free */
{
	free(x->x_ctl->c_buf);
}

static t_class *sigreson_class;

void *sigreson_new(t_floatarg f,  t_floatarg g)
{
	t_sigreson *x = (t_sigreson *)pd_new(sigreson_class);
	outlet_new(&x->x_obj, &s_signal);

		/* things in "cspace" are things you'll actually use at DSP time */
	x->x_cspace.c_phase = 0;
	if (!(x->x_cspace.c_buf = (float *)malloc(BUFSIZE * sizeof(float))))
	{
		error("buffer alloc failed");
		return (0);
	}
	x->x_cspace.c_samprate = 44100.f;	    /* just a plausible default */

		/* control block is in the garage at startup */
	x->x_ctl = &x->x_cspace;
	sigreson_float(x, (t_float)f);		    /* setup params */
	sigreson_ft1(x, g);
		/* make a "float" inlet */
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	return (x);
}

void reson_tilde_setup()
{
	sigreson_class = class_new(gensym("reson~"), (t_newmethod)sigreson_new,
			(t_method)sigreson_ff, sizeof(t_sigreson), 0,
			A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addfloat(sigreson_class, (t_method)sigreson_float);
	class_addmethod(sigreson_class, (t_method)sigreson_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(sigreson_class, (t_method)nullfn, &s_signal, A_NULL);
	class_addmethod(sigreson_class, (t_method)sigreson_dsp, gensym("dsp"), A_NULL);
}

