/*
 *   blosc.c  - bandlimited oscillators 
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


#include "DSPIcomplex.h"
#include "DSPIfilters.h"

typedef unsigned long long u64;
typedef unsigned long u32;



#define LPHASOR      (8*sizeof(u32)) // the phasor logsize
#define VOICES       8 // the number of waveform voices
#define LLENGTH      6 // the loglength of a fractional delayed basic waveform
#define LOVERSAMPLE  6 // the log of the oversampling factor (nb of fract delayed waveforms)
#define LPAD         1 // the log of the time padding factor (to reduce time aliasing) 
#define LTABLE       (LLENGTH+LOVERSAMPLE)
#define N            (1<<LTABLE)
#define M            (1<<(LTABLE+LPAD))
#define S            (1<<LOVERSAMPLE)
#define L            (1<<LLENGTH)
#define LMASK        (L-1)

#define WALPHA       0.1 // windowing alpha (0 = cos -> 1 = rect)
#define CUTOFF       0.8 // fraction of nyquist for impulse cutoff
#define NBPERIODS    ((t_float)(L) * CUTOFF / 2.0)

/* sample buffers */
static t_float bli[N]; // band limited impulse
static t_float bls[N]; // band limited step
static t_float blr[N]; // band limited ramp


typedef struct bloscctl
{
    t_int c_index[VOICES];      // array of indices in sample table
    t_float c_frac[VOICES];     // array of fractional indices
    t_float c_vscale[VOICES];   // array of scale factors
    t_int c_next_voice;         // next voice to steal (round robin)
    u32 c_phase;                // phase of main oscillator
    u32 c_phase2;               // phase of secondairy oscillator
    t_float c_state;            // state of the square wave
    t_float c_prev_amp;         // previous input of comparator
    t_float c_phase_inc_scale;
    t_float c_scale;
    t_float c_scale_update;
    DSPIfilterSeries* c_butter; // the series filter 
    t_symbol *c_waveform;

} t_bloscctl;

typedef struct blosc
{
  t_object x_obj;
  t_float x_f;
  t_bloscctl x_ctl;
} t_blosc;


/* phase converters */
static inline t_float _phase_to_float(u32 p){return ((t_float)p) * (1.0 / 4294967296.0);}
static inline u32 _float_to_phase(t_float f){return ((u32)(f * 4294967296.0)) & ~(S-1);}


/* flat table: better for linear interpolation */
static inline t_float _play_voice_lint(t_float *table, t_int *index, t_float frac, t_float scale)
{
    int i = *index;

    /* perform linear interpolation */
    t_float f = (((1.0 - frac) * table[i]) + (table[i+1] * frac)) * scale;

    /* increment phase index if next 2 elements will still be inside table
       if not there's no increment and the voice will keep playing the same sample */

    i += (((i+S+1) >> LTABLE) ^ 1) << LOVERSAMPLE; 

    *index = i;
    return f;
}

/* get one sample from the bandlimited discontinuity wavetable playback syth */
static inline t_float _get_bandlimited_discontinuity(t_bloscctl *ctl, t_float *table)
{
    t_float sum = 0.0;
    int i;
    /* sum  all voices */
    for (i=0; i<VOICES; i++){
	sum += _play_voice_lint(table, ctl->c_index+i, ctl->c_frac[i], ctl->c_vscale[i]);
    }

    return sum;
}


/* update waveplayers on zero cross */
static void _bang_comparator(t_bloscctl *ctl, t_float prev, t_float curr)
{

    /* check for sign change */
    if ((prev * curr) < 0.0){

	int voice;

	/* determine the location of the discontinuity (in oversampled coordiates
 	  using linear interpolation */

	t_float f = (t_float)S * curr / (curr - prev);

	/* get the offset in the oversample table */

	u32 table_index = (u32)f;

	/* determine the fractional part (in oversampled coordinates)
	   for linear interpolation */

	t_float table_frac_index = f - (t_float)table_index;

	/* set state (+ or -) */

	ctl->c_state =  (curr > 0.0) ? 0.5 : -0.5;
	
	/* steal the oldest voice */

	voice = ctl->c_next_voice++;
	ctl->c_next_voice &= VOICES-1;
	    
	/* initialize the new voice index and interpolation fraction */

	ctl->c_index[voice] = table_index;
	ctl->c_frac[voice] = table_frac_index;
	ctl->c_vscale[voice] = -ctl->c_scale * 2.0 * ctl->c_state;

    }

}


