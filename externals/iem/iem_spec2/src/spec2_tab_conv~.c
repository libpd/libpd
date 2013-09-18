/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"

/* -- spec2_tab_conv~ - convolute a spectrum with a table --- */

typedef struct spec2_tab_conv_tilde
{
  t_object  x_obj;
  t_float   *x_spec;
  iemarray_t   *x_beg_array;
  int       x_blocksize;
  int       x_winsize;
  int       x_has_changed;
  t_symbol  *x_sym_array;
  t_float   x_msi;
} t_spec2_tab_conv_tilde;

t_class *spec2_tab_conv_tilde_class;

static void spec2_tab_conv_tilde_set(t_spec2_tab_conv_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 2) && IS_A_SYMBOL(argv, 0) && IS_A_FLOAT(argv, 1))
  {
    x->x_sym_array = (t_symbol *)(atom_getsymbol(argv));
    x->x_winsize = (int)(atom_getint(argv+1));
    x->x_has_changed = 1;
  }
}

static t_int *spec2_tab_conv_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_tab_conv_tilde *x = (t_spec2_tab_conv_tilde *)(w[3]);
  t_float sum=0.0f;
  t_float *vec1, *vec2, *vec3;
  iemarray_t*win;
  int i, m, n = (int)(w[4])+1;
  int j, ws=x->x_winsize;
  
  vec2 = x->x_spec + n;
  vec1 = vec2;
  vec3 = vec2 + 2*n - 2;
  
  for(i=0; i<n; i++)
  {
    sum = in[i];
    *vec2++ = sum;
    *vec1-- = sum;
    *vec3-- = sum;
  }
  vec2 = x->x_spec + n - ws/2;
  win = x->x_beg_array;
  
  for(i=0; i<n; i++)
  {
    sum = 0.0f;
    for(j=0; j<ws; j++)
      sum += iemarray_getfloat(win, j) * vec2[j];
    out[i] = sum;
    vec2++;
  }
  return(w+5);
}

static void spec2_tab_conv_tilde_dsp(t_spec2_tab_conv_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  t_garray *a;
  int n_points;
  
  if(x->x_has_changed)
  {
    x->x_has_changed = 0;
    if(!(a = (t_garray *)pd_findbyclass(x->x_sym_array, garray_class)))
    {
      if(*x->x_sym_array->s_name)
        error("spec2_tab_conv~: %s: no such array", x->x_sym_array->s_name);
    }
    else if(!iemarray_getarray(a, &n_points, &x->x_beg_array))
      error("%s: bad template for spec2_tab_conv~", x->x_sym_array->s_name);
    else 
    {
      if(n_points > (n+1))
        n_points = n+1;
      if(x->x_winsize < 0)
        x->x_winsize = 0;
      if(x->x_winsize > n_points)
        x->x_winsize = n_points;
    }
  }
  
  if(!x->x_blocksize)
  {
    x->x_spec = (t_float *)getbytes(3*(n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  else if(x->x_blocksize != n)
  {
    x->x_spec = (t_float *)resizebytes(x->x_spec, 3*(x->x_blocksize+1)*sizeof(t_float), 3*(n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  
  dsp_add(spec2_tab_conv_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void spec2_tab_conv_tilde_free(t_spec2_tab_conv_tilde *x)
{
  if(x->x_spec)
    freebytes(x->x_spec, 3*(x->x_blocksize+1)*sizeof(t_float));
}

static void *spec2_tab_conv_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_spec2_tab_conv_tilde *x = (t_spec2_tab_conv_tilde *)pd_new(spec2_tab_conv_tilde_class);
  
  if((argc >= 2) && IS_A_SYMBOL(argv,0) && IS_A_FLOAT(argv,1))
  {
    x->x_sym_array = (t_symbol *)(atom_getsymbol(argv));
    x->x_winsize = (int)(atom_getint(argv+1));
    x->x_spec = (t_float *)0;
    x->x_beg_array = (iemarray_t *)0;
    x->x_blocksize = 0;
    x->x_has_changed = 1;
    outlet_new(&x->x_obj, &s_signal);
    x->x_msi = 0.0f;
    return(x);
  }
  else
  {
    post("spec2_tab_conv~-ERROR: needs 2 args: <sym> convolution-array-name + <float> convolution-array-size !!!");
    return(0);
  }
}

void spec2_tab_conv_tilde_setup(void)
{
  spec2_tab_conv_tilde_class = class_new(gensym("spec2_tab_conv~"), (t_newmethod)spec2_tab_conv_tilde_new,
    (t_method)spec2_tab_conv_tilde_free, sizeof(t_spec2_tab_conv_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(spec2_tab_conv_tilde_class, t_spec2_tab_conv_tilde, x_msi);
  class_addmethod(spec2_tab_conv_tilde_class, (t_method)spec2_tab_conv_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(spec2_tab_conv_tilde_class, (t_method)spec2_tab_conv_tilde_set, gensym("set"), A_GIMME, 0);
//  class_sethelpsymbol(spec2_tab_conv_tilde_class, gensym("iemhelp/spec2_tab_conv~-help"));
}
