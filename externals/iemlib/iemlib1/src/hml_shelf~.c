/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ---------- hml_shelf~ - high-middle-low-shelving filter ----------- */

typedef struct _hml_shelf_tilde
{
  t_object  x_obj;
  t_float   wn1;
  t_float   wn2;
  t_float   a0;
  t_float   a1;
  t_float   a2;
  t_float   b1;
  t_float   b2;
  t_float   sr;
  t_float   cur_lf;
  t_float   cur_hf;
  t_float   cur_lg;
  t_float   cur_mg;
  t_float   cur_hg;
  t_float   delta_lf;
  t_float   delta_hf;
  t_float   delta_lg;
  t_float   delta_mg;
  t_float   delta_hg;
  t_float   end_lf;
  t_float   end_hf;
  t_float   end_lg;
  t_float   end_mg;
  t_float   end_hg;
  t_float   ticks_per_interpol_time;
  t_float   rcp_ticks;
  t_float   interpol_time;
  int       ticks;
  int       counter_lf;
  int       counter_hf;
  int       counter_lg;
  int       counter_mg;
  int       counter_hg;
  int       event_mask;
  void      *x_debug_outlet;
  t_atom    x_at[5];
  t_float   x_msi;
} t_hml_shelf_tilde;

static t_class *hml_shelf_tilde_class;

static void hml_shelf_tilde_calc(t_hml_shelf_tilde *x)
{
  t_float rf = x->cur_hf/x->cur_lf;
  t_float mf = x->cur_hf*x->cur_lf;
  t_float lg = x->cur_lg;
  t_float rcplg = 1.0f/lg;
  t_float mg = x->cur_mg;
  t_float rcpmg = 1.0f/mg;
  t_float hg = x->cur_hg;
  t_float rcphg = 1.0f/hg;
  t_float f = mf*x->sr;
  t_float l = cos(f)/sin(f);
  t_float k1 = rf*l;
  t_float k2 = l/rf;
  t_float k3 = l*l;
  t_float k4 = k3*hg;
  t_float k5 = k3*rcphg;
  t_float k6 = rcplg + k5;
  t_float k7 = rcpmg*k1 + k2*rcplg*rcphg*mg;
  t_float k8 = lg + k4;
  t_float k9 = mg*k1 + k2*lg*hg*rcpmg;
  t_float k10 = 1.0f/(k6 + k7);
  
  x->b2 = k10*(k7 - k6);
  x->b1 = k10*2.0f*(k5 - rcplg);
  x->a2 = k10*(k8 - k9);
  x->a1 = k10*2.0f*(lg - k4);
  x->a0 = k10*(k8 + k9);
}

