/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"


/******************** tab16read~ ***********************/

static t_class *tab16read_tilde_class;

typedef struct _tab16read_tilde{
  t_object x_obj;
  int x_npoints;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16read_tilde;

static void *tab16read_tilde_new(t_symbol *s){
  t_tab16read_tilde *x = (t_tab16read_tilde *)pd_new(tab16read_tilde_class);
  x->x_arrayname = s;
  x->x_vec = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

static t_int *tab16read_tilde_perform(t_int *w){
  t_tab16read_tilde *x = (t_tab16read_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);    
  int maxindex;
  t_iem16_16bit *buf = x->x_vec;
  int i;
    
  maxindex = x->x_npoints - 1;
  if (!buf) goto zero;

  for (i = 0; i < n; i++)    {
    int index = *in++;
    if (index < 0) index = 0;
    else if (index > maxindex) index = maxindex;
    *out++ = buf[index]*IEM16_SCALE_DOWN;
  }
  return (w+5);
 zero:
  while (n--) *out++ = 0;

  return (w+5);
}

void tab16read_tilde_set(t_tab16read_tilde *x, t_symbol *s){
  t_table16 *a;
    
  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))  {
    if (*s->s_name)
      error("tab16read~: %s: no such array", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_npoints, &x->x_vec)) {
    error("%s: bad template for tab16read~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16read_tilde_dsp(t_tab16read_tilde *x, t_signal **sp){
  tab16read_tilde_set(x, x->x_arrayname);

  dsp_add(tab16read_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tab16read_tilde_free(t_tab16read_tilde *x){}

void tab16read_tilde_setup(void){
  tab16read_tilde_class = class_new(gensym("tab16read~"),
				    (t_newmethod)tab16read_tilde_new, (t_method)tab16read_tilde_free,
				    sizeof(t_tab16read_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16read_tilde_class, t_tab16read_tilde, x_f);
  class_addmethod(tab16read_tilde_class, (t_method)tab16read_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16read_tilde_class, (t_method)tab16read_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
}
