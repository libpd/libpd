/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"

/* -------------------------- tab_const ------------------------------ */
/*   x_beg_mem_dst[i] = c   */

typedef struct _tab_const
{
  t_object  x_obj;
  int       x_size_dst;
  int       x_offset_dst;
  t_float   x_const;
  iemarray_t *x_beg_mem_dst;
  t_symbol  *x_sym_dst;
} t_tab_const;

static t_class *tab_const_class;

static void tab_const_dst(t_tab_const *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_const_bang(t_tab_const *x)
{
  int i, n;
  int ok_dst;
  iemarray_t *vec_dst;
  
  ok_dst = iem_tab_check_arrays(gensym("tab_const"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
  if(ok_dst)
  {
    n = x->x_size_dst;
    vec_dst = x->x_beg_mem_dst;
    if(n)
    {
      t_garray *a;
      
      for(i=0; i<n; i++)
        iemarray_setfloat(vec_dst, i, 0.0f);
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_const_float(t_tab_const *x, t_floatarg c)
{
  int i, n;
  int ok_dst;
  iemarray_t *vec_dst;
  
  ok_dst = iem_tab_check_arrays(gensym("tab_const"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
  if(ok_dst)
  {
    n = x->x_size_dst;
    vec_dst = x->x_beg_mem_dst;
    if(n)
    {
      t_garray *a;
      
      for(i=0; i<n; i++)
        iemarray_setfloat(vec_dst, i, c);
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_const_list(t_tab_const *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_dst;
  int i, n;
  int ok_dst;
  iemarray_t *vec_dst;
  t_float c;
  
  if((argc >= 3) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2))
  {
    beg_dst = (int)atom_getintarg(0, argc, argv);
    n = (int)atom_getintarg(1, argc, argv);
    c = (t_float)atom_getfloatarg(2, argc, argv);
    if(beg_dst < 0)
      beg_dst = 0;
    if(n < 0)
      n = 0;
    
    ok_dst = iem_tab_check_arrays(gensym("tab_const"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, beg_dst+n);
    
    if(ok_dst)
    {
      vec_dst = x->x_beg_mem_dst + beg_dst;
      if(n)
      {
        t_garray *a;
        
        for(i=0; i<n; i++)
          iemarray_setfloat(vec_dst, i, c);
        outlet_bang(x->x_obj.ob_outlet);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
        garray_redraw(a);
      }
    }
  }
  else
  {
    post("tab_const-ERROR: list need 3 float arguments:");
    post("  destination_offset + number_of_samples_to_copy + constant-value");
  }
}

static void tab_const_free(t_tab_const *x)
{
}

static void *tab_const_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_const *x = (t_tab_const *)pd_new(tab_const_class);
  t_symbol  *dst;
  
  if((argc >= 1) &&
    IS_A_SYMBOL(argv,0))
  {
    dst = (t_symbol *)atom_getsymbolarg(0, argc, argv);
  }
  else
  {
    post("tab_const-ERROR: need 1 symbol argument:");
    post("  destination_array_name");
    return(0);
  }
  
  x->x_sym_dst = dst;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_const_setup(void)
{
  tab_const_class = class_new(gensym("tab_const"), (t_newmethod)tab_const_new, (t_method)tab_const_free,
    sizeof(t_tab_const), 0, A_GIMME, 0);
  class_addbang(tab_const_class, (t_method)tab_const_bang);
  class_addfloat(tab_const_class, (t_method)tab_const_float);
  class_addlist(tab_const_class, (t_method)tab_const_list);
  class_addmethod(tab_const_class, (t_method)tab_const_dst, gensym("dst"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_const_class, gensym("iemhelp2/tab_const-help"));
}
