/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"
#include <math.h>


/* -------------------------- tab_rifft ------------------------------ */
/*   complex spectral domain inverse FFT to real time domain   */

typedef struct _tab_rifft
{
  t_object  x_obj;
  int       x_size_src_re;
  int       x_size_src_im;
  int       x_size_dst;
  int       x_offset_src_re;
  int       x_offset_src_im;
  int       x_offset_dst;
  int       x_fftsize;
  iemarray_t   *x_beg_mem_src_re;
  iemarray_t   *x_beg_mem_src_im;
  iemarray_t   *x_beg_mem_dst_re;
  iemarray_t   *x_beg_mem_dst_im;
  TAB_COMPLEX   *x_sin_cos;
  t_symbol  *x_sym_src_re;
  t_symbol  *x_sym_src_im;
  t_symbol  *x_sym_dst;
} t_tab_rifft;

static t_class *tab_rifft_class;

static void tab_rifft_init(t_tab_rifft *x)
{
  int i, fftsize = x->x_fftsize;
  t_float f, g;
  TAB_COMPLEX *sincos = x->x_sin_cos;
  
  g = 2.0 * 3.141592653589793 / (t_float)fftsize;
  for(i=0; i<fftsize; i++)
  {
    f = g * (t_float)i;
    (*sincos).real = cos(f);
    (*sincos).imag = sin(f);/*IFFT*/
    sincos++;
  }
}

static void tab_rifft_ifftsize(t_tab_rifft *x, t_floatarg f)
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
    x->x_beg_mem_dst_im = (iemarray_t *)resizebytes(x->x_beg_mem_dst_im, x->x_fftsize*sizeof(iemarray_t), i*sizeof(iemarray_t));
    x->x_fftsize = i;
  }
  tab_rifft_init(x);
}

static void tab_rifft_dst(t_tab_rifft *x, t_symbol *s)
{
  x->x_sym_dst = s;
}

static void tab_rifft_src_re(t_tab_rifft *x, t_symbol *s)
{
  x->x_sym_src_re = s;
}

static void tab_rifft_src_im(t_tab_rifft *x, t_symbol *s)
{
  x->x_sym_src_im = s;
}

static void tab_rifft_bang(t_tab_rifft *x)
{
  int i, j, k;
  int ok_src_re, ok_src_im, ok_dst;
  int w_index, w_inc, i_inc, v_index;
  int fftsize = x->x_fftsize;
  int fs1 = fftsize - 1;
  int fs2 = fftsize / 2;
  TAB_COMPLEX w;
  TAB_COMPLEX *sincos = x->x_sin_cos;
  iemarray_t *vec_src_re, *vec_src_im, *vec_dst_re, *vec_dst_im;
  t_float old1_re, old1_im, old2_re, old2_im, g;
  
  ok_src_re = iem_tab_check_arrays(gensym("tab_rifft"), x->x_sym_src_re, &x->x_beg_mem_src_re, &x->x_size_src_re, fftsize);
  ok_src_im = iem_tab_check_arrays(gensym("tab_rifft"), x->x_sym_src_im, &x->x_beg_mem_src_im, &x->x_size_src_im, fftsize);
  ok_dst = iem_tab_check_arrays(gensym("tab_rifft"), x->x_sym_dst, &x->x_beg_mem_dst_re, &x->x_size_dst, fftsize);
  
  if(ok_src_re && ok_src_im && ok_dst)
  {
    t_garray *a;
    
    vec_src_re=x->x_beg_mem_src_re;
    vec_src_im=x->x_beg_mem_src_im;
    vec_dst_re=x->x_beg_mem_dst_re;
    vec_dst_im=x->x_beg_mem_dst_im;
    
    iemarray_setfloat(vec_dst_re, 0, iemarray_getfloat(vec_src_re, 0));
    iemarray_setfloat(vec_dst_im, 0, 0.0);
    for(j=1; j<fs2; j++)
    {
      iemarray_setfloat(vec_dst_re, j, iemarray_getfloat(vec_src_re, j));
      iemarray_setfloat(vec_dst_im, j, iemarray_getfloat(vec_src_im, j));
    }
    iemarray_setfloat(vec_dst_re, fs2, iemarray_getfloat(vec_src_re, fs2));
    iemarray_setfloat(vec_dst_im, fs2, 0.0);
    
    for(k=1, j=fftsize-1; k<fs2; k++, j--)
    {
      iemarray_setfloat(vec_dst_re, j, iemarray_getfloat(vec_dst_re, k));
      iemarray_setfloat(vec_dst_im, j, -iemarray_getfloat(vec_dst_im, k));
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
    
    g = 1.0 / (t_float)fftsize;
    for(i = 0; i < fftsize; i++)
    {
      iemarray_setfloat(vec_dst_re, i, iemarray_getfloat(vec_dst_re, i)*g);
    }
    
    outlet_bang(x->x_obj.ob_outlet);
    a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
    garray_redraw(a);
  }
}

static void tab_rifft_list(t_tab_rifft *x, t_symbol *s, int argc, t_atom *argv)
{
  int beg_src_re, beg_src_im, beg_dst;
  int i, j, k;
  int ok_src_re, ok_src_im, ok_dst;
  int w_index, w_inc, i_inc, v_index;
  int fftsize = x->x_fftsize;
  int fs1 = fftsize - 1;
  int fs2 = fftsize / 2;
  TAB_COMPLEX w;
  TAB_COMPLEX *sincos = x->x_sin_cos;
  iemarray_t *vec_src_re, *vec_src_im, *vec_dst_re, *vec_dst_im;
  t_float old1_re, old1_im, old2_re, old2_im, g;
  
  if((argc >= 3) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2))
  {
    beg_src_re = (int)atom_getintarg(0, argc, argv);
    beg_src_im = (int)atom_getintarg(1, argc, argv);
    beg_dst = (int)atom_getintarg(2, argc, argv);
    if(beg_src_re < 0)
      beg_src_re = 0;
    if(beg_src_im < 0)
      beg_src_im = 0;
    if(beg_dst < 0)
      beg_dst = 0;
    
    ok_src_re = iem_tab_check_arrays(gensym("tab_rifft"), x->x_sym_src_re, &x->x_beg_mem_src_re, &x->x_size_src_re, beg_src_re+fftsize);
    ok_src_im = iem_tab_check_arrays(gensym("tab_rifft"), x->x_sym_src_im, &x->x_beg_mem_src_im, &x->x_size_src_im, beg_src_im+fftsize);
    ok_dst = iem_tab_check_arrays(gensym("tab_rifft"), x->x_sym_dst, &x->x_beg_mem_dst_re, &x->x_size_dst, beg_dst+fftsize);
    
    if(ok_src_re && ok_src_im && ok_dst)
    {
      t_garray *a;
      
      vec_src_re=x->x_beg_mem_src_re + beg_src_re;
      vec_src_im=x->x_beg_mem_src_im + beg_src_im;
      vec_dst_re=x->x_beg_mem_dst_re + beg_dst;
      vec_dst_im=x->x_beg_mem_dst_im;
      
      for(j=0; j<fs2; j++)
      {
        iemarray_setfloat(vec_dst_re, j, iemarray_getfloat(vec_src_re, j));
        iemarray_setfloat(vec_dst_im, j, iemarray_getfloat(vec_src_im, j));
      }
      iemarray_setfloat(vec_dst_re, fs2, iemarray_getfloat(vec_src_re, fs2));
      iemarray_setfloat(vec_dst_im, fs2, 0.0);
      for(k=1, j=fftsize-1; k<fs2; k++, j--)
      {
        iemarray_setfloat(vec_dst_re, j, iemarray_getfloat(vec_src_re, k));
        iemarray_setfloat(vec_dst_im, j, -iemarray_getfloat(vec_src_im, k));
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
        vec_src_re[i] *= g;
        vec_src_im[i] *= g;
        }
      */
      
      g = 1.0 / (t_float)fftsize;
      for(i = 0; i < fftsize; i++)
      {
        iemarray_setfloat(vec_dst_re, i, iemarray_getfloat(vec_dst_re, i)*g);
      }
      
      outlet_bang(x->x_obj.ob_outlet);
      a = (t_garray *)pd_findbyclass(x->x_sym_dst, garray_class);
      garray_redraw(a);
    }
  }
  else
  {
    post("tab_rifft-ERROR: list need 3 float arguments:");
    post("  source_real_offset + source_imag_offset + destination_offset");
  }
}