/*
high- & low- shelving-filter:
L....sqrt(lowlevel);
rL...rsqrt(lowlevel);
M....sqrt(mediumlevel);
rM...rsqrt(mediumlevel);
H....sqrt(highlevel);
rH...rsqrt(highlevel);
V....sqrt(highfrequency/lowfrequency);
P....j*2*pi*f/(2*pi*V*lowfrequency);

Y/X = [M/(1/M)] * [(L/M + PV)/(M/L + PV)] * [(1 + HP/(VM))/(1 + MP/(VH))];
Y/X = (L + P*(M*V + L*H/(V*M)) + P*P*H) / (rL + P*(rM*V + rL*rH/(V*rM)) + P*P*rH);
  
hlshlv: lowlevel: ll; mediumlevel: lm; highlevel: hl; lowfrequency: fl; highfrequency: fh; samplerate: sr;
    
V = sqrt(fh/fl);
f = fl*V;
L = sqrt(ll);
rL = 1.0/L;
M = sqrt(lm);
rM = 1.0/M;
H = sqrt(lh);
rH = 1.0/H;
   
l = cot(f*3.14159265358979323846/sr);
k1 = V*l;
k2 = l/V;
l2 = l*l;
l3 = l2*H;
l4 = l2*rH;
m1 = k2*L*H*rM;
m2 = k2*rL*rH*M;
n1 = rL + l4;
n2 = rM*k1 + m2;
p1 = L + l3;
p2 = M*k1 + m1;
a012 = 1.0/(n1 + n2);
    
b2 = a012*(n2 - n1);
b1 = a012*2.0*(l4 - rL);
a2 = a012*(p1 - p2);
a1 = a012*2.0*(L - l3);
a0 = a012*(p1 + p2);

rf = sqrt(fh/fl);
mf = fl*rf;
L = sqrt(ll);
rL = 1.0/L;
M = sqrt(lm);
rM = 1.0/M;
H = sqrt(lh);
rH = 1.0/H;
                    
l = cot(fm*3.14159265358979323846/sr);
k1 = V*l;
k2 = l/V;
k3 = l*l;
k4 = k3*H;
k5 = k3*rH;
k6 = rL + k5;
k7 = rM*k1 + k2*rL*rH*M;
k8 = L + k4;
k9 = M*k1 + k2*L*H*rM;
k10 = 1.0/(k6 + k7);
                     
b2 = k10*(k7 - k6);
b1 = k10*2.0*(k5 - rL);
a2 = k10*(k8 - k9);
a1 = k10*2.0*(L - k4);
a0 = k10*(k8 + k9);
*/
                        

