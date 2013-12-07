/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- t3_line~ ------------------------------ */
static t_class *t3_line_tilde_class;

typedef struct _t3_line_tilde
{
  t_object x_obj;
  t_clock  *x_clock;
  t_float  *x_beg;
  double   x_cur_val;
  double   x_dst_val;
  double   x_inlet_val;
  double   x_inc64;
  double   x_inc;
  double   x_ms2samps;
  double   x_ticks2ms;
  double   x_inlet_time;
  double   x_dst_time;
  int      x_cur_samps;
  int      x_dur_samps;
  int      x_n;
  int      x_t3_bang_samps;
  int      x_transient;
} t_t3_line_tilde;

static void t3_line_tilde_nontransient(t_float *vec, t_t3_line_tilde *x, int n)
{
  int cur_samps = x->x_cur_samps, i;
  double inc = x->x_inc;
  double cur_val = x->x_cur_val;
  
  if(cur_samps)
  {
    if(cur_samps > n)
    {
      x->x_cur_samps -= n;
      while(n--)
      {
        cur_val += inc;
        *vec++ = (t_float)cur_val;
      }
      x->x_cur_val += x->x_inc64;
    }
    else if(cur_samps == n)
    {
      x->x_cur_samps = 0;
      while(n--)
      {
        cur_val += inc;
        *vec++ = (t_float)cur_val;
      }
      x->x_cur_val = x->x_dst_val;
    }
    else
    {
      for(i=0; i<cur_samps; i++)
      {
        cur_val += inc;
        *vec++ = (t_float)cur_val;
      }
      x->x_cur_val = cur_val = x->x_dst_val;
      for(i=cur_samps; i<n; i++)
        *vec++ = (t_float)cur_val;
      x->x_cur_samps = 0;
    }
  }
  else
  {
    while(n--)
      *vec++ = (t_float)cur_val;
  }
}

static t_int *t3_line_tilde_perform(t_int *w)
{
  t_float *out = (t_float *)(w[1]);
  t_t3_line_tilde *x = (t_t3_line_tilde *)(w[2]);
  int n = (int)(w[3]);
  
  if(x->x_transient)
  {
    t_float *trans = x->x_beg;
    
    while(n--)
      *out++ = *trans++;
    x->x_transient = 0;
  }
  else
    t3_line_tilde_nontransient(out, x, n);
  return(w+4);
}

static void t3_line_tilde_tick(t_t3_line_tilde *x)
{
  t_float *trans = x->x_beg;
  int n = x->x_n, t3_bang_samps, cur_samps, i;
  double inc, cur_val;
  
  if(!x->x_transient)
    t3_line_tilde_nontransient(trans, x, n);
  t3_bang_samps = x->x_t3_bang_samps;
  x->x_dst_val = x->x_inlet_val;
  if(x->x_inlet_time <= 0.0)
  {
    x->x_inlet_time = 0.0;
    x->x_dst_time = 0.0;
    x->x_dur_samps = 0;
    x->x_cur_samps = 0;
    cur_val = x->x_cur_val = x->x_dst_val;
    for(i=t3_bang_samps; i<n; i++)
      trans[i] = (t_float)cur_val;
  }
  else
  {
    int diff, end;
    
    x->x_dst_time = x->x_inlet_time;
    x->x_inlet_time = 0.0;
    cur_samps = (int)(x->x_dst_time * x->x_ms2samps);
    if(!cur_samps)
      cur_samps = 1;
    x->x_dur_samps = cur_samps;
    x->x_cur_samps = cur_samps;
    cur_val = x->x_cur_val = (double)trans[t3_bang_samps];
    inc = x->x_inc = (x->x_dst_val - cur_val)/(double)cur_samps;
    x->x_inc64 = (double)x->x_n * inc;
    diff = n - t3_bang_samps;
    if(cur_samps > diff)
    {
      for(i=t3_bang_samps; i<n; i++)
      {
        cur_val += inc;
        trans[i] = (t_float)cur_val;
      }
      x->x_cur_val += (double)diff * inc;
      x->x_cur_samps -= diff;
    }
    else if(cur_samps == diff)
    {
      for(i=t3_bang_samps; i<n; i++)
      {
        cur_val += inc;
        trans[i] = (t_float)cur_val;
      }
      x->x_cur_val = x->x_dst_val;
      x->x_cur_samps = 0;
    }
    else
    {
      end = t3_bang_samps + cur_samps;
      for(i=t3_bang_samps; i<end; i++)
      {
        cur_val += inc;
        trans[i] = (t_float)cur_val;
      }
      cur_val = x->x_cur_val = x->x_dst_val;
      x->x_cur_samps = 0;
      for(i=end; i<n; i++)
        trans[i] = (t_float)cur_val;
    }
  }
  x->x_transient = 1;
}

