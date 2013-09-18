/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"
#include <math.h>


/* -------------------------- tab_rfft ------------------------------ */
/*   real time domain FFT to complex spectral domain   */

typedef struct _tab_rfft
{
  t_object  x_obj;
  int       x_size_src1;
  int       x_size_dst_re;
  int       x_size_dst_im;
  int       x_offset_src1;
  int       x_offset_dst_re;
  int       x_offset_dst_im;
  int       x_fftsize;
  iemarray_t   *x_beg_mem_src1;
  iemarray_t   *x_beg_mem_dst_re;
  iemarray_t   *x_beg_mem_dst_im;
  TAB_COMPLEX   *x_sin_cos;
  t_symbol  *x_sym_src1;
  t_symbol  *x_sym_dst_re;
  t_symbol  *x_sym_dst_im;
} t_tab_rfft;

static t_class *tab_rfft_class;

static void tab_rfft_init(t_tab_rfft *x)
{
  int i, fftsize = x->x_fftsize;
  t_float f, g;
  TAB_COMPLEX *sincos = x->x_sin_cos;
  
  g = 2.0 * 3.141592653589793 / (t_float)fftsize;
  for(i=0; i<fftsize; i++)
  {
    f = g * (t_float)i;
    (*sincos).real = cos(f);
    (*sincos).imag = -sin(f);/*FFT*/
    sincos++;
  }
}

static void tab_rfft_fftsize(t_tab_rfft *x, t_floatarg f)
{
  int i=1, fftsize = (int)f;
  
  if(fftsize < 8)
    fftsize = 8;
  
  while(i <= fftsize)
    i *= 2;
  i /= 2;
  
  if(i != x->x_fftsize)
  {
    x->x_sin_cos = (TAB_COMPLEX *)resizebytes(x->x_sin_cos, x->x_fftsize*sizeof(TAB_COMPLEX), i*sizeof(TAB_COMPLEX));
    x->x_fftsize = i;
  }
  tab_rfft_init(x);
}

static void tab_rfft_src(t_tab_rfft *x, t_symbol *s)
{
  x->x_sym_src1 = s;
}

static void tab_rfft_dst_re(t_tab_rfft *x, t_symbol *s)
{
  x->x_sym_dst_re = s;
}

static void tab_rfft_dst_im(t_tab_rfft *x, t_symbol *s)
{
  x->x_sym_dst_im = s;
}

static void tab_rfft_bang(t_tab_rfft *x)
{
  int i, j, k;
  int ok_src, ok_dst_re, ok_dst_im;
  int w_index, w_inc, i_inc, v_index;
  int fftsize = x->x_fftsize;
  int fs1 = fftsize - 1;
  int fs2 = fftsize / 2;
  TAB_COMPLEX w;
  TAB_COMPLEX *sincos = x->x_sin_cos;
  iemarray_t *vec_src, *vec_dst_re, *vec_dst_im;
  t_float old1_re, old1_im, old2_re, old2_im;
  
  ok_src = iem_tab_check_arrays(gensym("tab_rfft"), x->x_sym_src1, &x->x_beg_mem_src1, &x->x_size_src1, fftsize);
  ok_dst_re = iem_tab_check_arrays(gensym("tab_rfft"), x->x_sym_dst_re, &x->x_beg_mem_dst_re, &x->x_size_dst_re, fftsize);
  ok_dst_im = iem_tab_check_arrays(gensym("tab_rfft"), x->x_sym_dst_im, &x->x_beg_mem_dst_im, &x->x_size_dst_im, fftsize);
  if(ok_src && ok_dst_re && ok_dst_im)
  {
    t_garray *a;
    
    vec_src=x->x_beg_mem_src1;
    vec_dst_re=x->x_beg_mem_dst_re;
    vec_dst_im=x->x_beg_mem_dst_im;
    
    for(k=0; k<fftsize; k++)
    {
      iemarray_setfloat(vec_dst_re, k, iemarray_getfloat(vec_src, k));
      iemarray_setfloat(vec_dst_im, k, 0.0);
    }
    
    i_inc = fs2;
    w_inc = 1;
    for(i=1; i<fftsize; i<<=1)
    {
      v_index = 0;
      for(j=0; j<i; j++)
      {
        w_index = 0;
        for(k=0; k<i_inc; k++)
        {
          old1_re = iemarray_getfloat(vec_dst_re, v_index);
          old1_im = iemarray_getfloat(vec_dst_im, v_index);
          old2_re = iemarray_getfloat(vec_dst_re, v_index+i_inc);
          old2_im = iemarray_getfloat(vec_dst_im, v_index+i_inc);
          
          w = sincos[w_index];
          iemarray_setfloat(vec_dst_re, v_index+i_inc, (old1_re - old2_re)*w.real - (old1_im - old2_im)*w.imag);
          iemarray_setfloat(vec_dst_im, v_index+i_inc, (old1_im - old2_im)*w.real + (old1_re - old2_re)*w.imag);
          iemarray_setfloat(vec_dst_re, v_index, old1_re + old2_re);
          iemarray_setfloat(vec_dst_im, v_index, old1_im + old2_im);
          w_index += w_inc;
          v_index++;
        }
        v_index += i_inc;
      }
      w_inc <<= 1;
      i_inc >>= 1;
    }
    
    j = 0;
    for(i=1;i<fs1;i++)
    {
      k = fs2;
      while(k <= j)
      {
        j = j - k;
        k >>= 1;
      }
      j = j + k;
      if(i < j)
      {
        old1_re = iemarray_getfloat(vec_dst_re, j);
        old1_im = iemarray_getfloat(vec_dst_im, j);
        iemarray_setfloat(vec_dst_re, j, iemarray_getfloat(vec_dst_re, i));
        iemarray_setfloat(vec_dst_im, j, iemarray_getfloat(vec_dst_im, i));
        iemarray_setfloat(vec_dst_re, i, old1_re);
        iemarray_setfloat(vec_dst_im, i, old1_im);
      }
    }
    
    //    g = 2.0f / (t_float)fftsize;
    /*
    ein fehler tritt auf beim 0.sample, hier sollte nur mal 1.0 multipliziert werden
    wenn gelten soll : Energie im zeitfenster == Energie im Frequenz-dichte-fenster
    
      g = 1.0f;
      for(i = 0; i < fs2; i++)
      {
      vec_dst_re[i] *= g;
      vec_dst_im[i] *= g;
      }
    */
    
    iemarray_setfloat(vec_dst_im, 0, 0.0);
    iemarray_setfloat(vec_dst_im, fs2, 0.0);
    for(i = fs2+1; i < fftsize; i++)
    {
      iemarray_setfloat(vec_dst_re, i, 0.0);
      iemarray_setfloat(vec_dst_im, i, 0.0);
    }
    
    outlet_bang(x->x_obj.ob_outlet);
    a = (t_garray *)pd_findbyclass(x->x_sym_dst_re, garray_class);
    garray_redraw(a);
    a = (t_garray *)pd_findbyclass(x->x_sym_dst_im, garray_class);
    garray_redraw(a);
  }
}

