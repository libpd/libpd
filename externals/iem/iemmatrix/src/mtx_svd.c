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
#include <gsl/gsl_linalg.h>
#endif

static t_class *mtx_svd_class;

typedef struct _MTXSvd_ MTXSvd;
struct _MTXSvd_
{
  t_object x_obj;
#ifdef HAVE_LIBGSL
  gsl_matrix *u;
  gsl_vector *s;
  gsl_matrix *v;
  gsl_vector *w;
#endif
  t_outlet *list_u_out;
  t_outlet *list_s_out;
  t_outlet *list_v_out;
  t_atom *list_u;
  t_atom *list_s;
  t_atom *list_v;
  int rows;
  int columns;
};

#ifdef HAVE_LIBGSL
static void allocMTXusvw (MTXSvd *x) 
{
     x->u=(gsl_matrix*)gsl_matrix_alloc(x->rows,x->columns);
     x->s=(gsl_vector*)gsl_vector_alloc(x->columns);
     x->v=(gsl_matrix*)gsl_matrix_alloc(x->columns,x->columns);
     x->w=(gsl_vector*)gsl_vector_alloc(x->columns);

     x->list_u=(t_atom*)calloc(sizeof(t_atom),x->rows*x->columns+2);
     x->list_s=(t_atom*)calloc(sizeof(t_atom),x->columns);
     x->list_v=(t_atom*)calloc(sizeof(t_atom),x->columns*x->columns+2);
}

static void deleteMTXusvw (MTXSvd *x) 
{
   if (x->list_u!=0)
      free(x->list_u);
   if (x->list_s!=0)
      free(x->list_s);
   if (x->list_v!=0)
      free(x->list_v);

   x->list_u = x->list_s = x->list_v = 0;

   if (x->u!=0)
      gsl_matrix_free(x->u);
   if (x->s!=0)
      gsl_vector_free(x->s);
   if (x->v!=0)
      gsl_matrix_free(x->v);
   if (x->w!=0)
      gsl_vector_free(x->w);

   x->u = 0;
   x->s = 0;
   x->v = 0;
   x->w = 0;
}
#endif

static void deleteMTXSvd (MTXSvd *x) 
{
#ifdef HAVE_LIBGSL
   deleteMTXusvw(x);
#endif
}

static void *newMTXSvd (t_symbol *s, int argc, t_atom *argv)
{
  MTXSvd *x = (MTXSvd *) pd_new (mtx_svd_class);
  x->list_u_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_s_out = outlet_new (&x->x_obj, gensym("list"));
  x->list_v_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_u = 0; 
  x->list_s = 0;
  x->list_v = 0;
#ifdef HAVE_LIBGSL
  x->u=0;
  x->s=0;
  x->v=0;
  x->w=0;
#endif
  
  return ((void *) x);
} 

static void mTXSvdBang (MTXSvd *x)
{
  if (x->list_u) {
    outlet_anything(x->list_v_out, gensym("matrix"), x->columns*x->columns+2, x->list_v);
    outlet_anything(x->list_s_out, gensym("list"), x->columns, x->list_s);
    outlet_anything(x->list_u_out, gensym("matrix"), x->rows*x->columns+2, x->list_u);
  }
}

static void mTXSvdMatrix (MTXSvd *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  int n;


#ifdef HAVE_LIBGSL
  /* size check */
  if (!size) 
    post("mtx_svd: invalid dimensions");
  else if (in_size<size) 
    post("mtx_svd: sparse matrix not yet supported: use \"mtx_check\"");
  else if (rows<columns)
     post("mtx_svd: gsl_linalg_SVD_decomp does not support M<N");
  else {
     x->rows=rows;
     x->columns=columns;

    deleteMTXusvw(x);
    allocMTXusvw(x);

    for (n=0;n<in_size;n++) 
       x->u->data[n]=(double) atom_getfloat(argv++);
    
    gsl_linalg_SV_decomp(x->u,x->v,x->s,x->w);

    SETFLOAT((x->list_u),(float) x->rows);
    SETFLOAT((x->list_u+1),(float) x->columns);
    for (n=0;n<in_size;n++)
       SETFLOAT((x->list_u+2+n), (float) x->u->data[n]);

    for (n=0;n<x->columns;n++)
       SETFLOAT((x->list_s+n),(float) x->s->data[n]);

    SETFLOAT((x->list_v),(float) x->columns);
    SETFLOAT((x->list_v+1),(float) x->columns);
    in_size=x->columns*x->columns;
    for (n=0;n<in_size;n++)
       SETFLOAT((x->list_v+n+2), (float) x->v->data[n]);

    mTXSvdBang(x);
  }
#else
    post("mtx_svd: implementation requires gsl");
#endif

}

void mtx_svd_setup (void)
{
  mtx_svd_class = class_new 
    (gensym("mtx_svd"),
     (t_newmethod) newMTXSvd,
     (t_method) deleteMTXSvd,
     sizeof (MTXSvd),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_svd_class, (t_method) mTXSvdBang);
  class_addmethod (mtx_svd_class, (t_method) mTXSvdMatrix, gensym("matrix"), A_GIMME,0);
}

void iemtx_svd_setup(void){
  mtx_svd_setup();
}
