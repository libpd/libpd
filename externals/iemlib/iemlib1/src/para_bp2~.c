/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>


/* ---------- para_bp2~ - parametric bandpass 2. order ----------- */

typedef struct _para_bp2_tilde
{
  t_object x_obj;
  t_float  wn1;
  t_float  wn2;
  t_float  a0;
  t_float  a1;
  t_float  a2;
  t_float  b1;
  t_float  b2;
  t_float  sr;
  t_float  cur_f;
  t_float  cur_l;
  t_float  cur_a;
  t_float  cur_g;
  t_float  delta_f;
  t_float  delta_a;
  t_float  delta_g;
  t_float  end_f;
  t_float  end_a;
  t_float  end_g;
  t_float  ticks_per_interpol_time;
  t_float  rcp_ticks;
  t_float  interpol_time;
  int      ticks;
  int      counter_f;
  int      counter_a;
  int      counter_g;
  int      event_mask;
  void     *x_debug_outlet;
  t_atom   x_at[5];
  t_float  x_msi;
} t_para_bp2_tilde;

static t_class *para_bp2_tilde_class;

static void para_bp2_tilde_calc(t_para_bp2_tilde *x)
{
  t_float l, al, gal, l2, rcp;
  
  l = x->cur_l;
  l2 = l*l + 1.0f;
  al = l*x->cur_a;
  gal = al*x->cur_g;
  rcp = 1.0f/(al + l2);
  x->a0 = rcp*(l2 + gal);
  x->a1 = rcp*2.0f*(2.0f - l2);
  x->a2 = rcp*(l2 - gal);
  x->b1 = -x->a1;
  x->b2 = rcp*(al - l2);
}

