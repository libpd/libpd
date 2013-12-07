#include "m_pd.h"
#include <math.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ delta~ ----------------------------- */

/* tilde object to take difference value. */

static t_class *delta_class;

typedef struct _delta
{
  t_object x_obj;
  t_sample x_last;
} t_delta;

static t_int *delta_perform(t_int *w)
{
  t_delta *x  = (t_delta *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--)
    {
      float f = *(in++);
      // *out++ = (f > 0 ? f : -f);	
      *out++ = (f > x->x_last ? fabs(f - x->x_last) : -fabs(f - x->x_last));
      x->x_last = f;
    }
  return (w+5);
}

static void delta_dsp(t_delta *x, t_signal **sp)
{
  dsp_add(delta_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *delta_new(void)
{
  t_delta *x = (t_delta *)pd_new(delta_class);
  x->x_last = 0.;
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

void delta_tilde_setup(void)
{
  delta_class = class_new(gensym("delta~"), (t_newmethod)delta_new, 0,
			  sizeof(t_delta), 0, A_DEFFLOAT, 0);
  class_addmethod(delta_class, nullfn, gensym("signal"), 0);
  class_addmethod(delta_class, (t_method)delta_dsp, gensym("dsp"), 0);
}
