/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"


/* -------------------------- tab_mls ------------------------------ */

typedef struct _tab_mls
{
  t_object  x_obj;
  int       x_size_dst;
  int       x_offset_dst;
  iemarray_t   *x_beg_mem_dst;
  t_symbol  *x_sym_dst;
} t_tab_mls;

static t_class *tab_mls_class;

static int tab_mls_exp2(int mls_order)
{
  int i, j=1;
  
  for(i=0; i<mls_order; i++)
    j *= 2;
  return(j);
}

static void tab_mls_calc(iemarray_t *vec, int mls_order)
{
  int i, j;
  int work1=1, work2, exor;
  int mask, source, mls_size;
  
  switch(mls_order)
  {
  case 3:
    mask = 2+1;
    source = 4;
    break;
  case 4:
    mask = 2+1;
    source = 8;
    break;
  case 5:
    mask = 4+1;
    source = 16;
    break;
  case 6:
    mask = 2+1;
    source = 32;
    break;
  case 7:
    mask = 8+1;
    source = 64;
    break;
  case 8:
    mask = 32+8+2+1;
    source = 128;
    break;
  case 9:
    mask = 16+1;
    source = 256;
    break;
  case 10:
    mask = 8+1;
    source = 512;
    break;
  case 11:
    mask = 4+1;
    source = 1024;
    break;
  case 12:
    mask = 64+16+2+1;
    source = 2048;
    break;
  case 13:
    mask = 256+128+8+1;
    source = 4096;
    break;
  case 14:
    mask = 1024+64+2+1;
    source = 8192;
    break;
  case 15:
    mask = 2+1;
    source = 16384;
    break;
  case 16:
    mask = 32+8+4+1;
    source = 32768;
    break;
  case 17:
    mask = 8+1;
    source = 65536;
    break;
  case 18:
    mask = 128+1;
    source = 131072;
    break;
  case 19:
    mask = 32+4+2+1;
    source = 262144;
    break;
  case 20:
    mask = 8+1;
    source = 524288;
    break;
  }
  
  mls_size = 2*source - 1;
  
  for(i=0; i<mls_size; i++)
  {
    i = work1 & mask;
    work2 = work1 >> 1;
    exor = 0;
    for(j=0; j<mls_order; j++)
    {
      exor += 1; //??? exor +=  & 1; ???
      work1 >>= 1;
    }
    if(exor & 1)
    {
      iemarray_setfloat(vec, i, 1.0f);
      work1 = work2 | source;
    }
    else
    {
      iemarray_setfloat(vec, i, -1.0f);
      work1 = work2;
    }
  }
  return;
}

static void tab_mls_dst(t_tab_mls *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_mls_float(t_tab_mls *x, t_floatarg fmls_order)
{
  int mls_order=(int)fmls_order;
  int ok_dst, mls_size;
  
  mls_size = tab_mls_exp2(mls_order) - 1;
  ok_dst = iem_tab_check_arrays(gensym("tab_mls"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, mls_size);
  
  if(ok_dst)
  {
    if((mls_order >= 3) && (mls_order <= 20))
    {
      t_garray *a;
      
      tab_mls_calc(x->x_beg_mem_dst, mls_order);
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
}

static void tab_mls_list(t_tab_mls *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_dst;
  int i, n;
  int ok_dst;
  t_float c;
  iemarray_t *vec_dst;
  
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
    
    ok_dst = iem_tab_check_arrays(gensym("tab_mls"), x->x_sym_dst, &x->x_beg_mem_dst, &x->x_size_dst, beg_dst+n);
    
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
    post("tab_mls-ERROR: list need 3 float arguments:");
    post("  destination_offset + number_of_samples_to_copy + constant-value");
  }
}

static void tab_mls_free(t_tab_mls *x)
{
}

static void *tab_mls_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_mls *x = (t_tab_mls *)pd_new(tab_mls_class);
  t_symbol  *dst;
  t_float time;
  
  if((argc >= 1) &&
    IS_A_SYMBOL(argv,0))
  {
    dst = (t_symbol *)atom_getsymbolarg(0, argc, argv);
  }
  else
  {
    post("tab_mls-ERROR: need 1 symbol argument:");
    post("  destination_array_name");
    return(0);
  }
  
  x->x_sym_dst = dst;
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_mls_setup(void)
{
  tab_mls_class = class_new(gensym("tab_mls"), (t_newmethod)tab_mls_new, (t_method)tab_mls_free,
    sizeof(t_tab_mls), 0, A_GIMME, 0);
  class_addfloat(tab_mls_class, (t_method)tab_mls_float);
  class_addlist(tab_mls_class, (t_method)tab_mls_list);
  class_addmethod(tab_mls_class, (t_method)tab_mls_dst, gensym("dst"), A_DEFSYMBOL, 0);
//  class_sethelpsymbol(tab_mls_class, gensym("iemhelp2/tab_mls-help"));
}