static void para_bp2_tilde_dsp_tick(t_para_bp2_tilde *x)
{
  if(x->event_mask)
  {
    t_float discriminant;
    
    if(x->counter_f)
    {
      t_float l, si, co;
      
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
      l = x->cur_f * x->sr;
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
    if(x->counter_g)
    {
      if(x->counter_g <= 1)
      {
        x->cur_g = x->end_g;
        x->counter_g = 0;
        x->event_mask &= 3;/*set event_mask_bit 2 = 0*/
      }
      else
      {
        x->counter_g--;
        x->cur_g *= x->delta_g;
      }
    }
    
    para_bp2_tilde_calc(x);
    
    /* stability check */
    
    discriminant = x->b1 * x->b1 + 4.0f * x->b2;
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
}

static t_int *para_bp2_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_para_bp2_tilde *x = (t_para_bp2_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn0, wn1=x->wn1, wn2=x->wn2;
  t_float a0=x->a0, a1=x->a1, a2=x->a2;
  t_float b1=x->b1, b2=x->b2;
  
  para_bp2_tilde_dsp_tick(x);
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
  
  x->wn1 = wn1;
  x->wn2 = wn2;
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

static t_int *para_bp2_tilde_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_para_bp2_tilde *x = (t_para_bp2_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn[10];
  t_float a0=x->a0, a1=x->a1, a2=x->a2;
  t_float b1=x->b1, b2=x->b2;
  
  para_bp2_tilde_dsp_tick(x);
  wn[0] = x->wn2;
  wn[1] = x->wn1;
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
  
  x->wn1 = wn[1];
  x->wn2 = wn[0];
  return(w+5);
}

static void para_bp2_tilde_ft4(t_para_bp2_tilde *x, t_floatarg t)
{
  int i = (int)((x->ticks_per_interpol_time)*t);
  
  x->interpol_time = t;
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
}

static void para_bp2_tilde_ft3(t_para_bp2_tilde *x, t_floatarg l)
{
  t_float g = exp(0.11512925465 * l);
  
  if(g != x->cur_g)
  {
    x->end_g = g;
    x->counter_g = x->ticks;
    x->delta_g = exp(log(g/x->cur_g)*x->rcp_ticks);
    x->event_mask |= 4;/*set event_mask_bit 2 = 1*/
  }
}

static void para_bp2_tilde_ft2(t_para_bp2_tilde *x, t_floatarg q)
{
  t_float a;
  
  if(q <= 0.0f)
    q = 0.000001f;
  a = 1.0f/q;
  if(a != x->cur_a)
  {
    x->end_a = a;
    x->counter_a = x->ticks;
    x->delta_a = exp(log(a/x->cur_a)*x->rcp_ticks);
    x->event_mask |= 2;/*set event_mask_bit 1 = 1*/
  }
}

static void para_bp2_tilde_ft1(t_para_bp2_tilde *x, t_floatarg f)
{
  if(f <= 0.0f)
    f = 0.000001f;
  if(f != x->cur_f)
  {
    x->end_f = f;
    x->counter_f = x->ticks;
    x->delta_f = exp(log(f/x->cur_f)*x->rcp_ticks);
    x->event_mask |= 1;/*set event_mask_bit 0 = 1*/
  }
}

static void para_bp2_tilde_print(t_para_bp2_tilde *x)
{
  //  post("fb1 = %g, fb2 = %g, ff1 = %g, ff2 = %g, ff3 = %g", x->b1, x->b2, x->a0, x->a1, x->a2);
  x->x_at[0].a_w.w_float = x->b1;
  x->x_at[1].a_w.w_float = x->b2;
  x->x_at[2].a_w.w_float = x->a0;
  x->x_at[3].a_w.w_float = x->a1;
  x->x_at[4].a_w.w_float = x->a2;
  outlet_list(x->x_debug_outlet, &s_list, 5, x->x_at);
}

static void para_bp2_tilde_dsp(t_para_bp2_tilde *x, t_signal **sp)
{
  t_float si, co, f;
  int i, n=(int)sp[0]->s_n;
  
  x->sr = 3.14159265358979323846f / (t_float)(sp[0]->s_sr);
  x->ticks_per_interpol_time = 0.001f * (t_float)(sp[0]->s_sr) / (t_float)n;
  i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
  f = x->cur_f * x->sr;
  if(f < 1.0e-20f)
    x->cur_l = 1.0e20f;
  else if(f > 1.57079632f)
    x->cur_l = 0.0f;
  else
  {
    si = sin(f);
    co = cos(f);
    x->cur_l = co/si;
  }
  if(n&7)
    dsp_add(para_bp2_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
  else
    dsp_add(para_bp2_tilde_perf8, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *para_bp2_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_para_bp2_tilde *x = (t_para_bp2_tilde *)pd_new(para_bp2_tilde_class);
  int i;
  t_float si, co, f=0.0f, q=1.0f, l=0.0f, interpol=0.0f;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft3"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft4"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_debug_outlet = outlet_new(&x->x_obj, &s_list);
  x->x_msi = 0;
  
  x->x_at[0].a_type = A_FLOAT;
  x->x_at[1].a_type = A_FLOAT;
  x->x_at[2].a_type = A_FLOAT;
  x->x_at[3].a_type = A_FLOAT;
  x->x_at[4].a_type = A_FLOAT;
  
  x->event_mask = 1;
  x->counter_f = 1;
  x->counter_a = 0;
  x->counter_g = 0;
  x->delta_f = 0.0f;
  x->delta_a = 0.0f;
  x->delta_g = 0.0f;
  x->interpol_time = 500.0f;
  x->wn1 = 0.0f;
  x->wn2 = 0.0f;
  x->a0 = 0.0f;
  x->a1 = 0.0f;
  x->a2 = 0.0f;
  x->b1 = 0.0f;
  x->b2 = 0.0f;
  x->sr = 3.14159265358979323846f / 44100.0f;
  x->cur_a = 1.0f;
  if((argc == 4)&&IS_A_FLOAT(argv,3)&&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,1)&&IS_A_FLOAT(argv,0))
  {
    f = (t_float)atom_getfloatarg(0, argc, argv);
    q = (t_float)atom_getfloatarg(1, argc, argv);
    l = (t_float)atom_getfloatarg(2, argc, argv);
    interpol = (t_float)atom_getfloatarg(3, argc, argv);
  }
  if(f <= 0.0f)
    f = 0.000001f;
  x->cur_f = f;
  f *= x->sr;
  if(f < 1.0e-20f)
    x->cur_l = 1.0e20f;
  else if(f > 1.57079632f)
    x->cur_l = 0.0f;
  else
  {
    si = sin(f);
    co = cos(f);
    x->cur_l = co/si;
  }
  if(q <= 0.0f)
    q = 0.000001f;
  x->cur_a = 1.0f/q;
  x->cur_g = exp(0.11512925465 * l);
  if(interpol <= 0.0f)
    interpol = 0.0f;
  x->interpol_time = interpol;
  x->ticks_per_interpol_time = 0.5f;
  i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
  x->end_f = x->cur_f;
  x->end_a = x->cur_a;
  x->end_g = x->cur_g;
  return(x);
}

void para_bp2_tilde_setup(void)
{
  para_bp2_tilde_class = class_new(gensym("para_bp2~"), (t_newmethod)para_bp2_tilde_new,
        0, sizeof(t_para_bp2_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(para_bp2_tilde_class, t_para_bp2_tilde, x_msi);
  class_addmethod(para_bp2_tilde_class, (t_method)para_bp2_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(para_bp2_tilde_class, (t_method)para_bp2_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(para_bp2_tilde_class, (t_method)para_bp2_tilde_ft2, gensym("ft2"), A_FLOAT, 0);
  class_addmethod(para_bp2_tilde_class, (t_method)para_bp2_tilde_ft3, gensym("ft3"), A_FLOAT, 0);
  class_addmethod(para_bp2_tilde_class, (t_method)para_bp2_tilde_ft4, gensym("ft4"), A_FLOAT, 0);
  class_addmethod(para_bp2_tilde_class, (t_method)para_bp2_tilde_print, gensym("print"), 0);
//  class_sethelpsymbol(para_bp2_tilde_class, gensym("iemhelp/help-para_bp2~"));
}
