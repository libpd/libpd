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


/* ------------------------ nlms3~ ------------------------- */

static t_class *nlms3_tilde_class;

typedef struct _nlms3
{
  t_object x_obj;
  t_float f;
  t_atom *coef;
  t_sample *buf;
  t_sample *xbuf;
  t_sample *in_tmp;
  t_sample *y_tmp;
  t_sample *e_tmp;
  t_int bufsize;
  t_outlet *c_out;
  int adapt; // enable/disable adaptation
  
  t_int N; //number of coefficients of the adaptive system
  t_float *c; // coefficients of the system
  t_float mu; // step-size parameter (learning rate)
  t_float alpha; // small constant to avoid division by zero

  t_canvas *x_canvas;
} t_nlms3_tilde;

static void nlms3_tilde_a(t_nlms3_tilde *x, t_floatarg f)
{
  x->adapt = (f==0) ? 0 : 1;
  
  if(!x->adapt)
  {
    int i;
    
    // clear temp buffers
    for(i=0; i<x->N-1; i++)
      x->buf[i] = 0;
    for(i=0; i<x->N-1; i++)
      x->xbuf[i] = 0;
  }
}

static void nlms3_tilde_geta(t_nlms3_tilde *x)
{
  if(x->adapt==0)
    post("nlms3~: adaptation is currently OFF");
  else
    post("nlms3~: adaptation is currently ON");
}

static void nlms3_tilde_mu(t_nlms3_tilde *x, t_floatarg f)
{
  x->mu = f;
}

static void nlms3_tilde_getmu(t_nlms3_tilde *x)
{
  post("mu (step-size parameter): %f", x->mu);
}

static void nlms3_tilde_alpha(t_nlms3_tilde *x, t_floatarg f)
{
  x->alpha = f;
}

static void nlms3_tilde_getalpha(t_nlms3_tilde *x)
{
  post("alpha: %f", x->alpha);
}

static void nlms3_tilde_getN(t_nlms3_tilde *x)
{
  post("N (number of coefficients): %d", x->N);
}

static void nlms3_tilde_clear(t_nlms3_tilde *x)
{
  int i;
  
  // clear coefficients
  for(i=0; i<x->N; i++)
    x->c[i] = 0;
  
  // clear temp buffers
  for(i=0; i<x->N-1; i++)
    x->buf[i] = 0;
  for(i=0; i<x->N-1; i++)
    x->xbuf[i] = 0;
}

static void nlms3_tilde_init(t_nlms3_tilde *x)
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
  for(i=0; i<x->N-1; i++)
    x->xbuf[i] = 0;
}

static void nlms3_tilde_print(t_nlms3_tilde *x)
{
  int i;
  
  // print coefficients
  post("\nNr. of coefficients: %d",x->N);
  post("coefficients:");
  for(i=0; i<x->N; i++)
    post("\t%d: %f",i,x->c[i]);
}

static void nlms3_tilde_write(t_nlms3_tilde *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  // save to file
  adaptation_write(filename, x->N, x->mu, x->c);
}

static void nlms3_tilde_read(t_nlms3_tilde *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  int n = x->N;
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);
  
  // read file
  adaptation_read(filename, &x->N, &x->mu, x->c, x->buf);
  
  // if length changes:
  if(x->N != n)
  {
    if(x->coef) freebytes(x->coef, sizeof(t_atom) * x->N);
    x->coef = (t_atom *)getbytes(sizeof(t_atom) * x->N);
  }
}

