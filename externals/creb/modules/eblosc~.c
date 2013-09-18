/*
 *   eblosc.c  - bandlimited oscillators with infinite support discontinuities 
 *   using minimum phase impulse, step & ramp
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

#include "filters.h"


typedef unsigned long long u64;
typedef unsigned long u32;



#define LPHASOR      (8*sizeof(u32)) // the phasor logsize
#define VOICES       8 // the number of oscillators
#define CUTOFF       0.8f // fraction of nyquist for impulse cutoff



typedef struct ebloscctl
{
    t_float c_pole[VOICES*2];      // complex poles
    t_float c_gain[VOICES*2];      // complex gains (waveform specific constants)
    t_float c_state[VOICES*2];     // complex state

    u32 c_phase;                // phase of main oscillator
    u32 c_phase2;               // phase of secondairy oscillator
    t_float c_prev_amp;         // previous input of comparator
    t_float c_phase_inc_scale;
    t_float c_scale;
    t_float c_scale_update;
    t_symbol *c_waveform;

} t_ebloscctl;

typedef struct eblosc
{
  t_object x_obj;
  t_float x_f;
  t_ebloscctl x_ctl;
} t_eblosc;


/* phase converters */
static inline float _phase_to_float(u32 p){
    return ((t_float)p) * (1.0f / 4294967296.0f);
}
static inline u32 _float_to_phase(t_float f){
    return (u32)(f * 4294967296.0f);
}



/* get one sample from the oscillator bank and perform time tick */
static inline t_float _osc_tick(t_ebloscctl *ctl)
{
    float sum = 0.0f;
    int i;
    /* sum  all voices */
    for (i=0; i<VOICES*2; i+=2){
	/* rotate state */
	vcmul2(ctl->c_state+i, ctl->c_pole+i);

	/* get real part and add to output */
	sum += ctl->c_state[0];
    }

    return sum;
}

/* add shifted impulse */
static inline void _add_impulse(t_ebloscctl *ctl, t_float t0)
{
    int i;
    for (i=0; i<VOICES*2; i+=2){
	/* contribution is a_i z_i^t_0 */

	float real = 1.0f;
	float imag = 0.0f;
	    
	ctl->c_state[0] += real;
	ctl->c_state[1] += imag;
    }
}


/* add step */
static inline void _add_step(t_ebloscctl *ctl)
{
    int i;
    for (i=0; i<VOICES*2; i+=2){
	/* contribution is a_i (1 - z_i) */

	float real = 1.0f;
	float imag = 0.0f;
	    
	ctl->c_state[0] += real;
	ctl->c_state[1] += imag;
    }
}


/* add shifted step */
static inline void _add_shifted_step(t_ebloscctl *ctl, t_float t0)
{
    int i;
    for (i=0; i<VOICES*2; i+=2){
	/* contribution is a_i (1 - z_i^t_0) */

	float real = 1.0f;
	float imag = 0.0f;
	    
	ctl->c_state[0] += real;
	ctl->c_state[1] += imag;
    }
}


#if 0
/* update waveplayers on zero cross */
static void _bang_comparator(t_ebloscctl *ctl, float prev, float curr)
{

    /* check for sign change */
    if ((prev * curr) < 0.0f){

	int voice;

	/* determine the location of the discontinuity (in oversampled
 	  coordiates using linear interpolation */

	float f = (t_float)S * curr / (curr - prev);

	/* get the offset in the oversample table */

	u32 table_index = (u32)f;

	/* determine the fractional part (in oversampled coordinates)
	   for linear interpolation */

	float table_frac_index = f - (t_float)table_index;

	/* set state (+ or -) */

	ctl->c_state =  (curr > 0.0f) ? 0.5f : -0.5f;
	
	/* steal the oldest voice */

	voice = ctl->c_next_voice++;
	ctl->c_next_voice &= VOICES-1;
	    
	/* initialize the new voice index and interpolation fraction */

	ctl->c_index[voice] = table_index;
	ctl->c_frac[voice] = table_frac_index;
	ctl->c_vscale[voice] = -ctl->c_scale * 2.0f * ctl->c_state;

    }

}

