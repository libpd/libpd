/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2010 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"
#include <math.h>

#define IEMTABLOGTEN 2.302585092994046

/* -------------------------- tab_dbtorms ------------------------------ */
/*   x_beg_mem_dst[i] = sqrt(x_beg_mem_src1[i])   */

typedef struct _tab_dbtorms
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
} t_tab_dbtorms;

static t_class *tab_dbtorms_class;

static void tab_dbtorms_src(t_tab_dbtorms *x, t_symbol *s)
{
  x->x_sym_scr1 = s;
}

static void tab_dbtorms_dst(t_tab_dbtorms *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_dbtorms_bang(t_tab_dbtorms *x)
{
  int i, n;
  int ok_src, ok_dst;
  iemarray_t *vec_src, *vec_dst;
  
  ok_src = iem_tab_check_arrays(gensym("tab_dbtorms"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, 0);
  ok_dst = iem_tab_check_arrays(gensym("tab_dbtorms"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
  if(ok_src && ok_dst)
  {
    if(x->x_size_src1 < x->x_size_dst)
      n = x->x_size_src1;
    else
      n = x->x_size_dst;
    vec_src = x->x_beg_mem_src1;
    vec_dst = x->x_beg_mem_dst;
    if(n)
    {
      t_garray *a;
      
      for(i=0; i<n; i++)
      {
        t_float f = iemarray_getfloat(vec_src, i);
        
        if(f <= 0.0)
          iemarray_setfloat(vec_dst, i, 0.0);
        else
        {
          if(f > 485.0)
            f = 485.0;
          iemarray_setfloat(vec_dst, i, exp((IEMTABLOGTEN * 0.05) * (f-100.0)));
        }
      }
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_dbtorms_list(t_tab_dbtorms *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_src, beg_dst;
  int i, n;
  int ok_src, ok_dst;
  iemarray_t *vec_src, *vec_dst;
  
  if((argc >= 3) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2))
  {
    beg_src = (int)atom_getintarg(0, argc, argv);
    beg_dst = (int)atom_getintarg(1, argc, argv);
    n = (int)atom_getintarg(2, argc, argv);
    if(beg_src < 0)
      beg_src = 0;
    if(beg_dst < 0)
      beg_dst = 0;
    if(n < 0)
      n = 0;
    
    ok_src = iem_tab_check_arrays(gensym("tab_dbtorms"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, beg_src+n);
    ok_dst = iem_tab_check_arrays(gensym("tab_dbtorms"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, beg_dst+n);
    
    if(ok_src && ok_dst)
    {
      vec_src = x->x_beg_mem_src1 + beg_src;
      vec_dst = x->x_beg_mem_dst + beg_dst;
      if(n)
      {
        t_garray *a;
        
        for(i=0; i<n; i++)
        {
          t_float f = iemarray_getfloat(vec_src, i);
          
          if(f <= 0.0)
            iemarray_setfloat(vec_dst, i, 0.0);
          else
          {
            if(f > 485.0)
              f = 485.0;
            iemarray_setfloat(vec_dst, i, exp((IEMTABLOGTEN * 0.05) * (f-100.0)));
          }
        }
        outlet_bang(x->x_obj.ob_outlet);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
        garray_redraw(a);
      }
    }
  }
  else
  {
    post("tab_dbtorms-ERROR: list need 3 float arguments:");
    post("  source_offset + destination_offset + number_of_samples_to_dbtorms");
  }
}

static void tab_dbtorms_free(t_tab_dbtorms *x)
{
}

static void *tab_dbtorms_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_dbtorms *x = (t_tab_dbtorms *)pd_new(tab_dbtorms_class);
  t_symbol  *src, *dst;
  
  if((argc >= 2) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1))
  {
    src = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    dst = (t_symbol *)atom_getsymbolarg(1, argc, argv);
  }
  else if((argc >= 1) &&
    IS_A_SYMBOL(argv,0))
  {
    src = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    dst = src;
  }
  else
  {
    post("tab_dbtorms-ERROR: need 2 symbols arguments:");
    post("  source_array_name + destination_array_name");
    return(0);
  }
  
  x->x_sym_scr1 = src;
  x->x_sym_dst = dst;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_dbtorms_setup(void)
{
  tab_dbtorms_class = class_new(gensym("tab_dbtorms"), (t_newmethod)tab_dbtorms_new, (t_method)tab_dbtorms_free,
    sizeof(t_tab_dbtorms), 0, A_GIMME, 0);
  class_addbang(tab_dbtorms_class, (t_method)tab_dbtorms_bang);
  class_addlist(tab_dbtorms_class, (t_method)tab_dbtorms_list);
  class_addmethod(tab_dbtorms_class, (t_method)tab_dbtorms_src, gensym("src"), A_DEFSYMBOL, 0);
  class_addmethod(tab_dbtorms_class, (t_method)tab_dbtorms_src, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_dbtorms_class, (t_method)tab_dbtorms_dst, gensym("dst"), A_DEFSYMBOL, 0);
}
