/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- spec2_block_delay~ ------------------------------ */
static t_class *spec2_block_delay_tilde_class;

typedef struct _spec2_block_delay_tilde
{
  t_object  x_obj;
  t_float   *x_begmem;
  int       x_blocksize;
  t_float   x_msi;
} t_spec2_block_delay_tilde;

static t_int *spec2_block_delay_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_block_delay_tilde *x = (t_spec2_block_delay_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float *rw_vec;
  
  rw_vec = x->x_begmem;
  for(i=0; i<=n; i++)
  {
    t_float f = in[i];
    out[i] = rw_vec[i];
    rw_vec[i] = f;
  }
  return(w+5);
}

static t_int *spec2_block_delay_tilde_perf16(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_spec2_block_delay_tilde *x = (t_spec2_block_delay_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float *rw_vec, ff;
  
  rw_vec = x->x_begmem;
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
    
    out[0] = rw_vec[0];
    out[1] = rw_vec[1];
    out[2] = rw_vec[2];
    out[3] = rw_vec[3];
    out[4] = rw_vec[4];
    out[5] = rw_vec[5];
    out[6] = rw_vec[6];
    out[7] = rw_vec[7];
    out[8] = rw_vec[8];
    out[9] = rw_vec[9];
    out[10] = rw_vec[10];
    out[11] = rw_vec[11];
    out[12] = rw_vec[12];
    out[13] = rw_vec[13];
    out[14] = rw_vec[14];
    out[15] = rw_vec[15];
    
    rw_vec[0] = f[0];
    rw_vec[1] = f[1];
    rw_vec[2] = f[2];
    rw_vec[3] = f[3];
    rw_vec[4] = f[4];
    rw_vec[5] = f[5];
    rw_vec[6] = f[6];
    rw_vec[7] = f[7];
    rw_vec[8] = f[8];
    rw_vec[9] = f[9];
    rw_vec[10] = f[10];
    rw_vec[11] = f[11];
    rw_vec[12] = f[12];
    rw_vec[13] = f[13];
    rw_vec[14] = f[14];
    rw_vec[15] = f[15];
    
    rw_vec += 16;
    in += 16;
    out += 16;
    n -= 16;
  }
  ff = in[0];
  out[0] = rw_vec[0];
  rw_vec[0] = ff;
  return(w+5);
}

static void spec2_block_delay_tilde_dsp(t_spec2_block_delay_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(!x->x_blocksize)/*first time*/
  {
    x->x_begmem = (t_float *)getbytes((n+1) * sizeof(t_float));
    x->x_blocksize = n;
  }
  else if(x->x_blocksize != n)
  {
    x->x_begmem = (t_float *)resizebytes(x->x_begmem, (x->x_blocksize+1)*sizeof(t_float), (n+1)*sizeof(t_float));
    x->x_blocksize = n;
  }
  if(n&15)
    dsp_add(spec2_block_delay_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
  else
    dsp_add(spec2_block_delay_tilde_perf16, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *spec2_block_delay_tilde_new(void)
{
  t_spec2_block_delay_tilde *x = (t_spec2_block_delay_tilde *)pd_new(spec2_block_delay_tilde_class);
  
  x->x_blocksize = 0;
  x->x_begmem = (t_float *)0;
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

static void spec2_block_delay_tilde_free(t_spec2_block_delay_tilde *x)
{
  if(x->x_begmem)
    freebytes(x->x_begmem, (x->x_blocksize+1) * sizeof(t_float));
}

void spec2_block_delay_tilde_setup(void)
{
  spec2_block_delay_tilde_class = class_new(gensym("spec2_block_delay~"), (t_newmethod)spec2_block_delay_tilde_new, (t_method)spec2_block_delay_tilde_free,
    sizeof(t_spec2_block_delay_tilde), 0, 0);
  CLASS_MAINSIGNALIN(spec2_block_delay_tilde_class, t_spec2_block_delay_tilde, x_msi);
  class_addmethod(spec2_block_delay_tilde_class, (t_method)spec2_block_delay_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_block_delay_tilde_class, gensym("iemhelp2/spec2_block_delay~-help"));
}