/* advance phasor and update waveplayers on phase wrap */
static void _bang_phasor(t_ebloscctl *ctl, float freq)
{
    u32 phase = ctl->c_phase;
    u32 phase_inc; 
    u32 oldphase;
    int voice;
    float scale = ctl->c_scale;

    /* get increment */
    float inc = freq * ctl->c_phase_inc_scale;

    /* calculate new phase
       the increment (and the phase) should be a multiple of S */
    if (inc < 0.0f) inc = -inc;
    phase_inc = ((u32)inc) & ~(S-1);
    oldphase = phase;
    phase += phase_inc;


    /* check for phase wrap */
    if (phase < oldphase){
	u32 phase_inc_decimated = phase_inc >> LOVERSAMPLE;
	u32 table_index;
	u32 table_phase;
	
	/* steal the oldest voice if we have a phase wrap */
	    
	voice = ctl->c_next_voice++;
	ctl->c_next_voice &= VOICES-1;
	    
	/* determine the location of the discontinuity (in oversampled
	   coordinates) which is S * (new phase) / (increment) */
	    
	table_index = phase / phase_inc_decimated;
	    
	/* determine the fractional part (in oversampled coordinates)
	   for linear interpolation */

	table_phase = phase - (table_index * phase_inc_decimated);
	    
	/* use it to initialize the new voice index and interpolation
	 * fraction */
	    
	ctl->c_index[voice] = table_index;
	ctl->c_frac[voice] = (t_float)table_phase / (t_float)phase_inc_decimated;
	ctl->c_vscale[voice] = scale;
	scale = scale * ctl->c_scale_update;

    }

    /* save state */
    ctl->c_phase = phase;
    ctl->c_scale = scale;
}


/* the 2 oscillator version: the second osc can reset the first osc's
   phase (hence it determines the pitch) the first osc determines the
   waveform */

static void _bang_hardsync_phasor(t_ebloscctl *ctl, float freq, float freq2)
{
    u32 phase = ctl->c_phase;
    u32 phase2 = ctl->c_phase2;
    u32 phase_inc; 
    u32 phase_inc2; 
    u32 oldphase;
    u32 oldphase2;
    int voice;
    float scale = ctl->c_scale;


    /* get increment */
    float inc = freq * ctl->c_phase_inc_scale;
    float inc2 = freq2 * ctl->c_phase_inc_scale;

    /* calculate new phases
       the increment (and the phase) should be a multiple of S */

    /* save previous phases */
    oldphase = phase;
    oldphase2 = phase2;

    /* update second osc */
    if (inc2 < 0.0f) inc2 = -inc2;
    phase_inc2 = ((u32)inc2) & ~(S-1);
    phase2 += phase_inc2;
    
    /* update first osc (freq should be >= freq of sync osc */
    if (inc < 0.0f) inc = -inc;
    phase_inc = ((u32)inc) & ~(S-1);
    if (phase_inc < phase_inc2) phase_inc = phase_inc2;
    phase += phase_inc;


    /* check for sync discontinuity (osc 2) */
    if (phase2 < oldphase2) {

	/* adjust phase depending on the location of the discontinuity in phase2:
	   phase/phase_inc == phase2/phase_inc2 */
	
	u64 pi = phase_inc >> LOVERSAMPLE;
	u64 pi2 = phase_inc2 >> LOVERSAMPLE;
	u64 lphase = ((u64)phase2 * pi) / pi2;
	phase = lphase & ~(S-1);
    }


    /* check for phase discontinuity (osc 1) */
    if (phase < oldphase){
	u32 phase_inc_decimated = phase_inc >> LOVERSAMPLE;
	u32 table_index;
	u32 table_phase;
	float stepsize;
	
	/* steal the oldest voice if we have a phase wrap */
	    
	voice = ctl->c_next_voice++;
	ctl->c_next_voice &= VOICES-1;
	    
	/* determine the location of the discontinuity (in oversampled
	   coordinates) which is S * (new phase) / (increment) */

	table_index = phase / phase_inc_decimated;
	    
	/* determine the fractional part (in oversampled coordinates)
	   for linear interpolation */

	table_phase = phase - (table_index * phase_inc_decimated);

	/* determine the step size. as opposed to saw/impulse
	   waveforms, the step is not always equal to one. it is:
	   oldphase - phase + phase_inc but for the unit step this
	   will overflow to zero, so we reduce the bit depth to
	   prevent overflow */

	stepsize = _phase_to_float(((oldphase-phase) >> LOVERSAMPLE)
				   + phase_inc_decimated) * (t_float)S;
	    
	/* use it to initialize the new voice index and interpolation fraction */
	    
	ctl->c_index[voice] = table_index;
	ctl->c_frac[voice] = (t_float)table_phase / (t_float)phase_inc_decimated;
	ctl->c_vscale[voice] = scale * stepsize;
	scale = scale * ctl->c_scale_update;

    }

    /* save state */
    ctl->c_phase = phase;
    ctl->c_phase2 = phase2;
    ctl->c_scale = scale;
}


