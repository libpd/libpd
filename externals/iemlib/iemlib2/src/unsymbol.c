/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* ----------- unsymbol ------------------------- */
/* -- converts a symbol to an anything message -- */

static t_class *unsymbol_class;

typedef struct _unsymbol
{
  t_object x_obj;
  t_atom   x_at;
} t_unsymbol;

static void unsymbol_symbol(t_unsymbol *x, t_symbol *s)
{
  outlet_anything(x->x_obj.ob_outlet, s, 0, &(x->x_at));
}

static void *unsymbol_new(void)
{
  t_unsymbol *x = (t_unsymbol *)pd_new(unsymbol_class);
  
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void unsymbol_setup(void)
{
  unsymbol_class = class_new(gensym("unsymbol"), (t_newmethod)unsymbol_new,
    0, sizeof(t_unsymbol), 0, 0);
  class_addcreator((t_newmethod)unsymbol_new, gensym("unsym"), 0);
  class_addsymbol(unsymbol_class, unsymbol_symbol);
//  class_sethelpsymbol(unsymbol_class, gensym("iemhelp/help-unsymbol"));
}
