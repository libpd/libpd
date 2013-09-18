/* (C) Oswald Berthold  <opt@web.fm> */


#include "math.h"
#include <m_pd.h>

/* ----------------------------- mixer ----------------------------- */
static t_class *mixer_class;



typedef struct _mixer
{
  t_object x_obj;
  t_int x_n;
  t_float* x_m;
} t_mixer;

static void *mixer_new(t_symbol *s, t_floatarg num)
{
  int i;
  t_mixer *x = (t_mixer *)pd_new(mixer_class);
  if (num < 1)  x->x_n = 1;
  else x->x_n = (int) num;
	
  x->x_m = getbytes(sizeof(t_float)*x->x_n);

  for (i=0;i<x->x_n /* - 1 */ ;i++) {
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->x_m[i] = 1.;
  }	

  

  outlet_new(&x->x_obj, &s_signal);
  return (x);
}

void mixer_list(t_mixer* x,t_symbol* s,t_int argc, t_atom* argv) 
{
  int chan;
  t_float val;

  if (argc != 2) {
    post("unknown");
    return;
  }

  chan = (int) atom_getfloat(argv++);
  val =   atom_getfloat(argv++);


  x->x_m[chan] = val;

}


t_int *mixer_perform(t_int *w)
{
  t_mixer* x = (t_mixer*) (w[1]);
  int n = (int)(w[2]);
  t_float **in = getbytes(sizeof(float)*x->x_n);
  t_float *out;
  int i,j;
  int offset = 3;

  for (i=0;i < x->x_n;i++) {
    in[i] = (t_float *)(w[offset+i]);
  }

  out = (t_float *)(w[offset+i]);
  
  while (n--) {
    *out = 0.;
    for (j=0;j<x->x_n;j++) {
      *out += *(in[j]++) * x->x_m[j];
    }
    out++;
  }
  return (w+offset+1+i);
}

static void mixer_dsp(t_mixer *x, t_signal **sp)
{
  int i;
  t_int** myvec = getbytes(sizeof(t_int)*(x->x_n + 3));
  
  myvec[0] = (t_int*)x;
  myvec[1] = (t_int*)sp[0]->s_n;

  for (i=0;i < x->x_n+1;i++)
    myvec[2 + i] = (t_int*)sp[i]->s_vec;

  dsp_addv(mixer_perform, x->x_n + 3, (t_int*)myvec);
  freebytes(myvec,sizeof(t_int)*(x->x_n + 3));
}

void mixer_tilde_setup(void)
{
  mixer_class = class_new(gensym("mixer~"), (t_newmethod)mixer_new, 0,
			  sizeof(t_mixer), 0, A_DEFFLOAT, A_DEFSYM,A_NULL);
  class_addmethod(mixer_class, nullfn, gensym("signal"), 0);
  class_addmethod(mixer_class, (t_method)mixer_dsp, gensym("dsp"), 0);
  
  class_addlist(mixer_class,mixer_list);
}

