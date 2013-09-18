/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* -------------------------- iem_i_route ------------------------------ */
/* ---- routes a list beginning with a float to an outlet, which is ---- */
/* -- defined by the 3 initial arguments first index, last index and --- */
/* ---------------------- offset index --------------------------------- */

static t_class *iem_i_route_class;

typedef struct _iem_i_route
{
  t_object  x_obj;
  int       x_first_element;
  int       x_last_element;
  t_outlet  **x_out;
} t_iem_i_route;

static void iem_i_route_list(t_iem_i_route *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_outlet **out;
  int first, last, i;
  
  if(!argc)
    return;
  i = (int)atom_getintarg(0, argc, argv);
  first = x->x_first_element;
  last = x->x_last_element;
  out = x->x_out;
  if((i >= first)&&(i <= last))
  {
    out += i - first;
    if(argc >= 3)
    {
      if(IS_A_FLOAT(argv,1))
        outlet_list(*out, &s_list, argc-1, argv+1);
      else if(IS_A_SYMBOL(argv,1))
        outlet_anything(*out, atom_getsymbolarg(1, argc, argv), argc-2, argv+2);
    }
    else if(argc >= 2)
    {
      if(IS_A_FLOAT(argv,1))
        outlet_float(*out, (float)atom_getfloatarg(1, argc, argv));
      else if(IS_A_SYMBOL(argv,1))
        outlet_anything(*out, atom_getsymbolarg(1, argc, argv), 0, argv+2);
    }
    else
      outlet_bang(*out);
  }
  else
  {
    out += last - first + 1;
    outlet_list(*out, &s_list, argc, argv);
  }
}

static void iem_i_route_free(t_iem_i_route *x)
{
  freebytes(x->x_out, (x->x_last_element-x->x_first_element+2) * sizeof(t_outlet *));
}

static void *iem_i_route_new(t_symbol *s, int argc, t_atom *argv)
{
  int n, i;
  t_outlet **out;
  t_iem_i_route *x = (t_iem_i_route *)pd_new(iem_i_route_class);
  
  if((argc >= 2)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1))
  {
    x->x_first_element = (int)atom_getintarg(0, argc, argv);
    x->x_last_element = (int)atom_getintarg(1, argc, argv);
    if((argc >= 3)&&IS_A_FLOAT(argv,2))
    {
      i = (int)atom_getintarg(2, argc, argv);
      x->x_first_element += i;
      x->x_last_element += i;
    }
    x->x_out = (t_outlet **)getbytes((x->x_last_element-x->x_first_element+2) * sizeof(t_outlet *));
    n = x->x_last_element - x->x_first_element + 2;
    for(i=0, out=x->x_out; i<n; i++, out++)
      *out = outlet_new(&x->x_obj, &s_list);
    return (x);
  }
  else
  {
    post("iem_i_route-ERROR: needs 3 floats!!");
    return(0);
  }
}

void iem_i_route_setup(void)
{
  iem_i_route_class = class_new(gensym("iem_i_route"), (t_newmethod)iem_i_route_new,
        (t_method)iem_i_route_free, sizeof(t_iem_i_route), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)iem_i_route_new, gensym("iiroute"), A_GIMME, 0);
  class_addlist(iem_i_route_class, iem_i_route_list);
//  class_sethelpsymbol(iem_i_route_class, gensym("iemhelp/help-iem_i_route"));
}