static void hml_shelf_tilde_dsp_tick(t_hml_shelf_tilde *x)
{
  if(x->event_mask)
  {
    t_float discriminant;
    
    if(x->counter_lg)
    {
      if(x->counter_lg <= 1)
      {
        x->cur_lg = x->end_lg;
        x->counter_lg = 0;
        x->event_mask &= 30;/*set event_mask_bit 0 = 0*/
      }
      else
      {
        x->counter_lg--;
        x->cur_lg *= x->delta_lg;
      }
    }
    if(x->counter_lf)
    {
      if(x->counter_lf <= 1)
      {
        x->cur_lf = x->end_lf;
        x->counter_lf = 0;
        x->event_mask &= 29;/*set event_mask_bit 1 = 0*/
      }
      else
      {
        x->counter_lf--;
        x->cur_lf *= x->delta_lf;
      }
    }
    if(x->counter_mg)
    {
      if(x->counter_mg <= 1)
      {
        x->cur_mg = x->end_mg;
        x->counter_mg = 0;
        x->event_mask &= 27;/*set event_mask_bit 2 = 0*/
      }
      else
      {
        x->counter_mg--;
        x->cur_mg *= x->delta_mg;
      }
    }
    if(x->counter_hf)
    {
      if(x->counter_hf <= 1)
      {
        x->cur_hf = x->end_hf;
        x->counter_hf = 0;
        x->event_mask &= 23;/*set event_mask_bit 3 = 0*/
      }
      else
      {
        x->counter_hf--;
        x->cur_hf *= x->delta_hf;
      }
    }
    if(x->counter_hg)
    {
      if(x->counter_hg <= 1)
      {
        x->cur_hg = x->end_hg;
        x->counter_hg = 0;
        x->event_mask &= 15;/*set event_mask_bit 4 = 0*/
      }
      else
      {
        x->counter_hg--;
        x->cur_hg *= x->delta_hg;
      }
    }
    hml_shelf_tilde_calc(x);
    
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

static t_int *hml_shelf_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_hml_shelf_tilde *x = (t_hml_shelf_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn0, wn1=x->wn1, wn2=x->wn2;
  t_float a0=x->a0, a1=x->a1, a2=x->a2;
  t_float b1=x->b1, b2=x->b2;
  
  hml_shelf_tilde_dsp_tick(x);
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
y/x = (a0 + a1*z-1 + a2*z-2)/(1 - b1*z-1 - b2*z-2);
*/

static t_int *hml_shelf_tilde_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_hml_shelf_tilde *x = (t_hml_shelf_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float wn[10];
  t_float a0=x->a0, a1=x->a1, a2=x->a2;
  t_float b1=x->b1, b2=x->b2;
  
  hml_shelf_tilde_dsp_tick(x);
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

static void hml_shelf_tilde_ft6(t_hml_shelf_tilde *x, t_floatarg t)
{
  int i = (int)((x->ticks_per_interpol_time)*t);
  
  x->interpol_time = t;
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
}

static void hml_shelf_tilde_ft5(t_hml_shelf_tilde *x, t_floatarg hl)
{
  t_float hg = exp(0.057564627325 * hl);
  
  if(hg != x->cur_hg)
  {
    x->end_hg = hg;
    x->counter_hg = x->ticks;
    x->delta_hg = exp(log(hg/x->cur_hg)*x->rcp_ticks);
    x->event_mask |= 16;/*set event_mask_bit 4 = 1*/
  }
}

static void hml_shelf_tilde_ft4(t_hml_shelf_tilde *x, t_floatarg hf)
{
  t_float sqhf;
  
  if(hf <= 0.0f)
    hf = 0.000001f;
  sqhf = sqrt(hf);
  if(sqhf != x->cur_hf)
  {
    x->end_hf = sqhf;
    x->counter_hf = x->ticks;
    x->delta_hf = exp(log(sqhf/x->cur_hf)*x->rcp_ticks);
    x->event_mask |= 8;/*set event_mask_bit 3 = 1*/
  }
}

static void hml_shelf_tilde_ft3(t_hml_shelf_tilde *x, t_floatarg ml)
{
  t_float mg = exp(0.057564627325 * ml);
  
  if(mg != x->cur_mg)
  {
    x->end_mg = mg;
    x->counter_mg = x->ticks;
    x->delta_mg = exp(log(mg/x->cur_mg)*x->rcp_ticks);
    x->event_mask |= 4;/*set event_mask_bit 2 = 1*/
  }
}

static void hml_shelf_tilde_ft2(t_hml_shelf_tilde *x, t_floatarg lf)
{
  t_float sqlf;
  
  if(lf <= 0.0f)
    lf = 0.000001f;
  sqlf = sqrt(lf);
  if(sqlf != x->cur_lf)
  {
    x->end_lf = sqlf;
    x->counter_lf = x->ticks;
    x->delta_lf = exp(log(sqlf/x->cur_lf)*x->rcp_ticks);
    x->event_mask |= 2;/*set event_mask_bit 1 = 1*/
  }
}

static void hml_shelf_tilde_ft1(t_hml_shelf_tilde *x, t_floatarg ll)
{
  t_float lg = exp(0.057564627325 * ll);
  
  if(lg != x->cur_lg)
  {
    x->end_lg = lg;
    x->counter_lg = x->ticks;
    x->delta_lg = exp(log(lg/x->cur_lg)*x->rcp_ticks);
    x->event_mask |= 1;/*set event_mask_bit 0 = 1*/
  }
}

static void hml_shelf_tilde_print(t_hml_shelf_tilde *x)
{
  //  post("fb1 = %g, fb2 = %g, ff1 = %g, ff2 = %g, ff3 = %g", x->b1, x->b2, x->a0, x->a1, x->a2);
  x->x_at[0].a_w.w_float = x->b1;
  x->x_at[1].a_w.w_float = x->b2;
  x->x_at[2].a_w.w_float = x->a0;
  x->x_at[3].a_w.w_float = x->a1;
  x->x_at[4].a_w.w_float = x->a2;
  outlet_list(x->x_debug_outlet, &s_list, 5, x->x_at);
}

static void hml_shelf_tilde_dsp(t_hml_shelf_tilde *x, t_signal **sp)
{
  int i, n=(int)sp[0]->s_n;
  
  x->sr = 3.14159265358979323846f / (t_float)(sp[0]->s_sr);
  x->ticks_per_interpol_time = 0.001f * (t_float)(sp[0]->s_sr) / (t_float)n;
  i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
  if(n&7)
    dsp_add(hml_shelf_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
  else
    dsp_add(hml_shelf_tilde_perf8, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *hml_shelf_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_hml_shelf_tilde *x = (t_hml_shelf_tilde *)pd_new(hml_shelf_tilde_class);
  int i;
  t_float lf=200.0f, hf=2000.0f, ll=0.0f, ml=0.0f, hl=0.0f, interpol=0.0f;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft3"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft4"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft5"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft6"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_debug_outlet = outlet_new(&x->x_obj, &s_list);
  x->x_msi = 0;
  
  x->x_at[0].a_type = A_FLOAT;
  x->x_at[1].a_type = A_FLOAT;
  x->x_at[2].a_type = A_FLOAT;
  x->x_at[3].a_type = A_FLOAT;
  x->x_at[4].a_type = A_FLOAT;
  
  x->event_mask = 2;
  x->counter_lg = 0;
  x->counter_lf = 1;
  x->counter_mg = 0;
  x->counter_hf = 0;
  x->counter_hg = 0;
  x->delta_lg = 0.0f;
  x->delta_lf = 0.0f;
  x->delta_mg = 0.0f;
  x->delta_hf = 0.0f;
  x->delta_hg = 0.0f;
  x->interpol_time = 0.0f;
  x->wn1 = 0.0f;
  x->wn2 = 0.0f;
  x->a0 = 0.0f;
  x->a1 = 0.0f;
  x->a2 = 0.0f;
  x->b1 = 0.0f;
  x->b2 = 0.0f;
  x->sr = 3.14159265358979323846f / 44100.0f;
  if((argc == 6)&&IS_A_FLOAT(argv,5)&&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,3)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,1)&&IS_A_FLOAT(argv,0))
  {
    ll = (t_float)atom_getfloatarg(0, argc, argv);
    lf = (t_float)atom_getfloatarg(1, argc, argv);
    ml = (t_float)atom_getfloatarg(2, argc, argv);
    hf = (t_float)atom_getfloatarg(3, argc, argv);
    hl = (t_float)atom_getfloatarg(4, argc, argv);
    interpol = (t_float)atom_getfloatarg(5, argc, argv);
  }
  x->cur_lg = exp(0.057564627325 * ll);
  x->cur_mg = exp(0.057564627325 * ml);
  x->cur_hg = exp(0.057564627325 * hl);
  if(lf <= 0.0f)
    lf = 0.000001f;
  if(hf <= 0.0f)
    hf = 0.000001f;
  x->cur_lf = sqrt(lf);
  x->cur_hf = sqrt(hf);
  if(interpol < 0.0f)
    interpol = 0.0f;
  x->interpol_time = interpol;
  x->ticks_per_interpol_time = 0.5f;
  i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
  x->end_lf = x->cur_lf;
  x->end_hf = x->cur_hf;
  x->end_lg = x->cur_lg;
  x->end_mg = x->cur_mg;
  x->end_hg = x->cur_hg;
  return(x);
}

void hml_shelf_tilde_setup(void)
{
  hml_shelf_tilde_class = class_new(gensym("hml_shelf~"), (t_newmethod)hml_shelf_tilde_new,
    0, sizeof(t_hml_shelf_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(hml_shelf_tilde_class, t_hml_shelf_tilde, x_msi);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_ft2, gensym("ft2"), A_FLOAT, 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_ft3, gensym("ft3"), A_FLOAT, 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_ft4, gensym("ft4"), A_FLOAT, 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_ft5, gensym("ft5"), A_FLOAT, 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_ft6, gensym("ft6"), A_FLOAT, 0);
  class_addmethod(hml_shelf_tilde_class, (t_method)hml_shelf_tilde_print, gensym("print"), 0);
//  class_sethelpsymbol(hml_shelf_tilde_class, gensym("iemhelp/help-hml_shelf~"));
}