/* advance phasor and update waveplayers on phase wrap */
static void _bang_phasor(t_bloscctl *ctl, t_float freq)
{
    u32 phase = ctl->c_phase;
    u32 phase_inc; 
    u32 oldphase;
    int voice;
    t_float scale = ctl->c_scale;

    /* get increment */
    t_float inc = freq * ctl->c_phase_inc_scale;

    /* calculate new phase
       the increment (and the phase) should be a multiple of S */
    if (inc < 0.0) inc = -inc;
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
	    
	/* determine the location of the discontinuity (in oversampled coordinates)
	   which is S * (new phase) / (increment) */
	    
	table_index = phase / phase_inc_decimated;
	    
	/* determine the fractional part (in oversampled coordinates)
	   for linear interpolation */

	table_phase = phase - (table_index * phase_inc_decimated);
	    
	/* use it to initialize the new voice index and interpolation fraction */
	    
	ctl->c_index[voice] = table_index;
	ctl->c_frac[voice] = (t_float)table_phase / (t_float)phase_inc_decimated;
	ctl->c_vscale[voice] = scale;
	scale = scale * ctl->c_scale_update;

    }

    /* save state */
    ctl->c_phase = phase;
    ctl->c_scale = scale;
}


/* the 2 oscillator version:
   the second osc can reset the first osc's phase (hence it determines the pitch)
   the first osc determines the waveform */

