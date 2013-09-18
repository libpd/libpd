/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ init ---------------------------- */
/* -------- a combination of loadbang and any --------------- */

static t_class *init_class;

typedef struct _init
{
  t_object   x_obj;
  int        x_n;
  int        x_ac;
  t_atom     *x_at;
  t_symbol   *x_sym;
  t_atomtype x_type;
} t_init;

static void init_bang(t_init *x)
{
  if(x->x_type == A_FLOAT)
    outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->x_at));
  else if(x->x_type == A_SYMBOL)
    outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(x->x_at));
  else if(x->x_type == A_NULL)
    outlet_bang(x->x_obj.ob_outlet);
  else if(x->x_type == A_COMMA)
    outlet_anything(x->x_obj.ob_outlet, x->x_sym, x->x_ac, x->x_at);
  else if(x->x_type == A_GIMME)
    outlet_list(x->x_obj.ob_outlet, &s_list, x->x_ac, x->x_at);
  else if(x->x_type == A_POINTER)
    outlet_pointer(x->x_obj.ob_outlet, (t_gpointer *)x->x_at->a_w.w_gpointer);
}

static void init_loadbang(t_init *x)
{
  if(!sys_noloadbang)
    init_bang(x);
}

static void init_float(t_init *x, t_floatarg f)
{
  x->x_ac = 1;
  SETFLOAT(x->x_at, f);
  x->x_sym = &s_float;
  x->x_type = A_FLOAT;
  outlet_float(x->x_obj.ob_outlet, f);
}

static void init_symbol(t_init *x, t_symbol *s)
{
  x->x_ac = 1;
  SETSYMBOL(x->x_at, s);
  x->x_sym = &s_symbol;
  x->x_type = A_SYMBOL;
  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void init_pointer(t_init *x, t_gpointer *gp)
{
  x->x_ac = 1;
  SETPOINTER(x->x_at, gp);
  x->x_sym = &s_pointer;
  x->x_type = A_POINTER;
  outlet_pointer(x->x_obj.ob_outlet, gp);
}

static void init_list(t_init *x, t_symbol *s, int ac, t_atom *av)
{
  t_atom *at;
  
  if(ac > x->x_n)
  {
    if(x->x_at)
      freebytes(x->x_at, x->x_n * sizeof(t_atom));
    x->x_n = ac;
    x->x_at = (t_atom *)getbytes(x->x_n * sizeof(t_atom));
  }
  x->x_ac = ac;
  x->x_sym = &s_list;
  at = x->x_at;
  while(ac--)
    *at++ = *av++;
  x->x_type = A_GIMME;
  outlet_list(x->x_obj.ob_outlet, &s_list, x->x_ac, x->x_at);
}

static void init_anything(t_init *x, t_symbol *s, int ac, t_atom *av)
{
  t_atom *at;
  
  if(ac > x->x_n)
  {
    if(x->x_at)
      freebytes(x->x_at, x->x_n * sizeof(t_atom));
    x->x_n = ac;
    x->x_at = (t_atom *)getbytes(x->x_n * sizeof(t_atom));
  }
  x->x_ac = ac;
  x->x_sym = s;
  at = x->x_at;
  while(ac--)
    *at++ = *av++;
  x->x_type = A_COMMA;
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, x->x_ac, x->x_at);
}

static void init_free(t_init *x)
{
  if(x->x_at)
    freebytes(x->x_at, x->x_n * sizeof(t_atom));
}

static void *init_new(t_symbol *s, int ac, t_atom *av)
{
  t_init *x = (t_init *)pd_new(init_class);
  int i;
  
  x->x_type = A_NULL;
  if(!ac)
  {
    x->x_type = A_NULL;
    x->x_sym = &s_bang;
    x->x_n = 1;
    x->x_ac = 0;
    x->x_at = (t_atom *)getbytes(x->x_n * sizeof(t_atom));
  }
  else if(ac == 1)
  {
    if(IS_A_SYMBOL(av,0))
    {
      x->x_type = A_COMMA;
      x->x_sym = atom_getsymbol(av);
      x->x_n = 1;
      x->x_ac = 0;
      x->x_at = (t_atom *)getbytes(x->x_n * sizeof(t_atom));
    }
    else
    {
      if(IS_A_FLOAT(av,0))
      {
        x->x_type = A_FLOAT;
        x->x_sym = &s_float;
      }
      else if(IS_A_POINTER(av,0))
      {
        x->x_type = A_POINTER;
        x->x_sym = &s_pointer;
      }
      x->x_n = x->x_ac = 1;
      x->x_at = (t_atom *)getbytes(x->x_n * sizeof(t_atom));
      x->x_at[0] = *av;
    }
  }
  else
  {
    if(IS_A_SYMBOL(av,0))
    {
      x->x_type = A_COMMA;/*for anything*/
      x->x_sym = atom_getsymbol(av++);
      ac--;
    }
    else
    {
      x->x_type = A_GIMME;
      x->x_sym = &s_list;
    }
    x->x_n = x->x_ac = ac;
    x->x_at = (t_atom *)getbytes(x->x_n * sizeof(t_atom));
    for(i=0; i<ac; i++)
      x->x_at[i] = *av++;
  }
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void init_setup(void)
{
  init_class = class_new(gensym("init"), (t_newmethod)init_new,
    (t_method)init_free, sizeof(t_init), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)init_new, gensym("ii"), A_GIMME, 0);
  class_addmethod(init_class, (t_method)init_loadbang, gensym("loadbang"), 0);
  class_addbang(init_class, (t_method)init_bang);
  class_addanything(init_class, init_anything);
  class_addlist(init_class, init_list);
  class_addpointer(init_class, init_pointer);
  class_addfloat(init_class, (t_method)init_float);
  class_addsymbol(init_class, init_symbol);
//  class_sethelpsymbol(init_class, gensym("iemhelp/help-init"));
}
