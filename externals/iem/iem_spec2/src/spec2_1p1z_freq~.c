/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* -- spec2_1p1z_freq~ - filter the spectrum with a 1.order IIR twice, once forwards, once backwards --- */

typedef struct spec2_1p1z_freq_tilde
{
  t_object  x_obj;
  t_float   x_a0;
  t_float   x_a1;
  t_float   x_b1;
  t_float   *x_begmem;
  int       x_blocksize;
  t_float   x_msi;
} t_spec2_1p1z_freq_tilde;

t_class *spec2_1p1z_freq_tilde_class;

static void spec2_1p1z_freq_tilde_list(t_spec2_1p1z_freq_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 3) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2))
  {
    x->x_a0 = (t_float)atom_getfloatarg(0, argc, argv);
    x->x_a1 = (t_float)atom_getfloatarg(1, argc, argv);
    x->x_b1 = (t_float)atom_getfloatarg(2, argc, argv);
  }
}

static t_int *spec2_1p1z_freq_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_1p1z_freq_tilde *x = (t_spec2_1p1z_freq_tilde *)(w[3]);
  int i, m, n = (int)(w[4]);
  t_float a0 = x->x_a0;
  t_float a1 = x->x_a1;
  t_float b1 = x->x_b1;
  t_float *vec1, *vec2, *vec3;
  t_float in_old, out_old, f;
  
  vec2 = x->x_begmem + n + 1;
  vec1 = vec2 - 1;
  vec3 = vec2 + 2*n;
  
  *vec2++ = in[0];
  for(i=1; i<n; i++)
  {
    f = in[i];
    *vec2++ = f;
    *vec1-- = f;
    *vec3-- = f;
  }
  *vec2 = in[n];
  
  m = 3*n - 1;
  
  vec2 = x->x_begmem + 2;
  in_old = 0.0f;
  out_old = 0.0f;
  for(i=0; i<m; i++)
  {
    f = *vec2;
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    *vec2++ = out_old;
  }
  
  vec2 = x->x_begmem + 3*n - 2;
  in_old = 0.0f;
  out_old = 0.0f;
  for(i=0; i<m; i++)
  {
    f = *vec2;
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    *vec2-- = out_old;
  }
  
  vec2 = x->x_begmem + n + 1;
  for(i=0; i<=n; i++)
  {
    out[i] = *vec2++;
  }
  
  return(w+5);
}