static void tab_rfft_list(t_tab_rfft *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_src, beg_dst_re, beg_dst_im;
  int i, j, k;
  int ok_src, ok_dst_re, ok_dst_im;
  int w_index, w_inc, i_inc, v_index;
  int fftsize = x->x_fftsize;
  int fs1 = fftsize - 1;
  int fs2 = fftsize / 2;
  TAB_COMPLEX w;
  TAB_COMPLEX *sincos = x->x_sin_cos;
  iemarray_t *vec_src, *vec_dst_re, *vec_dst_im;
  t_float old1_re, old1_im, old2_re, old2_im;
  
  if((argc >= 3) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2))
  {
    beg_src = (int)atom_getintarg(0, argc, argv);
    beg_dst_re = (int)atom_getintarg(1, argc, argv);
    beg_dst_im = (int)atom_getintarg(2, argc, argv);
    if(beg_src < 0)
      beg_src = 0;
    if(beg_dst_re < 0)
      beg_dst_re = 0;
    if(beg_dst_im < 0)
      beg_dst_im = 0;
    
    ok_src = iem_tab_check_arrays(gensym("tab_rfft"), x->x_sym_src1, &x->x_beg_mem_src1, &x->x_size_src1, beg_src+fftsize);
    ok_dst_re = iem_tab_check_arrays(gensym("tab_rfft"), x->x_sym_dst_re, &x->x_beg_mem_dst_re, &x->x_size_dst_re, beg_dst_re+fftsize);
    ok_dst_im = iem_tab_check_arrays(gensym("tab_rfft"), x->x_sym_dst_im, &x->x_beg_mem_dst_im, &x->x_size_dst_im, beg_dst_im+fftsize);
    
    if(ok_src && ok_dst_re && ok_dst_im)
    {
      t_garray *a;
      
      vec_src=x->x_beg_mem_src1 + beg_src;
      vec_dst_re=x->x_beg_mem_dst_re + beg_dst_re;
      vec_dst_im=x->x_beg_mem_dst_im + beg_dst_im;
      
      for(k=0; k<fftsize; k++)
      {
        iemarray_setfloat(vec_dst_re, k, iemarray_getfloat(vec_src, k));
        iemarray_setfloat(vec_dst_im, k, 0.0);
      }
      
      i_inc = fs2;
      w_inc = 1;
      for(i=1; i<fftsize; i<<=1)
      {
        v_index = 0;
        for(j=0; j<i; j++)
        {
          w_index = 0;
          for(k=0; k<i_inc; k++)
          {
            old1_re = iemarray_getfloat(vec_dst_re, v_index);
            old1_im = iemarray_getfloat(vec_dst_im, v_index);
            old2_re = iemarray_getfloat(vec_dst_re, v_index+i_inc);
            old2_im = iemarray_getfloat(vec_dst_im, v_index+i_inc);
            
            w = sincos[w_index];
            iemarray_setfloat(vec_dst_re, v_index+i_inc, (old1_re - old2_re)*w.real - (old1_im - old2_im)*w.imag);
            iemarray_setfloat(vec_dst_im, v_index+i_inc, (old1_im - old2_im)*w.real + (old1_re - old2_re)*w.imag);
            iemarray_setfloat(vec_dst_re, v_index, old1_re + old2_re);
            iemarray_setfloat(vec_dst_im, v_index, old1_im + old2_im);
            w_index += w_inc;
            v_index++;
          }
          v_index += i_inc;
        }
        w_inc <<= 1;
        i_inc >>= 1;
      }
      
      j = 0;
      for(i=1;i<fs1;i++)
      {
        k = fs2;
        while(k <= j)
        {
          j = j - k;
          k >>= 1;
        }
        j = j + k;
        if(i < j)
        {
          old1_re = iemarray_getfloat(vec_dst_re, j);
          old1_im = iemarray_getfloat(vec_dst_im, j);
          iemarray_setfloat(vec_dst_re, j, iemarray_getfloat(vec_dst_re, i));
          iemarray_setfloat(vec_dst_im, j, iemarray_getfloat(vec_dst_im, i));
          iemarray_setfloat(vec_dst_re, i, old1_re);
          iemarray_setfloat(vec_dst_im, i, old1_im);
        }
      }
      
      //    g = 2.0f / (t_float)fftsize;
      /*
      ein fehler tritt auf beim 0.sample, hier sollte nur mal 1.0 multipliziert werden
      wenn gelten soll : Energie im zeitfenster == Energie im Frequenz-dichte-fenster
      
        g = 1.0f;
        for(i = 0; i < fs2; i++)
        {
        vec_dst_re[i] *= g;
        vec_dst_im[i] *= g;
        }
      */
      
      iemarray_setfloat(vec_dst_im, 0, 0.0);
      iemarray_setfloat(vec_dst_im, fs2, 0.0);
      for(i = fs2+1; i < fftsize; i++)
      {
        iemarray_setfloat(vec_dst_re, i, 0.0);
        iemarray_setfloat(vec_dst_im, i, 0.0);
      }
      
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst_re, garray_class);
      garray_redraw(a);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst_im, garray_class);
      garray_redraw(a);
    }
  }
  else
  {
    post("tab_rfft-ERROR: list need 3 float arguments:");
    post("  source_offset + destination_real_offset + destination_imag_offset");
  }
}

