/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------ wrap ----------------- */
/* -- leave the fractal part of a float message -- */

typedef struct _wrap
{
  t_object  x_obj;
  t_float     x_f;
} t_wrap;

static t_class *wrap_class;

static void wrap_bang(t_wrap *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_f);
}

static void wrap_float(t_wrap *x, t_floatarg f)
{
  int i=(int)f;
  
  if(f > 0.0)
    x->x_f = f - (t_float)i;
  else
    x->x_f = f - (t_float)(i - 1);
  wrap_bang(x);
}

static void wrap_list(t_wrap *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc > 0) && (IS_A_FLOAT(argv, 0)))
    wrap_float(x, atom_getfloat(argv));
}

static void *wrap_new(void)
{
  t_wrap *x = (t_wrap *)pd_new(wrap_class);
  
  outlet_new(&x->x_obj, &s_float);
  x->x_f = 0.0;
  return (x);
}

void wrap_setup(void)
{
  wrap_class = class_new(gensym("wrap"), (t_newmethod)wrap_new, 0,
    sizeof(t_wrap), 0, 0);
  class_addbang(wrap_class, (t_method)wrap_bang);
  class_addfloat(wrap_class, (t_method)wrap_float);
  class_addlist(wrap_class, (t_method)wrap_list);
//  class_sethelpsymbol(wrap_class, gensym("iemhelp/help-wrap"));
}
