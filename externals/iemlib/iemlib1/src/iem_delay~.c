/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* -------------------------- iem_delay~ ------------------------------ */

static t_class *iem_delay_tilde_class;

#define IEMDELAY_DEF_VEC_SIZE 64

typedef struct _iem_delay_tilde
{
  t_object  x_obj;
  int       x_mallocsize;
  t_float   x_max_delay_ms;
  t_float   x_current_delay_ms;
  t_float   *x_begmem1;
  t_float   *x_begmem2;
  int       x_writeindex;
  int       x_blocksize;
  int       x_delay_samples;
  t_float   x_sr;
  t_float   x_msi;
} t_iem_delay_tilde;

static void iem_delay_tilde_cur_del(t_iem_delay_tilde *x, t_floatarg f)
{
  if(f < 0.0f)
    f = 0.0f;
  else if(f > x->x_max_delay_ms)
    f = x->x_max_delay_ms;
  x->x_current_delay_ms = f;
  x->x_delay_samples = (int)(0.001f*x->x_sr * f + 0.5f);
}

static t_int *iem_delay_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_iem_delay_tilde *x = (t_iem_delay_tilde *)(w[3]);
  int n=(int)(w[4]);
  int writeindex = x->x_writeindex;
  t_float *vec1, *vec2, *vec3;
  
  vec1 = x->x_begmem1 + writeindex;
  vec2 = x->x_begmem2 + writeindex;
  vec3 = x->x_begmem2 + writeindex - x->x_delay_samples;
  writeindex += n;
  while(n--)
  {
    *vec1++ = *vec2++ = *in++;
    *out++ = *vec3++;
  }
  if(writeindex >= x->x_mallocsize)
  {
    writeindex -= x->x_mallocsize;
  }
  x->x_writeindex = writeindex;
  return(w+5);
}

static t_int *iem_delay_tilde_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_iem_delay_tilde *x = (t_iem_delay_tilde *)(w[3]);
  int i, n=(int)(w[4]);
  int writeindex = x->x_writeindex;
  t_float *vec1, *vec2;
  
  vec1 = x->x_begmem1 + writeindex;
  vec2 = x->x_begmem2 + writeindex;
  for(i=0; i<n; i+=8)
  {
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
    *vec1++ = *vec2++ = *in++;
  }
  
  vec2 = x->x_begmem2 + writeindex - x->x_delay_samples;
  for(i=0; i<n; i+=8)
  {
    *out++ = *vec2++;
    *out++ = *vec2++;
    *out++ = *vec2++;
    *out++ = *vec2++;
    *out++ = *vec2++;
    *out++ = *vec2++;
    *out++ = *vec2++;
    *out++ = *vec2++;
  }
  
  writeindex += n;
  if(writeindex >= x->x_mallocsize)
  {
    writeindex -= x->x_mallocsize;
  }
  x->x_writeindex = writeindex;
  return(w+5);
}

static void iem_delay_tilde_dsp(t_iem_delay_tilde *x, t_signal **sp)
{
  int blocksize = sp[0]->s_n, i;
  
  if(!x->x_blocksize)/*first time*/
  {
    int nsamps = x->x_max_delay_ms * (t_float)sp[0]->s_sr * 0.001f;
    
    if(nsamps < 1)
      nsamps = 1;
    nsamps += ((- nsamps) & (blocksize - 1));
    nsamps += blocksize;
    x->x_mallocsize = nsamps;
    x->x_begmem1 = (t_float *)getbytes(2 * x->x_mallocsize * sizeof(t_float));
    x->x_begmem2 = x->x_begmem1 + x->x_mallocsize;
    post("beginn = %x", (unsigned long)x->x_begmem1);
    x->x_writeindex = blocksize;
    x->x_sr = (t_float)sp[0]->s_sr;
    x->x_blocksize = blocksize;
    x->x_delay_samples = (int)(0.001f*x->x_sr * x->x_current_delay_ms + 0.5f);
  }
  else if((x->x_blocksize != blocksize) || ((t_float)sp[0]->s_sr != x->x_sr))
  {
    int nsamps = x->x_max_delay_ms * (t_float)sp[0]->s_sr * 0.001f;
    
    if(nsamps < 1)
      nsamps = 1;
    nsamps += ((- nsamps) & (blocksize - 1));
    nsamps += blocksize;
    
    x->x_begmem1 = (t_float *)resizebytes(x->x_begmem1, 2*x->x_mallocsize*sizeof(t_float), 2*nsamps*sizeof(t_float));
    x->x_mallocsize = nsamps;
    x->x_begmem2 = x->x_begmem1 + x->x_mallocsize;
    post("beginn = %x", (unsigned long)x->x_begmem1);
    if(x->x_writeindex >= nsamps)
      x->x_writeindex -= nsamps;
    x->x_sr = (t_float)sp[0]->s_sr;
    x->x_blocksize = blocksize;
    x->x_delay_samples = (int)(0.001f*x->x_sr * x->x_current_delay_ms + 0.5f);
  }
  
  if(blocksize&7)
    dsp_add(iem_delay_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, blocksize);
  else
    dsp_add(iem_delay_tilde_perf8, 4, sp[0]->s_vec, sp[1]->s_vec, x, blocksize);
}

static void *iem_delay_tilde_new(t_floatarg max_delay_ms, t_floatarg current_delay_ms)
{
  t_iem_delay_tilde *x = (t_iem_delay_tilde *)pd_new(iem_delay_tilde_class);
  int nsamps;
  
  if(max_delay_ms < 2.0f)
    max_delay_ms = 2.0f;
  x->x_max_delay_ms = max_delay_ms;
  if(current_delay_ms < 0.0f)
    current_delay_ms = 0.0f;
  else if(current_delay_ms > max_delay_ms)
    current_delay_ms = max_delay_ms;
  x->x_current_delay_ms = current_delay_ms;
  nsamps = max_delay_ms * sys_getsr() * 0.001f;
  if(nsamps < 1)
    nsamps = 1;
  nsamps += ((- nsamps) & (IEMDELAY_DEF_VEC_SIZE - 1));
  nsamps += IEMDELAY_DEF_VEC_SIZE;
  x->x_mallocsize = nsamps;
  x->x_begmem1 = (t_float *)getbytes(2 * x->x_mallocsize * sizeof(t_float));
  x->x_begmem2 = x->x_begmem1 + x->x_mallocsize;
  x->x_writeindex = IEMDELAY_DEF_VEC_SIZE;
  x->x_blocksize = 0;
  x->x_sr = 0.0f;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

static void iem_delay_tilde_free(t_iem_delay_tilde *x)
{
  freebytes(x->x_begmem1, 2 * x->x_mallocsize * sizeof(t_float));
}

void iem_delay_tilde_setup(void)
{
  iem_delay_tilde_class = class_new(gensym("iem_delay~"), (t_newmethod)iem_delay_tilde_new, (t_method)iem_delay_tilde_free,
    sizeof(t_iem_delay_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(iem_delay_tilde_class, t_iem_delay_tilde, x_msi);
  class_addmethod(iem_delay_tilde_class, (t_method)iem_delay_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(iem_delay_tilde_class, (t_method)iem_delay_tilde_cur_del, gensym("ft1"), A_FLOAT, 0);
}
