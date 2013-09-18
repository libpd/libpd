/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) 2005, Franz Zotter
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "iemmatrix.h"
#include <stdlib.h>

static t_class *mtx_ifft_class;

typedef struct _MtxIFFT_ MtxIFFT;
struct _MtxIFFT_
{
  t_object x_obj;
  int size;
  float renorm_fac;

  t_float *f_re;
  t_float *f_im;

  t_outlet *list_re_out;
  t_outlet *list_im_out;
   
  t_atom *list_re;
  t_atom *list_im;
};

static void deleteMtxIFFT (MtxIFFT *x) 
{
  if (x->f_re)
     free (x->f_re);
  if (x->f_im) 
     free (x->f_im);
  if (x->list_re)
     free (x->list_re);
  if (x->list_im)
     free (x->list_im);
}

static void *newMtxIFFT (t_symbol *s, int argc, t_atom *argv)
{
  MtxIFFT *x = (MtxIFFT *) pd_new (mtx_ifft_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"),gensym(""));
  x->list_re_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_im_out = outlet_new (&x->x_obj, gensym("matrix"));

  x->size=0;
  x->f_re=x->f_im=0;
  x->list_re=x->list_im=0;
  
  return ((void *) x);
} 

static void mtxIFFTBang (MtxIFFT *x)
{
  if (x->list_im) {
    outlet_anything(x->list_im_out, gensym("matrix"), x->size, x->list_im);
    outlet_anything(x->list_re_out, gensym("matrix"), x->size, x->list_re);
  }
}

static void zeroFloatArray (int n, t_float *f)
{
  while (n--)
    *f++ = 0.0f;
}

static void writeFloatIntoList (int n, t_atom *l, t_float *f) 
{
  for (;n--;f++, l++) 
    SETFLOAT (l, *f);
}
static void readFloatFromList (int n, t_atom *l, t_float *f) 
{
  while (n--) 
    *f++ = atom_getfloat (l++);
}

static void multiplyVector (int n, t_float *f, t_float fac)
{
  while (n--)
    *f++ *= fac;
}

static void mtxIFFTMatrixCold (MtxIFFT *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  t_atom *list_re = x->list_re;
  t_atom *list_im = x->list_im;
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;

  /* fftsize check */
  if (!size)
    post("mtx_ifft: invalid dimensions");
  else if (in_size<size)
    post("mtx_ifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns < 4){
    post("mtx_ifft: matrix must have at least 4 columns");
  }
  else if (columns == (1 << ilog2(columns))) {
    /* ok, prepare real-part of FFT! */
    
     x->renorm_fac = 1.0f / columns;

    /* memory things */
    f_re=(t_float*)realloc(f_re, sizeof(t_float)*size);
    f_im=(t_float*)realloc(f_im, sizeof(t_float)*size);
    list_re=(t_atom*)realloc(list_re, sizeof(t_atom)*(size+2));
    list_im=(t_atom*)realloc(list_im, sizeof(t_atom)*(size+2));

    x->size = size;
    x->list_im = list_im;
    x->list_re = list_re;
    x->f_re = f_re;
    x->f_im = f_im;

    /* main part */
    readFloatFromList (size, argv, f_im);

  }
  else
    post("mtx_rowfft: rowvector size no power of 2!");

}


static void mtxIFFTMatrixHot (MtxIFFT *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  int fft_count;
  t_atom *list_re = x->list_re;
  t_atom *list_im = x->list_im;
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;

  /* fftsize check */
  if (!size)
    post("mtx_ifft: invalid dimensions");
  else if (in_size<size)
    post("mtx_ifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (size != x->size)
     post("mtx_ifft: left matrix has other dimensions than right matrix");
  else if (columns < 4){
    post("mtx_ifft: matrix must have at least 4 columns");
  }
  else if (columns == (1 << ilog2(columns))) {
    /* ok, do the FFT! */

    /* main part */
    readFloatFromList (size, argv, f_re);

    fft_count = rows;
    list_re += 2;
    list_im += 2;
    while (fft_count--){ 
      mayer_ifft (columns, f_re, f_im);
      multiplyVector (columns, f_re, x->renorm_fac);
      multiplyVector (columns, f_im, x->renorm_fac);
      writeFloatIntoList (columns, list_re, f_re);
      writeFloatIntoList (columns, list_im, f_im);
      f_im += columns;
      f_re += columns;
      list_re += columns;
      list_im += columns;
    }

    list_re = x->list_re;
    list_im = x->list_im;
      
    SETSYMBOL(list_re, gensym("matrix"));
    SETSYMBOL(list_im, gensym("matrix"));
    SETFLOAT(list_re, rows);
    SETFLOAT(list_im, rows);
    SETFLOAT(list_re+1, columns);
    SETFLOAT(list_im+1, columns);
    outlet_anything(x->list_im_out, gensym("matrix"), 
		    x->size+2, list_im);
    outlet_anything(x->list_re_out, gensym("matrix"), 
		    x->size+2, list_re);
  }
  else
    post("mtx_rowfft: rowvector size no power of 2!");

}

void mtx_ifft_setup (void)
{
  mtx_ifft_class = class_new 
    (gensym("mtx_ifft"),
     (t_newmethod) newMtxIFFT,
     (t_method) deleteMtxIFFT,
     sizeof (MtxIFFT),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_ifft_class, (t_method) mtxIFFTBang);
  class_addmethod (mtx_ifft_class, (t_method) mtxIFFTMatrixHot, gensym("matrix"), A_GIMME,0);
  class_addmethod (mtx_ifft_class, (t_method) mtxIFFTMatrixCold, gensym(""), A_GIMME,0);
}

void iemtx_ifft_setup(void){
  mtx_ifft_setup();
}
