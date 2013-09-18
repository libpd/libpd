/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"

/* -------------------------- tab_complex_inv ------------------------------ */
/*   x_beg_mem_dst_re[i] = x_beg_mem_src1_re[i] / (x_beg_mem_src1_re[i]*x_beg_mem_src1_re[i] + x_beg_mem_src1_im[i]*x_beg_mem_src1_im[i])   */
/*   x_beg_mem_dst_im[i] = -x_beg_mem_src1_im[i] / (x_beg_mem_src1_re[i]*x_beg_mem_src1_re[i] + x_beg_mem_src1_im[i]*x_beg_mem_src1_im[i])   */

typedef struct _tab_complex_inv
{
  t_object  x_obj;
  int       x_size_src1_re;
  int       x_size_dst_re;
  int       x_size_src1_im;
  int       x_size_dst_im;
  int       x_offset_src1_re;
  int       x_offset_dst_re;
  int       x_offset_src1_im;
  int       x_offset_dst_im;
  iemarray_t   *x_beg_mem_src1_re;
  iemarray_t   *x_beg_mem_dst_re;
  iemarray_t   *x_beg_mem_src1_im;
  iemarray_t   *x_beg_mem_dst_im;
  t_symbol  *x_sym_scr1_re;
  t_symbol  *x_sym_dst_re;
  t_symbol  *x_sym_scr1_im;
  t_symbol  *x_sym_dst_im;
} t_tab_complex_inv;

static t_class *tab_complex_inv_class;

static void tab_complex_inv_src1_re(t_tab_complex_inv *x, t_symbol *s)
{
  x->x_sym_scr1_re = s;
}

static void tab_complex_inv_src1_im(t_tab_complex_inv *x, t_symbol *s)
{
  x->x_sym_scr1_im = s;
}

static void tab_complex_inv_dst_re(t_tab_complex_inv *x, t_symbol *s)
{
  x->x_sym_dst_re = s;
}

static void tab_complex_inv_dst_im(t_tab_complex_inv *x, t_symbol *s)
{
  x->x_sym_dst_im = s;
}

static void tab_complex_inv_bang(t_tab_complex_inv *x)
{
  int i, n;
  int ok_src1_re, ok_dst_re;
  int ok_src1_im, ok_dst_im;
  iemarray_t *vec_src1_re, *vec_dst_re;
  iemarray_t *vec_src1_im, *vec_dst_im;
  
  ok_src1_re = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_scr1_re, &x->x_beg_mem_src1_re, &x->x_size_src1_re, 0);
  ok_dst_re = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_dst_re, &x->x_beg_mem_dst_re, &x->x_size_dst_re, 0);
  ok_src1_im = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_scr1_im, &x->x_beg_mem_src1_im, &x->x_size_src1_im, 0);
  ok_dst_im = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_dst_im, &x->x_beg_mem_dst_im, &x->x_size_dst_im, 0);
  
  if(ok_src1_re && ok_dst_re && ok_src1_im && ok_dst_im)
  {
    if(x->x_size_src1_re < x->x_size_dst_re)
      n = x->x_size_src1_re;
    else
      n = x->x_size_dst_re;
    if(x->x_size_src1_im < n)
      n = x->x_size_src1_im;
    if(x->x_size_dst_im < n)
      n = x->x_size_dst_im;
    
    vec_src1_re = x->x_beg_mem_src1_re;
    vec_dst_re = x->x_beg_mem_dst_re;
    vec_src1_im = x->x_beg_mem_src1_im;
    vec_dst_im = x->x_beg_mem_dst_im;
    if(n)
    {
      t_garray *a;
      
      for(i=0; i<n; i++)
      {
        t_float re, im, abs;
        
        re = iemarray_getfloat(vec_src1_re, i);
        im = iemarray_getfloat(vec_src1_im, i);
        abs = 1.0f / (re*re + im*im);
        iemarray_setfloat(vec_dst_re, i, re*abs);
        iemarray_setfloat(vec_dst_im, i, -im*abs);
      }
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst_re, garray_class);
      garray_redraw(a);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst_im, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_complex_inv_list(t_tab_complex_inv *x, t_symbol *s, int argc, t_atom *argv)
{
  int i, n;
  int beg_src1_re, beg_dst_re;
  int beg_src1_im, beg_dst_im;
  int ok_src1_re, ok_dst_re;
  int ok_src1_im, ok_dst_im;
  iemarray_t *vec_src1_re, *vec_dst_re;
  iemarray_t *vec_src1_im, *vec_dst_im;
  
  if((argc >= 5) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2) &&
    IS_A_FLOAT(argv,3) &&
    IS_A_FLOAT(argv,4))
  {
    beg_src1_re = (int)atom_getintarg(0, argc, argv);
    beg_src1_im = (int)atom_getintarg(1, argc, argv);
    beg_dst_re = (int)atom_getintarg(2, argc, argv);
    beg_dst_im = (int)atom_getintarg(3, argc, argv);
    n = (int)atom_getintarg(4, argc, argv);
    if(beg_src1_re < 0)
      beg_src1_re = 0;
    if(beg_dst_re < 0)
      beg_dst_re = 0;
    if(beg_src1_im < 0)
      beg_src1_im = 0;
    if(beg_dst_im < 0)
      beg_dst_im = 0;
    if(n < 0)
      n = 0;
    
    ok_src1_re = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_scr1_re, &x->x_beg_mem_src1_re, &x->x_size_src1_re, beg_src1_re+n);
    ok_dst_re = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_dst_re, &x->x_beg_mem_dst_re, &x->x_size_dst_re, beg_dst_re+n);
    ok_src1_im = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_scr1_im, &x->x_beg_mem_src1_im, &x->x_size_src1_im, beg_src1_im+n);
    ok_dst_im = iem_tab_check_arrays(gensym("tab_complex_inv"), x->x_sym_dst_im, &x->x_beg_mem_dst_im, &x->x_size_dst_im, beg_dst_im+n);
    
    if(ok_src1_re && ok_dst_re && ok_src1_im && ok_dst_im)
    {
      vec_src1_re = x->x_beg_mem_src1_re + beg_src1_re;
      vec_dst_re = x->x_beg_mem_dst_re + beg_dst_re;
      vec_src1_im = x->x_beg_mem_src1_im + beg_src1_im;
      vec_dst_im = x->x_beg_mem_dst_im + beg_dst_im;
      
      if(n)
      {
        t_garray *a;
        
        for(i=0; i<n; i++)
        {
          t_float re, im, abs;
          
          re = iemarray_getfloat(vec_src1_re, i);
          im = iemarray_getfloat(vec_src1_im, i);
          abs = 1.0f / (re*re + im*im);
          iemarray_setfloat(vec_dst_re, i, re*abs);
          iemarray_setfloat(vec_dst_im, i, -im*abs);
        }
        outlet_bang(x->x_obj.ob_outlet);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst_re, garray_class);
        garray_redraw(a);
        a = (t_garray *)pd_findbyclass(x->x_sym_dst_im, garray_class);
        garray_redraw(a);
      }
    }
  }
  else
  {
    post("tab_complex_inv-ERROR: list need 5 float arguments:");
    post("  source1_real_offset + source1_imag_offset + destination_real_offset + destination_imag_offset + number_of_samples_to_complex_mul");
  }
}

