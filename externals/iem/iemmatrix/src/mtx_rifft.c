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

static t_class *mtx_rifft_class;

#ifdef HAVE_FFTW3_H
enum ComplexPart { REALPART=0,  IMAGPART=1};
#endif

typedef struct _MTXRifft_
{
  t_object x_obj;
  int rows;
  int columns;
  int columns_re;
  int size;
  int size2;
  t_float renorm_fac;
#ifdef HAVE_FFTW3_H  
  fftw_plan *fftplan;
  fftw_complex *f_in;
  double *f_out;
#else
  t_float *f_re;
  t_float *f_im;
#endif

  t_outlet *list_re_out;
  t_outlet *list_im_out;
   
  t_atom *list_re;
  t_atom *list_im;
} MTXRifft;


/* helper functions: these should really go into a separate file! */


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

/*--------------inverse real fft */

static void multiplyVector (int n, t_float *f, t_float fac)
{
  while (n--)
    *f++ *= fac;
}


static void ifftPrepareReal (int n, t_float *re, t_float *im) 
{
  n >>= 1;
  re += n;
  im += n;
   
  while (--n) 
    *++re = -*--im;
}

#ifdef HAVE_FFTW3_H
static void readFFTWComplexPartFromList (int n, t_atom *l, fftw_complex *f, enum ComplexPart p) 
{
  for (;n--;) 
    f[n][p] = (double) atom_getfloat (l+n);
}
static void writeDoubleIntoList (int n, t_atom *l, double *d) 
{
   t_float f;
  while (n--) { 
    f=(t_float) d[n];
    SETFLOAT (l+n,f);
  }
}
static void multiplyDoubleVector (int n, double *f, t_float fac)
{
   double fd=(double)fac;
  while (n--)
    *f++ *= (double)fd;
}

#endif

