/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>


/* ---------- filter~ - slow dynamic filter-kernel 1. and 2. order ----------- */
/* ---------- with double precision option ----------------------------------- */

typedef struct _float_filter_para_tilde
{
  t_float   wn1;// old wn of biquad recursion
  t_float   wn2;// two times old wn of biquad recursion
  t_float   a0;// wn factor of numerator term of biquad recursion
  t_float   a1;// wn1 factor of numerator term of biquad recursion
  t_float   a2;// wn2 factor of numerator term of biquad recursion
  t_float   b1;// wn1 factor of denominator term of biquad recursion
  t_float   b2;// wn2 factor of denominator term of biquad recursion
  t_float   pi_over_sr;//   pi/samplerate
  t_float   cur_f;// current frequency
  t_float   cur_l;// current bilinear transformed frequency
  t_float   cur_a;// current damping
  t_float   cur_b;// current freqency shifting factor
  t_float   delta_f;// frequency ratio from previous to current frequency
  t_float   delta_a;// damping ratio from previous to current damping
  t_float   delta_b;// freqency shift ratio from previous to current freqency shift
  t_float   end_f;// destination frequency
  t_float   end_a;// destination damping
  t_float   end_b;// destination freqency shift
  t_float   ticks_per_interpol_time;// dsp ticks per interpolation time intervall
  t_float   rcp_ticks;// reciprocal number of dsp ticks within intervall
  t_float   interpol_time;// interpolation time
  int       ticks;// number of dsp ticks within intervall
  int       counter_f;// number of dsp ticks to compute new frequency
  int       counter_a;// number of dsp ticks to compute new damping
  int       counter_b;// number of dsp ticks to compute new frequency shift
  int       inlet3_is_Q1_or_damping0;// if flag is HIGH, the third inlet (Q) has to be inverted to damping
  int       filter_function_is_highpass;// flag is HIGH if filter has highpass characteristic
  int       filter_function_is_first_order;// flag is HIGH if filter is first order
  int       event_mask;// a three bit mask: Bit 0 is HIGH during frequency ramp, Bit 1 is HIGH during damping ramp, Bit 2 is HIGH during frequency shift ramp
  void      (*calc)();
} t_float_filter_para_tilde;

typedef struct _double_filter_para_tilde
{
  double    wn1;// old wn of biquad recursion
  double    wn2;// two times old wn of biquad recursion
  double    a0;// wn factor of numerator term of biquad recursion
  double    a1;// wn1 factor of numerator term of biquad recursion
  double    a2;// wn2 factor of numerator term of biquad recursion
  double    b1;// wn1 factor of denominator term of biquad recursion
  double    b2;// wn2 factor of denominator term of biquad recursion
  double    pi_over_sr;//   pi/samplerate
  double    cur_f;// current frequency
  double    cur_l;// current bilinear transformed frequency
  double    cur_a;// current damping
  double    cur_b;// current freqency shifting factor
  double    delta_f;// frequency ratio from previous to current frequency
  double    delta_a;// damping ratio from previous to current damping
  double    delta_b;// freqency shift ratio from previous to current freqency shift
  double    end_f;// destination frequency
  double    end_a;// destination damping
  double    end_b;// destination freqency shift
  double    ticks_per_interpol_time;// dsp ticks per interpolation time intervall
  double    rcp_ticks;// reciprocal number of dsp ticks within intervall
  double    interpol_time;// interpolation time
  int       ticks;// number of dsp ticks within intervall
  int       counter_f;// number of dsp ticks to compute new frequency
  int       counter_a;// number of dsp ticks to compute new damping
  int       counter_b;// number of dsp ticks to compute new frequency shift
  int       inlet3_is_Q1_or_damping0;// if flag is HIGH, the third inlet (Q) has to be inverted to damping
  int       filter_function_is_highpass;// flag is HIGH if filter has highpass characteristic
  int       filter_function_is_first_order;// flag is HIGH if filter is first order
  int       event_mask;// a three bit mask: Bit 0 is HIGH during frequency ramp, Bit 1 is HIGH during damping ramp, Bit 2 is HIGH during frequency shift ramp
  void      (*calc)();
} t_double_filter_para_tilde;

typedef struct _filter_tilde
{
  t_object  x_obj;
  t_float_filter_para_tilde fp;
  t_double_filter_para_tilde dp;
  int       precision_d1_f0;
  void      *x_debug_outlet;
  t_atom    x_at[5];
  t_float   x_msi;
} t_filter_tilde;

static t_class *filter_tilde_class;

static void filter_tilde_snafu(t_float_filter_para_tilde *x)
{
  
}

static void filter_tilde_lp1(t_float_filter_para_tilde *x)
{
  t_float al;
  
  al = x->cur_a * x->cur_l;
  x->a0 = 1.0f/(1.0f + al);
  x->a1 = x->a0;
  x->b1 = (al - 1.0f)*x->a0;
}

static void filter_tilde_lp2(t_float_filter_para_tilde *x)
{
  t_float l, al, bl2, rcp;
  
  l = x->cur_l;
  al = l*x->cur_a;
  bl2 = l*l*x->cur_b + 1.0f;
  rcp = 1.0f/(al + bl2);
  x->a0 = rcp;
  x->a1 = 2.0f*rcp;
  x->a2 = x->a0;
  x->b1 = rcp*2.0f*(bl2 - 2.0f);
  x->b2 = rcp*(al - bl2);
}

static void filter_tilde_hp1(t_float_filter_para_tilde *x)
{
  t_float al, rcp;
  
  al = x->cur_a * x->cur_l;
  rcp = 1.0f/(1.0f + al);
  x->a0 = rcp*al;
  x->a1 = -x->a0;
  x->b1 = rcp*(al - 1.0f);
}

static void filter_tilde_hp2(t_float_filter_para_tilde *x)
{
  t_float l, al, bl2, rcp;
  
  l = x->cur_l;
  bl2 = l*l*x->cur_b + 1.0f;
  al = l*x->cur_a;
  rcp = 1.0f/(al + bl2);
  x->a0 = rcp*(bl2 - 1.0f);
  x->a1 = -2.0f*x->a0;
  x->a2 = x->a0;
  x->b1 = rcp*2.0f*(bl2 - 2.0f);
  x->b2 = rcp*(al - bl2);
}

