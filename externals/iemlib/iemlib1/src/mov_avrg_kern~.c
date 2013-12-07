/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* -- mov_avrg_kern~ - kernel for a moving-average-filter with IIR - */

typedef struct _mov_avrg_kern_tilde
{
  t_object x_obj;
  double   x_wn1;
  double   x_a0;
  double   x_sr;
  double   x_mstime;
  int      x_nsamps;
  int      x_counter;
  t_float  x_msi;
} t_mov_avrg_kern_tilde;

static t_class *mov_avrg_kern_tilde_class;

static t_int *mov_avrg_kern_tilde_perform(t_int *w)
{
  t_float *in_direct = (t_float *)(w[1]);
  t_float *in_delayed = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  t_mov_avrg_kern_tilde *x = (t_mov_avrg_kern_tilde *)(w[4]);
  int i, n = (int)(w[5]);
  double wn0, wn1=x->x_wn1, a0=x->x_a0;
  
  if(x->x_counter)
  {
    int counter = x->x_counter;
    
    if(counter >= n)
    {
      x->x_counter = counter - n;
      for(i=0; i<n; i++)
      {
        wn0 = wn1 + a0*(double)(*in_direct++);
        *out++ = (t_float)wn0;
        wn1 = wn0;
      }
    }
    else
    {
      x->x_counter = 0;
      for(i=0; i<counter; i++)
      {
        wn0 = wn1 + a0*(double)(*in_direct++);
        *out++ = (t_float)wn0;
        wn1 = wn0;
      }
      for(i=counter; i<n; i++)
      {
        wn0 = wn1 + a0*(double)(*in_direct++ - *in_delayed++);
        *out++ = (t_float)wn0;
        wn1 = wn0;
      }
    }
  }
  else
  {
    for(i=0; i<n; i++)
    {
      wn0 = wn1 + a0*(double)(*in_direct++ - *in_delayed++);
      *out++ = (t_float)wn0;
      wn1 = wn0;
    }
  }
  x->x_wn1 = wn1;
  return(w+6);
}

static void mov_avrg_kern_tilde_ft1(t_mov_avrg_kern_tilde *x, t_floatarg mstime)
{
  if(mstime < 0.04)
    mstime = 0.04;
  x->x_mstime = (double)mstime;
  x->x_nsamps = (int)(x->x_sr * x->x_mstime);
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
  x->x_a0 = 1.0/(double)(x->x_nsamps);
}

static void mov_avrg_kern_tilde_reset(t_mov_avrg_kern_tilde *x)
{
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
}

static void mov_avrg_kern_tilde_dsp(t_mov_avrg_kern_tilde *x, t_signal **sp)
{
  x->x_sr = 0.001*(double)(sp[0]->s_sr);
  x->x_nsamps = (int)(x->x_sr * x->x_mstime);
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
  x->x_a0 = 1.0/(double)(x->x_nsamps);
  dsp_add(mov_avrg_kern_tilde_perform, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
}

static void *mov_avrg_kern_tilde_new(t_floatarg mstime)
{
  t_mov_avrg_kern_tilde *x = (t_mov_avrg_kern_tilde *)pd_new(mov_avrg_kern_tilde_class);
  
  if(mstime < 0.04)
    mstime = 0.04;
  x->x_mstime = (double)mstime;
  x->x_sr = 44.1;
  x->x_nsamps = (int)(x->x_sr * x->x_mstime);
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
  x->x_a0 = 1.0/(double)(x->x_nsamps);
  
  inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  return(x);
}

void mov_avrg_kern_tilde_setup(void)
{
  mov_avrg_kern_tilde_class = class_new(gensym("mov_avrg_kern~"), (t_newmethod)mov_avrg_kern_tilde_new,
        0, sizeof(t_mov_avrg_kern_tilde), 0, A_FLOAT, 0);
  CLASS_MAINSIGNALIN(mov_avrg_kern_tilde_class, t_mov_avrg_kern_tilde, x_msi);
  class_addmethod(mov_avrg_kern_tilde_class, (t_method)mov_avrg_kern_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(mov_avrg_kern_tilde_class, (t_method)mov_avrg_kern_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(mov_avrg_kern_tilde_class, (t_method)mov_avrg_kern_tilde_reset, gensym("reset"), 0);
}
