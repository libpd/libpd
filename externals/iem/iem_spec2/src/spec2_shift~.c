/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* -- spec2_shift~ - shift spectral bins to left (lower, negative) or to right (higher, positiv) --- */

typedef struct spec2_shift_tilde
{
  t_object  x_obj;
  int       x_blocksize;
  t_float   x_add;
  t_float   *x_spec;
  t_float   x_msi;
} t_spec2_shift_tilde;

t_class *spec2_shift_tilde_class;

static t_int *spec2_shift_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_shift_tilde *x = (t_spec2_shift_tilde *)(w[3]);
  int i, j, n = (t_int)(w[4])+1;
  t_float *spec=x->x_spec;
  t_float add=x->x_add;
  
  if((add >= n) || (add <= -n))
  {
    for(i=0; i<n; i++)/* clear residal of spec-buffer */
      out[i] = 0.0f;
  }
  else
  {
    for(i=0; i<n; i++)/* copy spec into buffer */
      spec[i] = in[i];
    
    if(add >= 0)
    {
      for(i=0; i<add; i++)/* clear residal of spec-buffer */
        out[i] = 0.0f;
      for(j=0; i<n; i++, j++)/* copy spec into buffer */
        out[i] = spec[j];
    }
    else
    {
      add *= -1;
      for(i=0, j=add; j<n; i++, j++)/* copy spec into buffer */
        out[i] = spec[j];
      for(; i<n; i++)/* clear residal of spec-buffer */
        out[i] = 0.0f;
    }
  }
  return(w+5);
}

static void spec2_shift_tilde_add(t_spec2_shift_tilde *x, t_floatarg add)
{
  x->x_add = add;
}

static void spec2_shift_tilde_dsp(t_spec2_shift_tilde *x, t_signal **sp)
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
  dsp_add(spec2_shift_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *spec2_shift_tilde_new(t_floatarg add)
{
  t_spec2_shift_tilde *x = (t_spec2_shift_tilde *)pd_new(spec2_shift_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_blocksize = 0;
  x->x_add = add;
  x->x_spec = (t_float *)0;
  return (x);
}

static void spec2_shift_tilde_free(t_spec2_shift_tilde *x)
{
  if(x->x_spec)
    freebytes(x->x_spec, (x->x_blocksize+1) * sizeof(t_float));
}

void spec2_shift_tilde_setup(void)
{
  spec2_shift_tilde_class = class_new(gensym("spec2_shift~"), (t_newmethod)spec2_shift_tilde_new,
    0, sizeof(t_spec2_shift_tilde), 0, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(spec2_shift_tilde_class, t_spec2_shift_tilde, x_msi);
  class_addmethod(spec2_shift_tilde_class, (t_method)spec2_shift_tilde_dsp, gensym("dsp"), 0);
  class_addfloat(spec2_shift_tilde_class, (t_method)spec2_shift_tilde_add);
//  class_sethelpsymbol(spec2_shift_tilde_class, gensym("iemhelp/spec2_shift~-help"));
}
