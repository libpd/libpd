/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"

/* -------------------------- tab_counter ------------------------------ */
/*   for(i=0; i<x_size_src1; i++)      */
/*   {                                 */
/*     if(x_beg_mem_src1[i] == 0.0f)   */
/*       x_beg_mem_dst[i] = 0.0f;      */
/*     else                            */
/*       x_beg_mem_dst[i] += 1.0f;     */
/*   }                                 */

typedef struct _tab_counter
{
  t_object  x_obj;
  int       x_size_src1;
  int       x_size_dst;
  int       x_offset_src1;
  int       x_offset_dst;
  iemarray_t   *x_beg_mem_src1;
  iemarray_t   *x_beg_mem_dst;
  t_symbol  *x_sym_scr1;
  t_symbol  *x_sym_dst;
} t_tab_counter;

static t_class *tab_counter_class;

static void tab_counter_src(t_tab_counter *x, t_symbol *s)
{
  x->x_sym_scr1 = s;
}

static void tab_counter_bang(t_tab_counter *x)
{
  int i, n;
  int ok_src1, ok_dst;
  iemarray_t *vec_src1, *vec_dst;
  
  ok_src1 = iem_tab_check_arrays(gensym("tab_counter"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, 0);
  ok_dst = iem_tab_check_arrays(gensym("tab_counter"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
  if(ok_src1 && ok_dst)
  {
    if(x->x_size_src1 < x->x_size_dst)
      n = x->x_size_src1;
    else
      n = x->x_size_dst;
    
    vec_src1 = x->x_beg_mem_src1;
    vec_dst = x->x_beg_mem_dst;
    if(n)
    {
      t_garray *a;
      
      for(i=0; i<n; i++)
      {
        if(iemarray_getfloat(vec_src1, i) == 0.0f)
          iemarray_setfloat(vec_dst, i, 0.0f);
        else
          iemarray_setfloat(vec_dst, i, iemarray_getfloat(vec_dst, i)+1.0f);
      }
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_counter_dst(t_tab_counter *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_counter_list(t_tab_counter *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_src1, beg_dst;
  int i, n;
  int ok_src1, ok_dst;
  iemarray_t *vec_src1, *vec_dst;
  
  if((argc >= 3) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2))
  {
    beg_src1 = (int)atom_getintarg(0, argc, argv);
    beg_dst = (int)atom_getintarg(1, argc, argv);
    n = (int)atom_getintarg(2, argc, argv);
    if(beg_src1 < 0)
      beg_src1 = 0;
    if(beg_dst < 0)
      beg_dst = 0;
    if(n < 0)
      n = 0;
    
    ok_src1 = iem_tab_check_arrays(gensym("tab_counter"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, beg_src1+n);
    ok_dst = iem_tab_check_arrays(gensym("tab_counter"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, beg_dst+n);
    
    if(ok_src1 && ok_dst)
    {
      vec_src1 = x->x_beg_mem_src1 + beg_src1;
      vec_dst = x->x_beg_mem_dst + beg_dst;
      if(n)
      {
        t_garray *a;
        
        for(i=0; i<n; i++)
        {
          if(iemarray_getfloat(vec_src1, i) == 0.0f)
            iemarray_setfloat(vec_dst, i, 0.0f);
          else
            iemarray_setfloat(vec_dst, i, iemarray_getfloat(vec_dst, i)+1.0f);
        }
        outlet_bang(x->x_obj.ob_outlet);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
        garray_redraw(a);
      }
    }
  }
  else
  {
    post("tab_counter-ERROR: list need 4 float arguments:");
    post("  source1_offset + destination_offset + number_of_samples_to_add + add_scalar");
  }
}

static void tab_counter_reset(t_tab_counter *x)
{
  int beg_dst;
  int i, n;
  int ok_dst;
  iemarray_t *vec_dst;
  
  ok_dst = iem_tab_check_arrays(gensym("tab_counter"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
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

static void tab_counter_free(t_tab_counter *x)
{
}

static void *tab_counter_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_counter *x = (t_tab_counter *)pd_new(tab_counter_class);
  t_symbol  *src1, *dst;
  
  if((argc >= 2) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1))
  {
    src1 = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    dst = (t_symbol *)atom_getsymbolarg(1, argc, argv);
  }
  else if((argc >= 1) &&
    IS_A_SYMBOL(argv,0))
  {
    src1 = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    dst = src1;
  }
  else
  {
    post("tab_counter-ERROR: need 2 symbol arguments:");
    post("  source_array_name + destination_array_name");
    return(0);
  }
  
  x->x_sym_scr1 = src1;
  x->x_sym_dst = dst;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_counter_setup(void)
{
  tab_counter_class = class_new(gensym("tab_counter"), (t_newmethod)tab_counter_new, (t_method)tab_counter_free,
    sizeof(t_tab_counter), 0, A_GIMME, 0);
  class_addbang(tab_counter_class, (t_method)tab_counter_bang);
  class_addlist(tab_counter_class, (t_method)tab_counter_list);
  class_addmethod(tab_counter_class, (t_method)tab_counter_src, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_counter_class, (t_method)tab_counter_src, gensym("src"), A_DEFSYMBOL, 0);
  class_addmethod(tab_counter_class, (t_method)tab_counter_dst, gensym("dst"), A_DEFSYMBOL, 0);
  class_addmethod(tab_counter_class, (t_method)tab_counter_reset, gensym("reset"), 0);
//  class_sethelpsymbol(tab_counter_class, gensym("iemhelp2/tab_counter-help"));
}
