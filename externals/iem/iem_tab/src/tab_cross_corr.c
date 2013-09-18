/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"


/* -------------------------- tab_cross_corr ------------------------------ */

typedef struct _tab_cross_corr
{
  t_object  x_obj;
  int       x_size_src1;
  int       x_size_src2;
  int       x_size_dst;
  int       x_n;
  iemarray_t   *x_beg_mem_src1;
  iemarray_t   *x_beg_mem_src2;
  iemarray_t   *x_beg_mem_dst;
  t_float   x_factor;
  t_symbol  *x_sym_scr1;
  t_symbol  *x_sym_scr2;
  t_symbol  *x_sym_dst;
  t_float   x_delay;
  int       x_counter;
  void      *x_clock;
} t_tab_cross_corr;

static t_class *tab_cross_corr_class;

static void tab_cross_corr_tick(t_tab_cross_corr *x)
{
  x->x_counter++;
  if(x->x_counter < x->x_n)
  {
    iemarray_t *vec_src1, *vec_src2, *vec_dst;
    t_float sum;
    int j, m;
    
    vec_src1 = x->x_beg_mem_src1 + x->x_counter;
    vec_src2 = x->x_beg_mem_src2;
    vec_dst = x->x_beg_mem_dst + x->x_counter;
    m = x->x_size_src2;
    sum = 0.0f;
    for(j=0; j<m; j++)
    {
      sum += iemarray_getfloat(vec_src1, j)*iemarray_getfloat(vec_src2, j);
    }
    iemarray_setfloat(vec_dst, 0, sum*x->x_factor);
    clock_delay(x->x_clock, x->x_delay);
  }
  else
  {
    t_garray *a;
    
    clock_unset(x->x_clock);
    outlet_bang(x->x_obj.ob_outlet);
    a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
    garray_redraw(a);
  }
}

static void tab_cross_corr_time(t_tab_cross_corr *x, t_floatarg dtime)
{
  if(dtime < 0.0f)
    dtime = 0.0f;
  
  x->x_delay = dtime;
}

static void tab_cross_corr_factor(t_tab_cross_corr *x, t_floatarg factor)
{
  x->x_factor = factor;
}

static void tab_cross_corr_src1(t_tab_cross_corr *x, t_symbol *s)
{
  x->x_sym_scr1 = s;
}

static void tab_cross_corr_src2(t_tab_cross_corr *x, t_symbol *s)
{
  x->x_sym_scr2 = s;
}

static void tab_cross_corr_dst(t_tab_cross_corr *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_cross_corr_bang(t_tab_cross_corr *x)
{
  int i, j, m, n;
  int ok_src1, ok_src2, ok_dst;
  iemarray_t *vec_src1, *vec_src2, *vec_dst;
  t_float sum, f;
  
  ok_src1 = iem_tab_check_arrays(gensym("tab_cross_corr"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, 0);
  ok_src2 = iem_tab_check_arrays(gensym("tab_cross_corr"), x->x_sym_scr2, &x->x_beg_mem_src2, &x->x_size_src2, 0);
  ok_dst = iem_tab_check_arrays(gensym("tab_cross_corr"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
  if(ok_src1 && ok_src2 && ok_dst)
  {
    if(x->x_size_src1 > x->x_size_src2)
      n = x->x_size_src1 - x->x_size_src2;
    else
      n = 0;
    if(n > x->x_size_dst)
      x->x_n = x->x_size_dst;
    else
      x->x_n = n;
    f = x->x_factor;
    if(n)
    {
      if(x->x_delay == 0.0f)
      {
        t_garray *a;
        
        vec_src1 = x->x_beg_mem_src1;
        vec_src2 = x->x_beg_mem_src2;
        vec_dst = x->x_beg_mem_dst;
        m = x->x_size_src2;
        for(i=0; i<n; i++)
        {
          sum = 0.0f;
          for(j=0; j<m; j++)
          {
            sum += iemarray_getfloat(vec_src1, i+j)*iemarray_getfloat(vec_src2, j);
          }
          iemarray_setfloat(vec_dst, i, sum*f);
        }
        outlet_bang(x->x_obj.ob_outlet);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
        garray_redraw(a);
      }
      else
      {
        x->x_counter = 0;
        vec_src1 = x->x_beg_mem_src1 + x->x_counter;
        vec_src2 = x->x_beg_mem_src2;
        vec_dst = x->x_beg_mem_dst + x->x_counter;
        m = x->x_size_src2;
        sum = 0.0f;
        for(j=0; j<m; j++)
        {
          sum += iemarray_getfloat(vec_src1, j)*iemarray_getfloat(vec_src2, j);
        }
        iemarray_setfloat(vec_dst, 0, sum*f);
        clock_delay(x->x_clock, x->x_delay);
      }
    }
  }
}

static void tab_cross_corr_free(t_tab_cross_corr *x)
{
  clock_free(x->x_clock);
}

static void *tab_cross_corr_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_cross_corr *x = (t_tab_cross_corr *)pd_new(tab_cross_corr_class);
  t_symbol  *src1, *src2, *dst;
  t_float dtime=0.0f, factor=1.0f;
  
  if((argc >= 5) && IS_A_FLOAT(argv,4))
    dtime = (t_float)atom_getfloatarg(4, argc, argv);
  if((argc >= 4) && IS_A_FLOAT(argv,3))
    factor = (t_float)atom_getfloatarg(3, argc, argv);
  if((argc >= 3) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
    IS_A_SYMBOL(argv,2))
  {
    src1 = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    src2 = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst = (t_symbol *)atom_getsymbolarg(2, argc, argv);
  }
  else
  {
    post("tab_cross_corr-ERROR: need 3 symbol + 2 float arguments:");
    post("  source_reference_array_name + source_measure_array_name + destination_array_name + norm_factor + calculation-time-per-sample_ms");
    return(0);
  }
  
  if(dtime < 0.0f)
    dtime = 0.0f;
  
  x->x_delay = dtime;
  x->x_factor = factor;
  x->x_sym_scr1 = src1;
  x->x_sym_scr2 = src2;
  x->x_sym_dst = dst;
  outlet_new(&x->x_obj, &s_bang);
  x->x_clock = clock_new(x, (t_method)tab_cross_corr_tick);
  return(x);
}

void tab_cross_corr_setup(void)
{
  tab_cross_corr_class = class_new(gensym("tab_cross_corr"), (t_newmethod)tab_cross_corr_new, (t_method)tab_cross_corr_free,
    sizeof(t_tab_cross_corr), 0, A_GIMME, 0);
  class_addbang(tab_cross_corr_class, (t_method)tab_cross_corr_bang);
  class_addmethod(tab_cross_corr_class, (t_method)tab_cross_corr_time, gensym("time"), A_DEFFLOAT, 0);
  class_addmethod(tab_cross_corr_class, (t_method)tab_cross_corr_factor, gensym("factor"), A_DEFFLOAT, 0);
  class_addmethod(tab_cross_corr_class, (t_method)tab_cross_corr_src2, gensym("src2"), A_DEFSYMBOL, 0);
  class_addmethod(tab_cross_corr_class, (t_method)tab_cross_corr_src1, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_cross_corr_class, (t_method)tab_cross_corr_dst, gensym("dst"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_cross_corr_class, gensym("iemhelp2/tab_cross_corr-help"));
}
