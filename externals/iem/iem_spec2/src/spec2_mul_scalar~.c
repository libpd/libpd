/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- spec2_mul_scalar~ ------------------------------ */
static t_class *spec2_mul_scalar_tilde_class;

typedef struct _spec2_mul_scalar_tilde
{
  t_object  x_obj;
  t_float   x_f;
  t_float   x_msi;
} t_spec2_mul_scalar_tilde;

static void spec2_mul_scalar_tilde_ft1(t_spec2_mul_scalar_tilde *x, t_floatarg f)
{
  x->x_f = f;
}

static t_int *spec2_mul_scalar_tilde_perform(t_int *w)
{
  t_float *io = (t_float *)(w[1]);
  t_spec2_mul_scalar_tilde *x = (t_spec2_mul_scalar_tilde *)(w[2]);
  int i, n = (t_int)(w[3]);
  t_float f = x->x_f;
  
  for(i=0; i<=n; i++)
  {
    io[i] *= f;
  }
  return(w+4);
}

static t_int *spec2_mul_scalar_tilde_perf16(t_int *w)
{
  t_float *io = (t_float *)(w[1]);
  t_spec2_mul_scalar_tilde *x = (t_spec2_mul_scalar_tilde *)(w[2]);
  int n = (t_int)(w[3]);
  t_float f = x->x_f;
  
  while(n)
  {
    io[0] *= f;
    io[1] *= f;
    io[2] *= f;
    io[3] *= f;
    io[4] *= f;
    io[5] *= f;
    io[6] *= f;
    io[7] *= f;
    io[8] *= f;
    io[9] *= f;
    io[10] *= f;
    io[11] *= f;
    io[12] *= f;
    io[13] *= f;
    io[14] *= f;
    io[15] *= f;
    
    io += 16;
    n -= 16;
  }
  io[0] *= f;
  return(w+4);
}

static void spec2_mul_scalar_tilde_dsp(t_spec2_mul_scalar_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(n&15)
    dsp_add(spec2_mul_scalar_tilde_perform, 3, sp[0]->s_vec, x, n);
  else
    dsp_add(spec2_mul_scalar_tilde_perf16, 3, sp[0]->s_vec, x, n);
}

static void *spec2_mul_scalar_tilde_new(t_floatarg f)
{
  t_spec2_mul_scalar_tilde *x = (t_spec2_mul_scalar_tilde *)pd_new(spec2_mul_scalar_tilde_class);
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_f = f;
  x->x_msi = 0.0f;
  return (x);
}

static void spec2_mul_scalar_tilde_free(t_spec2_mul_scalar_tilde *x)
{
}

void spec2_mul_scalar_tilde_setup(void)
{
  spec2_mul_scalar_tilde_class = class_new(gensym("spec2_mul_scalar~"), (t_newmethod)spec2_mul_scalar_tilde_new, (t_method)spec2_mul_scalar_tilde_free,
    sizeof(t_spec2_mul_scalar_tilde), 0, A_DEFFLOAT, 0);
  class_addcreator((t_newmethod)spec2_mul_scalar_tilde_new, gensym("spec2*s~"), A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(spec2_mul_scalar_tilde_class, t_spec2_mul_scalar_tilde, x_msi);
  class_addmethod(spec2_mul_scalar_tilde_class, (t_method)spec2_mul_scalar_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(spec2_mul_scalar_tilde_class, (t_method)spec2_mul_scalar_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_mul_scalar_tilde_class, gensym("iemhelp2/spec2_mul_scalar~-help"));
}
