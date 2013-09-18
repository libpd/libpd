/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16read, tab16read4, tab16write */

#include "iem16_table.h"

/* ------------------ tab16write: control ------------------------ */

static t_class *tab16write_class;

typedef struct _tab16write {
  t_object x_obj;
  t_symbol *x_arrayname;
  float x_ft1;
  int x_set;
} t_tab16write;

static void tab16write_float(t_tab16write *x, t_float f) {
  int vecsize;
  t_table16 *a;
  t_iem16_16bit *vec;

  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!table16_getarray16(a, &vecsize, &vec))
    error("%s: bad template for tab16write", x->x_arrayname->s_name);
  else    {
    int n = x->x_ft1;
    if (n < 0) n = 0;
    else if (n >= vecsize) n = vecsize-1;
    vec[n] = f;
  }
}

static void tab16write_set(t_tab16write *x, t_symbol *s){
  x->x_arrayname = s;
}

static void tab16write_free(t_tab16write *x){}

static void *tab16write_new(t_symbol *s){
  t_tab16write *x = (t_tab16write *)pd_new(tab16write_class);
  x->x_ft1 = 0;
  x->x_arrayname = s;
  floatinlet_new(&x->x_obj, &x->x_ft1);
  return (x);
}

void tab16write_setup(void){
  tab16write_class = class_new(gensym("tab16write"), (t_newmethod)tab16write_new,
			       (t_method)tab16write_free, sizeof(t_tab16write), 0, A_DEFSYM, 0);
  class_addfloat(tab16write_class, (t_method)tab16write_float);
  class_addmethod(tab16write_class, (t_method)tab16write_set, gensym("set"), A_SYMBOL, 0);
}