static void tab_rfft_free(t_tab_rfft *x)
{
  freebytes(x->x_sin_cos, x->x_fftsize * sizeof(TAB_COMPLEX));
}

static void *tab_rfft_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_rfft *x = (t_tab_rfft *)pd_new(tab_rfft_class);
  t_symbol  *src, *dst_re, *dst_im;
  int fftsize, i=1;
  
  if((argc >= 4) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
    IS_A_SYMBOL(argv,2) &&
    IS_A_FLOAT(argv,3))
  {
    src = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    dst_re = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst_im = (t_symbol *)atom_getsymbolarg(2, argc, argv);
    fftsize = (int)atom_getintarg(3, argc, argv);
  }
  else
  {
    post("tab_rfft-ERROR: need 3 symbols + 1 float arguments:");
    post("  source_array_name + destination_real_array_name + destination_imag_array_name + FFT-size");
    return(0);
  }
  
  if(fftsize < 8)
    fftsize = 8;
  
  while(i <= fftsize)
    i *= 2;
  i /= 2;
  fftsize = i;
  
  x->x_fftsize = fftsize;
  x->x_sym_src1 = src;
  x->x_sym_dst_re = dst_re;
  x->x_sym_dst_im = dst_im;
  x->x_sin_cos = (TAB_COMPLEX *)getbytes(x->x_fftsize * sizeof(TAB_COMPLEX));
  tab_rfft_init(x);
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_rfft_setup(void)
{
  tab_rfft_class = class_new(gensym("tab_rfft"), (t_newmethod)tab_rfft_new, (t_method)tab_rfft_free,
    sizeof(t_tab_rfft), 0, A_GIMME, 0);
  class_addbang(tab_rfft_class, (t_method)tab_rfft_bang);
  class_addlist(tab_rfft_class, (t_method)tab_rfft_list);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_fftsize, gensym("fftsize"), A_DEFFLOAT, 0);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_src, gensym("src"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_src, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_dst_re, gensym("dst_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_dst_im, gensym("dst_im"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_dst_re, gensym("dst1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rfft_class, (t_method)tab_rfft_dst_im, gensym("dst2"), A_DEFSYMBOL, 0);
}