static void filter_tilde_rp2(t_float_filter_para_tilde *x)
{
  t_float l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*l;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0f*(l2 - 2.0f);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_bp2(t_float_filter_para_tilde *x)
{
  t_float l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*al;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0f*(l2 - 2.0f);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_bs2(t_float_filter_para_tilde *x)
{
  t_float l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*l2;
  x->a1 = rcp*2.0f*(2.0f - l2);
  x->a2 = x->a0;
  x->b1 = -x->a1;
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_rpw2(t_float_filter_para_tilde *x)
{
  t_float l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a/x->cur_f;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*l;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0f*(l2 - 2.0f);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_bpw2(t_float_filter_para_tilde *x)
{
  t_float l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a/x->cur_f;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*al;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0f*(l2 - 2.0f);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_bsw2(t_float_filter_para_tilde *x)
{
  t_float l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a/x->cur_f;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*l2;
  x->a1 = rcp*2.0f*(2.0f - l2);
  x->a2 = x->a0;
  x->b1 = -x->a1;
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_ap1(t_float_filter_para_tilde *x)
{
  t_float al;
  
  al = x->cur_a * x->cur_l;
  x->a0 = (1.0f - al)/(1.0f + al);
  x->b1 = -x->a0;
}

static void filter_tilde_ap2(t_float_filter_para_tilde *x)
{
  t_float l, al, bl2, rcp;
  
  l = x->cur_l;
  bl2 = l*l*x->cur_b + 1.0f;
  al = l*x->cur_a;
  rcp = 1.0f/(al + bl2);
  x->a1 = rcp*2.0f*(2.0f - bl2);
  x->a0 = rcp*(bl2 - al);
  x->b1 = -x->a1;
  x->b2 = -x->a0;
}

/*static void filter_tilde_bp2(t_filter_tilde *x)
{
t_float l, al, l2, rcp;

  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*al;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0f*(2.0f - l2);
  x->b2 = rcp*(l2 - al);
}*/

static void filter_tilde_dlp1(t_double_filter_para_tilde *x)
{
  double al;
  
  al = x->cur_a * x->cur_l;
  x->a0 = 1.0/(1.0 + al);
  x->a1 = x->a0;
  x->b1 = (al - 1.0)*x->a0;
}

static void filter_tilde_dlp2(t_double_filter_para_tilde *x)
{
  double l, al, bl2, rcp;
  
  l = x->cur_l;
  al = l*x->cur_a;
  bl2 = l*l*x->cur_b + 1.0;
  rcp = 1.0/(al + bl2);
  x->a0 = rcp;
  x->a1 = 2.0*rcp;
  x->a2 = x->a0;
  x->b1 = rcp*2.0*(bl2 - 2.0);
  x->b2 = rcp*(al - bl2);
}

static void filter_tilde_dhp1(t_double_filter_para_tilde *x)
{
  double al, rcp;
  
  al = x->cur_a * x->cur_l;
  rcp = 1.0/(1.0 + al);
  x->a0 = rcp*al;
  x->a1 = -x->a0;
  x->b1 = rcp*(al - 1.0);
}

static void filter_tilde_dhp2(t_double_filter_para_tilde *x)
{
  double l, al, bl2, rcp;
  
  l = x->cur_l;
  bl2 = l*l*x->cur_b + 1.0;
  al = l*x->cur_a;
  rcp = 1.0/(al + bl2);
  x->a0 = rcp*(bl2 - 1.0);
  x->a1 = -2.0*x->a0;
  x->a2 = x->a0;
  x->b1 = rcp*2.0*(bl2 - 2.0);
  x->b2 = rcp*(al - bl2);
}

static void filter_tilde_drp2(t_double_filter_para_tilde *x)
{
  double l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a;
  rcp = 1.0/(al + l2);
  x->a0 = rcp*l;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0*(l2 - 2.0);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_dbp2(t_double_filter_para_tilde *x)
{
  double l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a;
  rcp = 1.0/(al + l2);
  x->a0 = rcp*al;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0*(l2 - 2.0);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_dbs2(t_double_filter_para_tilde *x)
{
  double l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a;
  rcp = 1.0/(al + l2);
  x->a0 = rcp*l2;
  x->a1 = rcp*2.0*(2.0 - l2);
  x->a2 = x->a0;
  x->b1 = -x->a1;
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_drpw2(t_double_filter_para_tilde *x)
{
  double l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a/x->cur_f;
  rcp = 1.0/(al + l2);
  x->a0 = rcp*l;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0*(l2 - 2.0);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_dbpw2(t_double_filter_para_tilde *x)
{
  double l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a/x->cur_f;
  rcp = 1.0/(al + l2);
  x->a0 = rcp*al;
  x->a2 = -x->a0;
  x->b1 = rcp*2.0*(l2 - 2.0);
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_dbsw2(t_double_filter_para_tilde *x)
{
  double l, al, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0;
  al = l*x->cur_a/x->cur_f;
  rcp = 1.0/(al + l2);
  x->a0 = rcp*l2;
  x->a1 = rcp*2.0*(2.0 - l2);
  x->a2 = x->a0;
  x->b1 = -x->a1;
  x->b2 = rcp*(al - l2);
}

static void filter_tilde_dap1(t_double_filter_para_tilde *x)
{
  double al;
  
  al = x->cur_a * x->cur_l;
  x->a0 = (1.0 - al)/(1.0 + al);
  x->b1 = -x->a0;
}

static void filter_tilde_dap2(t_double_filter_para_tilde *x)
{
  double l, al, bl2, rcp;
  
  l = x->cur_l;
  bl2 = l*l*x->cur_b + 1.0;
  al = l*x->cur_a;
  rcp = 1.0/(al + bl2);
  x->a1 = rcp*2.0*(2.0 - bl2);
  x->a0 = rcp*(bl2 - al);
  x->b1 = -x->a1;
  x->b2 = -x->a0;
}

static void filter_tilde_dsp_tick(t_float_filter_para_tilde *x)
{
  if(x->event_mask)
  {
    if(x->counter_f)
    {
      float l, si, co;
      
      if(x->counter_f <= 1)
      {
        x->cur_f = x->end_f;
        x->counter_f = 0;
        x->event_mask &= 6;/*set event_mask_bit 0 = 0*/
      }
      else
      {
        x->counter_f--;
        x->cur_f *= x->delta_f;
      }
      l = x->cur_f * x->pi_over_sr;
      if(l < 1.0e-20f)
        x->cur_l = 1.0e20f;
      else if(l > 1.57079632f)
        x->cur_l = 0.0f;
      else
      {
        si = sin(l);
        co = cos(l);
        x->cur_l = co/si;
      }
    }
    if(x->counter_a)
    {
      if(x->counter_a <= 1)
      {
        x->cur_a = x->end_a;
        x->counter_a = 0;
        x->event_mask &= 5;/*set event_mask_bit 1 = 0*/
      }
      else
      {
        x->counter_a--;
        x->cur_a *= x->delta_a;
      }
    }
    if(x->counter_b)
    {
      if(x->counter_b <= 1)
      {
        x->cur_b = x->end_b;
        x->counter_b = 0;
        x->event_mask &= 3;/*set event_mask_bit 2 = 0*/
      }
      else
      {
        x->counter_b--;
        x->cur_b *= x->delta_b;
      }
    }
    
    (*(x->calc))(x);
    
    /* stability check */
    if(x->filter_function_is_first_order)
    {
      if(x->b1 <= -0.9999998f)
        x->b1 = -0.9999998f;
      else if(x->b1 >= 0.9999998f)
        x->b1 = 0.9999998f;
    }
    else
    {
      float discriminant = x->b1 * x->b1 + 4.0f * x->b2;
      
      if(x->b1 <= -1.9999996f)
        x->b1 = -1.9999996f;
      else if(x->b1 >= 1.9999996f)
        x->b1 = 1.9999996f;
      
      if(x->b2 <= -0.9999998f)
        x->b2 = -0.9999998f;
      else if(x->b2 >= 0.9999998f)
        x->b2 = 0.9999998f;
      
      if(discriminant >= 0.0f)
      {
        if(0.9999998f - x->b1 - x->b2 < 0.0f)
          x->b2 = 0.9999998f - x->b1;
        if(0.9999998f + x->b1 - x->b2 < 0.0f)
          x->b2 = 0.9999998f + x->b1;
      }
    }
    //post("float a0=%f, a1=%f, a2=%f, b1=%f, b2=%f", x->a0, x->a1, x->a2, x->b1, x->b2);
  }
}

static void filter_tilde_dsp_dtick(t_double_filter_para_tilde *x)
{
  if(x->event_mask)
  {
    if(x->counter_f)
    {
      double l, si, co;
      
      if(x->counter_f <= 1)
      {
        x->cur_f = x->end_f;
        x->counter_f = 0;
        x->event_mask &= 6;/*set event_mask_bit 0 = 0*/
      }
      else
      {
        x->counter_f--;
        x->cur_f *= x->delta_f;
      }
      l = x->cur_f * x->pi_over_sr;
      if(l < 1.0e-20)
        x->cur_l = 1.0e20;
      else if(l > 1.57079632)
        x->cur_l = 0.0;
      else
      {
        si = sin(l);
        co = cos(l);
        x->cur_l = co/si;
      }
    }
    if(x->counter_a)
    {
      if(x->counter_a <= 1)
      {
        x->cur_a = x->end_a;
        x->counter_a = 0;
        x->event_mask &= 5;/*set event_mask_bit 1 = 0*/
      }
      else
      {
        x->counter_a--;
        x->cur_a *= x->delta_a;
      }
    }
    if(x->counter_b)
    {
      if(x->counter_b <= 1)
      {
        x->cur_b = x->end_b;
        x->counter_b = 0;
        x->event_mask &= 3;/*set event_mask_bit 2 = 0*/
      }
      else
      {
        x->counter_b--;
        x->cur_b *= x->delta_b;
      }
    }
    
    (*(x->calc))(x);
    
    /* stability check */
    if(x->filter_function_is_first_order)
    {
      if(x->b1 <= -0.9999998)
        x->b1 = -0.9999998;
      else if(x->b1 >= 0.9999998)
        x->b1 = 0.9999998;
    }
    else
    {
      double discriminant = x->b1 * x->b1 + 4.0 * x->b2;
      
      if(x->b1 <= -1.9999996)
        x->b1 = -1.9999996;
      else if(x->b1 >= 1.9999996)
        x->b1 = 1.9999996;
      
      if(x->b2 <= -0.9999998)
        x->b2 = -0.9999998;
      else if(x->b2 >= 0.9999998)
        x->b2 = 0.9999998;
      
      if(discriminant >= 0.0)
      {
        if(0.9999998 - x->b1 - x->b2 < 0.0)
          x->b2 = 0.9999998 - x->b1;
        if(0.9999998 + x->b1 - x->b2 < 0.0)
          x->b2 = 0.9999998 + x->b1;
      }
    }
    //post("double a0=%f, a1=%f, a2=%f, b1=%f, b2=%f", (t_float)x->a0, (t_float)x->a1, (t_float)x->a2, (t_float)x->b1, (t_float)x->b2);
  }
}

static t_int *filter_tilde_perform_2o(t_int *w)
{
  t_float *in = (float *)(w[1]);
  t_float *out = (float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn0, wn1=x->fp.wn1, wn2=x->fp.wn2;
  t_float a0=x->fp.a0, a1=x->fp.a1, a2=x->fp.a2;
  t_float b1=x->fp.b1, b2=x->fp.b2;
  
  filter_tilde_dsp_tick(&x->fp);
  for(i=0; i<n; i++)
  {
    wn0 = *in++ + b1*wn1 + b2*wn2;
    *out++ = a0*wn0 + a1*wn1 + a2*wn2;
    wn2 = wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn2))
    wn2 = 0.0f;
  if(IEM_DENORMAL(wn1))
    wn1 = 0.0f;
  
  x->fp.wn1 = wn1;
  x->fp.wn2 = wn2;
  return(w+5);
}
/*   yn0 = *out;
xn0 = *in;
*************
yn0 = a0*xn0 + a1*xn1 + a2*xn2 + b1*yn1 + b2*yn2;
yn2 = yn1;
yn1 = yn0;
xn2 = xn1;
xn1 = xn0;
*************************
y/x = (a0 + a1*z-1 + a2*z-2)/(1 - b1*z-1 - b2*z-2);*/

static t_int *filter_tilde_perf8_2o(t_int *w)
{
  t_float *in = (float *)(w[1]);
  t_float *out = (float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn[10];
  t_float a0=x->fp.a0, a1=x->fp.a1, a2=x->fp.a2;
  t_float b1=x->fp.b1, b2=x->fp.b2;
  
  filter_tilde_dsp_tick(&x->fp);
  wn[0] = x->fp.wn2;
  wn[1] = x->fp.wn1;
  for(i=0; i<n; i+=8, in+=8, out+=8)
  {
    wn[2] = in[0] + b1*wn[1] + b2*wn[0];
    out[0] = a0*wn[2] + a1*wn[1] + a2*wn[0];
    wn[3] = in[1] + b1*wn[2] + b2*wn[1];
    out[1] = a0*wn[3] + a1*wn[2] + a2*wn[1];
    wn[4] = in[2] + b1*wn[3] + b2*wn[2];
    out[2] = a0*wn[4] + a1*wn[3] + a2*wn[2];
    wn[5] = in[3] + b1*wn[4] + b2*wn[3];
    out[3] = a0*wn[5] + a1*wn[4] + a2*wn[3];
    wn[6] = in[4] + b1*wn[5] + b2*wn[4];
    out[4] = a0*wn[6] + a1*wn[5] + a2*wn[4];
    wn[7] = in[5] + b1*wn[6] + b2*wn[5];
    out[5] = a0*wn[7] + a1*wn[6] + a2*wn[5];
    wn[8] = in[6] + b1*wn[7] + b2*wn[6];
    out[6] = a0*wn[8] + a1*wn[7] + a2*wn[6];
    wn[9] = in[7] + b1*wn[8] + b2*wn[7];
    out[7] = a0*wn[9] + a1*wn[8] + a2*wn[7];
    wn[0] = wn[8];
    wn[1] = wn[9];
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn[0]))
    wn[0] = 0.0f;
  if(IEM_DENORMAL(wn[1]))
    wn[1] = 0.0f;
  
  x->fp.wn1 = wn[1];
  x->fp.wn2 = wn[0];
  return(w+5);
}

static t_int *filter_tilde_perform_1o(t_int *w)
{
  t_float *in = (float *)(w[1]);
  t_float *out = (float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn0, wn1=x->fp.wn1;
  t_float a0=x->fp.a0, a1=x->fp.a1;
  t_float b1=x->fp.b1;
  
  filter_tilde_dsp_tick(&x->fp);
  for(i=0; i<n; i++)
  {
    wn0 = *in++ + b1*wn1;
    *out++ = a0*wn0 + a1*wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn1))
    wn1 = 0.0f;
  
  x->fp.wn1 = wn1;
  return(w+5);
}

static t_int *filter_tilde_perf8_1o(t_int *w)
{
  t_float *in = (float *)(w[1]);
  t_float *out = (float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn[9];
  t_float a0=x->fp.a0, a1=x->fp.a1;
  t_float b1=x->fp.b1;
  
  filter_tilde_dsp_tick(&x->fp);
  wn[0] = x->fp.wn1;
  for(i=0; i<n; i+=8, in+=8, out+=8)
  {
    wn[1] = in[0] + b1*wn[0];
    out[0] = a0*wn[1] + a1*wn[0];
    wn[2] = in[1] + b1*wn[1];
    out[1] = a0*wn[2] + a1*wn[1];
    wn[3] = in[2] + b1*wn[2];
    out[2] = a0*wn[3] + a1*wn[2];
    wn[4] = in[3] + b1*wn[3];
    out[3] = a0*wn[4] + a1*wn[3];
    wn[5] = in[4] + b1*wn[4];
    out[4] = a0*wn[5] + a1*wn[4];
    wn[6] = in[5] + b1*wn[5];
    out[5] = a0*wn[6] + a1*wn[5];
    wn[7] = in[6] + b1*wn[6];
    out[6] = a0*wn[7] + a1*wn[6];
    wn[8] = in[7] + b1*wn[7];
    out[7] = a0*wn[8] + a1*wn[7];
    wn[0] = wn[8];
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn[0]))
    wn[0] = 0.0f;
  
  x->fp.wn1 = wn[0];
  return(w+5);
}

static t_int *filter_tilde_dperform_2o(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  double wn0, wn1=x->dp.wn1, wn2=x->dp.wn2;
  double a0=x->dp.a0, a1=x->dp.a1, a2=x->dp.a2;
  double b1=x->dp.b1, b2=x->dp.b2;
  
  filter_tilde_dsp_dtick(&x->dp);
  for(i=0; i<n; i++)
  {
    wn0 = (double)(*in++) + b1*wn1 + b2*wn2;
    *out++ = (t_float)(a0*wn0 + a1*wn1 + a2*wn2);
    wn2 = wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  //  if(IEM_DENORMAL(wn2))
  //    wn2 = 0.0f;
  //  if(IEM_DENORMAL(wn1))
  //    wn1 = 0.0f;
  
  x->dp.wn1 = wn1;
  x->dp.wn2 = wn2;
  return(w+5);
}
/*   yn0 = *out;
xn0 = *in;
*************
yn0 = a0*xn0 + a1*xn1 + a2*xn2 + b1*yn1 + b2*yn2;
yn2 = yn1;
yn1 = yn0;
xn2 = xn1;
xn1 = xn0;
*************************
y/x = (a0 + a1*z-1 + a2*z-2)/(1 - b1*z-1 - b2*z-2);*/

static t_int *filter_tilde_dperf8_2o(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  double wn[10];
  double a0=x->dp.a0, a1=x->dp.a1, a2=x->dp.a2;
  double b1=x->dp.b1, b2=x->dp.b2;
  
  filter_tilde_dsp_dtick(&x->dp);
  wn[0] = x->dp.wn2;
  wn[1] = x->dp.wn1;
  for(i=0; i<n; i+=8, in+=8, out+=8)
  {
    wn[2] = (double)(in[0]) + b1*wn[1] + b2*wn[0];
    out[0] = (t_float)(a0*wn[2] + a1*wn[1] + a2*wn[0]);
    wn[3] = (double)(in[1]) + b1*wn[2] + b2*wn[1];
    out[1] = (t_float)(a0*wn[3] + a1*wn[2] + a2*wn[1]);
    wn[4] = (double)(in[2]) + b1*wn[3] + b2*wn[2];
    out[2] = (t_float)(a0*wn[4] + a1*wn[3] + a2*wn[2]);
    wn[5] = (double)(in[3]) + b1*wn[4] + b2*wn[3];
    out[3] = (t_float)(a0*wn[5] + a1*wn[4] + a2*wn[3]);
    wn[6] = (double)(in[4]) + b1*wn[5] + b2*wn[4];
    out[4] = (t_float)(a0*wn[6] + a1*wn[5] + a2*wn[4]);
    wn[7] = (double)(in[5]) + b1*wn[6] + b2*wn[5];
    out[5] = (t_float)(a0*wn[7] + a1*wn[6] + a2*wn[5]);
    wn[8] = (double)(in[6]) + b1*wn[7] + b2*wn[6];
    out[6] = (t_float)(a0*wn[8] + a1*wn[7] + a2*wn[6]);
    wn[9] = (double)(in[7]) + b1*wn[8] + b2*wn[7];
    out[7] = (t_float)(a0*wn[9] + a1*wn[8] + a2*wn[7]);
    wn[0] = wn[8];
    wn[1] = wn[9];
  }
  /* NAN protect */
  //  if(IEM_DENORMAL(wn[0]))
  //    wn[0] = 0.0f;
  //  if(IEM_DENORMAL(wn[1]))
  //    wn[1] = 0.0f;
  
  /*BIGORSMALL: 
    tabfudge union double long[2]
    if((long[0]&0x7c000000==0x0x40000000)||(long[0]&0x7c000000==0x0x3c000000))
    double=0.0; 
  
  erstes bit ist signum
    die naechsten 11 bit sind exponent
    +2.0 ist 0x400
    +4.0 ist 0x401
    -4.0 ist 0xC01
    ca. +8.0e+019 ist Sprung von 0x43F auf 0x440   0100.0011.1111 - 0100.0100.0000
    ca. -8.0e+019 ist Sprung von 0xC3F auf 0xC40   1100.0011.1111 - 1100.0100.0000
    ca. +8.0e-019 ist Sprung von 0x3C0 auf 0x3BF   0011.1100.0000 - 0011.1011.1111
    ca. -8.0e-019 ist Sprung von 0xBC0 auf 0xBBF   1011.1100.0000 - 1011.1011.1111
    
    0100.0100.0000
    0100.0011.1111
    0011.1100.0000
    0011.1011.1111
    
    mask = 0x7c
    
    100.01
    100.00}\
    011.11}/
    011.10
    
    
    */
  
  x->dp.wn1 = wn[1];
  x->dp.wn2 = wn[0];
  return(w+5);
}

static t_int *filter_tilde_dperform_1o(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  double wn0, wn1=x->dp.wn1;
  double a0=x->dp.a0, a1=x->dp.a1;
  double b1=x->dp.b1;
  
  filter_tilde_dsp_dtick(&x->dp);
  for(i=0; i<n; i++)
  {
    wn0 = (double)(*in++) + b1*wn1;
    *out++ = (t_float)(a0*wn0 + a1*wn1);
    wn1 = wn0;
  }
  /* NAN protect */
  //  if(IEM_DENORMAL(wn1))
  //    wn1 = 0.0f;
  
  x->dp.wn1 = wn1;
  return(w+5);
}

static t_int *filter_tilde_dperf8_1o(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_filter_tilde *x = (t_filter_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  double wn[9];
  double a0=x->dp.a0, a1=x->dp.a1;
  double b1=x->dp.b1;
  
  filter_tilde_dsp_dtick(&x->dp);
  wn[0] = x->dp.wn1;
  for(i=0; i<n; i+=8, in+=8, out+=8)
  {
    wn[1] = (double)(in[0]) + b1*wn[0];
    out[0] = (t_float)(a0*wn[1] + a1*wn[0]);
    wn[2] = (double)(in[1]) + b1*wn[1];
    out[1] = (t_float)(a0*wn[2] + a1*wn[1]);
    wn[3] = (double)(in[2]) + b1*wn[2];
    out[2] = (t_float)(a0*wn[3] + a1*wn[2]);
    wn[4] = (double)(in[3]) + b1*wn[3];
    out[3] = (t_float)(a0*wn[4] + a1*wn[3]);
    wn[5] = (double)(in[4]) + b1*wn[4];
    out[4] = (t_float)(a0*wn[5] + a1*wn[4]);
    wn[6] = (double)(in[5]) + b1*wn[5];
    out[5] = (t_float)(a0*wn[6] + a1*wn[5]);
    wn[7] = (double)(in[6]) + b1*wn[6];
    out[6] = (t_float)(a0*wn[7] + a1*wn[6]);
    wn[8] = (double)(in[7]) + b1*wn[7];
    out[7] = (t_float)(a0*wn[8] + a1*wn[7]);
    wn[0] = wn[8];
  }
  /* NAN protect */
  //  if(IEM_DENORMAL(wn[0]))
  //    wn[0] = 0.0f;
  
  x->dp.wn1 = wn[0];
  return(w+5);
}

static void filter_tilde_ft4(t_filter_tilde *x, t_float t)
{
  if(x->precision_d1_f0)
  {
    double dt=(double)t;
    int di = (int)((x->dp.ticks_per_interpol_time)*dt+0.49999);
    
    x->dp.interpol_time = dt;
    if(di <= 0)
    {
      x->dp.ticks = 1;
      x->dp.rcp_ticks = 1.0;
    }
    else
    {
      x->dp.ticks = di;
      x->dp.rcp_ticks = 1.0 / (double)di;
    }
  }
  else
  {
    int i = (int)((x->fp.ticks_per_interpol_time)*t+0.49999f);
    
    x->fp.interpol_time = t;
    if(i <= 0)
    {
      x->fp.ticks = 1;
      x->fp.rcp_ticks = 1.0;
    }
    else
    {
      x->fp.ticks = i;
      x->fp.rcp_ticks = 1.0 / (t_float)i;
    }
  }
}

static void filter_tilde_ft3(t_filter_tilde *x, t_float b)
{
  if(x->precision_d1_f0)
  {
    double db=(double)b;
    
    if(db <= 0.0)
      db = 0.000001;
    if(x->dp.filter_function_is_highpass)
      db = 1.0 / db;
    if(db != x->dp.cur_b)
    {
      x->dp.end_b = db;
      x->dp.counter_b = x->dp.ticks;
      x->dp.delta_b = exp(log(db/x->dp.cur_b)*x->dp.rcp_ticks);
      x->dp.event_mask |= 4;/*set event_mask_bit 2 = 1*/
    }
  }
  else
  {
    if(b <= 0.0f)
      b = 0.000001f;
    if(x->fp.filter_function_is_highpass)
      b = 1.0f / b;
    if(b != x->fp.cur_b)
    {
      x->fp.end_b = b;
      x->fp.counter_b = x->fp.ticks;
      x->fp.delta_b = exp(log(b/x->fp.cur_b)*x->fp.rcp_ticks);
      x->fp.event_mask |= 4;/*set event_mask_bit 2 = 1*/
    }
  }
}

static void filter_tilde_ft2(t_filter_tilde *x, t_float a)
{
  if(x->precision_d1_f0)
  {
    double da=(double)a;
    
    if(da <= 0.0)
      da = 0.000001;
    if(x->dp.inlet3_is_Q1_or_damping0)
      da = 1.0 / da;
    if(x->dp.filter_function_is_highpass)
      da /= x->dp.cur_b;
    if(da != x->dp.cur_a)
    {
      x->dp.end_a = da;
      x->dp.counter_a = x->dp.ticks;
      x->dp.delta_a = exp(log(da/x->dp.cur_a)*x->dp.rcp_ticks);
      x->dp.event_mask |= 2;/*set event_mask_bit 1 = 1*/
    }
  }
  else
  {
    if(a <= 0.0f)
      a = 0.000001f;
    if(x->fp.inlet3_is_Q1_or_damping0)
      a = 1.0f / a;
    if(x->fp.filter_function_is_highpass)
      a /= x->fp.cur_b;
    if(a != x->fp.cur_a)
    {
      x->fp.end_a = a;
      x->fp.counter_a = x->fp.ticks;
      x->fp.delta_a = exp(log(a/x->fp.cur_a)*x->fp.rcp_ticks);
      x->fp.event_mask |= 2;/*set event_mask_bit 1 = 1*/
    }
  }
}

static void filter_tilde_ft1(t_filter_tilde *x, t_float f)
{
  if(x->precision_d1_f0)
  {
    double df=(double)f;
    
    if(df <= 0.0)
      df = 0.000001;
    if(df != x->dp.cur_f)
    {
      x->dp.end_f = df;
      x->dp.counter_f = x->dp.ticks;
      x->dp.delta_f = exp(log(df/x->dp.cur_f)*x->dp.rcp_ticks);
      x->dp.event_mask |= 1;/*set event_mask_bit 0 = 1*/
    }
  }
  else
  {
    if(f <= 0.0f)
      f = 0.000001f;
    if(f != x->fp.cur_f)
    {
      x->fp.end_f = f;
      x->fp.counter_f = x->fp.ticks;
      x->fp.delta_f = exp(log(f/x->fp.cur_f)*x->fp.rcp_ticks);
      x->fp.event_mask |= 1;/*set event_mask_bit 0 = 1*/
    }
  }
}

static void filter_tilde_print(t_filter_tilde *x)
{
  //  post("fb1 = %g, fb2 = %g, ff1 = %g, ff2 = %g, ff3 = %g", x->b1, x->b2, x->a0, x->a1, x->a2);
  if(x->precision_d1_f0)
  {
    x->x_at[0].a_w.w_float = (t_float)x->dp.b1;
    x->x_at[1].a_w.w_float = (t_float)x->dp.b2;
    x->x_at[2].a_w.w_float = (t_float)x->dp.a0;
    x->x_at[3].a_w.w_float = (t_float)x->dp.a1;
    x->x_at[4].a_w.w_float = (t_float)x->dp.a2;
  }
  else
  {
    x->x_at[0].a_w.w_float = x->fp.b1;
    x->x_at[1].a_w.w_float = x->fp.b2;
    x->x_at[2].a_w.w_float = x->fp.a0;
    x->x_at[3].a_w.w_float = x->fp.a1;
    x->x_at[4].a_w.w_float = x->fp.a2;
  }
  outlet_list(x->x_debug_outlet, &s_list, 5, x->x_at);
}

static void filter_tilde_dsp(t_filter_tilde *x, t_signal **sp)
{
  int i, n=(int)sp[0]->s_n;
  
  if(x->precision_d1_f0)
  {
    double si, co, f;
    
    x->dp.pi_over_sr = 3.14159265358979323846 / (double)(sp[0]->s_sr);
    x->dp.ticks_per_interpol_time = 0.001 * (double)(sp[0]->s_sr) / (double)n;
    i = (int)((x->dp.ticks_per_interpol_time)*(x->dp.interpol_time)+0.49999);
    if(i <= 0)
    {
      x->dp.ticks = 1;
      x->dp.rcp_ticks = 1.0;
    }
    else
    {
      x->dp.ticks = i;
      x->dp.rcp_ticks = 1.0 / (double)i;
    }
    f = x->dp.cur_f * x->dp.pi_over_sr;
    if(f < 1.0e-20)
      x->dp.cur_l = 1.0e20;
    else if(f > 1.57079632)
      x->dp.cur_l = 0.0;
    else
    {
      si = sin(f);
      co = cos(f);
      x->dp.cur_l = co/si;
    }
    if(x->dp.filter_function_is_first_order)
    {
      if(n&7)
        dsp_add(filter_tilde_dperform_1o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
      else
        dsp_add(filter_tilde_dperf8_1o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
    }
    else
    {
      if(n&7)
        dsp_add(filter_tilde_dperform_2o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
      else
        dsp_add(filter_tilde_dperf8_2o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
    }
  }
  else
  {
    t_float si, co, f;
    
    x->fp.pi_over_sr = 3.14159265358979323846f / (t_float)(sp[0]->s_sr);
    x->fp.ticks_per_interpol_time = 0.001f * (t_float)(sp[0]->s_sr) / (t_float)n;
    i = (int)((x->fp.ticks_per_interpol_time)*(x->fp.interpol_time)+0.49999f);
    if(i <= 0)
    {
      x->fp.ticks = 1;
      x->fp.rcp_ticks = 1.0f;
    }
    else
    {
      x->fp.ticks = i;
      x->fp.rcp_ticks = 1.0f / (t_float)i;
    }
    f = x->fp.cur_f * x->fp.pi_over_sr;
    if(f < 1.0e-20f)
      x->fp.cur_l = 1.0e20f;
    else if(f > 1.57079632f)
      x->fp.cur_l = 0.0f;
    else
    {
      si = sin(f);
      co = cos(f);
      x->fp.cur_l = co/si;
    }
    if(x->fp.filter_function_is_first_order)
    {
      if(n&7)
        dsp_add(filter_tilde_perform_1o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
      else
        dsp_add(filter_tilde_perf8_1o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
    }
    else
    {
      if(n&7)
        dsp_add(filter_tilde_perform_2o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
      else
        dsp_add(filter_tilde_perf8_2o, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
    }
  }
}

static void *filter_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_filter_tilde *x = (t_filter_tilde *)pd_new(filter_tilde_class);
  int i;
  t_symbol *filt_typ=gensym("");
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft3"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft4"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_debug_outlet = outlet_new(&x->x_obj, &s_list);
  x->x_msi = 0.0f;
  
  x->x_at[0].a_type = A_FLOAT;
  x->x_at[1].a_type = A_FLOAT;
  x->x_at[2].a_type = A_FLOAT;
  x->x_at[3].a_type = A_FLOAT;
  x->x_at[4].a_type = A_FLOAT;
  
  x->dp.delta_f = 0.0;
  x->dp.delta_a = 0.0;
  x->dp.delta_b = 0.0;
  x->dp.interpol_time = 0.0;
  x->dp.wn1 = 0.0;
  x->dp.wn2 = 0.0;
  x->dp.a0 = 0.0;
  x->dp.a1 = 0.0;
  x->dp.a2 = 0.0;
  x->dp.b1 = 0.0;
  x->dp.b2 = 0.0;
  x->dp.pi_over_sr = 3.14159265358979323846 / 44100.0;
  x->dp.event_mask = 1;
  x->dp.counter_f = 1;
  x->dp.counter_a = 0;
  x->dp.counter_b = 0;
  x->dp.filter_function_is_first_order = 0;
  
  x->fp.delta_f = 0.0f;
  x->fp.delta_a = 0.0f;
  x->fp.delta_b = 0.0f;
  x->fp.interpol_time = 0.0f;
  x->fp.wn1 = 0.0f;
  x->fp.wn2 = 0.0f;
  x->fp.a0 = 0.0f;
  x->fp.a1 = 0.0f;
  x->fp.a2 = 0.0f;
  x->fp.b1 = 0.0f;
  x->fp.b2 = 0.0f;
  x->fp.pi_over_sr = 3.14159265358979323846f / 44100.0f;
  x->fp.event_mask = 1;
  x->fp.counter_f = 1;
  x->fp.counter_a = 0;
  x->fp.counter_b = 0;
  x->fp.filter_function_is_first_order = 0;
  
  if((argc >= 1) && IS_A_SYMBOL(argv,0))
    filt_typ = atom_getsymbolarg(0, argc, argv);
  
  if(filt_typ->s_name[0] == 'd')
    x->precision_d1_f0 = 1;
  else
    x->precision_d1_f0 = 0;
  
  if(x->precision_d1_f0)
  {
    double si, co, f=0.0, a=0.0, b=0.0, interpol=0.0;
    
    if((argc >= 5) && IS_A_FLOAT(argv,4) && IS_A_FLOAT(argv,3) && IS_A_FLOAT(argv,2) && IS_A_FLOAT(argv,1))
    {
      f = (double)atom_getfloatarg(1, argc, argv);
      a = (double)atom_getfloatarg(2, argc, argv);
      b = (double)atom_getfloatarg(3, argc, argv);
      interpol = (double)atom_getfloatarg(4, argc, argv);
    }
    x->dp.cur_f = f;
    f *= x->dp.pi_over_sr;
    if(f < 1.0e-20)
      x->dp.cur_l = 1.0e20;
    else if(f > 1.57079632)
      x->dp.cur_l = 0.0;
    else
    {
      si = sin(f);
      co = cos(f);
      x->dp.cur_l = co/si;
    }
    if(a <= 0.0)
      a = 0.000001;
    if(b <= 0.0)
      b = 0.000001;
    
    if(interpol <= 0.0)
      interpol = 0.0;
    x->dp.interpol_time = interpol;
    x->dp.ticks_per_interpol_time = 0.001 * 44100.0 / 64.0;
    i = (int)((x->dp.ticks_per_interpol_time)*(x->dp.interpol_time)+0.49999);
    if(i <= 0)
    {
      x->dp.ticks = 1;
      x->dp.rcp_ticks = 1.0;
    }
    else
    {
      x->dp.ticks = i;
      x->dp.rcp_ticks = 1.0 / (double)i;
    }
    
    x->dp.cur_b = b;
    x->dp.cur_a = 1.0/a; /*"a" is default Q*/
    x->dp.inlet3_is_Q1_or_damping0 = 1;
    x->dp.filter_function_is_highpass = 0;
    x->dp.calc = filter_tilde_snafu;
    
    if(filt_typ->s_name)
    {
      if(filt_typ == gensym("dap1"))
      {
        x->dp.calc = filter_tilde_dap1;
        x->dp.a1 = 1.0;
        x->dp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("dap2"))
      {
        x->dp.calc = filter_tilde_dap2;
        x->dp.a2 = 1.0;
      }
      else if(filt_typ == gensym("dap1c"))
      {
        x->dp.calc = filter_tilde_dap1;
        x->dp.a1 = 1.0;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was damping*/
        x->dp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("dap2c"))
      {
        x->dp.calc = filter_tilde_dap2;
        x->dp.a2 = 1.0;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was damping*/
      }
      else if(filt_typ == gensym("dbpq2"))
      {
        x->dp.calc = filter_tilde_dbp2;
      }
      else if(filt_typ == gensym("drbpq2"))
      {
        x->dp.calc = filter_tilde_drp2;
      }
      else if(filt_typ == gensym("dbsq2"))
      {
        x->dp.calc = filter_tilde_dbs2;
      }
      else if(filt_typ == gensym("dbpw2"))
      {
        x->dp.calc = filter_tilde_dbpw2;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was bw*/
      }
      else if(filt_typ == gensym("drbpw2"))
      {
        x->dp.calc = filter_tilde_drpw2;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was bw*/
      }
      else if(filt_typ == gensym("dbsw2"))
      {
        x->dp.calc = filter_tilde_dbsw2;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was bw*/
      }
      else if(filt_typ == gensym("dhp1"))
      {
        x->dp.calc = filter_tilde_dhp1;
        x->dp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("dhp2"))
      {
        x->dp.calc = filter_tilde_dhp2;
      }
      else if(filt_typ == gensym("dlp1"))
      {
        x->dp.calc = filter_tilde_dlp1;
        x->dp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("dlp2"))
      {
        x->dp.calc = filter_tilde_dlp2;
      }
      else if(filt_typ == gensym("dhp1c"))
      {
        x->dp.calc = filter_tilde_dhp1;
        x->dp.cur_a = 1.0 / a;
        x->dp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("dhp2c"))
      {
        x->dp.calc = filter_tilde_dhp2;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a / b;
        x->dp.cur_b = 1.0 / b;
        x->dp.filter_function_is_highpass = 1;
      }
      else if(filt_typ == gensym("dlp1c"))
      {
        x->dp.calc = filter_tilde_dlp1;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was damping*/
        x->dp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("dlp2c"))
      {
        x->dp.calc = filter_tilde_dlp2;
        x->dp.inlet3_is_Q1_or_damping0 = 0;
        x->dp.cur_a = a; /*"a" was damping*/
      }
      else
      {
        post("filter~-Error: 1. initial-arguments: <sym> kind: \
lp1, lp2, hp1, hp2, \
lp1c, lp2c, hp1c, hp2c, \
ap1, ap2, ap1c, ap2c, \
bpq2, rbpq2, bsq2, \
bpw2, rbpw2, bsw2, \
dlp1, dlp2, dhp1, dhp2, \
dlp1c, dlp2c, dhp1c, dhp2c, \
dap1, dap2, dap1c, dap2c, \
dbpq2, drbpq2, dbsq2, \
dbpw2, drbpw2, dbsw2 !");
      }
      x->dp.end_f = x->dp.cur_f;
      x->dp.end_a = x->dp.cur_a;
      x->dp.end_b = x->dp.cur_b;
    }
  }
  else
  {
    t_float si, co, f=0.0f, a=0.0f, b=0.0f, interpol=0.0f;
    
    if((argc >= 5) && IS_A_FLOAT(argv,4) && IS_A_FLOAT(argv,3) && IS_A_FLOAT(argv,2) && IS_A_FLOAT(argv,1))
    {
      f = (t_float)atom_getfloatarg(1, argc, argv);
      a = (t_float)atom_getfloatarg(2, argc, argv);
      b = (t_float)atom_getfloatarg(3, argc, argv);
      interpol = (t_float)atom_getfloatarg(4, argc, argv);
    }
    x->fp.cur_f = f;
    f *= x->fp.pi_over_sr;
    if(f < 1.0e-20f)
      x->fp.cur_l = 1.0e20f;
    else if(f > 1.57079632f)
      x->fp.cur_l = 0.0f;
    else
    {
      si = sin(f);
      co = cos(f);
      x->fp.cur_l = co/si;
    }
    if(a <= 0.0f)
      a = 0.000001f;
    if(b <= 0.0f)
      b = 0.000001f;
    
    if(interpol <= 0.0f)
      interpol = 0.0f;
    x->fp.interpol_time = interpol;
    x->fp.ticks_per_interpol_time = 0.001f * 44100.0f / 64.0f;
    i = (int)((x->fp.ticks_per_interpol_time)*(x->fp.interpol_time)+0.49999f);
    if(i <= 0)
    {
      x->fp.ticks = 1;
      x->fp.rcp_ticks = 1.0f;
    }
    else
    {
      x->fp.ticks = i;
      x->fp.rcp_ticks = 1.0f / (t_float)i;
    }
    
    x->fp.cur_b = b;
    x->fp.cur_a = 1.0f/a; /*"a" is default Q*/
    x->fp.inlet3_is_Q1_or_damping0 = 1;
    x->fp.filter_function_is_highpass = 0;
    x->fp.calc = filter_tilde_snafu;
    
    if(filt_typ->s_name)
    {
      if(filt_typ == gensym("ap1"))
      {
        x->fp.calc = filter_tilde_ap1;
        x->fp.a1 = 1.0f;
        x->fp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("ap2"))
      {
        x->fp.calc = filter_tilde_ap2;
       x->fp.a2 = 1.0f;
      }
      else if(filt_typ == gensym("ap1c"))
      {
        x->fp.calc = filter_tilde_ap1;
        x->fp.a1 = 1.0f;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was damping*/
        x->fp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("ap2c"))
      {
        x->fp.calc = filter_tilde_ap2;
        x->fp.a2 = 1.0f;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was damping*/
      }
      else if(filt_typ == gensym("bpq2"))
      {
        x->fp.calc = filter_tilde_bp2;
      }
      else if(filt_typ == gensym("rbpq2"))
      {
        x->fp.calc = filter_tilde_rp2;
      }
      else if(filt_typ == gensym("bsq2"))
      {
        x->fp.calc = filter_tilde_bs2;
      }
      else if(filt_typ == gensym("bpw2"))
      {
        x->fp.calc = filter_tilde_bpw2;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was bw*/
      }
      else if(filt_typ == gensym("rbpw2"))
      {
        x->fp.calc = filter_tilde_rpw2;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was bw*/
      }
      else if(filt_typ == gensym("bsw2"))
      {
        x->fp.calc = filter_tilde_bsw2;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was bw*/
      }
      else if(filt_typ == gensym("hp1"))
      {
        x->fp.calc = filter_tilde_hp1;
        x->fp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("hp2"))
      {
        x->fp.calc = filter_tilde_hp2;
      }
      else if(filt_typ == gensym("lp1"))
      {
        x->fp.calc = filter_tilde_lp1;
        x->fp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("lp2"))
      {
        x->fp.calc = filter_tilde_lp2;
      }
      else if(filt_typ == gensym("hp1c"))
      {
        x->fp.calc = filter_tilde_hp1;
        x->fp.cur_a = 1.0f / a;
        x->fp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("hp2c"))
      {
        x->fp.calc = filter_tilde_hp2;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a / b;
        x->fp.cur_b = 1.0f / b;
        x->fp.filter_function_is_highpass = 1;
      }
      else if(filt_typ == gensym("lp1c"))
      {
        x->fp.calc = filter_tilde_lp1;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was damping*/
        x->fp.filter_function_is_first_order = 1;
      }
      else if(filt_typ == gensym("lp2c"))
      {
        x->fp.calc = filter_tilde_lp2;
        x->fp.inlet3_is_Q1_or_damping0 = 0;
        x->fp.cur_a = a; /*"a" was damping*/
      }
      else
      {
        post("filter~-Error: 1. initial-arguments: <sym> kind: \
lp1, lp2, hp1, hp2, \
lp1c, lp2c, hp1c, hp2c, \
ap1, ap2, ap1c, ap2c, \
bpq2, rbpq2, bsq2, \
bpw2, rbpw2, bsw2, \
dlp1, dlp2, dhp1, dhp2, \
dlp1c, dlp2c, dhp1c, dhp2c, \
dap1, dap2, dap1c, dap2c, \
dbpq2, drbpq2, dbsq2, \
dbpw2, drbpw2, dbsw2 !");
      }
      x->fp.end_f = x->fp.cur_f;
      x->fp.end_a = x->fp.cur_a;
      x->fp.end_b = x->fp.cur_b;
    }
  }
  return (x);
}

void filter_tilde_setup(void)
{
  filter_tilde_class = class_new(gensym("filter~"), (t_newmethod)filter_tilde_new,
        0, sizeof(t_filter_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(filter_tilde_class, t_filter_tilde, x_msi);
  class_addmethod(filter_tilde_class, (t_method)filter_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(filter_tilde_class, (t_method)filter_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(filter_tilde_class, (t_method)filter_tilde_ft2, gensym("ft2"), A_FLOAT, 0);
  class_addmethod(filter_tilde_class, (t_method)filter_tilde_ft3, gensym("ft3"), A_FLOAT, 0);
  class_addmethod(filter_tilde_class, (t_method)filter_tilde_ft4, gensym("ft4"), A_FLOAT, 0);
  class_addmethod(filter_tilde_class, (t_method)filter_tilde_print, gensym("print"), 0);
}
