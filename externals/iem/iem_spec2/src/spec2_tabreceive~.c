/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ spec2_tabreceive~ ------------------------- */

static t_class *spec2_tabreceive_tilde_class;

typedef struct _spec2_tabreceive_tilde
{
  t_object  x_obj;
  iemarray_t *x_vec;
  t_symbol  *x_arrayname;
} t_spec2_tabreceive_tilde;

static void spec2_tabreceive_tilde_symbol(t_spec2_tabreceive_tilde *x, t_symbol *s)
{
  x->x_arrayname = s;
}

static t_int *spec2_tabreceive_tilde_perform(t_int *w)
{
  t_spec2_tabreceive_tilde *x = (t_spec2_tabreceive_tilde *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3]+1, i;
  iemarray_t *vec = x->x_vec;
  
  if(vec)
  {
    for(i=0; i<n; i++)
      out[i] = iemarray_getfloat(vec, i);
  }
  else
  {
    while(n--)
      *out++ = 0.0f;
  }
  return(w+4);
}

static t_int *spec2_tabreceive_tilde_perf16(t_int *w)
{
  t_spec2_tabreceive_tilde *x = (t_spec2_tabreceive_tilde *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3];
  iemarray_t *vec = x->x_vec;
  
  if(vec)
  {
    while(n)
    {
      out[0] = iemarray_getfloat(vec, 0);
      out[1] = iemarray_getfloat(vec, 1);
      out[2] = iemarray_getfloat(vec, 2);
      out[3] = iemarray_getfloat(vec, 3);
      out[4] = iemarray_getfloat(vec, 4);
      out[5] = iemarray_getfloat(vec, 5);
      out[6] = iemarray_getfloat(vec, 6);
      out[7] = iemarray_getfloat(vec, 7);
      out[8] = iemarray_getfloat(vec, 8);
      out[9] = iemarray_getfloat(vec, 9);
      out[10] = iemarray_getfloat(vec, 10);
      out[11] = iemarray_getfloat(vec, 11);
      out[12] = iemarray_getfloat(vec, 12);
      out[13] = iemarray_getfloat(vec, 13);
      out[14] = iemarray_getfloat(vec, 14);
      out[15] = iemarray_getfloat(vec, 15);

      vec += 16;
      out += 16;
      n -= 16;
    }
    out[0] = iemarray_getfloat(vec, 0);
  }
  else
  {
    while(n)
    {
      out[0] = 0.0f;
      out[1] = 0.0f;
      out[2] = 0.0f;
      out[3] = 0.0f;
      out[4] = 0.0f;
      out[5] = 0.0f;
      out[6] = 0.0f;
      out[7] = 0.0f;
      out[8] = 0.0f;
      out[9] = 0.0f;
      out[10] = 0.0f;
      out[11] = 0.0f;
      out[12] = 0.0f;
      out[13] = 0.0f;
      out[14] = 0.0f;
      out[15] = 0.0f;
      
      out += 16;
      n -= 16;
    }
    out[0] = 0.0f;
  }
  return(w+4);
}

static void spec2_tabreceive_tilde_dsp(t_spec2_tabreceive_tilde *x, t_signal **sp)
{
  t_garray *a;
  int vecsize;
  
  if(!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
  {
    if(*x->x_arrayname->s_name)
      error("spec2_tabreceive~: %s: no such array", x->x_arrayname->s_name);
  }
  else if(!iemarray_getarray(a, &vecsize, &x->x_vec))
    error("%s: bad template for spec2_tabreceive~", x->x_arrayname->s_name);
  else 
  {
    int n = sp[0]->s_n;
    
    if(n < vecsize)
      vecsize = n;
    vecsize /= 2;
    if(vecsize&15)
      dsp_add(spec2_tabreceive_tilde_perform, 3, x, sp[0]->s_vec, vecsize);
    else
      dsp_add(spec2_tabreceive_tilde_perf16, 3, x, sp[0]->s_vec, vecsize);
  }
}

static void *spec2_tabreceive_tilde_new(t_symbol *s)
{
  t_spec2_tabreceive_tilde *x = (t_spec2_tabreceive_tilde *)pd_new(spec2_tabreceive_tilde_class);
  
  x->x_arrayname = s;
  outlet_new(&x->x_obj, &s_signal);
  return (x);
}

void spec2_tabreceive_tilde_setup(void)
{
  spec2_tabreceive_tilde_class = class_new(gensym("spec2_tabreceive~"), (t_newmethod)spec2_tabreceive_tilde_new,
    0, sizeof(t_spec2_tabreceive_tilde), 0, A_DEFSYM, 0);
  class_addmethod(spec2_tabreceive_tilde_class, (t_method)spec2_tabreceive_tilde_dsp, gensym("dsp"), 0);
  class_addsymbol(spec2_tabreceive_tilde_class, (t_method)spec2_tabreceive_tilde_symbol);
//  class_sethelpsymbol(spec2_tabreceive_tilde_class, gensym("iemhelp/spec2_tabreceive~-help"));
}
