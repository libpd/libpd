/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* 32 bit "pointer cast" union */
typedef union {
    float f;
    long i;
} ls_pcast32;

/* ------------------------ spec2_sqrt_tilde~ ------------------------- */

static t_class *spec2_sqrt_tilde_class;

#define SPEC2DUMTAB1SIZE 256
#define SPEC2DUMTAB2SIZE 1024

static t_float spec2_rsqrt_exptab[SPEC2DUMTAB1SIZE], spec2_rsqrt_mantissatab[SPEC2DUMTAB2SIZE];

static void init_spec2_rsqrt(void)
{
  int i;
  
  for (i=0; i<SPEC2DUMTAB1SIZE; i++)
  {
    long l = (i ? (i == SPEC2DUMTAB1SIZE-1 ? SPEC2DUMTAB1SIZE-2 : i) : 1)<< 23;
    ls_pcast32 *pc = (ls_pcast32 *)(&l);

    spec2_rsqrt_exptab[i] = 1.0f/sqrt((*pc).f); 
  }
  
  for (i=0; i<SPEC2DUMTAB2SIZE; i++)
  {
    t_float f = 1.0f + (1.0f / (t_float)SPEC2DUMTAB2SIZE) * (t_float)i;
    
    spec2_rsqrt_mantissatab[i] = 1.0f / sqrt(f);  
  }
}

typedef struct _spec2_sqrt_tilde
{
  t_object  x_obj;
  t_float   x_msi;
} t_spec2_sqrt_tilde;

static t_int *spec2_sqrt_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3]+1;
  
  while(n--)
  { 
    t_float f = *in;
    long l = *(long *)(in++);
    
    if(f < 0.0f)
      *out++ = 0.0f;
    else
    {
      t_float g = spec2_rsqrt_exptab[(l >> 23) & 0xff] * spec2_rsqrt_mantissatab[(l >> 13) & 0x3ff];
      
      *out++ = f*g*(1.5f - 0.5f*g*g*f);
    }
  }
  return(w+4);
}

static void spec2_sqrt_tilde_dsp(t_spec2_sqrt_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  dsp_add(spec2_sqrt_tilde_perform, 3, sp[0]->s_vec, sp[0]->s_vec, n);
}

static void *spec2_sqrt_tilde_new(void)
{
  t_spec2_sqrt_tilde *x = (t_spec2_sqrt_tilde *)pd_new(spec2_sqrt_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

void spec2_sqrt_tilde_setup(void)
{
  init_spec2_rsqrt();
  spec2_sqrt_tilde_class = class_new(gensym("spec2_sqrt~"), (t_newmethod)spec2_sqrt_tilde_new,
    0, sizeof(t_spec2_sqrt_tilde), 0, 0);
  CLASS_MAINSIGNALIN(spec2_sqrt_tilde_class, t_spec2_sqrt_tilde, x_msi);
  class_addmethod(spec2_sqrt_tilde_class, (t_method)spec2_sqrt_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_sqrt_tilde_class, gensym("iemhelp2/spec2_sqrt~-help"));
}
