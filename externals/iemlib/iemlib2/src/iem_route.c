/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* -------------------------- iem_route ------------------------------ */
/* -------- like millers route, but can output bangs ----------------- */

static t_class *iem_route_class;

typedef struct _iem_routeelement
{
  t_word   e_w;
  t_outlet *e_outlet;
} t_iem_routeelement;

typedef struct _iem_route
{
  t_object           x_obj;
  t_atomtype         x_type;
  t_int              x_nelement;
  t_iem_routeelement *x_vec;
  t_outlet           *x_rejectout;
} t_iem_route;

static void iem_route_anything(t_iem_route *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_iem_routeelement *e;
  int nelement;
  
  if(x->x_type == A_SYMBOL)
  {
    for(nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
    {
      if(e->e_w.w_symbol == sel)
      {
        if(!argc)
          outlet_bang(e->e_outlet);
        else
        {
          if(argc == 1)
          {
            if(argv[0].a_type == A_FLOAT)
              outlet_float(e->e_outlet, argv[0].a_w.w_float);
            else
              outlet_anything(e->e_outlet, argv[0].a_w.w_symbol, 0, argv+1);
          }
          else
          {
            if(argv[0].a_type == A_SYMBOL)
              outlet_anything(e->e_outlet, argv[0].a_w.w_symbol, argc-1, argv+1);
            else
              outlet_list(e->e_outlet, &s_list, argc, argv);
          }
        }
        return;
      }
    }
  }
  outlet_anything(x->x_rejectout, sel, argc, argv);
}

static void iem_route_list(t_iem_route *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_iem_routeelement *e;
  int nelement;
  
  if (x->x_type == A_FLOAT)
  {
    t_float f;
    
    if(!argc)
      return;
    f = atom_getfloat(argv);
    for(nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
    {
      if(e->e_w.w_float == f)
      {
        if(argc > 1 && argv[1].a_type == A_SYMBOL)
          outlet_anything(e->e_outlet, argv[1].a_w.w_symbol, argc-2, argv+2);
        else
        {
          if(argc == 1)
            outlet_bang(e->e_outlet);
          else if(argc == 2)
            outlet_float(e->e_outlet, atom_getfloat(argv+1));
          else
            outlet_list(e->e_outlet, &s_list, argc-1, argv+1);
        }
        return;
      }
    }
  }
  else  /* symbol arguments */
  {
    if(argc > 1)    /* 2 or more args: treat as "list" */
    {
      for(nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
      {
        if(e->e_w.w_symbol == &s_list)
        {
          if(argv[0].a_type == A_SYMBOL)
            outlet_anything(e->e_outlet, argv[0].a_w.w_symbol, argc-1, argv+1);
          else
            outlet_list(e->e_outlet, &s_list, argc, argv);
          return;
        }
      }
    }
    else if(argc == 0)    /* no args: treat as "bang" */
    {
      for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
      {
        if (e->e_w.w_symbol == &s_bang)
        {
          outlet_bang(e->e_outlet);
          return;
        }
      }
    }
    else if (argv[0].a_type == A_FLOAT)   /* one float arg */
    {
      for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
      {
        if (e->e_w.w_symbol == &s_float)
        {
          outlet_float(e->e_outlet, argv[0].a_w.w_float);
          return;
        }
      }
    }
    else
    {
      for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
      {
        if (e->e_w.w_symbol == &s_symbol)
        {
          outlet_symbol(e->e_outlet, argv[0].a_w.w_symbol);
          return;
        }
      }
    }
  }
  outlet_list(x->x_rejectout, &s_list, argc, argv);
}


static void iem_route_free(t_iem_route *x)
{
  freebytes(x->x_vec, x->x_nelement * sizeof(*x->x_vec));
}

static void *iem_route_new(t_symbol *s, int argc, t_atom *argv)
{
  int n;
  t_iem_routeelement *e;
  t_iem_route *x = (t_iem_route *)pd_new(iem_route_class);
  t_atom a;
  if (argc == 0)
  {
    argc = 1;
    SETFLOAT(&a, 0);
    argv = &a;
  }
  x->x_type = argv[0].a_type;
  x->x_nelement = argc;
  x->x_vec = (t_iem_routeelement *)getbytes(argc * sizeof(*x->x_vec));
  for (n = 0, e = x->x_vec; n < argc; n++, e++)
  {
    e->e_outlet = outlet_new(&x->x_obj, &s_list);
    if (x->x_type == A_FLOAT)
      e->e_w.w_float = atom_getfloatarg(n, argc, argv);
    else e->e_w.w_symbol = atom_getsymbolarg(n, argc, argv);
  }
  x->x_rejectout = outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_route_setup(void)
{
  iem_route_class = class_new(gensym("iem_route"), (t_newmethod)iem_route_new,
        (t_method)iem_route_free, sizeof(t_iem_route), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)iem_route_new, gensym("ir"), A_GIMME, 0);
  class_addlist(iem_route_class, iem_route_list);
  class_addanything(iem_route_class, iem_route_anything);
//  class_sethelpsymbol(iem_route_class, gensym("iemhelp/help-iem_route"));
}
