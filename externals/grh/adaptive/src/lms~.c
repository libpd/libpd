/******************************************************
 *
 * Adaptive Systems for PD
 *
 * copyleft (c) Gerda Strobl, Georg Holzmann
 * 2005
 *
 * for complaints, suggestions: grh@mur.at
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#include "adaptive.h"


/* ------------------------ lms~ ------------------------- */

static t_class *lms_tilde_class;

typedef struct _lms
{
  t_object x_obj;
  t_float f;
  t_sample *buf;
  t_sample *tmp;
  t_int bufsize;
  int adapt; // enable/disable adaptation
  
  t_int N; //number of coefficients of the adaptive system
  t_float *c; // coefficients of the system
  t_float mu; // step-size parameter (learning rate)

  t_canvas *x_canvas;
} t_lms_tilde;

static void lms_tilde_a(t_lms_tilde *x, t_floatarg f)
{
  x->adapt = (f==0) ? 0 : 1;
}

static void lms_tilde_geta(t_lms_tilde *x)
{
  if(x->adapt==0)
    post("lms~: adaptation is currently OFF");
  else
    post("lms~: adaptation is currently ON");
}

static void lms_tilde_mu(t_lms_tilde *x, t_floatarg f)
{
  x->mu = f;
}

static void lms_tilde_getmu(t_lms_tilde *x)
{
  post("mu (step-size parameter): %f", x->mu);
}

static void lms_tilde_getN(t_lms_tilde *x)
{
  post("N (number of coefficients): %d", x->N);
}

static void lms_tilde_clear(t_lms_tilde *x)
{
  int i;
  
  // clear coefficients
  for(i=0; i<x->N; i++)
    x->c[i] = 0;
  
  // clear temp buffer
  for(i=0; i<x->N-1; i++)
    x->buf[i] = 0;
}

static void lms_tilde_init(t_lms_tilde *x)
{
  int i;
  
  // set the first coefficient to 1, all others to 0
  // so this is a delay free transmission
  x->c[0] = 1;
  for(i=1; i<x->N; i++)
    x->c[i] = 0;
  
  // clear temp buffers
  for(i=0; i<x->N-1; i++)
    x->buf[i] = 0;
}

static void lms_tilde_print(t_lms_tilde *x)
{
  int i;
  
  // print coefficients
  post("\nNr. of coefficients: %d",x->N);
  post("coefficients:");
  for(i=0; i<x->N; i++)
    post("\t%d: %f",i,x->c[i]);
}

static void lms_tilde_write(t_lms_tilde *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  // save to file
  adaptation_write(filename, x->N, x->mu, x->c);
}

static void lms_tilde_read(t_lms_tilde *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);
  
  // read file
  adaptation_read(filename, &x->N, &x->mu, x->c, x->buf);
}

static t_int *lms_tilde_perform(t_int *w)
{
  t_lms_tilde *x = (t_lms_tilde *)(w[1]);
  t_sample *x_ = (t_sample *)(w[2]);
  t_sample *d_ = (t_sample *)(w[3]);
  t_sample *y_ = (t_sample *)(w[4]);
  int n = (int)(w[5]);
  int i, j, tmp;
  t_sample e=0;
  
  
  for(i=0; i<n; i++)
  {
    // calc output (filter)
    
    x->tmp[i]=0;
    
    // y_[i] += x->c[j] * x_[i-j];
    // so lets split in two halfs, so that
    // negative indezes get samples from the
    // last audioblock (x->buf) ...
    tmp = (i+1 - x->N)*(-1);
    tmp = tmp<0 ? 0 : tmp;
    
    for(j=0; j<x->N-tmp; j++)
      x->tmp[i] += x->c[j] * x_[i-j];
    
    for(j=x->N-tmp; j<x->N; j++)
      x->tmp[i] += x->c[j] * x->buf[(i-j)*(-1)-1];
    
    if(x->adapt)
    {
      // error computation
      e = d_[i] - x->tmp[i];
        
      // coefficient adaptation
      // (split in the same way as above)
    
      for(j=0; j<x->N-tmp; j++)
        x->c[j] = x->c[j] + x->mu * x_[i-j] * e;
    
      for(j=x->N-tmp; j<x->N; j++)
        x->c[j] = x->c[j] + x->mu * x->buf[(i-j)*(-1)-1] * e;
    }
    
    //post("%d: in %f, d: %f, out: %f, error: %f, c1:%f, c2:%f", i, x_[i], d_[i], x->tmp[i], e, x->c[0], x->c[1]);
  }

  // store last samples for next audiobuffer
  for(i=0; i<x->N-1; i++)
    x->buf[i] = x_[n-1-i];
  
  // now write tmp to outlet
  while(n--)
    y_[n] = x->tmp[n];
   
  return (w+6);
}