static void _bang_hardsync_phasor(t_bloscctl *ctl, t_float freq, t_float freq2)
{
    u32 phase = ctl->c_phase;
    u32 phase2 = ctl->c_phase2;
    u32 phase_inc; 
    u32 phase_inc2; 
    u32 oldphase;
    u32 oldphase2;
    int voice;
    t_float scale = ctl->c_scale;


    /* get increment */
    t_float inc = freq * ctl->c_phase_inc_scale;
    t_float inc2 = freq2 * ctl->c_phase_inc_scale;

    /* calculate new phases
       the increment (and the phase) should be a multiple of S */

    /* save previous phases */
    oldphase = phase;
    oldphase2 = phase2;

    /* update second osc */
    if (inc2 < 0.0) inc2 = -inc2;
    phase_inc2 = ((u32)inc2) & ~(S-1);
    phase2 += phase_inc2;
    
    /* update first osc (freq should be >= freq of sync osc */
    if (inc < 0.0) inc = -inc;
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
	t_float stepsize;
	
	/* steal the oldest voice if we have a phase wrap */
	    
	voice = ctl->c_next_voice++;
	ctl->c_next_voice &= VOICES-1;
	    
	/* determine the location of the discontinuity (in oversampled coordinates)
	   which is S * (new phase) / (increment) */

	table_index = phase / phase_inc_decimated;
	    
	/* determine the fractional part (in oversampled coordinates)
	   for linear interpolation */

	table_phase = phase - (table_index * phase_inc_decimated);

	/* determine the step size
	   as opposed to saw/impulse waveforms, the step is not always equal to one. it is:
           oldphase - phase + phase_inc 
	   but for the unit step this will overflow to zero, so we
	   reduce the bit depth to prevent overflow */

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


static t_int *blosc_perform_hardsync_saw(t_int *w)
{
    t_float *freq     = (t_float *)(w[3]);
    t_float *freq2     = (t_float *)(w[4]);
    t_float *out      = (t_float *)(w[5]);
    t_bloscctl *ctl  = (t_bloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;

    /* set postfilter cutoff */
    ctl->c_butter->setButterHP(0.85 * (*freq / sys_getsr()));
    
    while (n--) {
	t_float frequency = *freq++;
	t_float frequency2 = *freq2++;

	/* get the bandlimited discontinuity */
	t_float sample = _get_bandlimited_discontinuity(ctl, bls);

	/* add aliased sawtooth wave */
	sample += _phase_to_float(ctl->c_phase) - 0.5;

	/* highpass filter output to remove DC offset and low frequency aliasing */
	ctl->c_butter->BangSmooth(sample, sample, 0.05);

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_hardsync_phasor(ctl, frequency2, frequency);
	
    }
    
    return (w+6);
}

static t_int *blosc_perform_saw(t_int *w)
{
    t_float *freq     = (t_float *)(w[3]);
    t_float *out      = (t_float *)(w[4]);
    t_bloscctl *ctl  = (t_bloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;
    
    while (n--) {
	t_float frequency = *freq++;

	/* get the bandlimited discontinuity */
	t_float sample = _get_bandlimited_discontinuity(ctl, bls);

	/* add aliased sawtooth wave */
	sample += _phase_to_float(ctl->c_phase) - 0.5;

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_phasor(ctl, frequency);
	
    }
    
    return (w+5);
}



static t_int *blosc_perform_pulse(t_int *w)
{
    t_float *freq     = (t_float *)(w[3]);
    t_float *out      = (t_float *)(w[4]);
    t_bloscctl *ctl  = (t_bloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;


    /* set postfilter cutoff */
    ctl->c_butter->setButterHP(0.85 * (*freq / sys_getsr()));
    
    while (n--) {
	t_float frequency = *freq++;

	/* get the bandlimited discontinuity */
	t_float sample = _get_bandlimited_discontinuity(ctl, bli);

	/* highpass filter output to remove DC offset and low frequency aliasing */
	ctl->c_butter->BangSmooth(sample, sample, 0.05);

	/* send to output */
	*out++ = sample;

	/* advance phasor */
	_bang_phasor(ctl, frequency);
	
    }
    
    return (w+5);
}

static t_int *blosc_perform_comparator(t_int *w)
{
    t_float *amp      = (t_float *)(w[3]);
    t_float *out      = (t_float *)(w[4]);
    t_bloscctl *ctl  = (t_bloscctl *)(w[1]);
    t_int n           = (t_int)(w[2]);
    t_int i;
    t_float prev_amp = ctl->c_prev_amp;
    
    while (n--) {
	t_float curr_amp = *amp++;

	/* exact zero won't work for zero detection (sic) */
	if (curr_amp == 0.0) curr_amp = 0.0000001;

	/* get the bandlimited discontinuity */
	t_float sample = _get_bandlimited_discontinuity(ctl, bls);

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

static void blosc_phase(t_blosc *x, t_float f)
{
    x->x_ctl.c_phase = _float_to_phase(f);
    x->x_ctl.c_phase2 = _float_to_phase(f);
}

static void blosc_phase1(t_blosc *x, t_float f)
{
    x->x_ctl.c_phase = _float_to_phase(f);
}

static void blosc_phase2(t_blosc *x, t_float f)
{
    x->x_ctl.c_phase2 = _float_to_phase(f);
}

static void blosc_dsp(t_blosc *x, t_signal **sp)
{
  int n = sp[0]->s_n;

  /* set sampling rate scaling for phasors */
  x->x_ctl.c_phase_inc_scale = 4.0 * (t_float)(1<<(LPHASOR-2)) / sys_getsr();


  /* setup & register the correct process routine depending on the waveform */

  /* 2 osc */
  if (x->x_ctl.c_waveform == gensym("syncsaw")){
      x->x_ctl.c_scale = 1.0;
      x->x_ctl.c_scale_update = 1.0;
      dsp_add(blosc_perform_hardsync_saw, 5, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
  }

  /* 1 osc */
  else if (x->x_ctl.c_waveform == gensym("pulse")){
      x->x_ctl.c_scale = 1.0;
      x->x_ctl.c_scale_update = 1.0;
      dsp_add(blosc_perform_pulse, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
  }
  else if (x->x_ctl.c_waveform == gensym("pulse2")){
      x->x_ctl.c_phase_inc_scale *= 2;
      x->x_ctl.c_scale = 1.0;
      x->x_ctl.c_scale_update = -1.0;
      dsp_add(blosc_perform_pulse, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
  }
  else if (x->x_ctl.c_waveform == gensym("comparator")){
      x->x_ctl.c_scale = 1.0;
      x->x_ctl.c_scale_update = 1.0;
      dsp_add(blosc_perform_comparator, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
  }
  else{
       x->x_ctl.c_scale = 1.0;
      x->x_ctl.c_scale_update = 1.0;
      dsp_add(blosc_perform_saw, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
  }



}                                  
static void blosc_free(t_blosc *x)
{
    delete x->x_ctl.c_butter;
}

t_class *blosc_class;

static void *blosc_new(t_symbol *s)
{
    t_blosc *x = (t_blosc *)pd_new(blosc_class);
    int i;

    /* out 1 */
    outlet_new(&x->x_obj, gensym("signal"));

    /* optional signal inlets */
    if (s == gensym("syncsaw")){
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    }

    /* optional phase inlet */
    if (s != gensym("comparator")){
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("phase"));  
    }

    /* create the postfilter */
    x->x_ctl.c_butter = new DSPIfilterSeries(3);

    /* init oscillators */
    for (i=0; i<VOICES; i++) {
      x->x_ctl.c_index[i] = N-2;
      x->x_ctl.c_frac[i] = 0.0;
    }

    /* init rest of state data */
    blosc_phase(x, 0);
    blosc_phase2(x, 0);
    x->x_ctl.c_state = 0.0;
    x->x_ctl.c_prev_amp = 0.0;
    x->x_ctl.c_next_voice = 0;
    x->x_ctl.c_scale = 1.0;
    x->x_ctl.c_scale_update = 1.0;
    x->x_ctl.c_waveform = s;

    return (void *)x;
}







/* CLASS DATA INIT (tables) */


/* some vector ops */

/* clear a buffer */
static inline void _clear(t_float *array, int size)
{
  memset(array, 0, sizeof(t_float)*size);
}

/* compute complex log */
static inline void _clog(t_float *real, t_float *imag, int size)
{
    int k;
    for (k=0; k<size; k++){
	t_float r = real[k];
	t_float i = imag[k];
	t_float radius = sqrt(r*r+i*i);
	real[k] = log(radius);
	imag[k] = atan2(i,r);
    }
}

/* compute complex exp */
static inline void _cexp(t_float *real, t_float *imag, int size)
{
    int k;
    for (k=0; k<size; k++){
	t_float r = exp(real[k]);
	t_float i = imag[k];
	real[k] = r * cos(i);
	imag[k] = r * sin(i);
    }
}


/* compute fft */
static inline void _fft(t_float *real, t_float *imag, int size)
{
    int i;
    t_float scale = 1.0 / sqrt((t_float)size);
    for (i=0; i<size; i++){
	real[i] *= scale;
	imag[i] *= scale;
	// if (isnan(real[i])) post("fftpanic %d", i);
    }
    mayer_fft(size, real, imag);
}
/* compute ifft */
static inline void _ifft(t_float *real, t_float *imag, int size)
{
    int i;
    t_float scale = 1.0 / sqrt((t_float)size);
    for (i=0; i<size; i++){
	real[i] *= scale;
	imag[i] *= scale;
	// if (isnan(real[i])) post("ifftpanic %d", i);
    }
    mayer_ifft(size, real, imag);
}

/* convert an integer index to a phase: [0 -> pi, -pi -> 0] */
static inline t_float _i2theta(int i, int size){
    t_float p = 2.0 * M_PI * (t_float)i / (t_float)size;
    if (p >= M_PI) p -= 2.0 * M_PI;
    return p;
}


/* print matlab array */
static void _printm(t_float *array, char *name, int size)
{
    int i;
    fprintf(stderr, "%s = [", name);
    for (i=0; i<size; i++){
	fprintf(stderr, "%f;", array[i]);
    }
    fprintf(stderr, "];\n");
}

/* store oversampled waveform as decimated chunks */
static void _store_decimated(t_float *dst, t_float *src, t_float scale, int size)
{
    int i;
    for (i=0; i<size; i++){
	int offset = (i % S) * L;
	int index = i / S;
	dst[offset+index] = scale * src[i];
    }    

}

/* store waveform as one chunk */
static void _store(t_float *dst, t_float *src, t_float scale, int size)
{
    int i;
    for (i=0; i<size; i++){
	dst[i] = scale * src[i];
    }    

}

/* create a minimum phase bandlimited impulse */
static void build_tables(void)
{

  /* table size = M>=N (time padding to reduce time aliasing) */

  /* we work in the complex domain to eliminate the need to avoid
     negative spectral components */

    t_float real[M];
    t_float imag[M];
    t_float sum,scale;
    int i,j;


    /* create windowed sinc */
    _clear(imag, M); 
    real[0] = 1.0;
    for (i=1; i<M; i++){
	t_float tw = _i2theta(i,M);
	t_float ts = tw * NBPERIODS * (t_float)(M) / (t_float)(N);

	/* sinc */
	real[i] = sin(ts)/ts;

	/* blackman window */
	real[i] *= 0.42 + 0.5 * (cos(tw)) + 0.08 * (cos(2.0*tw));

	//real[i] *= 0.5f * (1.0f + WALPHA) + 0.5f * (1.0f - WALPHA) * (cos(tw)); 

	/* check for nan */
	//if (isnan(real[i])) post("sinc NaN panic %d", i);
	//if (isinf(real[i])) post("sinc Inf panic %d", i);

    }


    /* compute cepstrum */
    _fft(real, imag, M);
    _clog(real, imag, M);
    _ifft(real, imag, M);


    /* kill anti-causal part (contribution of non minimum phase zeros) */
    /* should we kill nyquist too ?? */
    for (i=M/2+1; i<M; i++){
	real[i] *= 0.0000;
	imag[i] *= 0.0000;
    }


    /* compute inverse cepstrum */
    _fft(real, imag, M);
    _cexp(real, imag, M);
    _ifft(real, imag, M);



    /* from here on, discard the padded part [N->M-1]
       and work with the first N samples */

    /* normalize impulse (integral = 1) */
    sum = 0.0;
    for (i=0; i<N; i++){sum += real[i];}
    scale = 1.0 / sum;
    for (i=0; i<N; i++){real[i] *= scale;}


    /* store bli table */
    _store(bli, real, (t_float)S, N);
    //_printm(bli, "h", N);


    /* integrate impulse and invert to produce a step function
       from 1->0 */
    sum = 0.0;
    for (i=0; i<N; i++){
	sum += real[i];
	real[i] = (1.0 - sum);
    }

    /* store decimated bls tables */
    _store(bls, real, 1.0, N);


}

extern "C"
{
    void blosc_tilde_setup(void)
    {
	//post("blosc~ v0.1");
	
	build_tables();
	
	blosc_class = class_new(gensym("blosc~"), (t_newmethod)blosc_new,
				(t_method)blosc_free, sizeof(t_blosc), 0, A_DEFSYMBOL, A_NULL);
	CLASS_MAINSIGNALIN(blosc_class, t_blosc, x_f);
	class_addmethod(blosc_class, (t_method)blosc_dsp, gensym("dsp"), A_NULL); 
	class_addmethod(blosc_class, (t_method)blosc_phase, gensym("phase"), A_FLOAT, A_NULL); 
	class_addmethod(blosc_class, (t_method)blosc_phase2, gensym("phase2"), A_FLOAT, A_NULL); 

	
    }

}
