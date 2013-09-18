/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16read, tab16read4, tab16write */

#include "iem16_table.h"

/* ---------- tab16read: control, non-interpolating ------------------------ */

static t_class *tab16read_class;

typedef struct _tab16read{
  t_object x_obj;
  t_symbol *x_arrayname;
} t_tab16read;

static void tab16read_float(t_tab16read *x, t_float f){
  t_table16 *a;
  int npoints;
  t_iem16_16bit *vec;

  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!table16_getarray16(a, &npoints, &vec))
    error("%s: bad template for tab16read", x->x_arrayname->s_name);
  else    {
    int n = f;
    if (n < 0) n = 0;
    else if (n >= npoints) n = npoints - 1;
    outlet_float(x->x_obj.ob_outlet, (npoints ? vec[n] : 0));
  }
}

static void tab16read_set(t_tab16read *x, t_symbol *s){
  x->x_arrayname = s;
}

static void *tab16read_new(t_symbol *s){
  t_tab16read *x = (t_tab16read *)pd_new(tab16read_class);
  x->x_arrayname = s;
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

void tab16read_setup(void){
  tab16read_class = class_new(gensym("tab16read"), (t_newmethod)tab16read_new,
			      0, sizeof(t_tab16read), 0, A_DEFSYM, 0);
  class_addfloat(tab16read_class, (t_method)tab16read_float);
  class_addmethod(tab16read_class, (t_method)tab16read_set, gensym("set"),
		  A_SYMBOL, 0);
}
