/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"

/* -------------------------- tab_find_peaks ------------------------------ */

#define IEMLIB_TAB_FIND_PEAKS_SORT_MODE_AMP 0
#define IEMLIB_TAB_FIND_PEAKS_SORT_MODE_FREQ 1

typedef struct _tab_find_peaks
{
  t_object  x_obj;
  int       x_size_src1;
  int       x_offset_src1;
  iemarray_t   *x_beg_mem_src1;
  int       x_work_alloc;
  int       *x_beg_mem_work1;
  t_float   *x_beg_mem_work2;
  int       x_sort_mode;
  t_float   x_hdiff;
  int       x_min_width;
  int       x_max_width;
  int       x_n_peaks;
  t_symbol  *x_sym_scr1;
  t_outlet  *x_bang_out;
  t_outlet  *x_sort_index_out;
  t_outlet  *x_peak_value_out;
  t_outlet  *x_peak_index_out;
} t_tab_find_peaks;

static t_class *tab_find_peaks_class;

static void tab_find_peaks_max_peaks(t_tab_find_peaks *x, t_floatarg fmax_peaks)
{
  int max_peaks = (int)fmax_peaks;
  
  if(max_peaks <= 0)
    max_peaks = 1;
  x->x_n_peaks = max_peaks;
}

static void tab_find_peaks_width_range(t_tab_find_peaks *x, t_symbol *s, int argc, t_atom *argv)
{
  int minw, maxw, h;
  
  if((argc >= 2) &&
    IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1))
  {
    minw = (int)atom_getintarg(0, argc, argv);
    maxw = (int)atom_getintarg(1, argc, argv);
    if(minw <= 0)
      minw = 1;
    if(maxw <= 0)
      maxw = 1;
    if(minw > maxw)
    {
      h = minw;
      minw = maxw;
      maxw = h;
    }
    x->x_min_width = minw;
    x->x_max_width = maxw;
  }
}

static void tab_find_peaks_abs_min_height_diff(t_tab_find_peaks *x, t_floatarg height_diff)
{
  if(height_diff < 0.0f)
    height_diff *= -1.0f;
  x->x_hdiff = height_diff;
}

static void tab_find_peaks_amp_sort(t_tab_find_peaks *x)
{
  x->x_sort_mode = IEMLIB_TAB_FIND_PEAKS_SORT_MODE_AMP;
}

static void tab_find_peaks_freq_sort(t_tab_find_peaks *x)
{
  x->x_sort_mode = IEMLIB_TAB_FIND_PEAKS_SORT_MODE_FREQ;
}

static void tab_find_peaks_src(t_tab_find_peaks *x, t_symbol *s)
{
  x->x_sym_scr1 = s;
}

