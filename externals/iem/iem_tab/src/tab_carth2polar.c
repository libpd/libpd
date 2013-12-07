/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"
#include <math.h>

/* -------------------------- tab_carth2polar ------------------------------ */
/*   x_beg_mem_dst_mag[i] = sqrt(x_beg_mem_src_re[i]*x_beg_mem_src_re[i] + x_beg_mem_src_im[i]*x_beg_mem_src_im[i])   */
/*   x_beg_mem_dst_arg[i] = atan2(x_beg_mem_src_im[i], x_beg_mem_src_re[i])   */

typedef struct _tab_carth2polar
{
  t_object  x_obj;
  int       x_size_src_re;
  int       x_size_dst_mag;
  int       x_size_src_im;
  int       x_size_dst_arg;
  int       x_offset_src_re;
  int       x_offset_dst_mag;
  int       x_offset_src_im;
  int       x_offset_dst_arg;
  iemarray_t   *x_beg_mem_src_re;
  iemarray_t   *x_beg_mem_dst_mag;
  iemarray_t   *x_beg_mem_src_im;
  iemarray_t   *x_beg_mem_dst_arg;
  t_symbol  *x_sym_src_re;
  t_symbol  *x_sym_dst_mag;
  t_symbol  *x_sym_src_im;
  t_symbol  *x_sym_dst_arg;
} t_tab_carth2polar;

static t_class *tab_carth2polar_class;

static void tab_carth2polar_src_re(t_tab_carth2polar *x, t_symbol *s)
{
  x->x_sym_src_re = s;
}

static void tab_carth2polar_src_im(t_tab_carth2polar *x, t_symbol *s)
{
  x->x_sym_src_im = s;
}

static void tab_carth2polar_dst_mag(t_tab_carth2polar *x, t_symbol *s)
{
  x->x_sym_dst_mag = s;
}

static void tab_carth2polar_dst_arg(t_tab_carth2polar *x, t_symbol *s)
{
  x->x_sym_dst_arg = s;
}

static void tab_carth2polar_bang(t_tab_carth2polar *x)
{
  int i, n;
  int ok_src_re, ok_dst_mag;
  int ok_src_im, ok_dst_arg;
  iemarray_t *vec_src_re, *vec_dst_mag;
  iemarray_t *vec_src_im, *vec_dst_arg;
  
  ok_src_re = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_src_re, &x->x_beg_mem_src_re, &x->x_size_src_re, 0);
  ok_dst_mag = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_dst_mag, &x->x_beg_mem_dst_mag, &x->x_size_dst_mag, 0);
  ok_src_im = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_src_im, &x->x_beg_mem_src_im, &x->x_size_src_im, 0);
  ok_dst_arg = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_dst_arg, &x->x_beg_mem_dst_arg, &x->x_size_dst_arg, 0);
  
  if(ok_src_re && ok_dst_mag && ok_src_im && ok_dst_arg)
  {
    if(x->x_size_src_re < x->x_size_dst_mag)
      n = x->x_size_src_re;
    else
      n = x->x_size_dst_mag;
    if(x->x_size_src_im < n)
      n = x->x_size_src_im;
    if(x->x_size_dst_arg < n)
      n = x->x_size_dst_arg;
    
    vec_src_re = x->x_beg_mem_src_re;
    vec_dst_mag = x->x_beg_mem_dst_mag;
    vec_src_im = x->x_beg_mem_src_im;
    vec_dst_arg = x->x_beg_mem_dst_arg;
    if(n)
    {
      t_garray *a;
      t_float rcp_two_pi=0.125f/atan(1.0);
      
      for(i=0; i<n; i++)
      {
        t_float re, im, mag, arg;
        
        re = iemarray_getfloat(vec_src_re, i);
        im = iemarray_getfloat(vec_src_im, i);
        mag = sqrt(re*re + im*im);
        arg = atan2(im, re) * rcp_two_pi;
        iemarray_setfloat(vec_dst_mag, i, mag);
        iemarray_setfloat(vec_dst_arg, i, arg);
      }
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst_mag, garray_class);
      garray_redraw(a);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst_arg, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_carth2polar_list(t_tab_carth2polar *x, t_symbol *s, int argc, t_atom *argv)
{
  int i, n;
  int beg_src_re, beg_dst_mag;
  int beg_src_im, beg_dst_arg;
  int ok_src_re, ok_dst_mag;
  int ok_src_im, ok_dst_arg;
  iemarray_t *vec_src_re, *vec_dst_mag;
  iemarray_t *vec_src_im, *vec_dst_arg;
  
  if((argc >= 5) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2) &&
    IS_A_FLOAT(argv,3) &&
    IS_A_FLOAT(argv,4))
  {
    beg_src_re = (int)atom_getintarg(0, argc, argv);
    beg_src_im = (int)atom_getintarg(1, argc, argv);
    beg_dst_mag = (int)atom_getintarg(2, argc, argv);
    beg_dst_arg = (int)atom_getintarg(3, argc, argv);
    n = (int)atom_getintarg(4, argc, argv);
    if(beg_src_re < 0)
      beg_src_re = 0;
    if(beg_dst_mag < 0)
      beg_dst_mag = 0;
    if(beg_src_im < 0)
      beg_src_im = 0;
    if(beg_dst_arg < 0)
      beg_dst_arg = 0;
    if(n < 0)
      n = 0;
    
    ok_src_re = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_src_re, &x->x_beg_mem_src_re, &x->x_size_src_re, beg_src_re+n);
    ok_dst_mag = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_dst_mag, &x->x_beg_mem_dst_mag, &x->x_size_dst_mag, beg_dst_mag+n);
    ok_src_im = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_src_im, &x->x_beg_mem_src_im, &x->x_size_src_im, beg_src_im+n);
    ok_dst_arg = iem_tab_check_arrays(gensym("tab_carth2polar"), x->x_sym_dst_arg, &x->x_beg_mem_dst_arg, &x->x_size_dst_arg, beg_dst_arg+n);
    
    if(ok_src_re && ok_dst_mag && ok_src_im && ok_dst_arg)
    {
      vec_src_re = x->x_beg_mem_src_re + beg_src_re;
      vec_dst_mag = x->x_beg_mem_dst_mag + beg_dst_mag;
      vec_src_im = x->x_beg_mem_src_im + beg_src_im;
      vec_dst_arg = x->x_beg_mem_dst_arg + beg_dst_arg;
      
      if(n)
      {
        t_garray *a;
        t_float rcp_two_pi=0.125f/atan(1.0);
        
        for(i=0; i<n; i++)
        {
          t_float re, im, mag, arg;
          
          re = iemarray_getfloat(vec_src_re, i);
          im = iemarray_getfloat(vec_src_im, i);
          mag = sqrt(re*re + im*im);
          arg = atan2(im, re) * rcp_two_pi;
          iemarray_setfloat(vec_dst_mag, i, mag);
          iemarray_setfloat(vec_dst_arg, i, arg);
        }
        outlet_bang(x->x_obj.ob_outlet);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst_mag, garray_class);
        garray_redraw(a);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst_arg, garray_class);
        garray_redraw(a);
      }
    }
  }
  else
  {
    post("tab_carth2polar-ERROR: list need 5 float arguments:");
    post("  source_real_offset + source_imag_offset + destination_magnitude_offset + destination_phase_argument_offset + number_of_samples_to_convert");
  }
}