static t_int *nlms3_tilde_perform(t_int *w)
{
  t_sample *in_ = (t_sample *)(w[1]);
  t_sample *x_  = (t_sample *)(w[2]);
  t_sample *d_  = (t_sample *)(w[3]);
  t_sample *out_= (t_sample *)(w[4]);
  t_sample *y_  = (t_sample *)(w[5]);
  t_sample *e_  = (t_sample *)(w[6]);
  int n = (int)(w[7]);
  t_nlms3_tilde *x = (t_nlms3_tilde *)(w[8]);
  int i, j, tmp;
  t_sample x_2;
  
  
  // calculate inlet2 (filter+adaptation)
  if(x->adapt)
  {
    for(i=0; i<n; i++)
    {
      x->y_tmp[i]=0;
      x_2=0;
    
      // y_[i] += x->c[j] * x_[i-j];
      // so lets split in two halfs, so that
      // negative indezes get samples from the
      // last audioblock (x->buf) ...
      tmp = (i+1 - x->N)*(-1);
      tmp = tmp<0 ? 0 : tmp;
    
      for(j=0; j<x->N-tmp; j++)
        x->y_tmp[i] += x->c[j] * x_[i-j];
    
      for(j=x->N-tmp; j<x->N; j++)
        x->y_tmp[i] += x->c[j] * x->xbuf[(i-j)*(-1)-1];
    
      // error computation
      x->e_tmp[i] = d_[i] - x->y_tmp[i];
        
      // Normalized LMS Adaptmsation Algorithm
      // (split in the same way as above)
      //
      // c[n] = c[n-1] + mu/(alpha + x'[n]*x[n])*e[n]*x[n]

      // calc x'[n]*x[n]

      for(j=0; j<x->N-tmp; j++)
        x_2 += x_[i-j] * x_[i-j];
      for(j=x->N-tmp; j<x->N; j++)
        x_2 += x->xbuf[(i-j)*(-1)-1] * x->xbuf[(i-j)*(-1)-1];
      
      for(j=0; j<x->N-tmp; j++)
        x->c[j] = x->c[j] + x->mu/(x->alpha+x_2) * x_[i-j] * x->e_tmp[i];
      for(j=x->N-tmp; j<x->N; j++)
        x->c[j] = x->c[j] + x->mu/(x->alpha+x_2) * x->xbuf[(i-j)*(-1)-1] * x->e_tmp[i];
    }
    
    // outlet coefficients
    for(i=0; i<x->N; i++)
      SETFLOAT(&x->coef[i],x->c[i]);
  
    outlet_list(x->c_out, &s_list, x->N, x->coef);
  
    // store last samples for next audiobuffer
    for(i=0; i<x->N-1; i++)
      x->xbuf[i] = x_[n-1-i];
  }
  
  
  // calculate filter output (inlet 1)
  for(i=0; i<n; i++)
  {
    x->in_tmp[i]=0;
    
    // y_[i] += x->c[j] * x_[i-j];
    // so lets split in two halfs, so that
    // negative indezes get samples from the
    // last audioblock (x->buf) ...
    tmp = (i+1 - x->N)*(-1);
    tmp = tmp<0 ? 0 : tmp;
    
    for(j=0; j<x->N-tmp; j++)
      x->in_tmp[i] += x->c[j] * in_[i-j];
    
    for(j=x->N-tmp; j<x->N; j++)
      x->in_tmp[i] += x->c[j] * x->buf[(i-j)*(-1)-1];
  }
  // store last samples for next audiobuffer
  for(i=0; i<x->N-1; i++)
    x->buf[i] = in_[n-1-i];
  
  
  // write to the outlets
  if(x->adapt)
  {
    while(n--)
    {
      out_[n] = x->in_tmp[n];
      y_[n] = x->y_tmp[n];
      e_[n] = x->e_tmp[n];
    }
  }
  else
  {
    while(n--)
    {
      out_[n] = x->in_tmp[n];
      y_[n] = 0;
      e_[n] = 0;
    }
  }

    
  return (w+9);
}

static void nlms3_tilde_dsp(t_nlms3_tilde *x, t_signal **sp)
{
  // allocate new temp buffer if buffersize changes
  if(x->bufsize != sp[0]->s_n)
  {
    if(sp[0]->s_n < x->N)
      post("nlms3~ WARNING: buffersize must be bigger than N, you will get wrong results !!!"); 
    
    if(x->in_tmp) freebytes(x->in_tmp, sizeof(t_sample) * x->bufsize);
    x->in_tmp = (t_sample *)getbytes(sizeof(t_sample) * sp[0]->s_n);
    
    if(x->y_tmp) freebytes(x->y_tmp, sizeof(t_sample) * x->bufsize);
    x->y_tmp = (t_sample *)getbytes(sizeof(t_sample) * sp[0]->s_n);
    
    if(x->e_tmp) freebytes(x->e_tmp, sizeof(t_sample) * x->bufsize);
    x->e_tmp = (t_sample *)getbytes(sizeof(t_sample) * sp[0]->s_n);
    
    x->bufsize =  sp[0]->s_n;
  }

  dsp_add(nlms3_tilde_perform, 8, sp[0]->s_vec, sp[1]->s_vec, 
          sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, 
          sp[5]->s_vec, sp[0]->s_n, x);
}