static void *newMTXRifft (t_symbol *s, int argc, t_atom *argv)
{
  MTXRifft *x = (MTXRifft *) pd_new (mtx_rifft_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"),gensym(""));
  x->list_re_out = outlet_new (&x->x_obj, gensym("matrix"));
  return ((void *) x);
} 


static void mTXRifftMatrixCold (MTXRifft *x, t_symbol *s, 
                                int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns_re = atom_getint (argv++);
  int in_size = argc-2;
  int columns = (columns_re-1)<<1;
  int size2 = columns_re * rows;
  int size = rows * columns;
  int ifft_count;
  t_atom *list_re = x->list_re;
#ifdef HAVE_FFTW3_H
  fftw_complex *f_in = x->f_in;
  double *f_out = x->f_out;
#else
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;
#endif

  /* ifftsize check */
  if (columns_re < 3)
    post("mtx_rifft: matrix must have at least 3 columns");
  else if (!size) 
    post("mtx_rifft: invalid dimensions");
  else if (in_size < size2)
    post("mtx_rifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns<4)
    post("mtx_rifft: too small matrices");
  else if (columns == (1 << ilog2(columns))) {

    /* memory things */
#ifdef HAVE_FFTW3_H
    if ((x->rows!=rows)||(columns!=x->columns)){
      for (ifft_count=0;ifft_count<x->rows;ifft_count++) {
        fftw_destroy_plan(x->fftplan[ifft_count]);
      }
      x->fftplan=(fftw_plan*)realloc(x->fftplan,sizeof(fftw_plan)*rows);
      f_in=(fftw_complex*)realloc(f_in,sizeof(fftw_complex)*size2);
      f_out=(double*)realloc(f_out,sizeof(double)*size);
      list_re=(t_atom*)realloc(list_re, sizeof(t_atom)*(size+2));
      x->list_re = list_re;
      x->f_out = f_out;
      x->f_in = f_in;
      for (ifft_count=0;ifft_count<rows;ifft_count++) {
        x->fftplan[ifft_count]=fftw_plan_dft_c2r_1d(columns,f_in,f_out,FFTW_ESTIMATE);
        f_out+=columns;
        f_in+=columns_re;
      }
      f_in=x->f_in;
      f_out=x->f_out;
    }
#else
    f_re=(t_float*)realloc(f_re, sizeof(t_float)*size);
    f_im=(t_float*)realloc(f_im, sizeof(t_float)*size);
    x->f_re = f_re;
    x->f_im = f_im;
    list_re=(t_atom*)realloc(list_re, sizeof(t_atom)*(size+2));
    x->list_re = list_re;
#endif

    x->size = size;
    x->size2 = size2;
    x->rows = rows;
    x->columns = columns;
    x->columns_re = columns_re;
      
    /* main part: reading imaginary part */
    ifft_count = rows;
    x->renorm_fac = 1.0f / columns;
    for (ifft_count=0;ifft_count<rows;ifft_count++) {
#ifdef HAVE_FFTW3_H
      readFFTWComplexPartFromList(columns_re, argv, f_in, IMAGPART);
      f_in += columns_re;
#else
      readFloatFromList (columns_re, argv, f_im);
      f_im += columns;
#endif
      argv += columns_re;
    }
    /* do nothing else! */
  }
  else
    post("mtx_rifft: rowvector 2*(size+1) no power of 2!");
}

static void mTXRifftMatrixHot (MTXRifft *x, t_symbol *s, 
                               int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns_re = atom_getint (argv++);
  int columns = x->columns;
  int size = x->size;
  int in_size = argc-2;
  int size2 = x->size2;
  int ifft_count;
#ifdef HAVE_FFTW3_H
  fftw_complex *f_in = x->f_in;
#else
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;
#endif
  t_float renorm_fac = x->renorm_fac;

  /* ifftsize check */
  if ((rows != x->rows) || 
      (columns_re != x->columns_re))
    post("mtx_rifft: matrix dimensions do not match");
  else if (in_size<size2)
    post("mtx_rifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (!x->size2)
    post("mtx_rifft: invalid right side matrix");
  else { /* main part */
    for (ifft_count=0;ifft_count<rows;ifft_count++){ 
#ifdef HAVE_FFTW3_H
      readFFTWComplexPartFromList(columns_re,argv,f_in,REALPART);
      fftw_execute(x->fftplan[ifft_count]);
      f_in+=columns_re;
#else
      readFloatFromList (columns_re, argv, f_re);
      ifftPrepareReal (columns, f_re, f_im);
      mayer_realifft (columns, f_re);
      f_im += columns;
      f_re += columns;
#endif
      argv += columns_re;
    }
#ifndef HAVE_FFTW3_H
    f_re = x->f_re;
#endif
    size2 = x->size2;

    SETFLOAT(x->list_re, rows);
    SETFLOAT(x->list_re+1, x->columns);
#ifdef HAVE_FFTW3_H
    multiplyDoubleVector (size, x->f_out, renorm_fac);
    writeDoubleIntoList (size, x->list_re+2, x->f_out);
#else
    multiplyVector (size, f_re, renorm_fac);
    writeFloatIntoList  (size, x->list_re+2, f_re);
#endif
    outlet_anything(x->list_re_out, gensym("matrix"), size+2, x->list_re);
  }
}

static void mTXRifftBang (MTXRifft *x)
{
  if (x->list_re)
    outlet_anything(x->list_re_out, gensym("matrix"), 
                    x->size+2, x->list_re);
}


static void deleteMTXRifft (MTXRifft *x) 
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
    free(x->f_re);
  if (x->f_im)
    free(x->f_im);
#endif
  if (x->list_re)
    free(x->list_re);
  if (x->list_im)
    free(x->list_im);
}

void mtx_rifft_setup (void)
{
  mtx_rifft_class = class_new 
    (gensym("mtx_rifft"),
     (t_newmethod) newMTXRifft,
     (t_method) deleteMTXRifft,
     sizeof (MTXRifft),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_rifft_class, (t_method) mTXRifftBang);
  class_addmethod (mtx_rifft_class, (t_method) mTXRifftMatrixHot, gensym("matrix"), A_GIMME,0);
  class_addmethod (mtx_rifft_class, (t_method) mTXRifftMatrixCold, gensym(""), A_GIMME,0);
}

void iemtx_rifft_setup(void){
  mtx_rifft_setup();
}