static t_int *spec2_1p1z_freq_tilde_perf16(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_1p1z_freq_tilde *x = (t_spec2_1p1z_freq_tilde *)(w[3]);
  int i, m, n = (int)(w[4]);
  t_float a0 = x->x_a0;
  t_float a1 = x->x_a1;
  t_float b1 = x->x_b1;
  t_float *vec1, *vec2, *vec3;
  t_float in_old, out_old, f;
  
  m = 3*n;
  
  vec2 = x->x_begmem + n + 1;
  vec1 = vec2;
  //  vec3 = vec2 + 2*n + 2 - 2;
  vec3 = vec2 + 2*n;
  
  x->x_begmem[0] = 0.0f;
  x->x_begmem[m-1] = 0.0f;
  
  i = n;
  while(i)
  {
    f = in[0];
    vec2[0] = f;
    vec1[0] = f;
    vec3[0] = f;
    
    f = in[1];
    vec2[1] = f;
    vec1[-1] = f;
    vec3[-1] = f;
    
    f = in[2];
    vec2[2] = f;
    vec1[-2] = f;
    vec3[-2] = f;
    
    f = in[3];
    vec2[3] = f;
    vec1[-3] = f;
    vec3[-3] = f;
    
    f = in[4];
    vec2[4] = f;
    vec1[-4] = f;
    vec3[-4] = f;
    
    f = in[5];
    vec2[5] = f;
    vec1[-5] = f;
    vec3[-5] = f;
    
    f = in[6];
    vec2[6] = f;
    vec1[-6] = f;
    vec3[-6] = f;
    
    f = in[7];
    vec2[7] = f;
    vec1[-7] = f;
    vec3[-7] = f;
    
    f = in[8];
    vec2[8] = f;
    vec1[-8] = f;
    vec3[-8] = f;
    
    f = in[9];
    vec2[9] = f;
    vec1[-9] = f;
    vec3[-9] = f;
    
    f = in[10];
    vec2[10] = f;
    vec1[-10] = f;
    vec3[-10] = f;
    
    f = in[11];
    vec2[11] = f;
    vec1[-11] = f;
    vec3[-11] = f;
    
    f = in[12];
    vec2[12] = f;
    vec1[-12] = f;
    vec3[-12] = f;
    
    f = in[13];
    vec2[13] = f;
    vec1[-13] = f;
    vec3[-13] = f;
    
    f = in[14];
    vec2[14] = f;
    vec1[-14] = f;
    vec3[-14] = f;
    
    f = in[15];
    vec2[15] = f;
    vec1[-15] = f;
    vec3[-15] = f;
    
    in += 16;
    vec1 -= 16;
    vec2 += 16;
    vec3 -= 16;
    i -= 16;
  }
  f = in[0];
  vec2[0] = f;
  vec1[0] = f;
  vec3[0] = f;
  
  vec2 = x->x_begmem;
  in_old = 0.0f;
  out_old = 0.0f;
  i = m;
  while(i)
  {
    f = vec2[0];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[0] = out_old;
    
    f = vec2[1];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[1] = out_old;
    
    f = vec2[2];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[2] = out_old;
    
    f = vec2[3];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[3] = out_old;
    
    f = vec2[4];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[4] = out_old;
    
    f = vec2[5];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[5] = out_old;
    
    f = vec2[6];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[6] = out_old;
    
    f = vec2[7];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[7] = out_old;
    
    f = vec2[8];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[8] = out_old;
    
    f = vec2[9];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[9] = out_old;
    
    f = vec2[10];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[10] = out_old;
    
    f = vec2[11];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[11] = out_old;
    
    f = vec2[12];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[12] = out_old;
    
    f = vec2[13];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[13] = out_old;
    
    f = vec2[14];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[14] = out_old;
    
    f = vec2[15];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[15] = out_old;
    
    vec2 += 16;
    i -= 16;
  }
  f = vec2[0];
  out_old = a0*f + a1*in_old + b1*out_old;
  in_old = f;
  vec2[0] = out_old;
  
  f = vec2[1];
  out_old = a0*f + a1*in_old + b1*out_old;
  in_old = f;
  vec2[1] = out_old;
  
  f = vec2[2];
  out_old = a0*f + a1*in_old + b1*out_old;
  in_old = f;
  vec2[2] = out_old;
  
  //  vec2 = x->x_begmem + 3*n - 1 + 3;
  vec2 = x->x_begmem + 3*n + 2;
  in_old = 0.0f;
  out_old = 0.0f;
  for(i=0; i<m; )
  {
    f = vec2[0];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[0] = out_old;
    
    f = vec2[-1];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-1] = out_old;
    
    f = vec2[-2];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-2] = out_old;
    
    f = vec2[-3];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-3] = out_old;
    
    f = vec2[-4];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-4] = out_old;
    
    f = vec2[-5];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-5] = out_old;
    
    f = vec2[-6];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-6] = out_old;
    
    f = vec2[-7];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-7] = out_old;
    
    f = vec2[-8];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-8] = out_old;
    
    f = vec2[-9];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-9] = out_old;
    
    f = vec2[-10];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-10] = out_old;
    
    f = vec2[-11];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-11] = out_old;
    
    f = vec2[-12];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-12] = out_old;
    
    f = vec2[-13];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-13] = out_old;
    
    f = vec2[-14];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-14] = out_old;
    
    f = vec2[-15];
    out_old = a0*f + a1*in_old + b1*out_old;
    in_old = f;
    vec2[-15] = out_old;
    
    vec2 -= 16;
    i -= 16;
  }
  f = vec2[0];
  out_old = a0*f + a1*in_old + b1*out_old;
  in_old = f;
  vec2[0] = out_old;
  
  f = vec2[-1];
  out_old = a0*f + a1*in_old + b1*out_old;
  in_old = f;
  vec2[-1] = out_old;
  
  f = vec2[-2];
  out_old = a0*f + a1*in_old + b1*out_old;
  in_old = f;
  vec2[-2] = out_old;
  
  vec2 = x->x_begmem + n + 1;
  i = n;
  while(i)
  {
    out[0] = vec2[0];
    out[1] = vec2[1];
    out[2] = vec2[2];
    out[3] = vec2[3];
    out[4] = vec2[4];
    out[5] = vec2[5];
    out[6] = vec2[6];
    out[7] = vec2[7];
    out[8] = vec2[8];
    out[9] = vec2[9];
    out[10] = vec2[10];
    out[11] = vec2[11];
    out[12] = vec2[12];
    out[13] = vec2[13];
    out[14] = vec2[14];
    out[15] = vec2[15];
    
    vec2 += 16;
    out += 16;
    i -= 16;
  }
  out[0] = vec2[0];
  
  return(w+5);
}

static void spec2_1p1z_freq_tilde_dsp(t_spec2_1p1z_freq_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(!x->x_blocksize)
  {
    x->x_begmem = (t_float *)getbytes(3*(n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  else if(x->x_blocksize != n)
  {
    x->x_begmem = (t_float *)resizebytes(x->x_begmem, 3*(x->x_blocksize+1)*sizeof(t_float), 3*(n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  
  if(n&15)
    dsp_add(spec2_1p1z_freq_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
  else
    dsp_add(spec2_1p1z_freq_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void spec2_1p1z_freq_tilde_free(t_spec2_1p1z_freq_tilde *x)
{
  if(x->x_begmem)
    freebytes(x->x_begmem, 3*(x->x_blocksize+1)*sizeof(t_float));
}

static void *spec2_1p1z_freq_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_spec2_1p1z_freq_tilde *x = (t_spec2_1p1z_freq_tilde *)pd_new(spec2_1p1z_freq_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_blocksize = 0;
  x->x_begmem = (t_float *)0;
  if(argc >= 3)
    spec2_1p1z_freq_tilde_list(x, s, argc, argv);
  else
  {
    x->x_a0 = 1.0f;
    x->x_a1 = 0.0f;
    x->x_b1 = 0.0f;
  }
  x->x_msi = 0.0f;
  return (x);
}

void spec2_1p1z_freq_tilde_setup(void)
{
  spec2_1p1z_freq_tilde_class = class_new(gensym("spec2_1p1z_freq~"), (t_newmethod)spec2_1p1z_freq_tilde_new,
    (t_method)spec2_1p1z_freq_tilde_free, sizeof(t_spec2_1p1z_freq_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(spec2_1p1z_freq_tilde_class, t_spec2_1p1z_freq_tilde, x_msi);
  class_addmethod(spec2_1p1z_freq_tilde_class, (t_method)spec2_1p1z_freq_tilde_dsp, gensym("dsp"), 0);
  class_addlist(spec2_1p1z_freq_tilde_class, (t_method)spec2_1p1z_freq_tilde_list);
//  class_sethelpsymbol(spec2_1p1z_freq_tilde_class, gensym("iemhelp2/spec2_1p1z_freq~-help"));
}
