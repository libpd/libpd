/*
 *   resofilt.c  -  some high quality time variant filters
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


/* some (expensive) time variant iir filters, 
   i.e. moog 4-pole transfer function using the impulse
   invariant transform with self osc and
   signal freq and reso inputs */


#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "filters.h"


typedef struct resofiltctl
{
    t_float c_state[4]; /* filter state */
    t_float c_f_prev;
    t_float c_r_prev;

} t_resofiltctl;

typedef struct resofilt
{
    t_object x_obj;
    t_float x_f;
    t_resofiltctl x_ctl;
    t_perfroutine x_dsp;
} t_resofilt;


static inline void _sat_state(t_float *x)
{
    const t_float norm_squared_max = 1.0;
    t_float norm_squared = x[0] * x[0] + x[1] * x[1];

    if (norm_squared > norm_squared_max){
	t_float scale = 1.0 / sqrt(norm_squared);
	x[0] *= scale;
	x[1] *= scale;
    }
}


/* the moog vcf discretized with an impulse invariant transform */

static t_int *resofilt_perform_fourpole(t_int *w)
{

  t_resofiltctl *ctl = (t_resofiltctl *)(w[1]);
  t_int n            = (t_int)(w[2]);
  t_float *in        = (t_float *)(w[3]);
  t_float *freq      = (t_float *)(w[4]);
  t_float *reso      = (t_float *)(w[5]);
  t_float *out       = (t_float *)(w[6]);

  int i;
  t_float inv_n = 1.0 / ((t_float)n);
  t_float inv_sr = 1.0 / sys_getsr();

  t_float phasor[2], phasor_rot[2];
  t_float radior[2], radior_rot[2];
  t_float scalor, scalor_inc;

  t_float f,r,freq_rms,reso_rms;
  t_float f_prev = ctl->c_f_prev;
  t_float r_prev = ctl->c_r_prev;

  /* use rms of input to drive freq and reso.  this is such that
     connecting a dsp signal to the inlets has a reasonable result,
     even if the inputs are oscillatory. the rms values will be
     interpolated linearly (that is, linearly in the "analog" domain,
     so exponentially in the z-domain)  */

  reso_rms = freq_rms = 0.0;
  for (i=0; i<n; i++){
      /* first input is the reso frequency (absolute) */
      t_float _freq = *freq++; 
      /* second input is the resonnance (0->1), >1 == self osc */
      t_float _reso = *reso++; 

      freq_rms += _freq * _freq;
      reso_rms += _reso * _reso;
  }
  freq_rms = sqrt(freq_rms * inv_n) * inv_sr;
  reso_rms = sqrt(reso_rms * inv_n);
  f = (freq_rms > 0.5) ? 0.5 : freq_rms;
  r = sqrt(sqrt(reso_rms));         


  /* calculate the new pole locations.  we use an impulse invariant
     transform: the moog vcf poles are located at 

     s_p = (-1 +- r \sqrt{+- j}, with r = (k/4)^(1/4) \in [0,1]

     the poles are always complex, so we can use an orthogonal
     implementation both conj pole pairs have the same angle, so we
     can use one phasor and 2 radii
  */

  /* compute phasor, radius and update eqs
     since these are at k-rate, the expense is justifyable */
  phasor[0] = cos(2.0 * M_PI * r_prev * f_prev);
  phasor[1] = sin(2.0 * M_PI * r_prev * f_prev);
  phasor_rot[0] = cos(2.0 * M_PI * (r*f - r_prev*f_prev) * inv_n);
  phasor_rot[1] = sin(2.0 * M_PI * (r*f - r_prev*f_prev) * inv_n);

  radior[0] = exp(f_prev * (-1.0 + r_prev)); /* dominant radius */
  radior[1] = exp(f_prev * (-1.0 - r_prev));
  radior_rot[0] = exp((f*(-1.0f + r) - f_prev * (-1.0 + r_prev)) * inv_n);
  radior_rot[1] = exp((f*(-1.0f - r) - f_prev * (-1.0 - r_prev)) * inv_n);

  /* moog like scaling */
  if (1){
      t_float r2 = r_prev * r_prev;
      t_float r4 = r2 * r2;
      scalor = 1.0f + (1.0 + 4.0 * r4);
      r2 = r * r;
      r4 = r2 * r2;
      scalor_inc = ((1.0 + (1.0 + 4.0 * r4)) - scalor) * inv_n;
  }

  /* no scaling */
  else{
      scalor = 1.0;
      scalor_inc = 0.0;
  }
  
  ctl->c_f_prev = f;
  ctl->c_r_prev = r;





  /* perform filtering */
  for (i=0; i<n; i++){
      t_float poleA[2], poleB[2];
      t_float *stateA = ctl->c_state;
      t_float *stateB = ctl->c_state+2;

      t_float x;

      /* compute poles */
      poleA[0] = radior[0] * phasor[0];
      poleA[1] = radior[0] * phasor[1];
      poleB[0] = radior[1] * phasor[0];
      poleB[1] = radior[1] * phasor[1];


      /* perform filtering: use 2 DC normalized complex one-poles:
	 y[k] = x[k] + a(y[k-1] - x[k]) or y(z) = (1-a)/(1-az^{-1}) x(z) */

      x = *in++ * scalor;

      /* nondominant pole first */
      stateB[0] -= x;
      vcmul2(stateB, poleB);
      x = stateB[0] += x;

      /* dominant pole second */
      stateA[0] -= x;
      vcmul2(stateA, poleA);
      x = stateA[0] += x;

      *out++ = x;

      /* saturate (normalize if pow > 1) state to prevent explosion
       * and to allow self-osc */
      _sat_state(stateA);
      _sat_state(stateB);
      
      /* interpolate filter coefficients */
      vcmul2(phasor, phasor_rot);
      radior[0] *= radior_rot[0];
      radior[1] *= radior_rot[1];
      scalor += scalor_inc;
      
  }

  return (w+7);
}





