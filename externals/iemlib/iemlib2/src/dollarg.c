/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "g_canvas.h"
#include "iemlib.h"


/* ------------------------- dollarg ---------------------------- */
/* --- dollar-arguments, output the initial-arguments and the --- */
/* ------- number of them of the parent abstraction-patch ------- */

static t_class *dollarg_class;

typedef struct _dollarg
{
  t_object  x_obj;
  void      *x_outlet_ac;
  t_atom    *x_at;
  int       x_ac;
} t_dollarg;

static void dollarg_float(t_dollarg *x, t_floatarg f)
{
  int i = (int)f;
  
  if(!i)
  {
    outlet_float(x->x_outlet_ac, x->x_ac);
    if(x->x_ac)
      outlet_list(x->x_obj.ob_outlet, &s_list, x->x_ac, x->x_at);
  }
  else if(i > 0)
  {
    if(i <= x->x_ac)
    {
      outlet_float(x->x_outlet_ac, i);
      if(IS_A_FLOAT(x->x_at, i-1))
        outlet_float(x->x_obj.ob_outlet, atom_getfloatarg(i-1, x->x_ac, x->x_at));
      else if(IS_A_SYMBOL(x->x_at, i-1))
        outlet_symbol(x->x_obj.ob_outlet, atom_getsymbolarg(i-1, x->x_ac, x->x_at));
    }
    else
      outlet_float(x->x_outlet_ac, 0);
  }
  else
  {
    int j = x->x_ac + i;
    
    if(j >= 0)
    {
      outlet_float(x->x_outlet_ac, j+1);
      if(IS_A_FLOAT(x->x_at, j))
        outlet_float(x->x_obj.ob_outlet, atom_getfloatarg(j, x->x_ac, x->x_at));
      else if(IS_A_SYMBOL(x->x_at, j))
        outlet_symbol(x->x_obj.ob_outlet, atom_getsymbolarg(j, x->x_ac, x->x_at));
    }
    else
      outlet_float(x->x_outlet_ac, 0);
  }
}

static void dollarg_bang(t_dollarg *x)
{
  dollarg_float(x, 0.0f);
}

static void dollarg_free(t_dollarg *x)
{
  if(x->x_ac)
    freebytes(x->x_at, x->x_ac * sizeof(t_atom));
}

static void *dollarg_new(void)
{
  t_dollarg *x = (t_dollarg *)pd_new(dollarg_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=glist_getcanvas(glist);
  int pargc;
  t_atom *pargv, *at;
  
  canvas_setcurrent(canvas);
  canvas_getargs(&pargc, &pargv);
  canvas_unsetcurrent(canvas);
  x->x_at = (t_atom *)getbytes(pargc*sizeof(t_atom));
  x->x_ac = pargc;
  at = x->x_at;
  while(pargc--)
    *at++ = *pargv++;
  outlet_new(&x->x_obj, &s_list);
  x->x_outlet_ac = outlet_new(&x->x_obj, &s_float);
  return (x);
}

void dollarg_setup(void)
{
  dollarg_class = class_new(gensym("dollarg"), (t_newmethod)dollarg_new,
    (t_method)dollarg_free, sizeof(t_dollarg), 0, 0);
  class_addcreator((t_newmethod)dollarg_new, gensym("$n"), 0);
  class_addbang(dollarg_class, (t_method)dollarg_bang);
  class_addfloat(dollarg_class, (t_method)dollarg_float);
//  class_sethelpsymbol(dollarg_class, gensym("iemhelp/help-dollarg"));
}
