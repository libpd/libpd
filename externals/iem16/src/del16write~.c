/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_delay.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* del16read~, del16write~, vd16~ */

#include "iem16_delay.h"



/* ----------------------------- del16write~ ----------------------------- */

/* routine to check that all del16writes/del16reads/vds have same vecsize */
void sigdel16write_checkvecsize(t_sigdel16write *x, int vecsize){
  if (x->x_rsortno != ugen_getsortno())    {
    x->x_vecsize = vecsize;
    x->x_rsortno = ugen_getsortno();
  }
  else if (vecsize != x->x_vecsize)
    pd_error(x, "del16read/del16write/vd vector size mismatch");
}

static void *sigdel16write_new(t_symbol *s, t_floatarg msec){
  int nsamps;
  t_sigdel16write *x = (t_sigdel16write *)pd_new(sigdel16write_class);
  if (!*s->s_name) s = gensym("del16write~");
  pd_bind(&x->x_obj.ob_pd, s);
  x->x_sym = s;
  nsamps = msec * sys_getsr() * (float)(0.001f);
  if (nsamps < 1) nsamps = 1;
  nsamps += ((- nsamps) & (SAMPBLK - 1));
  nsamps += DEFDELVS;
  x->x_cspace.c_n = nsamps;
  x->x_cspace.c_vec =
    (t_iem16_16bit *)getbytes((nsamps + XTRASAMPS) * sizeof(t_iem16_16bit));
  x->x_cspace.c_phase = XTRASAMPS;
  x->x_sortno = 0;
  x->x_vecsize = 0;
  x->x_f = 0;
  return (x);
}

static t_int *sigdel16write_perform(t_int *w){
  t_float *in = (t_float *)(w[1]);
  t_del16writectl *c = (t_del16writectl *)(w[2]);
  int n = (int)(w[3]);
  int phase = c->c_phase, nsamps = c->c_n;
  t_iem16_16bit *vp = c->c_vec, *bp = vp + phase, *ep = vp + (c->c_n + XTRASAMPS);
  phase += n;
  while (n--)    {
    *bp++ = (*in++*IEM16_SCALE_UP);
    if (bp == ep)  	{
      vp[0] = ep[-4];
      vp[1] = ep[-3];
      vp[2] = ep[-2];
      vp[3] = ep[-1];
      bp = vp + XTRASAMPS;
      phase -= nsamps;
    }
  }
  c->c_phase = phase; 
  return (w+4);
}

static void sigdel16write_dsp(t_sigdel16write *x, t_signal **sp){
  dsp_add(sigdel16write_perform, 3, sp[0]->s_vec, &x->x_cspace, sp[0]->s_n);
  x->x_sortno = ugen_getsortno();
  sigdel16write_checkvecsize(x, sp[0]->s_n);
}

static void sigdel16write_free(t_sigdel16write *x){
  pd_unbind(&x->x_obj.ob_pd, x->x_sym);
  freebytes(x->x_cspace.c_vec,
	    (x->x_cspace.c_n + XTRASAMPS) * sizeof(t_iem16_16bit));
}

static void sigdel16write_setup(void){
  sigdel16write_class = class_new(gensym("del16write~"), 
				  (t_newmethod)sigdel16write_new, (t_method)sigdel16write_free,
				  sizeof(t_sigdel16write), 0, A_DEFSYM, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(sigdel16write_class, t_sigdel16write, x_f);
  class_addmethod(sigdel16write_class, (t_method)sigdel16write_dsp,
		  gensym("dsp"), 0);
}

// G.Holzmann: for PD-extended build system
void del16write_tilde_setup(void)
{
  sigdel16write_setup();
}

