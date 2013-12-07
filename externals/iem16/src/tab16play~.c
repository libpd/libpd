/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"


/* ------------ tab16play~ - non-transposing sample playback --------------- */

static t_class *tab16play_tilde_class;

typedef struct _tab16play_tilde{
  t_object x_obj;
  t_outlet *x_bangout;
  int x_phase;
  int x_nsampsintab;
  int x_limit;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
} t_tab16play_tilde;

static void *tab16play_tilde_new(t_symbol *s){
  t_tab16play_tilde *x = (t_tab16play_tilde *)pd_new(tab16play_tilde_class);
  x->x_phase = 0x7fffffff;
  x->x_limit = 0;
  x->x_arrayname = s;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_bangout = outlet_new(&x->x_obj, gensym("bang"));
  return (x);
}

static t_int *tab16play_tilde_perform(t_int *w){
  t_tab16play_tilde *x = (t_tab16play_tilde *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_iem16_16bit *fp;
  int n = (int)(w[3]), phase = x->x_phase,
    endphase = (x->x_nsampsintab < x->x_limit ?
		x->x_nsampsintab : x->x_limit), nxfer, n3;
  if (!x->x_vec || phase >= endphase)	goto zero;
    
  nxfer = endphase - phase;
  fp = x->x_vec + phase;
  if (nxfer > n)
    nxfer = n;
  n3 = n - nxfer;
  phase += nxfer;
  while (nxfer--) *out++ = *fp++*IEM16_SCALE_DOWN;
  if (phase >= endphase)  {
    x->x_phase = 0x7fffffff;
    while (n3--) *out++ = 0;
  }
  else x->x_phase = phase;
    
  return (w+4);
 zero:
  while (n--) *out++ = 0;
  return (w+4);
}

void tab16play_tilde_set(t_tab16play_tilde *x, t_symbol *s){
  t_table16 *a;

  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))    {
    if (*s->s_name) pd_error(x, "tab16play~: %s: no such array",
			     x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_nsampsintab, &x->x_vec))    {
    error("%s: bad template for tab16play~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16play_tilde_dsp(t_tab16play_tilde *x, t_signal **sp){
  tab16play_tilde_set(x, x->x_arrayname);
  dsp_add(tab16play_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void tab16play_tilde_list(t_tab16play_tilde *x, t_symbol *s,
				 int argc, t_atom *argv){
  long start = atom_getfloatarg(0, argc, argv);
  long length = atom_getfloatarg(1, argc, argv);
  if (start < 0) start = 0;
  if (length <= 0)x->x_limit = 0x7fffffff;
  else	    x->x_limit = start + length;
  x->x_phase = start;
}

static void tab16play_tilde_stop(t_tab16play_tilde *x){
  x->x_phase = 0x7fffffff;
}

static void tab16play_tilde_free(t_tab16play_tilde *x){}

void tab16play_tilde_setup(void){
  tab16play_tilde_class = class_new(gensym("tab16play~"),
				    (t_newmethod)tab16play_tilde_new, (t_method)tab16play_tilde_free,
				    sizeof(t_tab16play_tilde), 0, A_DEFSYM, 0);
  class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_stop,
		  gensym("stop"), 0);
  class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_set,
		  gensym("set"), A_DEFSYM, 0);
  class_addlist(tab16play_tilde_class, tab16play_tilde_list);
}
