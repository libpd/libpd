/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */


#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"


/* ---------------------------- tab_conv ------------------------------- */
/*   for(i=0; i<x_size_src1; i++)                                        */
/*   {                                                                   */
/*     sum = 0.0f;                                                       */
/*     for(j=0; j<x_size_src2; j++)                                      */
/*       sum += x_beg_mem_src1[i+j-x_size_src2/2] * x_beg_mem_src2[j];   */
/*     x_beg_mem_dst[i] = sum;                                           */
/*   }                                                                   */

typedef struct _tab_conv
{
  t_object  x_obj;
  int       x_size_src1;
  int       x_size_src2;
  int       x_size_dst;
  int       x_offset_src1;
  int       x_offset_src2;
  int       x_offset_dst;
  iemarray_t   *x_beg_mem_src1;
  iemarray_t   *x_beg_mem_src2;
  iemarray_t   *x_beg_mem_dst;
  t_symbol  *x_sym_scr1;
  t_symbol  *x_sym_scr2;
  t_symbol  *x_sym_dst;
} t_tab_conv;

static t_class *tab_conv_class;

static void tab_conv_src1(t_tab_conv *x, t_symbol *s)
{
  x->x_sym_scr1 = s;
}

static void tab_conv_src2(t_tab_conv *x, t_symbol *s)
{
  x->x_sym_scr2 = s;
}

