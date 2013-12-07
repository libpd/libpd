/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_delay.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* del16read~, del16write~, vd16~ */

#include "iem16_delay.h"

/* ----------------------------- vd~ ----------------------------- */
static t_class *sig16vd_class;

typedef struct _sig16vd{
  t_object x_obj;
  t_symbol *x_sym;
  t_float x_sr;   	/* samples per msec */
  int x_zerodel;  	/* 0 or vecsize depending on read/write order */
  float x_f;
} t_sig16vd;

static void *sig16vd_new(t_symbol *s){
  t_sig16vd *x = (t_sig16vd *)pd_new(sig16vd_class);
  if (!*s->s_name) s = gensym("vd~");
  x->x_sym = s;
  x->x_sr = 1;
  x->x_zerodel = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

static t_int *sig16vd_perform(t_int *w){
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_del16writectl *ctl = (t_del16writectl *)(w[3]);
  t_sig16vd *x = (t_sig16vd *)(w[4]);
  int n = (int)(w[5]);

  int nsamps = ctl->c_n;
  float limit = nsamps - n - 1;
  float fn = n-4;
  t_iem16_16bit *vp = ctl->c_vec, *bp, *wp = vp + ctl->c_phase;
  float zerodel = x->x_zerodel;
  while (n--)    {
    float delsamps = x->x_sr * *in++ - zerodel, frac;
    int idelsamps;
    float a, b, c, d, cminusb;
    if (delsamps < 1.00001f) delsamps = 1.00001f;
    if (delsamps > limit) delsamps = limit;
    delsamps += fn;
    fn = fn - 1.0f;
    idelsamps = delsamps;
    frac = delsamps - (float)idelsamps;
    bp = wp - (idelsamps + 3);
    if (bp < vp + 4) bp += nsamps;
    d = bp[-3]*IEM16_SCALE_DOWN;
    c = bp[-2]*IEM16_SCALE_DOWN;
    b = bp[-1]*IEM16_SCALE_DOWN;
    a = bp[00]*IEM16_SCALE_DOWN;
    cminusb = c-b;
    *out++ = b + frac * (
			 cminusb - 0.5f * (frac-1.) * (
						       (a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
						       )
			 );
  }
  return (w+6);
}

static void sig16vd_dsp(t_sig16vd *x, t_signal **sp){
  t_sigdel16write *delwriter =
    (t_sigdel16write *)pd_findbyclass(x->x_sym, sigdel16write_class);
  x->x_sr = sp[0]->s_sr * 0.001;
  if (delwriter)    {
    sigdel16write_checkvecsize(delwriter, sp[0]->s_n);
    x->x_zerodel = (delwriter->x_sortno == ugen_getsortno() ?
		    0 : delwriter->x_vecsize);
    dsp_add(sig16vd_perform, 5,
    	    sp[0]->s_vec, sp[1]->s_vec,
	    &delwriter->x_cspace, x, sp[0]->s_n);
  }
  else error("vd~: %s: no such delwrite~",x->x_sym->s_name);
}

static void sig16vd_setup(void){
  sig16vd_class = class_new(gensym("vd16~"), (t_newmethod)sig16vd_new, 0,
			    sizeof(t_sig16vd), 0, A_DEFSYM, 0);
  class_addmethod(sig16vd_class, (t_method)sig16vd_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sig16vd_class, t_sig16vd, x_f);
}

// G.Holzmann: for PD-extended build system
void vd16_tilde_setup(void)
{
  sig16vd_setup();
}
