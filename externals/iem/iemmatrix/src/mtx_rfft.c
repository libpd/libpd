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

#ifdef HAVE_FFTW3_H
#include <fftw3.h>
#endif

static t_class *mtx_rfft_class;

#ifdef HAVE_FFTW3_H
enum ComplexPart { REALPART=0,  IMAGPART=1};
#endif

typedef struct _MTXRfft_ MTXRfft;
struct _MTXRfft_
{
  t_object x_obj;
  int size;
  int size2;
#ifdef HAVE_FFTW3_H
  int fftn;
  int rows;
  fftw_plan *fftplan;
  fftw_complex *f_out;
  double *f_in;
#else
  t_float *f_re;
  t_float *f_im;
#endif

  t_outlet *list_re_out;
  t_outlet *list_im_out;
   
  t_atom *list_re;
  t_atom *list_im;
};

static void deleteMTXRfft (MTXRfft *x) 
{
#ifdef HAVE_FFTW3_H
  int n;
  if (x->fftplan) {
     for (n=0; n<x->rows; n++) 
	fftw_destroy_plan(x->fftplan[n]);
     free(x->fftplan);
  }
  if (x->f_out)
     free(x->f_out);
  if (x->f_in)
     free(x->f_in);
#else
  if (x->f_re)
     free (x->f_re);
  if (x->f_im) 
     free (x->f_im);
#endif
  if (x->list_re)
     free (x->list_re);
  if (x->list_im)
     free (x->list_im);
}

static void *newMTXRfft (t_symbol *s, int argc, t_atom *argv)
{
  MTXRfft *x = (MTXRfft *) pd_new (mtx_rfft_class);
  x->list_re_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_im_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->size=x->size2=0;
#ifdef HAVE_FFTW3_H
  x->fftn=0;
  x->rows=0;
  x->f_in=0;
  x->f_out=0;
  x->fftplan=0;
#else
  x->f_re=x->f_im=0;
#endif
  x->list_re=x->list_im=0;
  
  return ((void *) x);
} 

static void mTXRfftBang (MTXRfft *x)
{
  if (x->list_im) {
    outlet_anything(x->list_im_out, gensym("matrix"), x->size2, x->list_im);
    outlet_anything(x->list_re_out, gensym("matrix"), x->size2, x->list_re);
  }
}

static void fftRestoreImag (int n, t_float *re, t_float *im) 
{
  t_float *im2;
  n >>= 1;
  *im=0;
  re += n;
  im += n;
  im2 = im;
  *im=0;
  while (--n) {
    *--im = -*++re;
    *++im2 = 0;
    *re = 0;
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

#ifdef HAVE_FFTW3_H
static void writeFFTWComplexPartIntoList (int n, t_atom *l, fftw_complex *c, enum ComplexPart p) 
{
   t_float f;
  while (n--) {
     f=(t_float)c[n][p];
    SETFLOAT (l+n, f);
  }
}
static void readDoubleFromList (int n, t_atom *l, double *f) 
{
  while (n--) 
    *f++ = (double)atom_getfloat (l++);
}
#endif

static void mTXRfftMatrix (MTXRfft *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int columns_re = (columns>>1)+1; /* N/2+1 samples needed for real part of realfft */
  int size = rows * columns;
  int in_size = argc-2;
  int size2 = columns_re * rows + 2; /* +2 since the list also contains matrix row+col */
  int fft_count;
  t_atom *list_re = x->list_re;
  t_atom *list_im = x->list_im;
#ifdef HAVE_FFTW3_H
  fftw_complex *f_out = x->f_out;
  double *f_in = x->f_in;
#else
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;
#endif

  /* fftsize check */
  if (!size)
    post("mtx_rfft: invalid dimensions");
  else if (in_size<size)
    post("mtx_rfft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns < 4){
    post("mtx_rfft: matrix must have at least 4 columns");
  }
  else if (columns == (1 << ilog2(columns))) {
    /* ok, do the FFT! */

    /* memory things */
#ifdef HAVE_FFTW3_H
    if ((x->rows!=rows)||(columns!=x->fftn)){
       f_out=(fftw_complex*)realloc(f_out, sizeof(fftw_complex)*(size2-2));
       f_in=(double*)realloc(f_in, sizeof(double)*size);
       x->f_in = f_in;
       x->f_out = f_out;
       for (fft_count=0; fft_count<x->rows; fft_count++) {
          fftw_destroy_plan(x->fftplan[fft_count]);
       }
       x->fftplan = (fftw_plan*)realloc(x->fftplan, sizeof(fftw_plan)*rows);
       for (fft_count=0; fft_count<rows; fft_count++, f_in+=columns, f_out+=columns_re) {
	  x->fftplan[fft_count] = fftw_plan_dft_r2c_1d (columns,f_in,f_out,FFTW_ESTIMATE);
       }
       x->fftn=columns;
       x->rows=rows;
       f_in=x->f_in;
       f_out=x->f_out;
    }
#else
    f_re=(t_float*)realloc(f_re, sizeof(t_float)*size);
    f_im=(t_float*)realloc(f_im, sizeof(t_float)*size);
    x->f_re = f_re;
    x->f_im = f_im;
#endif
    list_re=(t_atom*)realloc(list_re, sizeof(t_atom)*size2);
    list_im=(t_atom*)realloc(list_im, sizeof(t_atom)*size2);

    x->size = size;
    x->size2 = size2;
    x->list_im = list_im;
    x->list_re = list_re;

    /* main part */
#ifdef HAVE_FFTW3_H
    readDoubleFromList (size, argv, f_in);
#else
    readFloatFromList (size, argv, f_re);
#endif

    list_re += 2;
    list_im += 2;
    for (fft_count=0;fft_count<rows;fft_count++){ 
#ifdef HAVE_FFTW3_H
      fftw_execute(x->fftplan[fft_count]);
      writeFFTWComplexPartIntoList(columns_re,list_re,f_out,REALPART);
      writeFFTWComplexPartIntoList(columns_re,list_im,f_out,IMAGPART);
      f_out+=columns_re;
#else
      mayer_realfft (columns, f_re);
      fftRestoreImag (columns, f_re, f_im);
      writeFloatIntoList (columns_re, list_re, f_re);
      writeFloatIntoList (columns_re, list_im, f_im);
      f_im += columns;
      f_re += columns;
#endif
      list_re += columns_re;
      list_im += columns_re;
    }

    list_re = x->list_re;
    list_im = x->list_im;
      
    SETSYMBOL(list_re, gensym("matrix"));
    SETSYMBOL(list_im, gensym("matrix"));
    SETFLOAT(list_re, rows);
    SETFLOAT(list_im, rows);
    SETFLOAT(list_re+1, columns_re);
    SETFLOAT(list_im+1, columns_re);
    outlet_anything(x->list_im_out, gensym("matrix"), 
		    x->size2, list_im);
    outlet_anything(x->list_re_out, gensym("matrix"), 
		    x->size2, list_re);
  }
  else
    post("mtx_rowfft: rowvector size no power of 2!");

}

void mtx_rfft_setup (void)
{
  mtx_rfft_class = class_new 
    (gensym("mtx_rfft"),
     (t_newmethod) newMTXRfft,
     (t_method) deleteMTXRfft,
     sizeof (MTXRfft),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_rfft_class, (t_method) mTXRfftBang);
  class_addmethod (mtx_rfft_class, (t_method) mTXRfftMatrix, gensym("matrix"), A_GIMME,0);
}

void iemtx_rfft_setup(void){
  mtx_rfft_setup();
}
