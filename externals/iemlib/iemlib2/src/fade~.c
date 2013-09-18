/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ------------------------- fade~ ----------------------------- */
/* --- signal lookup tabel object with input range of 0 to 1 --- */
/* ---- converts a linear signal ramp to the half of a :  ------ */
/* -- sine-wave, hanning-wave, squareroot-wave or mixes of it -- */

t_float *iem_fade_tilde_table_lin=(t_float *)0L;
t_float *iem_fade_tilde_table_linsqrt=(t_float *)0L;
t_float *iem_fade_tilde_table_sqrt=(t_float *)0L;
t_float *iem_fade_tilde_table_sin=(t_float *)0L;
t_float *iem_fade_tilde_table_sinhann=(t_float *)0L;
t_float *iem_fade_tilde_table_hann=(t_float *)0L;

static t_class *fade_tilde_class;

typedef struct _fade_tilde
{
  t_object x_obj;
  t_float *x_table;
  t_float x_f;
} t_fade_tilde;

static void fade_tilde_set(t_fade_tilde *x, t_symbol *s)
{
  if(s == gensym("_lin"))
    x->x_table = iem_fade_tilde_table_lin;
  else if(s == gensym("_linsqrt"))
    x->x_table = iem_fade_tilde_table_linsqrt;
  else if(s == gensym("_sqrt"))
    x->x_table = iem_fade_tilde_table_sqrt;
  else if(s == gensym("_sin"))
    x->x_table = iem_fade_tilde_table_sin;
  else if(s == gensym("_sinhann"))
    x->x_table = iem_fade_tilde_table_sinhann;
  else if(s == gensym("_hann"))
    x->x_table = iem_fade_tilde_table_hann;
}

static void *fade_tilde_new(t_symbol *s)
{
  t_fade_tilde *x = (t_fade_tilde *)pd_new(fade_tilde_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  x->x_table = iem_fade_tilde_table_lin;
  fade_tilde_set(x, s);
  return (x);
}

static t_int *fade_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_fade_tilde *x = (t_fade_tilde *)(w[3]);
  int n = (int)(w[4]);
  t_float *tab = x->x_table, *addr, f1, f2, frac;
  double dphase;
  int normhipart;
  union tabfudge_d tf;
  
  tf.tf_d = UNITBIT32;
  normhipart = tf.tf_i[HIOFFSET];
  
#if 0     /* this is the readable version of the code. */
  while (n--)
  {
    dphase = (double)(*in++ * (t_float)(COSTABSIZE) * 0.99999) + UNITBIT32;
    tf.tf_d = dphase;
    addr = tab + (tf.tf_i[HIOFFSET] & (COSTABSIZE-1));
    tf.tf_i[HIOFFSET] = normhipart;
    frac = tf.tf_d - UNITBIT32;
    f1 = addr[0];
    f2 = addr[1];
    *out++ = f1 + frac * (f2 - f1);
  }
#endif
#if 1     /* this is the same, unwrapped by hand. */
  dphase = (double)(*in++ * (t_float)(COSTABSIZE) * 0.99999) + UNITBIT32;
  tf.tf_d = dphase;
  addr = tab + (tf.tf_i[HIOFFSET] & (COSTABSIZE-1));
  tf.tf_i[HIOFFSET] = normhipart;
  while (--n)
  {
    dphase = (double)(*in++ * (t_float)(COSTABSIZE) * 0.99999) + UNITBIT32;
    frac = tf.tf_d - UNITBIT32;
    tf.tf_d = dphase;
    f1 = addr[0];
    f2 = addr[1];
    addr = tab + (tf.tf_i[HIOFFSET] & (COSTABSIZE-1));
    *out++ = f1 + frac * (f2 - f1);
    tf.tf_i[HIOFFSET] = normhipart;
  }
  frac = tf.tf_d - UNITBIT32;
  f1 = addr[0];
  f2 = addr[1];
  *out++ = f1 + frac * (f2 - f1);
#endif
  return (w+5);
}

static void fade_tilde_dsp(t_fade_tilde *x, t_signal **sp)
{
  dsp_add(fade_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void fade_tilde_maketable(void)
{
  int i;
  t_float *fp, phase, fff,phsinc = 0.5*3.141592653 / ((t_float)COSTABSIZE*0.99999);
  union tabfudge_d tf;
  
  if(!iem_fade_tilde_table_sin)
  {
    iem_fade_tilde_table_sin = (t_float *)getbytes(sizeof(t_float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_fade_tilde_table_sin, phase=0; i--; fp++, phase+=phsinc)
      *fp = sin(phase);
  }
  if(!iem_fade_tilde_table_sinhann)
  {
    iem_fade_tilde_table_sinhann = (t_float *)getbytes(sizeof(t_float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_fade_tilde_table_sinhann, phase=0; i--; fp++, phase+=phsinc)
    {
      fff = sin(phase);
      *fp = fff*sqrt(fff);
    }
  }
  if(!iem_fade_tilde_table_hann)
  {
    iem_fade_tilde_table_hann = (t_float *)getbytes(sizeof(t_float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_fade_tilde_table_hann, phase=0; i--; fp++, phase+=phsinc)
    {
      fff = sin(phase);
      *fp = fff*fff;
    }
  }
  phsinc = 1.0 / ((t_float)COSTABSIZE*0.99999);
  if(!iem_fade_tilde_table_lin)
  {
    iem_fade_tilde_table_lin = (t_float *)getbytes(sizeof(t_float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_fade_tilde_table_lin, phase=0; i--; fp++, phase+=phsinc)
      *fp = phase;
  }
  if(!iem_fade_tilde_table_linsqrt)
  {
    iem_fade_tilde_table_linsqrt = (t_float *)getbytes(sizeof(t_float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_fade_tilde_table_linsqrt, phase=0; i--; fp++, phase+=phsinc)
      *fp = pow(phase, 0.75);
  }
  if(!iem_fade_tilde_table_sqrt)
  {
    iem_fade_tilde_table_sqrt = (t_float *)getbytes(sizeof(t_float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_fade_tilde_table_sqrt, phase=0; i--; fp++, phase+=phsinc)
      *fp = sqrt(phase);
  }
  tf.tf_d = UNITBIT32 + 0.5;
  if((unsigned)tf.tf_i[LOWOFFSET] != 0x80000000)
    bug("fade~: unexpected machine alignment");
}

void fade_tilde_setup(void)
{
  fade_tilde_class = class_new(gensym("fade~"), (t_newmethod)fade_tilde_new, 0,
    sizeof(t_fade_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(fade_tilde_class, t_fade_tilde, x_f);
  class_addmethod(fade_tilde_class, (t_method)fade_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(fade_tilde_class, (t_method)fade_tilde_set, gensym("set"), A_DEFSYM, 0);
//  class_sethelpsymbol(fade_tilde_class, gensym("iemhelp/help-fade~"));
  fade_tilde_maketable();
}
