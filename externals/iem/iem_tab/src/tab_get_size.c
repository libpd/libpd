/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"

/* -------------------------- tab_get_size ------------------------------ */

typedef struct _tab_get_size
{
  t_object  x_obj;
  t_symbol  *x_sym_src;
} t_tab_get_size;

static t_class *tab_get_size_class;

static void tab_get_size_set(t_tab_get_size *x, t_symbol *s)
{
  x->x_sym_src = s;
}

static void tab_get_size_bang(t_tab_get_size *x)
{
  int ok_src, size_src;
  iemarray_t *beg_mem_src;
  
  ok_src = iem_tab_check_arrays(gensym("tab_get_size"), x->x_sym_src, &beg_mem_src, &size_src, 0);
  
  if(ok_src)
  {
    outlet_float(x->x_obj.ob_outlet, (t_float)size_src);
  }
}

static void tab_get_size_free(t_tab_get_size *x)
{
}

static void *tab_get_size_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_get_size *x = (t_tab_get_size *)pd_new(tab_get_size_class);
  t_symbol  *src;
  
  if((argc >= 1) &&
    IS_A_SYMBOL(argv,0))
  {
    src = (t_symbol *)atom_getsymbolarg(0, argc, argv);
  }
  else
  {
    post("tab_get_size-ERROR: need 1 symbol argument:");
    post("  destination_array_name");
    return(0);
  }
  
  x->x_sym_src = src;
  outlet_new(&x->x_obj, &s_float);
  return(x);
}

void tab_get_size_setup(void)
{
  tab_get_size_class = class_new(gensym("tab_get_size"), (t_newmethod)tab_get_size_new, (t_method)tab_get_size_free,
    sizeof(t_tab_get_size), 0, A_GIMME, 0);
  class_addbang(tab_get_size_class, (t_method)tab_get_size_bang);
  class_addmethod(tab_get_size_class, (t_method)tab_get_size_set, gensym("set"), A_DEFSYMBOL, 0);
  class_addmethod(tab_get_size_class, (t_method)tab_get_size_set, gensym("get"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_get_size_class, gensym("iemhelp2/tab_get_size-help"));
}