/* 303-style modified moog vcf (3-pole) */

static t_int *resofilt_perform_threepole(t_int *w)
{

  t_resofiltctl *ctl = (t_resofiltctl *)(w[1]);
  t_int n            = (t_int)(w[2]);
  t_float *in        = (t_float *)(w[3]);
  t_float *freq      = (t_float *)(w[4]);
  t_float *reso      = (t_float *)(w[5]);
  t_float *out       = (t_float *)(w[6]);

  int i;
  t_float inv_n = 1.0f / ((t_float)n);
  t_float inv_sr = 1.0f / sys_getsr();

  t_float phasor[2], phasor_rot[2];
  t_float radior[2], radior_rot[2];
  t_float scalor, scalor_inc;

  t_float f,r,freq_rms,reso_rms;
  t_float f_prev = ctl->c_f_prev;
  t_float r_prev = ctl->c_r_prev;

  t_float sqrt5 = sqrt(5.0);

  /* use rms of input to drive freq and reso */
  reso_rms = freq_rms = 0.0;
  for (i=0; i<n; i++){
      /* first input is the reso frequency (absolute) */
      t_float _freq = *freq++;
      /* second input is the resonnance (0->1), >1 == self osc */
      t_float _reso = *reso++;
      freq_rms += _freq * _freq;
      reso_rms += _reso * _reso;
  }
  freq_rms = sqrt(freq_rms * inv_n) * inv_sr;
  reso_rms = sqrt(reso_rms * inv_n);
  f = (freq_rms > 0.5) ? 0.25 : freq_rms * 0.5;
  r = cbrt(reso_rms);         


  /* calculate the new pole locations. we use an impulse invariant
     transform: the 303 vcf poles are located at

     s_p = omega(-1 + r sqrt(5) e^{pi/3(1+2p)})

     real pole: omega * (-1 -r)
     complex pole:
       real = omega (-1 + r)
       imag = omega (+- 2 r)

     
     this is a strange beast. legend goes it was "invented" by taking
     the moog vcf and moving one cap up, such that the not-so
     controllable 3-pole that emerged would avoid the moog patent..

     so, the sound is not so much the locations of the poles, but how
     the filter reacts to time varying controls. i.e. the pole
     movement with constant reso, used in the tb-303.

  */

  /* compute phasor, radius and update eqs */
  phasor[0] = cos(2.0 * M_PI * r_prev * f_prev * 2.0);
  phasor[1] = sin(2.0 * M_PI * r_prev * f_prev * 2.0);
  phasor_rot[0] = cos(2.0 * M_PI * (r*f - r_prev*f_prev) * 2.0 * inv_n);
  phasor_rot[1] = sin(2.0 * M_PI * (r*f - r_prev*f_prev) * 2.0 * inv_n);

  radior[0] = exp(f_prev * (-1.0 + r_prev)); /* dominant radius */
  radior[1] = exp(f_prev * (-1.0 - sqrt5 * r_prev));
  radior_rot[0] = exp((f*(-1.0 + r) - f_prev * (-1.0 + r_prev)) * inv_n);
  radior_rot[1] = exp((f*(-1.0 - r) - f_prev * (-1.0 - sqrt5 * r_prev)) * inv_n);

  /* 303 like scaling */
  if (1){
      t_float r3 = r_prev * r_prev * r_prev;
      scalor = 1.0 + (1.0 + 3.0 * r_prev);
      r3 = r * r * r;
      scalor_inc = ((1.0f + (1.0f + 3.0 * r3)) - scalor) * inv_n;
  }

  /* no scaling */
  else{
      scalor = 1.0;
      scalor_inc = 0.0;
  }
  
  ctl->c_f_prev = f;
  ctl->c_r_prev = r;


  ctl->c_state[3] = 0.0;
  /* perform filtering */
  for (i=0; i<n; i++){
      t_float poleA[2], poleB[2];
      t_float *stateA = ctl->c_state;
      t_float *stateB = ctl->c_state+2;

      t_float x;

      /* compute poles */
      poleA[0] = radior[0] * phasor[0];
      poleA[1] = radior[0] * phasor[1];

      poleB[0] = radior[1];


      /* perform filtering: use (real part of) 2 DC normalized complex one-poles:
	 y[k] = x[k] + a(y[k-1] - x[k]) or y(z) = (1-a)/(1-az^{-1}) x(z) */

      x = *in++ * scalor;

      /* nondominant pole first */
      stateB[0] -= x;
      stateB[0] *= poleB[0];
      x = stateB[0] += x;

      /* dominant pole second */
      stateA[0] -= x;
      vcmul2(stateA, poleA);
      x = stateA[0] += x;

      *out++ = x;

      /* saturate (normalize if pow > 1) state to prevent explosion
       * and to allow self-osc */
      _sat_state(stateA);
      _sat_state(stateB);
      
      /* interpolate filter coefficients */
      vcmul2(phasor, phasor_rot);
      radior[0] *= radior_rot[0];
      radior[1] *= radior_rot[1];
      scalor += scalor_inc;
      
  }

  return (w+7);
}