static void tab_carth2polar_free(t_tab_carth2polar *x)
{
}

static void *tab_carth2polar_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_carth2polar *x = (t_tab_carth2polar *)pd_new(tab_carth2polar_class);
  t_symbol  *src_re, *dst_mag, *src_im, *dst_arg;
  
  if((argc >= 4) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
    IS_A_SYMBOL(argv,2) &&
    IS_A_SYMBOL(argv,3))
  {
    src_re = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    src_im = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst_mag = (t_symbol *)atom_getsymbolarg(2, argc, argv);
    dst_arg = (t_symbol *)atom_getsymbolarg(3, argc, argv);
  }
  else
  {
    post("tab_carth2polar-ERROR: need 4 symbols arguments:");
    post("  source_real_array_name + source_imag_array_name + destination_magnitude_array_name + destination_phase_argument_array_name");
    return(0);
  }
  
  x->x_sym_src_re = src_re;
  x->x_sym_src_im = src_im;
  x->x_sym_dst_mag = dst_mag;
  x->x_sym_dst_arg = dst_arg;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_carth2polar_setup(void)
{
  tab_carth2polar_class = class_new(gensym("tab_carth2polar"), (t_newmethod)tab_carth2polar_new, (t_method)tab_carth2polar_free,
    sizeof(t_tab_carth2polar), 0, A_GIMME, 0);
  class_addbang(tab_carth2polar_class, (t_method)tab_carth2polar_bang);
  class_addlist(tab_carth2polar_class, (t_method)tab_carth2polar_list);
  class_addmethod(tab_carth2polar_class, (t_method)tab_carth2polar_src_re, gensym("src_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_carth2polar_class, (t_method)tab_carth2polar_src_im, gensym("src_im"), A_DEFSYMBOL, 0);
  class_addmethod(tab_carth2polar_class, (t_method)tab_carth2polar_src_re, gensym("src1_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_carth2polar_class, (t_method)tab_carth2polar_src_im, gensym("src1_im"), A_DEFSYMBOL, 0);
  class_addmethod(tab_carth2polar_class, (t_method)tab_carth2polar_dst_mag, gensym("dst_mag"), A_DEFSYMBOL, 0);
  class_addmethod(tab_carth2polar_class, (t_method)tab_carth2polar_dst_arg, gensym("dst_arg"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_carth2polar_class, gensym("iemhelp2/help-tab_carth2polar"));
}
