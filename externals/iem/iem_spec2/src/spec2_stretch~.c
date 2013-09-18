/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* -- spec2_stretch~ - stretch spectral bins --- */

typedef struct spec2_stretch_tilde
{
  t_object  x_obj;
  int       x_blocksize;
  t_float   x_mul;
  t_float   *x_spec;
  t_float   x_msi;
} t_spec2_stretch_tilde;

t_class *spec2_stretch_tilde_class;

static t_int *spec2_stretch_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_stretch_tilde *x = (t_spec2_stretch_tilde *)(w[3]);
  int i, j, m, n = (t_int)(w[4])+1;
  t_float yn0, yn1, fract;
  t_float *spec=x->x_spec;
  t_float mul=x->x_mul;
  t_float rcp_mul = 1.0f / mul;
  
  for(i=0; i<n; i++)/* copy spec into buffer */
    spec[i] = in[i];
  
  m = (int)((t_float)n * mul);
  if(m > n)
    m = n;
  for(i=0; i<m; i++)/* stretch spec-buffer */
  {
    fract = (t_float)i * rcp_mul;
    j = (int)fract;
    fract -= (t_float)j;
    yn0 = spec[j];
    yn1 = spec[j+1];
    out[i] = (yn1 - yn0)*fract + yn0;
  }
  for(i=m; i<n; i++)/* clear residal of spec-buffer */
    out[i] = 0.0f;
  
  return(w+5);
}

static void spec2_stretch_tilde_mul(t_spec2_stretch_tilde *x, t_floatarg mul)
{
  if(mul <= 0.0f)
    mul = 1.0f;
  x->x_mul = mul;
}

static void spec2_stretch_tilde_dsp(t_spec2_stretch_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(!x->x_blocksize)
  {
    x->x_spec = (t_float *)getbytes((n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  else if(x->x_blocksize != n)
  {
    x->x_spec = (t_float *)resizebytes(x->x_spec, (x->x_blocksize+1)*sizeof(t_float), (n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  dsp_add(spec2_stretch_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *spec2_stretch_tilde_new(t_floatarg mul)
{
  t_spec2_stretch_tilde *x = (t_spec2_stretch_tilde *)pd_new(spec2_stretch_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  if(mul <= 0.0f)
    mul = 1.0f;
  x->x_blocksize = 0;
  x->x_mul = mul;
  x->x_spec = (t_float *)0;
  return (x);
}

static void spec2_stretch_tilde_free(t_spec2_stretch_tilde *x)
{
  if(x->x_spec)
    freebytes(x->x_spec, (x->x_blocksize+1) * sizeof(t_float));
}

void spec2_stretch_tilde_setup(void)
{
  spec2_stretch_tilde_class = class_new(gensym("spec2_stretch~"), (t_newmethod)spec2_stretch_tilde_new,
    0, sizeof(t_spec2_stretch_tilde), 0, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(spec2_stretch_tilde_class, t_spec2_stretch_tilde, x_msi);
  class_addmethod(spec2_stretch_tilde_class, (t_method)spec2_stretch_tilde_dsp, gensym("dsp"), 0);
  class_addfloat(spec2_stretch_tilde_class, (t_method)spec2_stretch_tilde_mul);
//  class_sethelpsymbol(spec2_stretch_tilde_class, gensym("iemhelp/spec2_stretch~-help"));
}
