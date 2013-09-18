/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* --------------------------------- add2_comma ----------------------------------- */
/* -- a special add2-message for messageboxes, which append a comma to a message -- */

static t_class *add2_comma_class;

typedef struct _add2_comma
{
  t_object   x_obj;
  int        x_size;
  t_atom     *x_at;
  t_symbol   *x_sym;
  t_atomtype x_type;
} t_add2_comma;

static void add2_comma_bang(t_add2_comma *x)
{
  SETCOMMA(x->x_at);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, x->x_at);
}

static void add2_comma_float(t_add2_comma *x, t_floatarg f)
{
  SETCOMMA(x->x_at);
  SETFLOAT(x->x_at+1, f);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 2, x->x_at);
}

static void add2_comma_symbol(t_add2_comma *x, t_symbol *s)
{
  SETCOMMA(x->x_at);
  SETSYMBOL(x->x_at+1, s);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 2, x->x_at);
}

/*static void add2_comma_pointer(t_add2_comma *x, t_gpointer *gp)
{
if(!x->x_at)
{
x->x_n = 1;
x->x_at = (t_atom *)getbytes(sizeof(t_atom));
}
x->x_ac = 1;
SETPOINTER(x->x_at, gp);
x->x_sym = &s_pointer;
outlet_pointer(x->x_obj.ob_outlet, gp);
}*/

static void add2_comma_list(t_add2_comma *x, t_symbol *s, int ac, t_atom *av)
{
  int i;
  
  if((ac+1) > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (ac+2)*sizeof(t_atom));
    x->x_size = ac+2;
  }
  SETCOMMA(x->x_at);
  for(i=1; i<=ac; i++)
    x->x_at[i] = av[i-1];
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, ac+1, x->x_at);
}

static void add2_comma_anything(t_add2_comma *x, t_symbol *s, int ac, t_atom *av)
{
  int i;
  
  if((ac+2) > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (ac+3)*sizeof(t_atom));
    x->x_size = ac+3;
  }
  SETCOMMA(x->x_at);
  SETSYMBOL(x->x_at+1, s);
  for(i=1; i<=ac; i++)
    x->x_at[i+1] = av[i-1];
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, ac+2, x->x_at);
}

static void add2_comma_free(t_add2_comma *x)
{
  if(x->x_at)
    freebytes(x->x_at, x->x_size * sizeof(t_atom));
}

static void *add2_comma_new(void)
{
  t_add2_comma *x = (t_add2_comma *)pd_new(add2_comma_class);
  
  x->x_size = 10;
  x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
  x->x_sym = gensym("add2");
  outlet_new(&x->x_obj, &s_list);
  return(x);
}

void add2_comma_setup(void)
{
  add2_comma_class = class_new(gensym("add2_comma"), (t_newmethod)add2_comma_new,
    (t_method)add2_comma_free, sizeof(t_add2_comma), 0, 0);
  class_addbang(add2_comma_class, (t_method)add2_comma_bang);
  class_addanything(add2_comma_class, add2_comma_anything);
  class_addlist(add2_comma_class, add2_comma_list);
  /*class_addpointer(add2_comma_class, add2_comma_pointer);*/
  class_addfloat(add2_comma_class, (t_method)add2_comma_float);
  class_addsymbol(add2_comma_class, add2_comma_symbol);
//  class_sethelpsymbol(add2_comma_class, gensym("iemhelp/help-add2_comma"));
}