static void t3_line_tilde_list(t_t3_line_tilde *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac >= 2)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1))
  {
    int t3_bang_samps, ticks;
    double dtime;
    
    x->x_inlet_val = (double)atom_getfloatarg(1, ac, av);
    t3_bang_samps = (int)((t_float)atom_getfloatarg(0, ac, av)*x->x_ms2samps);
    if(t3_bang_samps < 0)
      t3_bang_samps = 0;
    ticks = t3_bang_samps / x->x_n;
    x->x_t3_bang_samps = t3_bang_samps - x->x_n * ticks;
    if((ac >= 3)&&IS_A_FLOAT(av,2))
    {
      dtime = (double)atom_getfloatarg(2, ac, av);
      if(dtime < 0.0)
        dtime = 0.0;
      x->x_inlet_time = dtime;
    }
    if(ticks < 1)
      t3_line_tilde_tick(x);
    else
      clock_delay(x->x_clock, (double)ticks * x->x_ticks2ms);
  }
}

static void t3_line_tilde_ft1(t_t3_line_tilde *x, t_float ftime)
{
  if(ftime < 0.0)
    ftime = 0.0;
  x->x_inlet_time = (double)ftime;
}

static void t3_line_tilde_stop(t_t3_line_tilde *x)
{
  clock_unset(x->x_clock);
  x->x_cur_samps = x->x_dur_samps = x->x_transient = 0;
  x->x_inc = x->x_inc64 = x->x_inlet_time = x->x_dst_time = 0.0;
}

static void t3_line_tilde_dsp(t_t3_line_tilde *x, t_signal **sp)
{
  int i;
  t_float val, *trans;
  
  if(sp[0]->s_n > x->x_n)
  {
    freebytes(x->x_beg, x->x_n*sizeof(t_float));
    x->x_n = (int)sp[0]->s_n;
    x->x_beg = (t_float *)getbytes(x->x_n*sizeof(t_float));
  }
  else
    x->x_n = (int)sp[0]->s_n;
  i = x->x_n;
  val = x->x_cur_val;
  trans = x->x_beg;
  while(i--)
    *trans++ = val;
  x->x_ms2samps = 0.001*(double)sp[0]->s_sr;
  x->x_ticks2ms = (double)x->x_n / x->x_ms2samps;
  dsp_add(t3_line_tilde_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void t3_line_tilde_free(t_t3_line_tilde *x)
{
  if(x->x_beg)
    freebytes(x->x_beg, x->x_n*sizeof(t_float));
  clock_free(x->x_clock);
}

static void *t3_line_tilde_new(t_floatarg init_val)
{
  t_t3_line_tilde *x = (t_t3_line_tilde *)pd_new(t3_line_tilde_class);
  int i;
  
  x->x_n = (int)sys_getblksize();
  x->x_beg = (t_float *)getbytes(x->x_n*sizeof(t_float));
  x->x_inlet_val = x->x_cur_val = x->x_dst_val = init_val;
  x->x_t3_bang_samps = x->x_cur_samps = x->x_dur_samps = x->x_transient = 0;
  x->x_inlet_time = x->x_dst_time = 0.0;
  x->x_inc64 = x->x_inc = 0.0;
  x->x_ms2samps = 0.001 * (double)sys_getsr();
  x->x_ticks2ms = (double)x->x_n / x->x_ms2samps;
  x->x_clock = clock_new(x, (t_method)t3_line_tilde_tick);
  outlet_new(&x->x_obj, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
  return (x);
}

void t3_line_tilde_setup(void)
{
  t3_line_tilde_class = class_new(gensym("t3_line~"), (t_newmethod)t3_line_tilde_new,
    (t_method)t3_line_tilde_free, sizeof(t_t3_line_tilde), 0, A_DEFFLOAT, 0);
  class_addmethod(t3_line_tilde_class, (t_method)t3_line_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(t3_line_tilde_class, (t_method)t3_line_tilde_stop, gensym("stop"), 0);
  class_addmethod(t3_line_tilde_class, (t_method)t3_line_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addlist(t3_line_tilde_class, (t_method)t3_line_tilde_list);
//  class_sethelpsymbol(t3_line_tilde_class, gensym("iemhelp/help-t3_line~"));
}