static void resofilt_dsp(t_resofilt *x, t_signal **sp)
{
  int n = sp[0]->s_n;

  dsp_add(x->x_dsp,
	  6, 
	  &x->x_ctl, 
	  sp[0]->s_n, 
	  sp[0]->s_vec, 
	  sp[1]->s_vec, 
	  sp[2]->s_vec, 
	  sp[3]->s_vec);

}                                  
static void resofilt_free(t_resofilt *x)
{


}

t_class *resofilt_class;

static void *resofilt_new(t_floatarg algotype)
{
    t_resofilt *x = (t_resofilt *)pd_new(resofilt_class);

    /* in */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  

    /* out */
    outlet_new(&x->x_obj, gensym("signal")); 


    /* set algo type */
    if (algotype == 3.0){
	post("resofilt~: 3-pole lowpass ladder vcf");
	x->x_dsp = resofilt_perform_threepole;
    }
    else {
	post("resofilt~: 4-pole lowpass ladder vcf");
	x->x_dsp = resofilt_perform_fourpole;
    } 


    return (void *)x;
}

void resofilt_tilde_setup(void)
{
    resofilt_class = class_new(gensym("resofilt~"), (t_newmethod)resofilt_new,
    	(t_method)resofilt_free, sizeof(t_resofilt), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(resofilt_class, t_resofilt, x_f);
    class_addmethod(resofilt_class, (t_method)resofilt_dsp, gensym("dsp"), 0); 
}

