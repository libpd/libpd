/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* --------- gate ---------------------- */
/* ----------- like spigot  ------------ */

typedef struct _gate
{
  t_object  x_obj;
  t_float   x_state;
} t_gate;

static t_class *gate_class;

static void gate_bang(t_gate *x)
{
  if(x->x_state != 0)
    outlet_bang(x->x_obj.ob_outlet);
}

static void gate_pointer(t_gate *x, t_gpointer *gp)
{
  if(x->x_state != 0)
    outlet_pointer(x->x_obj.ob_outlet, gp);
}

static void gate_float(t_gate *x, t_floatarg f)
{
  if(x->x_state != 0)
    outlet_float(x->x_obj.ob_outlet, f);
}

static void gate_symbol(t_gate *x, t_symbol *s)
{
  if(x->x_state != 0)
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void gate_list(t_gate *x, t_symbol *s, int argc, t_atom *argv)
{
  if(x->x_state != 0)
    outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

static void gate_anything(t_gate *x, t_symbol *s, int argc, t_atom *argv)
{
  if(x->x_state != 0)
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void *gate_new(t_floatarg f)
{
  t_gate *x = (t_gate *)pd_new(gate_class);
  floatinlet_new(&x->x_obj, &x->x_state);
  outlet_new(&x->x_obj, 0);
  x->x_state = (f==0.0f)?0.0f:1.0f;
  return (x);
}

void gate_setup(void)
{
  gate_class = class_new(gensym("gate"), (t_newmethod)gate_new, 0,
    sizeof(t_gate), 0, A_DEFFLOAT, 0);
  class_addbang(gate_class, gate_bang);
  class_addpointer(gate_class, gate_pointer);
  class_addfloat(gate_class, gate_float);
  class_addsymbol(gate_class, gate_symbol);
  class_addlist(gate_class, gate_list);
  class_addanything(gate_class, gate_anything);
//  class_sethelpsymbol(gate_class, gensym("iemhelp/help-gate"));
}
