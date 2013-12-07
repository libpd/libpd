/* copyleft (c) 2003 forum::f�r::uml�ute -- IOhannes m zm�lnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"

/* ------------------------ tab16receive~ ------------------------- */

static t_class *tab16receive_class;

typedef struct _tab16receive{
  t_object x_obj;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
} t_tab16receive;

static t_int *tab16receive_perform(t_int *w){
  t_tab16receive *x = (t_tab16receive *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3];
  t_iem16_16bit *from = x->x_vec;
  if (from) while (n--) *out++ = *from++*IEM16_SCALE_DOWN;
  else while (n--) *out++ = 0;
  return (w+4);
}

static void tab16receive_dsp(t_tab16receive *x, t_signal **sp){
  t_table16 *a;
  int vecsize;
    
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class))) {
    if (*x->x_arrayname->s_name)
      error("tab16send~: %s: no such array", x->x_arrayname->s_name);
  }
  else if (!table16_getarray16(a, &vecsize, &x->x_vec))
    error("%s: bad template for tab16receive~", x->x_arrayname->s_name);
  else     {
    int n = sp[0]->s_n;
    if (n < vecsize) vecsize = n;
    table16_usedindsp(a);
    dsp_add(tab16receive_perform, 3, x, sp[0]->s_vec, vecsize);
  }
}

static void *tab16receive_new(t_symbol *s){
  t_tab16receive *x = (t_tab16receive *)pd_new(tab16receive_class);
  x->x_arrayname = s;
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void tab16receive_setup(void){
  tab16receive_class = class_new(gensym("tab16receive~"),
				 (t_newmethod)tab16receive_new, 0,
				 sizeof(t_tab16receive), 0, A_DEFSYM, 0);
  class_addmethod(tab16receive_class, (t_method)tab16receive_dsp,
		  gensym("dsp"), 0);
}

// G.Holzmann: for PD-extended build system
void tab16receive_tilde_setup(void)
{
  tab16receive_setup();
}
