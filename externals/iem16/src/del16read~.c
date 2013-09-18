/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_delay.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* del16read~, del16write~, vd16~ */

#include "iem16_delay.h"

/* ----------------------------- del16read~ ----------------------------- */
static t_class *sigdel16read_class;

typedef struct _sigdel16read{
  t_object x_obj;
  t_symbol *x_sym;
  t_float x_deltime;	/* delay in msec */
  int x_delsamps; 	/* delay in samples */
  t_float x_sr;   	/* samples per msec */
  t_float x_n;   	/* vector size */
  int x_zerodel;  	/* 0 or vecsize depending on read/write order */
} t_sigdel16read;

static void sigdel16read_16bit(t_sigdel16read *x, t_float f);

static void *sigdel16read_new(t_symbol *s, t_floatarg f){
  t_sigdel16read *x = (t_sigdel16read *)pd_new(sigdel16read_class);
  x->x_sym = s;
  x->x_sr = 1;
  x->x_n = 1;
  x->x_zerodel = 0;
  sigdel16read_16bit(x, f);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void sigdel16read_16bit(t_sigdel16read *x, t_float f){
  t_sigdel16write *delwriter =
    (t_sigdel16write *)pd_findbyclass(x->x_sym, sigdel16write_class);
  x->x_deltime = f;
  if (delwriter)    {
    x->x_delsamps = (int)(0.5 + x->x_sr * x->x_deltime)
      + x->x_n - x->x_zerodel;
    if (x->x_delsamps < x->x_n) x->x_delsamps = x->x_n;
    else if (x->x_delsamps > delwriter->x_cspace.c_n - DEFDELVS)
      x->x_delsamps = delwriter->x_cspace.c_n - DEFDELVS;
  }
}

static t_int *sigdel16read_perform(t_int *w){
  t_float *out = (t_float *)(w[1]);
  t_del16writectl *c = (t_del16writectl *)(w[2]);
  int delsamps = *(int *)(w[3]);
  int n = (int)(w[4]);
  int phase = c->c_phase - delsamps, nsamps = c->c_n;
  t_iem16_16bit *vp = c->c_vec, *bp, *ep = vp + (c->c_n + XTRASAMPS);

  if (phase < 0) phase += nsamps;
  bp = vp + phase;
  while (n--)    {
    *out++ = *bp++*IEM16_SCALE_DOWN;
    if (bp == ep) bp -= nsamps;
  }
  return (w+5);
}

static void sigdel16read_dsp(t_sigdel16read *x, t_signal **sp){
  t_sigdel16write *delwriter =
    (t_sigdel16write *)pd_findbyclass(x->x_sym, sigdel16write_class);
  x->x_sr = sp[0]->s_sr * 0.001;
  x->x_n = sp[0]->s_n;
  if (delwriter)    {
    sigdel16write_checkvecsize(delwriter, sp[0]->s_n);
    x->x_zerodel = (delwriter->x_sortno == ugen_getsortno() ?
		    0 : delwriter->x_vecsize);
    sigdel16read_16bit(x, x->x_deltime);
    dsp_add(sigdel16read_perform, 4,
    	    sp[0]->s_vec, &delwriter->x_cspace, &x->x_delsamps, sp[0]->s_n);
  }
  else if (*x->x_sym->s_name)
    error("delread~: %s: no such delwrite~",x->x_sym->s_name);
}

static void sigdel16read_setup(void){
  sigdel16read_class = class_new(gensym("del16read~"),
				 (t_newmethod)sigdel16read_new, 0,
				 sizeof(t_sigdel16read), 0, A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(sigdel16read_class, (t_method)sigdel16read_dsp,
		  gensym("dsp"), 0);
  class_addfloat(sigdel16read_class, (t_method)sigdel16read_16bit);
}

// G.Holzmann: for PD-extended build system
void del16read_tilde_setup(void)
{
  sigdel16read_setup();
}
