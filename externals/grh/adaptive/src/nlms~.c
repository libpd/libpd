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


/* ------------------------ nlms~ ------------------------- */

static t_class *nlms_tilde_class;

typedef struct _nlms
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
  t_float alpha; // small constant to avoid division by zero

  t_canvas *x_canvas;
} t_nlms_tilde;

static void nlms_tilde_a(t_nlms_tilde *x, t_floatarg f)
{
  x->adapt = (f==0) ? 0 : 1;
}

static void nlms_tilde_geta(t_nlms_tilde *x)
{
  if(x->adapt==0)
    post("nlms~: adaptation is currently OFF");
  else
    post("nlms~: adaptation is currently ON");
}

static void nlms_tilde_mu(t_nlms_tilde *x, t_floatarg f)
{
  x->mu = f;
}

static void nlms_tilde_getmu(t_nlms_tilde *x)
{
  post("mu (step-size parameter): %f", x->mu);
}

static void nlms_tilde_alpha(t_nlms_tilde *x, t_floatarg f)
{
  x->alpha = f;
}

static void nlms_tilde_getalpha(t_nlms_tilde *x)
{
  post("alpha: %f", x->alpha);
}

static void nlms_tilde_getN(t_nlms_tilde *x)
{
  post("N (number of coefficients): %d", x->N);
}

static void nlms_tilde_clear(t_nlms_tilde *x)
{
  int i;
  
  // clear coefficients
  for(i=0; i<x->N; i++)
    x->c[i] = 0;
  
  // clear temp buffer
  for(i=0; i<x->N-1; i++)
    x->buf[i] = 0;
}

static void nlms_tilde_init(t_nlms_tilde *x)
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

static void nlms_tilde_print(t_nlms_tilde *x)
{
  int i;
  
  // print coefficients
  post("\nNr. of coefficients: %d",x->N);
  post("coefficients:");
  for(i=0; i<x->N; i++)
    post("\t%d: %f",i,x->c[i]);
}

static void nlms_tilde_write(t_nlms_tilde *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  // save to file
  adaptation_write(filename, x->N, x->mu, x->c);
}

static void nlms_tilde_read(t_nlms_tilde *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);
  
  // read file
  adaptation_read(filename, &x->N, &x->mu, x->c, x->buf);
}

static t_int *nlms_tilde_perform(t_int *w)
{
  t_nlms_tilde *x = (t_nlms_tilde *)(w[1]);
  t_sample *x_ = (t_sample *)(w[2]);
  t_sample *d_ = (t_sample *)(w[3]);
  t_sample *y_ = (t_sample *)(w[4]);
  int n = (int)(w[5]);
  int i, j, tmp;
  t_sample e=0, x_2;
  
  
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
      x_2=0;
      
      // error computation
      e =d_[i] - x->tmp[i];
        
      // Normalized LMS Adaptmsation Algorithm
      // (split in the same way as above)
      //
      // c[n] = c[n-1] + mu/(alpha + x'[n]*x[n])*e[n]*x[n]

      // calc x'[n]*x[n]
      // TODO: Performance Optimization: save results from the past
      // so that this for loop should be obsolet ...
      for(j=0; j<x->N-tmp; j++)
        x_2 += x_[i-j] * x_[i-j];
      for(j=x->N-tmp; j<x->N; j++)
        x_2 += x->buf[(i-j)*(-1)-1] * x->buf[(i-j)*(-1)-1];
      

      for(j=0; j<x->N-tmp; j++)
        x->c[j] = x->c[j] + x->mu/(x->alpha+x_2) * x_[i-j] * e;
    
      for(j=x->N-tmp; j<x->N; j++)
        x->c[j] = x->c[j] + x->mu/(x->alpha+x_2) * x->buf[(i-j)*(-1)-1] * e;
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

static void nlms_tilde_dsp(t_nlms_tilde *x, t_signal **sp)
{
  // allocate new temp buffer if buffersize changes
  if(x->bufsize != sp[0]->s_n)
  {
    if(sp[0]->s_n < x->N)
      post("nlms~ WARNING: buffersize must be bigger than N, you will get wrong results !!!");
    
    if(x->tmp) freebytes(x->tmp, sizeof(t_sample) * x->bufsize);
    x->tmp = (t_sample *)getbytes(sizeof(t_sample) * sp[0]->s_n);
    
    x->bufsize =  sp[0]->s_n;
  }

  dsp_add(nlms_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, 
	  sp[2]->s_vec, sp[0]->s_n);
}

static void nlms_tilde_helper(void)
{
  post("\nnlms~: Adaptive transversal filter using normalized LMS");
  post("INPUT:");
  post("\tinlet1: input signal x[n]");
  post("\tinlet2: desired output signal d[n]");
  post("\tinit_arg1: number of coefficients of the adaptive system");
  post("\tinit_arg2, mu: step-size parameter (learning rate)");
  post("OUTPUT:");
  post("\toutlet1: output signal\n");
}

static void *nlms_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_nlms_tilde *x = (t_nlms_tilde *)pd_new(nlms_tilde_class);
  int i;
  
  // default values:
  x->N = 8;
  x->mu = 0.05;
  x->alpha = 0.0001;
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

static void nlms_tilde_free(t_nlms_tilde *x)
{
  if(x->c) freebytes(x->c, sizeof(t_float) * x->N);
  if(x->buf) freebytes(x->buf, sizeof(t_sample) * x->N-1);
  if(x->tmp) freebytes(x->tmp, sizeof(t_sample) * x->bufsize);
}

void nlms_tilde_setup(void)
{
  nlms_tilde_class = class_new(gensym("nlms~"), (t_newmethod)nlms_tilde_new, 
                    (t_method)nlms_tilde_free, sizeof(t_nlms_tilde), 
                    CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_a,
                  gensym("adaptation"), A_DEFFLOAT, 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_geta,
                  gensym("getadaptation"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_mu, 
                  gensym("mu"), A_DEFFLOAT, 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_getmu,
                  gensym("getmu"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_alpha, 
                  gensym("alpha"), A_DEFFLOAT, 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_getalpha,
                  gensym("getalpha"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_getN,
                  gensym("getN"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_init,
                  gensym("init_unity"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_clear,
                  gensym("clear"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_print,
                  gensym("print"), 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_write, 
                  gensym("write"), A_DEFSYMBOL, 0);
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_read, 
                  gensym("read"), A_DEFSYMBOL, 0);

  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(nlms_tilde_class, t_nlms_tilde, f);
  
  class_addmethod(nlms_tilde_class, (t_method)nlms_tilde_helper, gensym("help"), 0);
}