static void tab_complex_inv_free(t_tab_complex_inv *x)
{
}

static void *tab_complex_inv_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_complex_inv *x = (t_tab_complex_inv *)pd_new(tab_complex_inv_class);
  t_symbol  *src1_re, *src2_re, *dst_re, *src1_im, *src2_im, *dst_im;
  
  if((argc >= 4) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
    IS_A_SYMBOL(argv,2) &&
    IS_A_SYMBOL(argv,3))
  {
    src1_re = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    src1_im = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst_re = (t_symbol *)atom_getsymbolarg(2, argc, argv);
    dst_im = (t_symbol *)atom_getsymbolarg(3, argc, argv);
  }
  else if((argc >= 2) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1))
  {
    src1_re = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    src1_im = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst_re = src1_re;
    dst_im = src1_im;
  }
  else
  {
    post("tab_complex_inv-ERROR: need 4 symbols arguments:");
    post("  source1_real_array_name + source1_imag_array_name + destination_real_array_name + destination_imag_array_name");
    return(0);
  }
  
  x->x_sym_scr1_re = src1_re;
  x->x_sym_scr1_im = src1_im;
  x->x_sym_dst_re = dst_re;
  x->x_sym_dst_im = dst_im;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_complex_inv_setup(void)
{
  tab_complex_inv_class = class_new(gensym("tab_complex_inv"), (t_newmethod)tab_complex_inv_new, (t_method)tab_complex_inv_free,
    sizeof(t_tab_complex_inv), 0, A_GIMME, 0);
  class_addbang(tab_complex_inv_class, (t_method)tab_complex_inv_bang);
  class_addlist(tab_complex_inv_class, (t_method)tab_complex_inv_list);
  class_addmethod(tab_complex_inv_class, (t_method)tab_complex_inv_src1_re, gensym("src1_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_complex_inv_class, (t_method)tab_complex_inv_src1_re, gensym("src_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_complex_inv_class, (t_method)tab_complex_inv_dst_re, gensym("dst_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_complex_inv_class, (t_method)tab_complex_inv_src1_im, gensym("src1_im"), A_DEFSYMBOL, 0);
  class_addmethod(tab_complex_inv_class, (t_method)tab_complex_inv_src1_im, gensym("src_im"), A_DEFSYMBOL, 0);
  class_addmethod(tab_complex_inv_class, (t_method)tab_complex_inv_dst_im, gensym("dst_im"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_complex_inv_class, gensym("iemhelp2/tab_complex_inv-help"));
}
