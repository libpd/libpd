/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- spec2_1p1z_time~ ------------------------------ */

static t_class *spec2_1p1z_time_tilde_class;

typedef struct _spec2_1p1z_time_tilde
{
  t_object  x_obj;
  t_float   x_a0;
  t_float   x_a1;
  t_float   x_b1;
  t_float   *x_begmem_forw;
  t_float   *x_begmem_back;
  int       x_blocksize;
  t_float   x_msi;
} t_spec2_1p1z_time_tilde;

static void spec2_1p1z_time_tilde_list(t_spec2_1p1z_time_tilde *x, t_symbol *s, int argc, t_atom *argv)
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

static t_int *spec2_1p1z_time_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_1p1z_time_tilde *x = (t_spec2_1p1z_time_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float a0 = x->x_a0;
  t_float a1 = x->x_a1;
  t_float b1 = x->x_b1;
  t_float *vec_forw = x->x_begmem_forw;
  t_float *vec_back = x->x_begmem_back;
  t_float f;
  
  for(i=0; i<=n; i++)
  {
    f = in[i];
    out[i] = a0*f + a1*vec_forw[i] + b1*vec_back[i];
    vec_forw[i] = f;
    vec_back[i] = out[i];
  }
  return(w+5);
}

static t_int *spec2_1p1z_time_tilde_perf16(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_1p1z_time_tilde *x = (t_spec2_1p1z_time_tilde *)(w[3]);
  int n = (t_int)(w[4]);
  t_float a0 = x->x_a0;
  t_float a1 = x->x_a1;
  t_float b1 = x->x_b1;
  t_float *vec_forw = x->x_begmem_forw;
  t_float *vec_back = x->x_begmem_back;
  t_float ff;
  
  while(n)
  {
    t_float f[16];
    
    f[0] = in[0];
    f[1] = in[1];
    f[2] = in[2];
    f[3] = in[3];
    f[4] = in[4];
    f[5] = in[5];
    f[6] = in[6];
    f[7] = in[7];
    f[8] = in[8];
    f[9] = in[9];
    f[10] = in[10];
    f[11] = in[11];
    f[12] = in[12];
    f[13] = in[13];
    f[14] = in[14];
    f[15] = in[15];
    
    out[0] = a0*f[0] + a1*vec_forw[0] + b1*vec_back[0];
    out[1] = a0*f[1] + a1*vec_forw[1] + b1*vec_back[1];
    out[2] = a0*f[2] + a1*vec_forw[2] + b1*vec_back[2];
    out[3] = a0*f[3] + a1*vec_forw[3] + b1*vec_back[3];
    out[4] = a0*f[4] + a1*vec_forw[4] + b1*vec_back[4];
    out[5] = a0*f[5] + a1*vec_forw[5] + b1*vec_back[5];
    out[6] = a0*f[6] + a1*vec_forw[6] + b1*vec_back[6];
    out[7] = a0*f[7] + a1*vec_forw[7] + b1*vec_back[7];
    out[8] = a0*f[8] + a1*vec_forw[8] + b1*vec_back[8];
    out[9] = a0*f[9] + a1*vec_forw[9] + b1*vec_back[9];
    out[10] = a0*f[10] + a1*vec_forw[10] + b1*vec_back[10];
    out[11] = a0*f[11] + a1*vec_forw[11] + b1*vec_back[11];
    out[12] = a0*f[12] + a1*vec_forw[12] + b1*vec_back[12];
    out[13] = a0*f[13] + a1*vec_forw[13] + b1*vec_back[13];
    out[14] = a0*f[14] + a1*vec_forw[14] + b1*vec_back[14];
    out[15] = a0*f[15] + a1*vec_forw[15] + b1*vec_back[15];
    
    vec_forw[0] = f[0];
    vec_forw[1] = f[1];
    vec_forw[2] = f[2];
    vec_forw[3] = f[3];
    vec_forw[4] = f[4];
    vec_forw[5] = f[5];
    vec_forw[6] = f[6];
    vec_forw[7] = f[7];
    vec_forw[8] = f[8];
    vec_forw[9] = f[9];
    vec_forw[10] = f[10];
    vec_forw[11] = f[11];
    vec_forw[12] = f[12];
    vec_forw[13] = f[13];
    vec_forw[14] = f[14];
    vec_forw[15] = f[15];
    
    vec_back[0] = out[0];
    vec_back[1] = out[1];
    vec_back[2] = out[2];
    vec_back[3] = out[3];
    vec_back[4] = out[4];
    vec_back[5] = out[5];
    vec_back[6] = out[6];
    vec_back[7] = out[7];
    vec_back[8] = out[8];
    vec_back[9] = out[9];
    vec_back[10] = out[10];
    vec_back[11] = out[11];
    vec_back[12] = out[12];
    vec_back[13] = out[13];
    vec_back[14] = out[14];
    vec_back[15] = out[15];
    
    in += 16;
    out += 16;
    vec_forw += 16;
    vec_back += 16;
    n -= 16;
  }
  ff = in[0];
  out[0] = a0*ff + a1*vec_forw[0] + b1*vec_back[0];
  vec_forw[0] = ff;
  vec_back[0] = out[0];
  return(w+5);
}

static void spec2_1p1z_time_tilde_dsp(t_spec2_1p1z_time_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(!x->x_blocksize)/*first time*/
  {
    x->x_begmem_forw = (t_float *)getbytes(2 * (n+1) * sizeof(t_float));
    x->x_blocksize = n;
    x->x_begmem_back = x->x_begmem_forw + n + 1;
  }
  else if(x->x_blocksize != n)
  {
    x->x_begmem_forw = (t_float *)resizebytes(x->x_begmem_forw, 2*(x->x_blocksize+1)*sizeof(t_float), 2*(n+1)*sizeof(t_float));
    x->x_blocksize = n;
    x->x_begmem_back = x->x_begmem_forw + n +1;
  }
  if(n&15)
    dsp_add(spec2_1p1z_time_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
  else
    dsp_add(spec2_1p1z_time_tilde_perf16, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *spec2_1p1z_time_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_spec2_1p1z_time_tilde *x = (t_spec2_1p1z_time_tilde *)pd_new(spec2_1p1z_time_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_blocksize = 0;
  x->x_begmem_forw = (t_float *)0;
  if(argc >= 3)
    spec2_1p1z_time_tilde_list(x, s, argc, argv);
  else
  {
    x->x_a0 = 1.0f;
    x->x_a1 = 0.0f;
    x->x_b1 = 0.0f;
  }
  x->x_msi = 0.0f;
  return (x);
}

static void spec2_1p1z_time_tilde_free(t_spec2_1p1z_time_tilde *x)
{
  if(x->x_begmem_forw)
    freebytes(x->x_begmem_forw, 2 * (x->x_blocksize+1) * sizeof(t_float));
}

void spec2_1p1z_time_tilde_setup(void)
{
  spec2_1p1z_time_tilde_class = class_new(gensym("spec2_1p1z_time~"), (t_newmethod)spec2_1p1z_time_tilde_new, (t_method)spec2_1p1z_time_tilde_free,
    sizeof(t_spec2_1p1z_time_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(spec2_1p1z_time_tilde_class, t_spec2_1p1z_time_tilde, x_msi);
  class_addlist(spec2_1p1z_time_tilde_class, (t_method)spec2_1p1z_time_tilde_list);
  class_addmethod(spec2_1p1z_time_tilde_class, (t_method)spec2_1p1z_time_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_1p1z_time_tilde_class, gensym("iemhelp2/spec2_1p1z_time~-help"));
}
