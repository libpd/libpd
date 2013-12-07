/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <string.h>
#include <math.h>

/* ---------- vcf_filter~ - slow dynamic vcf_filter 1. and 2. order ----------- */

typedef struct _vcf_filter_tilde
{
  t_object x_obj;
  t_float  x_wn1;
  t_float  x_wn2;
  t_float  x_msi;
  char     x_filtname[6];
} t_vcf_filter_tilde;

static t_class *vcf_filter_tilde_class;

static t_int *vcf_filter_tilde_perform_snafu(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[4]);
  int n = (t_int)(w[6]);
  
  while(n--)
    *out++ = *in++;
  return(w+7);
}

/*
lp2
wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
*out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
wn2 = wn1;
wn1 = wn0;

  bp2
  wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*al*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
      rbp2
      wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
      *out++ = rcp*l*(wn0 - wn2);
      wn2 = wn1;
      wn1 = wn0;
      
        hp2
        wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
        *out++ = rcp*(wn0 - 2.0f*wn1 + wn2);
        wn2 = wn1;
        wn1 = wn0;
        
*/

static t_int *vcf_filter_tilde_perform_lp2(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *lp = (t_float *)(w[2]);
  t_float *q = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  t_vcf_filter_tilde *x = (t_vcf_filter_tilde *)(w[5]);
  int i, n = (t_int)(w[6]);
  t_float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
  t_float l, al, l2, rcp;
  
  for(i=0; i<n; i+=4)
  {
    l = lp[i];
    if(q[i] < 0.000001f)
      al = 1000000.0f*l;
    else if(q[i] > 1000000.0f)
      al = 0.000001f*l;
    else
      al = l/q[i];
    l2 = l*l + 1.0f;
    rcp = 1.0f/(al + l2);
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn2))
    wn2 = 0.0f;
  if(IEM_DENORMAL(wn1))
    wn1 = 0.0f;
  
  x->x_wn1 = wn1;
  x->x_wn2 = wn2;
  return(w+7);
}

static t_int *vcf_filter_tilde_perform_bp2(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *lp = (t_float *)(w[2]);
  t_float *q = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  t_vcf_filter_tilde *x = (t_vcf_filter_tilde *)(w[5]);
  int i, n = (t_int)(w[6]);
  t_float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
  t_float l, al, l2, rcp;
  
  for(i=0; i<n; i+=4)
  {
    l = lp[i];
    if(q[i] < 0.000001f)
      al = 1000000.0f*l;
    else if(q[i] > 1000000.0f)
      al = 0.000001f*l;
    else
      al = l/q[i];
    l2 = l*l + 1.0f;
    rcp = 1.0f/(al + l2);
    
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*al*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*al*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*al*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*al*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn2))
    wn2 = 0.0f;
  if(IEM_DENORMAL(wn1))
    wn1 = 0.0f;
  
  x->x_wn1 = wn1;
  x->x_wn2 = wn2;
  return(w+7);
}

static t_int *vcf_filter_tilde_perform_rbp2(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *lp = (t_float *)(w[2]);
  t_float *q = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  t_vcf_filter_tilde *x = (t_vcf_filter_tilde *)(w[5]);
  int i, n = (t_int)(w[6]);
  t_float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
  t_float al, l, l2, rcp;
  
  for(i=0; i<n; i+=4)
  {
    l = lp[i];
    if(q[i] < 0.000001f)
      al = 1000000.0f*l;
    else if(q[i] > 1000000.0f)
      al = 0.000001f*l;
    else
      al = l/q[i];
    l2 = l*l + 1.0f;
    rcp = 1.0f/(al + l2);
    
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*l*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*l*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*l*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = rcp*l*(wn0 - wn2);
    wn2 = wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn2))
    wn2 = 0.0f;
  if(IEM_DENORMAL(wn1))
    wn1 = 0.0f;
  
  x->x_wn1 = wn1;
  x->x_wn2 = wn2;
  return(w+7);
}

static t_int *vcf_filter_tilde_perform_hp2(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *lp = (t_float *)(w[2]);
  t_float *q = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  t_vcf_filter_tilde *x = (t_vcf_filter_tilde *)(w[5]);
  int i, n = (t_int)(w[6]);
  t_float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
  t_float l, al, l2, rcp, forw;
  
  for(i=0; i<n; i+=4)
  {
    l = lp[i];
    if(q[i] < 0.000001f)
      al = 1000000.0f*l;
    else if(q[i] > 1000000.0f)
      al = 0.000001f*l;
    else
      al = l/q[i];
    l2 = l*l + 1.0f;
    rcp = 1.0f/(al + l2);
    forw = rcp * (l2 - 1.0f);
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = forw*(wn0 - 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = forw*(wn0 - 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = forw*(wn0 - 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
    
    wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
    *out++ = forw*(wn0 - 2.0f*wn1 + wn2);
    wn2 = wn1;
    wn1 = wn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(wn2))
    wn2 = 0.0f;
  if(IEM_DENORMAL(wn1))
    wn1 = 0.0f;
  
  x->x_wn1 = wn1;
  x->x_wn2 = wn2;
  return(w+7);
}

static void vcf_filter_tilde_dsp(t_vcf_filter_tilde *x, t_signal **sp)
{
  if(!strcmp(x->x_filtname,"bp2"))
    dsp_add(vcf_filter_tilde_perform_bp2, 6, sp[0]->s_vec, sp[1]->s_vec, 
    sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
  else if(!strcmp(x->x_filtname,"rbp2"))
    dsp_add(vcf_filter_tilde_perform_rbp2, 6, sp[0]->s_vec, sp[1]->s_vec, 
    sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
  else if(!strcmp(x->x_filtname,"lp2"))
    dsp_add(vcf_filter_tilde_perform_lp2, 6, sp[0]->s_vec, sp[1]->s_vec,
    sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
  else if(!strcmp(x->x_filtname,"hp2"))
    dsp_add(vcf_filter_tilde_perform_hp2, 6, sp[0]->s_vec, sp[1]->s_vec,
    sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
  else
  {
    dsp_add(vcf_filter_tilde_perform_snafu, 6, sp[0]->s_vec, sp[1]->s_vec,
      sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
    post("vcf_filter~-Error: 1. initial-arguments: <sym> kind: lp2, bp2, rbp2, hp2!");
  }
}

static void *vcf_filter_tilde_new(t_symbol *filt_typ)
{
  t_vcf_filter_tilde *x = (t_vcf_filter_tilde *)pd_new(vcf_filter_tilde_class);
  char *c;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  x->x_wn1 = 0.0f;
  x->x_wn2 = 0.0f;
  c = (char *)filt_typ->s_name;
  c[5] = 0;
  strcpy(x->x_filtname, c);
  return(x);
}

void vcf_filter_tilde_setup(void)
{
  vcf_filter_tilde_class = class_new(gensym("vcf_filter~"), (t_newmethod)vcf_filter_tilde_new,
    0, sizeof(t_vcf_filter_tilde), 0, A_SYMBOL, 0);
  CLASS_MAINSIGNALIN(vcf_filter_tilde_class, t_vcf_filter_tilde, x_msi);
  class_addmethod(vcf_filter_tilde_class, (t_method)vcf_filter_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(vcf_filter_tilde_class, gensym("iemhelp/help-vcf_filter~"));
}