static void tab_find_peaks_bang(t_tab_find_peaks *x)
{
  int i, n, w, ww;
  int ok_src, peak_index=0;
  iemarray_t *vec_src;
  t_float *vec_work2;
  int *vec_work1;
  t_float max=-1.0e37;
  int max_peaks=x->x_n_peaks;
  int min_width=x->x_min_width;
  int max_width=x->x_max_width;
  t_float abs_min_height_diff=x->x_hdiff;
  
  ok_src = iem_tab_check_arrays(gensym("tab_find_peaks"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, 0);
  
  if(ok_src)
  {
    n = x->x_size_src1;
    if(n)
    {
      if(!x->x_work_alloc)
      {
        x->x_beg_mem_work1 = (int *)getbytes(n * sizeof(int));
        x->x_beg_mem_work2 = (t_float *)getbytes(n * sizeof(t_float));
        x->x_work_alloc = n;
      }
      else if(n != x->x_work_alloc)
      {
        x->x_beg_mem_work1 = (int *)resizebytes(x->x_beg_mem_work1, x->x_work_alloc*sizeof(int), n*sizeof(int));
        x->x_beg_mem_work2 = (t_float *)resizebytes(x->x_beg_mem_work2, x->x_work_alloc*sizeof(t_float), n*sizeof(t_float));
        x->x_work_alloc = n;
      }
      vec_src = x->x_beg_mem_src1;
      vec_work1 = x->x_beg_mem_work1;
      vec_work2 = x->x_beg_mem_work2;
      if(x->x_sort_mode == IEMLIB_TAB_FIND_PEAKS_SORT_MODE_FREQ) // FREQ_SORT BEGIN
      {
        int sort_index=1,old=0,j;
        
        for(i=0; i<n; i++)
        {
          vec_work1[i] = 0;
        }
        for(w=min_width; w<=max_width; w++) // w variiert zw. min u. max
        {
          for(ww=0; ww<w; ww++)
          {
            int beg=w-ww;
            int end=n-1-ww;
            int low_bord=beg-1;
            int high_bord=n-end-1;
            t_float diff_low,diff_high;
            
            for(i=beg; i<end; i++)
            {
              diff_low = iemarray_getfloat(vec_src, i-low_bord) - abs_min_height_diff;
              diff_high = iemarray_getfloat(vec_src, i+high_bord) - abs_min_height_diff;
              if((iemarray_getfloat(vec_src, i-low_bord-1) < diff_low) && !vec_work1[i-low_bord] && 
                 (iemarray_getfloat(vec_src, i+high_bord+1) < diff_high) && !vec_work1[i+high_bord])
              {
                for(j=i-low_bord; j<=i+high_bord; j++)
                  vec_work1[j] = 1;
                //post("f[%d]=%g, f[%d]=%g",i-low_bord,vec_src[i-low_bord],i+high_bord,vec_src[i+high_bord]);
              }
            }
          }
        }
        old = vec_work1[0];
        sort_index=1;
        for(i=1; i<n; i++)
        {
          if(!old && vec_work1[i])
          {
            vec_work1[i] = 0;
            j=i+1;
            while(vec_work1[j])
            {
              vec_work1[j] = 0;
              j++;
            }
            j--;
            peak_index = (i + j) / 2;
            if(sort_index <= max_peaks)
            {
              outlet_float(x->x_peak_value_out, iemarray_getfloat(vec_src, i));
              outlet_float(x->x_peak_index_out, (t_float)peak_index);
              outlet_float(x->x_sort_index_out, sort_index);
              sort_index++;
            }
            else
              i = n+1;
          }
          old = vec_work1[i];
        }
        outlet_bang(x->x_bang_out);
      }                                                               // FREQ_SORT END
      else if(x->x_sort_mode == IEMLIB_TAB_FIND_PEAKS_SORT_MODE_AMP)  // AMP_SORT BEGIN
      {
        int sort_index=1,old=0,j;
        
        for(i=0; i<n; i++)
        {
          vec_work1[i] = 0;
          vec_work2[i] = 0.0f;
        }
        for(w=min_width; w<=max_width; w++) // w variiert zw. min u. max
        {
          for(ww=0; ww<w; ww++)
          {
            int beg=w-ww;
            int end=n-1-ww;
            int low_bord=beg-1;
            int high_bord=n-end-1;
            t_float diff_low,diff_high;
            
            for(i=beg; i<end; i++)
            {
              diff_low = iemarray_getfloat(vec_src, i-low_bord) - abs_min_height_diff;
              diff_high = iemarray_getfloat(vec_src, i+high_bord) - abs_min_height_diff;
              if((iemarray_getfloat(vec_src, i-low_bord-1) < diff_low) && !vec_work1[i-low_bord] && 
                 (iemarray_getfloat(vec_src, i+high_bord+1) < diff_high) && !vec_work1[i+high_bord])
              {
                for(j=i-low_bord; j<=i+high_bord; j++)
                {
                  vec_work1[j] = 1;
                  vec_work2[j] = iemarray_getfloat(vec_src, j);
                }
                //post("a[%d]=%g, a[%d]=%g",i-low_bord,vec_src[i-low_bord],i+high_bord,vec_src[i+high_bord]);
              }
            }
          }
        }
        old = vec_work1[0];
        for(sort_index=1; sort_index<=max_peaks; sort_index++)
        {
          max = -1.0e37;
          peak_index = -1;
          for(i=0; i<n; i++)
          {
            if(vec_work1[i])
            {
              if(vec_work2[i] > max)
              {
                max = vec_work2[i];
                peak_index = i;
              }
            }
          }
          
          if(peak_index >= 0)
          {
            outlet_float(x->x_peak_value_out, max);
            outlet_float(x->x_peak_index_out, (t_float)peak_index);
            outlet_float(x->x_sort_index_out, sort_index);
            vec_work1[peak_index] = 0;
            vec_work2[peak_index] = 0.0f;
            j=peak_index+1;
            while(vec_work1[j])
            {
              vec_work1[j] = 0;
              j++;
            }
            j=peak_index-1;
            while(vec_work1[j])
            {
              vec_work1[j] = 0;
              j--;
            }
          }
          else
            sort_index = max_peaks+1;
        }
        outlet_bang(x->x_bang_out);
      }
    }                                                                            // AMP_SORT END
  }
  /*
  [n]                    zu  [n-1] u. [n+1]   (ww=0)(w=1)(beg=1)(end=n-1)
  [n-1] u. [n]           zu  [n-2] u. [n+1]   (ww=0)(w=2)(beg=2)(end=n-1)
  [n] u. [n+1]           zu  [n-1] u. [n+2]   (ww=1)(w=2)(beg=1)(end=n-2)
  [n-2] u. [n-1] u. [n]  zu  [n-3] u. [n+1]   (ww=0)(w=3)(beg=3)(end=n-1)
  [n-1] u. [n] u. [n+1]  zu  [n-2] u. [n+2]   (ww=1)(w=3)(beg=2)(end=n-2)
  [n] u. [n+1] u. [n+2]  zu  [n-1] u. [n+3]   (ww=2)(w=3)(beg=1)(end=n-3)
  */
}

/*static void tab_find_peaks_list(t_tab_find_peaks *x, t_symbol *s, int argc, t_atom *argv)
{
int beg_src;
int i, n;
int ok_src, max_index=0;
t_float *vec_src;
t_float max=-1.0e37;

  if((argc >= 2) &&
  IS_A_FLOAT(argv,0) &&
  IS_A_FLOAT(argv,1))
  {
  beg_src = (int)atom_getintarg(0, argc, argv);
  n = (int)atom_getintarg(1, argc, argv);
  if(beg_src < 0)
  beg_src = 0;
  if(n < 0)
  n = 0;
  
    ok_src = iem_tab_check_arrays(gensym("tab_find_peaks"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, beg_src+n);
    
      if(ok_src)
      {
      vec_src = x->x_beg_mem_src1 + beg_src;
      if(n)
      {
      for(i=0; i<n; i++)
      {
      if(vec_src[i] > max)
      {
      max = vec_src[i];
      max_index = i + beg_src;
      }
      }
      outlet_float(x->x_peak_value_out, max);
      outlet_float(x->x_peak_index_out, (t_float)max_index);
      outlet_bang(x->x_bang_out);
      }
      }
      }
      else
      {
      post("tab_find_peaks-ERROR: list need 2 float arguments:");
      post("  source_offset + number_of_samples_to_calc_max_index");
      }
}*/

static void tab_find_peaks_free(t_tab_find_peaks *x)
{
  if(x->x_work_alloc)
  {
    freebytes(x->x_beg_mem_work1, x->x_work_alloc * sizeof(int));
    freebytes(x->x_beg_mem_work2, x->x_work_alloc * sizeof(t_float));
  }
}

static void *tab_find_peaks_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tab_find_peaks *x = (t_tab_find_peaks *)pd_new(tab_find_peaks_class);
  t_symbol  *src;
  
  if((argc >= 1) &&
    IS_A_SYMBOL(argv,0))
  {
    src = (t_symbol *)atom_getsymbolarg(0, argc, argv);
  }
  else
  {
    post("tab_find_peaks-ERROR: need 1 symbol argument:");
    post("  source_array_name");
    return(0);
  }
  
  x->x_work_alloc = 0;
  x->x_beg_mem_work1 = (int *)0;
  x->x_beg_mem_work2 = (t_float *)0;
  
  x->x_sym_scr1 = src;
  x->x_bang_out = (t_outlet *)outlet_new(&x->x_obj, &s_bang); // ready
  x->x_sort_index_out = (t_outlet *)outlet_new(&x->x_obj, &s_float); // sort index
  x->x_peak_index_out = (t_outlet *)outlet_new(&x->x_obj, &s_float); // freq
  x->x_peak_value_out = (t_outlet *)outlet_new(&x->x_obj, &s_float); // value
  
  return(x);
}

void tab_find_peaks_setup(void)
{
  tab_find_peaks_class = class_new(gensym("tab_find_peaks"), (t_newmethod)tab_find_peaks_new, (t_method)tab_find_peaks_free,
    sizeof(t_tab_find_peaks), 0, A_GIMME, 0);
  class_addbang(tab_find_peaks_class, (t_method)tab_find_peaks_bang);
  /*class_addlist(tab_find_peaks_class, (t_method)tab_find_peaks_list);*/
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_src, gensym("src"), A_DEFSYMBOL, 0);
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_src, gensym("src1"), A_DEFSYMBOL, 0);
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_max_peaks, gensym("max_peaks"), A_DEFFLOAT, 0);
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_width_range, gensym("width_range"), A_GIMME, 0);
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_abs_min_height_diff, gensym("abs_min_height_diff"), A_DEFFLOAT, 0);
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_amp_sort, gensym("amp_sort"), 0);
  class_addmethod(tab_find_peaks_class, (t_method)tab_find_peaks_freq_sort, gensym("freq_sort"), 0);
//  class_sethelpsymbol(tab_find_peaks_class, gensym("iemhelp2/tab_find_peaks-help"));
}
