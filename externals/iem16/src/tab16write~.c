/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"

/* ------------------------- tab16write~ -------------------------- */

static t_class *tab16write_tilde_class;

typedef struct _tab16write_tilde {
  t_object x_obj;
  int x_phase;
  int x_nsampsintab;
  short *x_vec;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16write_tilde;

static void *tab16write_tilde_new(t_symbol *s) {
  t_tab16write_tilde *x = (t_tab16write_tilde *)pd_new(tab16write_tilde_class);
  x->x_phase = 0x7fffffff;
  x->x_arrayname = s;
  x->x_f = 0;
  return (x);
}

static t_int *tab16write_tilde_perform(t_int *w) {
  t_tab16write_tilde *x = (t_tab16write_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  int n = (int)(w[3]), phase = x->x_phase, endphase = x->x_nsampsintab;
  if (!x->x_vec) goto bad;
    
  if (endphase > phase)    {
    int nxfer = endphase - phase;
    t_iem16_16bit *fp = x->x_vec + phase;
    if (nxfer > n) nxfer = n;
    phase += nxfer;
    while (nxfer--)*fp++ = *in++*IEM16_SCALE_UP;
    x->x_phase = phase;
  }
 bad:
  return (w+4);
}

void tab16write_tilde_set(t_tab16write_tilde *x, t_symbol *s){
  t_table16 *a;

  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))    {
    if (*s->s_name) pd_error(x, "tab16write~: %s: no such array",
			     x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_nsampsintab, &x->x_vec))    {
    error("%s: bad template for tab16write~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16write_tilde_dsp(t_tab16write_tilde *x, t_signal **sp){
  tab16write_tilde_set(x, x->x_arrayname);
  dsp_add(tab16write_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void tab16write_tilde_bang(t_tab16write_tilde *x){
  x->x_phase = 0;
}

static void tab16write_tilde_stop(t_tab16write_tilde *x){}

static void tab16write_tilde_free(t_tab16write_tilde *x){}

void tab16write_tilde_setup(void){
  tab16write_tilde_class = class_new(gensym("tab16write~"),
				     (t_newmethod)tab16write_tilde_new, (t_method)tab16write_tilde_free,
				     sizeof(t_tab16write_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16write_tilde_class, t_tab16write_tilde, x_f);
  class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
  class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_stop,
		  gensym("stop"), 0);
  class_addbang(tab16write_tilde_class, tab16write_tilde_bang);
}

