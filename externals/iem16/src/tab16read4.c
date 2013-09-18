/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16read, tab16read4, tab16write */

#include "iem16_table.h"

/* ---------- tab16read4: control, non-interpolating ------------------------ */

static t_class *tab16read4_class;

typedef struct _tab16read4{
  t_object x_obj;
  t_symbol *x_arrayname;
} t_tab16read4;

static void tab16read4_float(t_tab16read4 *x, t_float f){
  t_table16 *a;
  int npoints;
  t_iem16_16bit *vec;

  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!table16_getarray16(a, &npoints, &vec))
    error("%s: bad template for tab16read4", x->x_arrayname->s_name);
  else if (npoints < 4)
    outlet_float(x->x_obj.ob_outlet, 0);
  else if (f <= 1)
    outlet_float(x->x_obj.ob_outlet, vec[1]);
  else if (f >= npoints - 2)
    outlet_float(x->x_obj.ob_outlet, vec[npoints - 2]);
  else    {
    int n = f;
    float a, b, c, d, cminusb, frac;
    t_iem16_16bit *fp;
    if (n >= npoints - 2) n = npoints - 3;
    fp = vec + n;
    frac = f - n;
    a = fp[-1];
    b = fp[0];
    c = fp[1];
    d = fp[2];
    cminusb = c-b;
    outlet_float(x->x_obj.ob_outlet, b + frac * (
						 cminusb - 0.5f * (frac-1.) * (
									       (a - d + 3.0f * cminusb) * frac + (b - a - cminusb))));
  }
}

static void tab16read4_set(t_tab16read4 *x, t_symbol *s){
  x->x_arrayname = s;
}

static void *tab16read4_new(t_symbol *s){
  t_tab16read4 *x = (t_tab16read4 *)pd_new(tab16read4_class);
  x->x_arrayname = s;
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

void tab16read4_setup(void){
  tab16read4_class = class_new(gensym("tab16read4"), (t_newmethod)tab16read4_new,
			       0, sizeof(t_tab16read4), 0, A_DEFSYM, 0);
  class_addfloat(tab16read4_class, (t_method)tab16read4_float);
  class_addmethod(tab16read4_class, (t_method)tab16read4_set, gensym("set"),
		  A_SYMBOL, 0);
}
