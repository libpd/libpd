/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *  this functions depends on the GNU scientific library
 *
 * Copyright (c) 2009, Franz Zotter
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "iemmatrix.h"
#include <stdlib.h>

#ifdef HAVE_LIBGSL
#include <gsl/gsl_eigen.h>
#endif

static t_class *mtx_eig_class;
enum WithEigenVectors {WITHEVS=1, WITHOUTEVS=0};
typedef struct _MTXEig_ MTXEig;
struct _MTXEig_
{
  t_object x_obj;
#ifdef HAVE_GSL_EIGEN_NONSYMM
  gsl_matrix *a;
  gsl_matrix_complex *q;
  gsl_vector_complex *l;
  gsl_eigen_nonsymm_workspace *w;
  gsl_eigen_nonsymmv_workspace *wv;
#endif
  t_outlet *list_q_out_re;
  t_outlet *list_q_out_im;
  t_outlet *list_l_out_re;
  t_outlet *list_l_out_im;
  t_atom *list_q_re;
  t_atom *list_q_im;
  t_atom *list_l_re;
  t_atom *list_l_im;
  int size;
  enum WithEigenVectors withevs;
};

#ifdef HAVE_GSL_EIGEN_NONSYMM
static void allocMTXqlw (MTXEig *x) 
{
     x->a=(gsl_matrix*)gsl_matrix_alloc(x->size,x->size);
     x->l=(gsl_vector_complex*)gsl_vector_complex_alloc(x->size);

     switch (x->withevs) {
        case WITHEVS:
           x->wv=(gsl_eigen_nonsymmv_workspace*)gsl_eigen_nonsymmv_alloc(x->size);
           x->q=(gsl_matrix_complex*)gsl_matrix_complex_alloc(x->size,x->size);
        break;
        case WITHOUTEVS:
        x->w=(gsl_eigen_nonsymm_workspace*)gsl_eigen_nonsymm_alloc(x->size);
     }

     x->list_q_re=(t_atom*)calloc(sizeof(t_atom),x->size*x->size+2);
     x->list_q_im=(t_atom*)calloc(sizeof(t_atom),x->size*x->size+2);
     x->list_l_re=(t_atom*)calloc(sizeof(t_atom),x->size);
     x->list_l_im=(t_atom*)calloc(sizeof(t_atom),x->size);
}

static void deleteMTXqlw (MTXEig *x) 
{
   if (x->list_q_re!=0)
      free(x->list_q_re);
   if (x->list_q_im!=0)
      free(x->list_q_im);
   if (x->list_l_re!=0)
      free(x->list_l_re);
   if (x->list_l_im!=0)
      free(x->list_l_im);

   x->list_q_re = 0;
   x->list_q_im = 0;
   x->list_l_re = 0;
   x->list_l_im = 0;

   if (x->a!=0)
      gsl_matrix_free(x->a);
   if (x->q!=0)
      gsl_matrix_complex_free(x->q);
   if (x->l!=0)
      gsl_vector_complex_free(x->l);
   if (x->w!=0)
      gsl_eigen_nonsymm_free(x->w);
   if (x->wv!=0)
      gsl_eigen_nonsymmv_free(x->wv);

   x->a = 0;
   x->q = 0;
   x->l = 0;
   x->w = 0;
   x->wv = 0;
}
#endif

static void deleteMTXEig (MTXEig *x) 
{
#ifdef HAVE_GSL_EIGEN_NONSYMM 
   deleteMTXqlw(x);
#endif
}

static void *newMTXEig (t_symbol *s, int argc, t_atom *argv)
{
  MTXEig *x = (MTXEig *) pd_new (mtx_eig_class);
  
  x->list_l_out_re = outlet_new (&x->x_obj, gensym("list"));
  x->list_l_out_im = outlet_new (&x->x_obj, gensym("list"));
  if (atom_getsymbol(argv)==gensym("v")) {
     x->withevs=1;
     x->list_q_out_re = outlet_new (&x->x_obj, gensym("matrix"));
     x->list_q_out_im = outlet_new (&x->x_obj, gensym("matrix"));
  }

  x->list_l_re = 0;
  x->list_l_im = 0;
  x->list_q_re = 0; 
  x->list_q_im = 0; 
#ifdef HAVE_GSL_EIGEN_NONSYMM
  x->a=0;
  x->q=0;
  x->l=0;
  x->w=0;
  x->wv=0;
#endif
  
  return ((void *) x);
} 

static void mTXEigBang (MTXEig *x)
{
  if (x->list_l_re) {
     switch (x->withevs) {
        case WITHEVS:
             outlet_anything(x->list_q_out_im, gensym("matrix"), x->size*x->size+2, x->list_q_im);
             outlet_anything(x->list_q_out_re, gensym("matrix"), x->size*x->size+2, x->list_q_re);
        case WITHOUTEVS:
             outlet_anything(x->list_l_out_im, gensym("list"), x->size, x->list_l_im);
             outlet_anything(x->list_l_out_re, gensym("list"), x->size, x->list_l_re);
     }
  }
}

static void mTXEigMatrix (MTXEig *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  int n,m;
  float f;

#ifdef HAVE_GSL_EIGEN_NONSYMM
  gsl_complex c;
  /* size check */
  if (!size) 
    post("mtx_eig: invalid dimensions");
  else if (in_size<size) 
    post("mtx_eig: sparse matrix not yet supported: use \"mtx_check\"");
  else if (rows!=columns)
     post("mtx_eig: Eigendecomposition works for square matrices only!");
  else {
     size=rows;
     x->size=size;

     deleteMTXqlw(x);
     allocMTXqlw(x);
  
    for (n=0;n<in_size;n++) 
       x->a->data[n]=(double) atom_getfloat(argv++);

     switch (x->withevs) {
        case WITHOUTEVS:
           gsl_eigen_nonsymm(x->a,x->l,x->w);
           break;
        case WITHEVS:
           gsl_eigen_nonsymmv(x->a,x->l,x->q,x->wv);
           SETFLOAT((x->list_q_re),(float) x->size);
           SETFLOAT((x->list_q_im),(float) x->size);
           SETFLOAT((x->list_q_re+1),(float) x->size);
           SETFLOAT((x->list_q_im+1),(float) x->size);
           for (n=0;n<in_size;n++) {
              SETFLOAT((x->list_q_im+2+n), (float) x->q->data[2*n+1]);
              SETFLOAT((x->list_q_re+2+n), (float) x->q->data[2*n]);
           }
           break;
     }
  
     for (n=0;n<x->size;n++) {
       f=(float) GSL_VECTOR_IMAG(x->l, n);
       SETFLOAT((x->list_l_im+n), f);
       f=(float) GSL_VECTOR_REAL(x->l, n);
       SETFLOAT((x->list_l_re+n), f);
    }

    mTXEigBang(x);
  }
#else
    post("mtx_eig: implementation requires more recent gsl version to handle nonsymmetric matrices");
#endif

}

void mtx_eig_setup (void)
{
  mtx_eig_class = class_new 
    (gensym("mtx_eig"),
     (t_newmethod) newMTXEig,
     (t_method) deleteMTXEig,
     sizeof (MTXEig),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_eig_class, (t_method) mTXEigBang);
  class_addmethod (mtx_eig_class, (t_method) mTXEigMatrix, gensym("matrix"), A_GIMME,0);
}

void iemtx_eig_setup(void){
  mtx_eig_setup();
}
