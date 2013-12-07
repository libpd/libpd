/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) IOhannes m zmölnig, forum::für::umläute
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#include "iemmatrix.h"

/* mtx_cholesky */

/*
 * calculate the "Cholesky Decomposition" of a "symmetric and positive definite matrix "
 * no check is done, whether the input matrix is really symmetric and positive definite.
 */

static t_class *mtx_cholesky_class;

static void mtx_cholesky_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* maybe we should do this in double or long double ? */
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  int i, j, k, row2=row*row;

  t_matrixfloat *original, *cholesky;

  if(row*col+2>argc){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  if (row!=col){
    post("mtx_cholesky: only symmetric and positive definite matrices can be cholesky-decomposed");
    return;
  }

  /* reserve memory for outputting afterwards */
  adjustsize(x, row, row);
  /* 1. get the 2 matrices : orig; invert (create as eye, but will be orig^(-1)) */
  cholesky = (t_matrixfloat *)getbytes(sizeof(t_matrixfloat)*row2);
  /* 1a extract values of A to float-buf */
  original=matrix2float(argv);

  /* 2 set the cholesky matrix to zero */
  for(i=0; i<row2; i++)cholesky[i]=0.;

  /* 3 do the cholesky decomposition */
  for(i=0; i<col; i++){
    /* 3a get the diagonal element */
    /* l_ii=sqrt(a_ii-sum(k=1..i-1)((l_ik)^2)) */
    t_matrixfloat sum=0.;
    t_matrixfloat result=0.f;

    for(k=0; k<i; k++){
      t_matrixfloat lik=cholesky[k*col+i];
      sum+=lik*lik;
    }
    if((result=original[i*(col+1)]-sum)<0){
      post("[mtx_cholesky]: only symmetric and positive definite matrices can be cholesky-decomposed");
      return;
    }
    result=sqrtf(result); /* LATER check whether this is real */
    cholesky[i*(col+1)]=result;
    /* 3b get the other elements within this row/col */
    /* l_ji=(a_ji-sum(k=1..i-1)(l_jk*l_ik))/l_ii */
    for(j=i+1; j<row; j++){
      sum=0.;
      for(k=0; k<i; k++){
	t_matrixfloat ljk=cholesky[k*col+j];
	t_matrixfloat lik=cholesky[k*col+i];

	sum+=ljk*lik;
      }
      cholesky[i*row+j]=(original[i*col+j]-sum)/result;
    }
  }

  /* 4. output the matrix */
  /* 4a convert the floatbuf to an atombuf; */
  float2matrix(x->atombuffer, cholesky);
  /* 4b destroy the buffers */
  freebytes(original, sizeof(t_matrixfloat)*row2);

  /* 4c output the atombuf; */
  matrix_bang(x);
}

static void *mtx_cholesky_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_cholesky_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;

  return (x);
}
void mtx_cholesky_setup(void)
{
  mtx_cholesky_class = class_new(gensym("mtx_cholesky"), (t_newmethod)mtx_cholesky_new, 
				(t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_cholesky_class, matrix_bang);
  class_addmethod(mtx_cholesky_class, (t_method)mtx_cholesky_matrix, gensym("matrix"), A_GIMME, 0);

}

void iemtx_cholesky_setup(void){
  mtx_cholesky_setup();
}