static t_int *eblosc_perform_hardsync_saw(t_int *w)
{
    t_float *freq     = (t_float *)(w[3]);
    t_float *freq2     = (t_float *)(w[4]);
    t_float *out      = (t_float *)(w[5]);
    t_ebloscctl *ctl  = (t_ebloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;

    /* set postfilter cutoff */
    ctl->c_butter->setButterHP(0.85f * (*freq / sys_getsr()));
    
    while (n--) {
	float frequency = *freq++;
	float frequency2 = *freq2++;

	/* get the bandlimited discontinuity */
	float sample = _get_bandlimited_discontinuity(ctl, bls);

	/* add aliased sawtooth wave */
	sample += _phase_to_float(ctl->c_phase) - 0.5f;

	/* highpass filter output to remove DC offset and low
	 * frequency aliasing */
	ctl->c_butter->BangSmooth(sample, sample, 0.05f);

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_hardsync_phasor(ctl, frequency2, frequency);
	
    }
    
    return (w+6);
}

static t_int *eblosc_perform_saw(t_int *w)
{
    t_float *freq     = (t_float *)(w[3]);
    t_float *out      = (t_float *)(w[4]);
    t_ebloscctl *ctl  = (t_ebloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;
    
    while (n--) {
	float frequency = *freq++;

	/* get the bandlimited discontinuity */
	float sample = _get_bandlimited_discontinuity(ctl, bls);

	/* add aliased sawtooth wave */
	sample += _phase_to_float(ctl->c_phase) - 0.5f;

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_phasor(ctl, frequency);
	
    }
    
    return (w+5);
}



static t_int *eblosc_perform_pulse(t_int *w)
{
    t_float *freq     = (t_float *)(w[3]);
    t_float *out      = (t_float *)(w[4]);
    t_ebloscctl *ctl  = (t_ebloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;


    /* set postfilter cutoff */
    ctl->c_butter->setButterHP(0.85f * (*freq / sys_getsr()));
    
    while (n--) {
	float frequency = *freq++;

	/* get the bandlimited discontinuity */
	float sample = _get_bandlimited_discontinuity(ctl, bli);

	/* highpass filter output to remove DC offset and low
	 * frequency aliasing */
	ctl->c_butter->BangSmooth(sample, sample, 0.05f);

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_phasor(ctl, frequency);
	
    }
    
    return (w+5);
}

static t_int *eblosc_perform_comparator(t_int *w)
{
    t_float *amp      = (t_float *)(w[3]);
    t_float *out      = (t_float *)(w[4]);
    t_ebloscctl *ctl  = (t_ebloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;
    t_float prev_amp = ctl->c_prev_amp;
    
    while (n--) {
	float curr_amp = *amp++;

	/* exact zero won't work for zero detection (sic) */
	if (curr_amp == 0.0f) curr_amp = 0.0000001f;

	/* get the bandlimited discontinuity */
	float sample = _get_bandlimited_discontinuity(ctl, bls);

	/* add the block wave state */
	sample += ctl->c_state;

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_comparator(ctl, prev_amp, curr_amp);

	prev_amp = curr_amp;
	
    }

    ctl->c_prev_amp = prev_amp;
    
    return (w+5);
}

static void eblosc_phase(t_eblosc *x, t_float f)
{
    x->x_ctl.c_phase = _float_to_phase(f);
    x->x_ctl.c_phase2 = _float_to_phase(f);
}

static void eblosc_phase1(t_eblosc *x, t_float f)
{
    x->x_ctl.c_phase = _float_to_phase(f);
}

static void eblosc_phase2(t_eblosc *x, t_float f)
{
    x->x_ctl.c_phase2 = _float_to_phase(f);
}

static void eblosc_dsp(t_eblosc *x, t_signal **sp)
{
  int n = sp[0]->s_n;

  /* set sampling rate scaling for phasors */
  x->x_ctl.c_phase_inc_scale = 4.0f * (t_float)(1<<(LPHASOR-2)) / sys_getsr();


  /* setup & register the correct process routine depending on the waveform */

  /* 2 osc */
  if (x->x_ctl.c_waveform == gensym("syncsaw")){
      x->x_ctl.c_scale = 1.0f;
      x->x_ctl.c_scale_update = 1.0f;
      dsp_add(eblosc_perform_hardsync_saw, 5, &x->x_ctl, sp[0]->s_n,
	      sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
  }

  /* 1 osc */
  else if (x->x_ctl.c_waveform == gensym("pulse")){
      x->x_ctl.c_scale = 1.0f;
      x->x_ctl.c_scale_update = 1.0f;
      dsp_add(eblosc_perform_pulse, 4, &x->x_ctl, sp[0]->s_n,
	      sp[0]->s_vec, sp[1]->s_vec);
  }
  else if (x->x_ctl.c_waveform == gensym("pulse2")){
      x->x_ctl.c_phase_inc_scale *= 2;
      x->x_ctl.c_scale = 1.0f;
      x->x_ctl.c_scale_update = -1.0f;
      dsp_add(eblosc_perform_pulse, 4, &x->x_ctl, sp[0]->s_n,
	      sp[0]->s_vec, sp[1]->s_vec);
  }
  else if (x->x_ctl.c_waveform == gensym("comparator")){
      x->x_ctl.c_scale = 1.0f;
      x->x_ctl.c_scale_update = 1.0f;
      dsp_add(eblosc_perform_comparator, 4, &x->x_ctl,
	      sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
  }
  else{
       x->x_ctl.c_scale = 1.0f;
      x->x_ctl.c_scale_update = 1.0f;
      dsp_add(eblosc_perform_saw, 4, &x->x_ctl, sp[0]->s_n,
	      sp[0]->s_vec, sp[1]->s_vec);
  }



}                                  
static void eblosc_free(t_eblosc *x)
{
    delete x->x_ctl.c_butter;
}

t_class *eblosc_class;

static void *eblosc_new(t_symbol *s)
{
    t_eblosc *x = (t_eblosc *)pd_new(eblosc_class);
    int i;

    /* out 1 */
    outlet_new(&x->x_obj, gensym("signal"));

    /* optional signal inlets */
    if (s == gensym("syncsaw")){
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,
		  gensym("signal"), gensym("signal"));  
    }

    /* optional phase inlet */
    if (s != gensym("comparator")){
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,
		  gensym("float"), gensym("phase"));  
    }

    /* create the postfilter */
    x->x_ctl.c_butter = new DSPIfilterSeries(3);

    /* init oscillators */
    for (i=0; i<VOICES; i++) {
      x->x_ctl.c_index[i] = N-2;
      x->x_ctl.c_frac[i] = 0.0f;
    }

    /* init rest of state data */
    eblosc_phase(x, 0);
    eblosc_phase2(x, 0);
    x->x_ctl.c_state = 0.0;
    x->x_ctl.c_prev_amp = 0.0;
    x->x_ctl.c_next_voice = 0;
    x->x_ctl.c_scale = 1.0f;
    x->x_ctl.c_scale_update = 1.0f;
    x->x_ctl.c_waveform = s;

    return (void *)x;
}





extern "C"
{
    void eblosc_tilde_setup(void)
    {
	//post("eblosc~ v0.1");
	
	build_tables();
	
	eblosc_class = class_new(gensym("eblosc~"), (t_newmethod)eblosc_new,
				(t_method)eblosc_free, sizeof(t_eblosc),
				 0, A_DEFSYMBOL, A_NULL);
	CLASS_MAINSIGNALIN(eblosc_class, t_eblosc, x_f);
	class_addmethod(eblosc_class, (t_method)eblosc_dsp,
			gensym("dsp"), A_NULL); 
	class_addmethod(eblosc_class, (t_method)eblosc_phase,
			gensym("phase"), A_FLOAT, A_NULL); 
	class_addmethod(eblosc_class, (t_method)eblosc_phase2,
			gensym("phase2"), A_FLOAT, A_NULL); 
    }
}

#endif
