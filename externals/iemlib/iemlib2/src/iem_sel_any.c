/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ------------------------ iem_sel_any ---------------------------- */
/* -- stores an array of symbols, random access by index ----------- */

static t_class *iem_sel_any_class;

typedef struct _iem_sel_any
{
  t_object  x_obj;
  int       x_ac;
  int       x_max_ac;
  t_symbol  **x_any;
  t_symbol  *x_set;
  void      *x_out_any;
  void      *x_out_set_any;
} t_iem_sel_any;


static void iem_sel_any_float(t_iem_sel_any *x, t_floatarg f)
{
  int i = (int)f;
  t_atom at;
  
  if(x->x_ac > 0)
  {
    if(i < 0)
      i = 0;
    if(i >= x->x_ac)
      i = x->x_ac - 1;
    SETSYMBOL(&at, x->x_any[i]);
    outlet_anything(x->x_out_any, x->x_any[i], 0, 0);
    outlet_anything(x->x_out_set_any, x->x_set, 1, &at);
  }
}

static void iem_sel_any_add(t_iem_sel_any *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac >= 2) && (IS_A_FLOAT(av, 0)))
  {
    int i = (int)atom_getintarg(0, ac, av);
    
    if((i >= 0) && (i < x->x_max_ac))
    {
      if(IS_A_SYMBOL(av, 1))
        x->x_any[i] = atom_getsymbolarg(1, ac, av);
      else if(IS_A_FLOAT(av, 1))
      {
        char str[100];
        
        sprintf(str, "%g", atom_getfloatarg(1, ac, av));
        x->x_any[i] = gensym(str);
      }
      if(i >= x->x_ac)
        x->x_ac = i+1;
    }
  }
}

static void iem_sel_any_set_item_name(t_iem_sel_any *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac >= 2) && (IS_A_FLOAT(av, 1)))
  {
    int i = (int)atom_getintarg(1, ac, av);
    
    if((i >= 0) && (i < x->x_max_ac))
    {
      if(IS_A_SYMBOL(av, 0))
        x->x_any[i] = atom_getsymbolarg(0, ac, av);
      else if(IS_A_FLOAT(av, 0))
      {
        char str[100];
        
        sprintf(str, "%g", atom_getfloatarg(0, ac, av));
        x->x_any[i] = gensym(str);
      }
      if(i >= x->x_ac)
        x->x_ac = i+1;
    }
  }
}

static void iem_sel_any_clear(t_iem_sel_any *x)
{
  x->x_ac = 0;
}

static void iem_sel_any_free(t_iem_sel_any *x)
{
  freebytes(x->x_any, x->x_max_ac * sizeof(t_symbol *));
}

static void *iem_sel_any_new(t_floatarg ffmax)
{
  t_iem_sel_any *x = (t_iem_sel_any *)pd_new(iem_sel_any_class);
  int i;
  t_symbol *default_sym=gensym("no_entry");
  
  if(ffmax <= 0.0)
    ffmax = 10.0;
  x->x_max_ac = (int)ffmax;
  x->x_any = (t_symbol **)getbytes(x->x_max_ac * sizeof(t_symbol *));
  x->x_ac = 0;
  x->x_set = gensym("set");
  for(i=0; i<x->x_max_ac; i++)
    x->x_any[i] = default_sym;
  x->x_out_set_any = outlet_new(&x->x_obj, &s_list);
  x->x_out_any = outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_sel_any_setup(void)
{
  iem_sel_any_class = class_new(gensym("iem_sel_any"), (t_newmethod)iem_sel_any_new,
    (t_method)iem_sel_any_free, sizeof(t_iem_sel_any), 0, A_DEFFLOAT, 0);
  class_addmethod(iem_sel_any_class, (t_method)iem_sel_any_add, gensym("add"), A_GIMME, 0);
  class_addmethod(iem_sel_any_class, (t_method)iem_sel_any_set_item_name, gensym("set_item_name"), A_GIMME, 0);
  class_addmethod(iem_sel_any_class, (t_method)iem_sel_any_clear, gensym("clear"), 0);
  class_addfloat(iem_sel_any_class, (t_method)iem_sel_any_float);
//  class_sethelpsymbol(iem_sel_any_class, gensym("iemhelp/help-iem_sel_any"));
}