static void tab_rifft_free(t_tab_rifft *x)
{
  freebytes(x->x_sin_cos, x->x_fftsize * sizeof(TAB_COMPLEX));
  freebytes(x->x_beg_mem_dst_im, x->x_fftsize * sizeof(t_float));
}

static void *tab_rifft_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_rifft *x = (t_tab_rifft *)pd_new(tab_rifft_class);
  t_symbol  *src_re, *src_im, *dst;
  int fftsize, i=1;
  
  if((argc >= 4) &&
    IS_A_SYMBOL(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
    IS_A_SYMBOL(argv,2) &&
    IS_A_FLOAT(argv,3))
  {
    src_re = (t_symbol *)atom_getsymbolarg(0, argc, argv);
    src_im = (t_symbol *)atom_getsymbolarg(1, argc, argv);
    dst = (t_symbol *)atom_getsymbolarg(2, argc, argv);
    fftsize = (int)atom_getintarg(3, argc, argv);
  }
  else
  {
    post("tab_rifft-ERROR: need 3 symbols + 1 float arguments:");
    post("  source_real_array_name + source_imag_array_name + destination_array_name + IFFT-size");
    return(0);
  }
  
  if(fftsize < 8)
    fftsize = 8;
  
  while(i <= fftsize)
    i *= 2;
  i /= 2;
  fftsize = i;
  
  x->x_fftsize = fftsize;
  x->x_sym_src_re = src_re;
  x->x_sym_src_im = src_im;
  x->x_sym_dst = dst;
  x->x_sin_cos = (TAB_COMPLEX *)getbytes(x->x_fftsize * sizeof(TAB_COMPLEX));
  x->x_beg_mem_dst_im = (iemarray_t *)getbytes(x->x_fftsize * sizeof(iemarray_t));
  tab_rifft_init(x);
  outlet_new(&x->x_obj, &s_bang);
  return(x);
}

void tab_rifft_setup(void)
{
  tab_rifft_class = class_new(gensym("tab_rifft"), (t_newmethod)tab_rifft_new, (t_method)tab_rifft_free,
    sizeof(t_tab_rifft), 0, A_GIMME, 0);
  class_addbang(tab_rifft_class, (t_method)tab_rifft_bang);
  class_addlist(tab_rifft_class, (t_method)tab_rifft_list);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_ifftsize, gensym("ifftsize"), A_DEFFLOAT, 0);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_src_re, gensym("src_re"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_src_im, gensym("src_im"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_src_re, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_src_im, gensym("src2"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_dst, gensym("dst"), A_DEFSYMBOL, 0);
  class_addmethod(tab_rifft_class, (t_method)tab_rifft_dst, gensym("dst1"), A_DEFSYMBOL, 0);
}