static void nlms3_tilde_helper(void)
{
  post("\nnlms3~: Adaptive transversal filter using normalized LMS");
  post("INPUT:");
  post("\tinlet1: input signal without adaptation, only filter");
  post("\tinlet2: input signal for adaptation x[n]");
  post("\tinlet3: desired output signal d[n]");
  post("\tinit_arg1: number of coefficients of the adaptive system");
  post("\tinit_arg2, mu: step-size parameter (learning rate)");
  post("OUTPUT:");
  post("\toutlet1: output signal from inlet1");
  post("\toutlet2: output signal from inlet2");
  post("\toutlet3: error signal e[n]");
  post("\toutlet4: coefficients c[n] (only per block)\n");
}

static void *nlms3_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_nlms3_tilde *x = (t_nlms3_tilde *)pd_new(nlms3_tilde_class);
  int i;
  
  // default values:
  x->N = 8;
  x->mu = 0.05;
  x->alpha = 0.000001;
  x->adapt = 0;
  x->in_tmp = NULL;
  x->y_tmp = NULL;
  x->e_tmp = NULL;
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
  
  // allocate mem for temp buffers
  x->buf = (t_sample *)getbytes(sizeof(t_sample) * x->N-1);
  for(i=0; i<x->N-1; i++)
    x->buf[i] = 0;
  x->xbuf = (t_sample *)getbytes(sizeof(t_sample) * x->N-1);
  for(i=0; i<x->N-1; i++)
    x->xbuf[i] = 0;

  // for output atoms (coefficients):
  x->coef = (t_atom *)getbytes(sizeof(t_atom) * x->N);
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->c_out = outlet_new(&x->x_obj, 0);

  x->x_canvas = canvas_getcurrent();

  return (x);
}

static void nlms3_tilde_free(t_nlms3_tilde *x)
{
  if(x->c) freebytes(x->c, sizeof(t_float) * x->N);
  if(x->buf) freebytes(x->buf, sizeof(t_sample) * x->N-1);
  if(x->xbuf) freebytes(x->xbuf, sizeof(t_sample) * x->N-1);
  if(x->in_tmp) freebytes(x->y_tmp, sizeof(t_sample) * x->bufsize);
  if(x->y_tmp) freebytes(x->y_tmp, sizeof(t_sample) * x->bufsize);
  if(x->e_tmp) freebytes(x->e_tmp, sizeof(t_sample) * x->bufsize);
  if(x->coef) freebytes(x->coef, sizeof(t_atom) * x->N);
}

void nlms3_tilde_setup(void)
{
  nlms3_tilde_class = class_new(gensym("nlms3~"), (t_newmethod)nlms3_tilde_new, 
                    (t_method)nlms3_tilde_free, sizeof(t_nlms3_tilde), 
                    CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_a,
                  gensym("adaptation"), A_DEFFLOAT, 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_geta,
                  gensym("getadaptation"), 0);  
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_mu, 
                  gensym("mu"), A_DEFFLOAT, 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_getmu,
                  gensym("getmu"), 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_alpha, 
                  gensym("alpha"), A_DEFFLOAT, 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_getalpha,
                  gensym("getalpha"), 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_getN,
                  gensym("getN"), 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_init,
                  gensym("init_unity"), 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_clear,
                  gensym("clear"), 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_print,
                  gensym("print"), 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_write, 
                  gensym("write"), A_DEFSYMBOL, 0);
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_read, 
                  gensym("read"), A_DEFSYMBOL, 0);

  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(nlms3_tilde_class, t_nlms3_tilde, f);
  
  class_addmethod(nlms3_tilde_class, (t_method)nlms3_tilde_helper, gensym("help"), 0);
}