static void lms_tilde_dsp(t_lms_tilde *x, t_signal **sp)
{
  // allocate new temp buffer if buffersize changes
  if(x->bufsize != sp[0]->s_n)
  {
    if(sp[0]->s_n < x->N)
      post("lms~ WARNING: buffersize must be bigger than N, you will get wrong results !!!");
    
    if(x->tmp) freebytes(x->tmp, sizeof(t_sample) * x->bufsize);
    x->tmp = (t_sample *)getbytes(sizeof(t_sample) * sp[0]->s_n);
    
    x->bufsize =  sp[0]->s_n;
  }

  dsp_add(lms_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, 
	  sp[2]->s_vec, sp[0]->s_n);
}

static void lms_tilde_helper(void)
{
  post("\nlms~: Adaptive transversal filter using LMS");
  post("INPUT:");
  post("\tinlet1: input signal x[n]");
  post("\tinlet2: desired output signal d[n]");
  post("\tinit_arg1: number of coefficients of the adaptive system");
  post("\tinit_arg2, mu: step-size parameter (learning rate)");
  post("OUTPUT:");
  post("\toutlet1: output signal\n");
}

static void *lms_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_lms_tilde *x = (t_lms_tilde *)pd_new(lms_tilde_class);
  int i;
  
  // default values:
  x->N = 8;
  x->mu = 0.05;
  x->adapt = 0;
  x->tmp = NULL;
  x->bufsize = 0;
  
  switch(argc)
  {
    case 2:
      x->mu = atom_getfloat(argv+1);
    case 1:
      x->N = atom_getint(argv);
      x->N = (x->N<=0) ? 1 : x->N;
  }
  
  // allocate mem and init coefficients
  x->c = (t_float *)getbytes(sizeof(t_float) * x->N);
  for(i=0; i<x->N; i++)
    x->c[i] = 0;
  
  // allocate mem for temp buffer
  x->buf = (t_sample *)getbytes(sizeof(t_sample) * x->N-1);
  for(i=0; i<x->N-1; i++)
    x->buf[i] = 0;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->x_canvas = canvas_getcurrent();

  return (x);
}

static void lms_tilde_free(t_lms_tilde *x)
{
  if(x->c) freebytes(x->c, sizeof(t_float) * x->N);
  if(x->buf) freebytes(x->buf, sizeof(t_sample) * x->N-1);
  if(x->tmp) freebytes(x->tmp, sizeof(t_sample) * x->bufsize);
}

void lms_tilde_setup(void)
{
  lms_tilde_class = class_new(gensym("lms~"), (t_newmethod)lms_tilde_new, 
                    (t_method)lms_tilde_free, sizeof(t_lms_tilde), 
                    CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(lms_tilde_class, (t_method)lms_tilde_a,
                  gensym("adaptation"), A_DEFFLOAT, 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_geta,
                  gensym("getadaptation"), 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_mu, 
                  gensym("mu"), A_DEFFLOAT, 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_getmu,
                  gensym("getmu"), 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_getN,
                  gensym("getN"), 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_init,
                  gensym("init_unity"), 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_clear,
                  gensym("clear"), 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_print,
                  gensym("print"), 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_write, 
                  gensym("write"), A_DEFSYMBOL, 0);
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_read, 
                  gensym("read"), A_DEFSYMBOL, 0);

  class_addmethod(lms_tilde_class, (t_method)lms_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(lms_tilde_class, t_lms_tilde, f);
  
  class_addmethod(lms_tilde_class, (t_method)lms_tilde_helper, gensym("help"), 0);
}
