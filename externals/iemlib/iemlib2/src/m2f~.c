/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ----------- m2f~ ----------- */
/* --------- obsolete --------- */

#define M2FTILDETABSIZE 2048

t_float *iem_m2f_tilde_table=(t_float *)0L;

static t_class *m2f_tilde_class;

typedef struct _m2f
{
  t_object x_obj;
  t_float x_msi;
} t_m2f_tilde;

static void *m2f_tilde_new(void)
{
  t_m2f_tilde *x = (t_m2f_tilde *)pd_new(m2f_tilde_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_msi = 0;
  return (x);
}

static t_int *m2f_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_m2f_tilde *x = (t_m2f_tilde *)(w[3]);
  int n = (int)(w[4]);
  t_float *tab = iem_m2f_tilde_table, *addr, f1, f2, frac, iinn;
  double dphase;
  int normhipart;
  union tabfudge_d tf;
  
  tf.tf_d = UNITBIT32;
  normhipart = tf.tf_i[HIOFFSET];
  
#if 0       /* this is the readable version of the code. */
  while (n--)
  {
    iinn = (*in++)*10.0+670.0;
    dphase = (double)iinn + UNITBIT32;
    tf.tf_d = dphase;
    addr = tab + (tf.tf_i[HIOFFSET] & (M2FTILDETABSIZE-1));
    tf.tf_i[HIOFFSET] = normhipart;
    frac = tf.tf_d - UNITBIT32;
    f1 = addr[0];
    f2 = addr[1];
    *out++ = f1 + frac * (f2 - f1);
  }
#endif
#if 1       /* this is the same, unwrapped by hand. */
  iinn = (*in++)*10.0+670.0;
  dphase = (double)iinn + UNITBIT32;
  tf.tf_d = dphase;
  addr = tab + (tf.tf_i[HIOFFSET] & (M2FTILDETABSIZE-1));
  tf.tf_i[HIOFFSET] = normhipart;
  while (--n)
  {
    iinn = (*in++)*10.0+670.0;
    dphase = (double)iinn + UNITBIT32;
    frac = tf.tf_d - UNITBIT32;
    tf.tf_d = dphase;
    f1 = addr[0];
    f2 = addr[1];
    addr = tab + (tf.tf_i[HIOFFSET] & (M2FTILDETABSIZE-1));
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

static void m2f_tilde_dsp(t_m2f_tilde *x, t_signal **sp)
{
  dsp_add(m2f_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void m2f_tilde_maketable(void)
{
  union tabfudge_d tf;
  
  if(!iem_m2f_tilde_table)
  {
    int i;
    t_float *fp, midi, refexp=440.0*exp(-5.75*log(2.0));
    
    iem_m2f_tilde_table = (t_float *)getbytes(sizeof(t_float) * (M2FTILDETABSIZE+1));
    for(i=0, fp=iem_m2f_tilde_table, midi=-67.0; i<=M2FTILDETABSIZE; i++, fp++, midi+=0.1)
      *fp = refexp * exp(0.057762265047 * midi);
  }
  tf.tf_d = UNITBIT32 + 0.5;
  if((unsigned)tf.tf_i[LOWOFFSET] != 0x80000000)
    bug("m2f~: unexpected machine alignment");
}

void m2f_tilde_setup(void)
{
  m2f_tilde_class = class_new(gensym("m2f~"), (t_newmethod)m2f_tilde_new, 0,
    sizeof(t_m2f_tilde), 0, 0);
  CLASS_MAINSIGNALIN(m2f_tilde_class, t_m2f_tilde, x_msi);
  class_addmethod(m2f_tilde_class, (t_method)m2f_tilde_dsp, gensym("dsp"), 0);
  m2f_tilde_maketable();
//  class_sethelpsymbol(m2f_tilde_class, gensym("iemhelp/help-m2f~"));
}