static void tab_conv_dst(t_tab_conv *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_conv_bang(t_tab_conv *x)
{
  int i, j, k, l, m, n, p, q;
  int ok_src1, ok_src2, ok_dst;
  iemarray_t *vec_sig, *vec_ir, *vec_dst;
  t_float sum=0.0f;
  
  ok_src1 = iem_tab_check_arrays(gensym("tab_conv"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, 0);
  ok_src2 = iem_tab_check_arrays(gensym("tab_conv"), x->x_sym_scr2, &x->x_beg_mem_src2, &x->x_size_src2, 0);
  ok_dst = iem_tab_check_arrays(gensym("tab_conv"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, 0);
  
  if(ok_src1 && ok_src2 && ok_dst)
  {
    t_garray *a;
    
    if((x->x_size_src1+x->x_size_src2-1) <= x->x_size_dst)// ok, the last part of dst is zero
    {
      if(x->x_size_src1 > x->x_size_src2)//  src2(t-tau) is impuls response
      {
        vec_sig = x->x_beg_mem_src1;
        vec_ir = x->x_beg_mem_src2;
        n = x->x_size_src1;
        m = x->x_size_src2;
      }
      else//  src1(t-tau) is impuls response
      {
        vec_sig = x->x_beg_mem_src2;
        vec_ir = x->x_beg_mem_src1;
        n = x->x_size_src2;
        m = x->x_size_src1;
      }
      vec_dst = x->x_beg_mem_dst;
      
      for(i=1; i<m; i++)
      {
        sum = 0.0f;
        for(j=0; j<i; j++)
          sum += iemarray_getfloat(vec_sig, -j) * iemarray_getfloat(vec_ir, j);
        iemarray_setfloat(vec_dst, 0, sum);
        vec_sig++;
        vec_dst++;
      }
      
      l = n - m + 1;
      for(i=0; i<l; i++)
      {
        sum = 0.0f;
        for(j=0; j<m; j++)
          sum += iemarray_getfloat(vec_sig, -j) * iemarray_getfloat(vec_ir, j);
        iemarray_setfloat(vec_dst, 0, sum);
        vec_sig++;
        vec_dst++;
      }
      
      for(i=m-1, k=0; i>0; i--, k++)
      {
        sum = 0.0f;
        for(j=1; j<=i; j++)
          sum += iemarray_getfloat(vec_sig, -j) * iemarray_getfloat(vec_ir, j+k);
        iemarray_setfloat(vec_dst, 0, sum);
        vec_dst++;
      }
    }
    else
    {
      if(x->x_size_src1 > x->x_size_src2)//  src2(t-tau) is impuls response
      {
        vec_sig = x->x_beg_mem_src1;
        vec_ir = x->x_beg_mem_src2;
        n = x->x_size_src1;
        m = x->x_size_src2;
      }
      else//  src1(t-tau) is impuls response
      {
        vec_sig = x->x_beg_mem_src2;
        vec_ir = x->x_beg_mem_src1;
        n = x->x_size_src2;
        m = x->x_size_src1;
      }
      vec_dst = x->x_beg_mem_dst;
      p = x->x_size_dst;
      q = 0;
      
      for(i=1; i<m; i++)
      {
        sum = 0.0f;
        for(j=0; j<i; j++)
          sum += iemarray_getfloat(vec_sig, -j) * iemarray_getfloat(vec_ir, j);
        iemarray_setfloat(vec_dst, 0, sum);
        vec_sig++;
        vec_dst++;
        q++;
        if(q >= p)
          goto tab_conv_bang_end;
      }
      
      l = n - m + 1;
      for(i=0; i<l; i++)
      {
        sum = 0.0f;
        for(j=0; j<m; j++)
          sum += iemarray_getfloat(vec_sig, -j) * iemarray_getfloat(vec_ir, j);
        iemarray_setfloat(vec_dst, 0, sum);
        vec_sig++;
        vec_dst++;
        q++;
        if(q >= p)
          goto tab_conv_bang_end;
      }
      
      for(i=m-1, k=0; i>0; i--, k++)
      {
        sum = 0.0f;
        for(j=1; j<=i; j++)
          sum += iemarray_getfloat(vec_sig, -j) * iemarray_getfloat(vec_ir, j+k);
        iemarray_setfloat(vec_dst, 0, sum);
        vec_dst++;
        q++;
        if(q >= p)
          goto tab_conv_bang_end;
      }
      for(i=m-1; i>0; i--)
      {
        sum = 0.0f;
        for(j=0, k=i-1; j<i; j++, k--)
        {
          sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          post("dst_%d=sig_%d*ir_%d=%g*%g",q,j+q,k,iemarray_getfloat(vec_sig, j),iemarray_getfloat(vec_ir, k));
        }
        iemarray_setfloat(vec_dst, 0, sum);
        vec_sig++;
        vec_dst++;
        q++;
        if(q >= p)
          goto tab_conv_bang_end;
      }
    }
tab_conv_bang_end:
    outlet_bang(x->x_obj.ob_outlet);
    a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
    garray_redraw(a);
  }
}

static void tab_conv_list(t_tab_conv *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_src1, beg_src2, beg_dst;
  int n_src1, n_src2;
  int i, j, k, l, m, n, p;
  int ok_src1, ok_src2, ok_dst;
  iemarray_t *vec_sig, *vec_ir, *vec_dst;
  t_float sum=0.0f;
  
  if((argc >= 5) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2) &&
    IS_A_FLOAT(argv,3) &&
    IS_A_FLOAT(argv,4))
  {
    beg_src1 = (int)atom_getintarg(0, argc, argv);
    beg_src2 = (int)atom_getintarg(1, argc, argv);
    beg_dst = (int)atom_getintarg(2, argc, argv);
    n_src1 = (int)atom_getintarg(3, argc, argv);
    n_src2 = (int)atom_getintarg(4, argc, argv);
    if(beg_src1 < 0)
      beg_src1 = 0;
    if(beg_src2 < 0)
      beg_src2 = 0;
    if(beg_dst < 0)
      beg_dst = 0;
    if(n_src1 < 0)
      n_src1 = 0;
    if(n_src2 < 0)
      n_src2 = 0;
    
    ok_src1 = iem_tab_check_arrays(gensym("tab_conv"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, beg_src1+n_src1);
    ok_src2 = iem_tab_check_arrays(gensym("tab_conv"), x->x_sym_scr2, &x->x_beg_mem_src2, &x->x_size_src2, beg_src2+n_src2);
    ok_dst = iem_tab_check_arrays(gensym("tab_conv"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, beg_dst+n_src1);
    
    if(ok_src1 && ok_src2 && ok_dst)
    {
      t_garray *a;
      
      if((x->x_size_src1+x->x_size_src2-1) <= x->x_size_dst)// ok, the last part of dst is zero
      {
        if(x->x_size_src1 > x->x_size_src2)//  src2(t-tau) is impuls response
        {
          vec_sig = x->x_beg_mem_src1;
          vec_ir = x->x_beg_mem_src2;
          n = x->x_size_src1;
          m = x->x_size_src2;
        }
        else//  src1(t-tau) is impuls response
        {
          vec_sig = x->x_beg_mem_src2;
          vec_ir = x->x_beg_mem_src1;
          n = x->x_size_src2;
          m = x->x_size_src1;
        }
        vec_dst = x->x_beg_mem_dst;
        
        l = m - 1;
        for(i=0; i<l; i++)
        {
          sum = 0.0f;
          for(j=0, k=i-1; j<i; j++, k--)
            sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          iemarray_setfloat(vec_dst, 0, sum);
          vec_sig++;
          vec_dst++;
        }
        
        l = n - m + 1;
        for(i=0; i<l; i++)
        {
          sum = 0.0f;
          for(j=0, k=m-1; j<m; j++, k--)
            sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          iemarray_setfloat(vec_dst, 0, sum);
          vec_sig++;
          vec_dst++;
        }
        
        l = m - 1;
        for(i=l-1; i>=0; i--)
        {
          sum = 0.0f;
          for(j=0, k=i-1; j<i; j++, k--)
            sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          iemarray_setfloat(vec_dst, 0, sum);
          vec_sig++;
          vec_dst++;
        }
      }
      else
      {
        if(x->x_size_src1 > x->x_size_src2)//  src2(t-tau) is impuls response
        {
          vec_sig = x->x_beg_mem_src1;
          vec_ir = x->x_beg_mem_src2;
          n = x->x_size_src1;
          m = x->x_size_src2;
        }
        else//  src1(t-tau) is impuls response
        {
          vec_sig = x->x_beg_mem_src2;
          vec_ir = x->x_beg_mem_src1;
          n = x->x_size_src2;
          m = x->x_size_src1;
        }
        vec_dst = x->x_beg_mem_dst;
        p = x->x_size_dst;
        k = 0;
        
        l = m - 1;
        for(i=0; i<l; i++)
        {
          sum = 0.0f;
          for(j=0, k=i-1; j<i; j++, k--)
            sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          iemarray_setfloat(vec_dst, 0, sum);
          vec_sig++;
          vec_dst++;
          k++;
          if(k >= p)
            goto tab_conv_list_end;
        }
        
        l = n - m + 1;
        for(i=0; i<l; i++)
        {
          sum = 0.0f;
          for(j=0, k=m-1; j<m; j++, k--)
            sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          iemarray_setfloat(vec_dst, 0, sum);
          vec_sig++;
          vec_dst++;
          k++;
          if(k >= p)
            goto tab_conv_list_end;
        }
        
        l = m - 1;
        for(i=l-1; i>=0; i--)
        {
          sum = 0.0f;
          for(j=0, k=i-1; j<i; j++, k--)
            sum += iemarray_getfloat(vec_sig, j) * iemarray_getfloat(vec_ir, k);
          iemarray_setfloat(vec_dst, 0, sum);
          vec_sig++;
          vec_dst++;
          k++;
          if(k >= p)
            goto tab_conv_list_end;
        }
      }
tab_conv_list_end:
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
  else
  {
    post("tab_conv-ERROR: list need 5 float arguments:");
    post("  source1_offset + source2_offset + destination_offset + number_of_samples_to_convolute + convolution_window_width");
  }
}

static void tab_conv_free(t_tab_conv *x)
{
}

static void *tab_conv_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_conv *x = (t_tab_conv *)pd_new(tab_conv_class);
  t_symbol  *src1, *src2, *dst;
  
  if((argc >= 3) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
    IS_A_SYMBOL(argv,2))
  {
    src1 = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    src2 = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst = (t_symbol *)atom_getsymbolarg(2, argc, argv);
  }
  else if((argc >= 2) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1))
  {
    src1 = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    dst = src1;
    src2 = (t_symbol *)atom_getsymbolarg(1, argc, argv);
  }
  else
  {
    post("tab_conv-ERROR: need 3 symbols arguments:");
    post("  source1_array_name + source2_array_name + destination_array_name");
    return(0);
  }
  
  x->x_sym_scr1 = src1;
  x->x_sym_scr2 = src2;
  x->x_sym_dst = dst;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_conv_setup(void)
{
  tab_conv_class = class_new(gensym("tab_conv"), (t_newmethod)tab_conv_new, (t_method)tab_conv_free,
    sizeof(t_tab_conv), 0, A_GIMME, 0);
  class_addbang(tab_conv_class, (t_method)tab_conv_bang);
 // class_addlist(tab_conv_class, (t_method)tab_conv_list);
  class_addmethod(tab_conv_class, (t_method)tab_conv_src1, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_conv_class, (t_method)tab_conv_src2, gensym("src2"), A_DEFSYMBOL, 0);
  class_addmethod(tab_conv_class, (t_method)tab_conv_dst, gensym("dst"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_conv_class, gensym("iemhelp2/tab_conv-help"));
}
