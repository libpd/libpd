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

/* mtx_gauss */

/*
 * gauss elimination of a matrix (without semi-pivoting)
 */

static t_class *mtx_gauss_class;

static void mtx_gauss_xch(t_matrixfloat*a, t_matrixfloat*b, int count){
  while(count--){
    t_matrixfloat dummy=*a;
    *a++=*b;
    *b++=dummy;
  }
}
static void mtx_gauss_mulsub(t_matrixfloat*a, t_matrixfloat*b, int count, t_matrixfloat f){
  t_matrixfloat f2=1./f;
  while(count--){
    t_matrixfloat dummy=(f* (*b) - *a++)*f2;
    *b++=dummy;
  }
}

static void mtx_gauss_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* maybe we should do this in double or long double ? */
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  int i, j;
  const t_matrixfloat singrange = 1.0e-10;

  t_matrixfloat *original;
  t_matrixfloat *a1, *a2;  /* dummy pointers */


  if(row*col+2>argc){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  if (row!=col){
    post("mtx_gauss: only square matrices can be gauss eliminated");
    return;
  }

  /* reserve memory for outputting afterwards */
  adjustsize(x, row, row);
  original=matrix2float(argv);

  /* Gauss elimination */
  for(i=0; i<row; i++) {
    int nz = 0;
    a1=original+i*(col+1);
    for(j=i; j<row; j++){
      const t_matrixfloat f=*a1;
      if((f>singrange)||(f<-singrange)){
	nz=j;
	break;
      }
      a1+=col;
    }
    /*if(nz)*/
    {
      /* exchange rows "nz" and "i" */
      if(nz != i)mtx_gauss_xch(original+i*col+i, original+nz*col+i, col-i);

      for(j=i+1; j<row; j++){
        t_matrixfloat f=0.;
        a1=original+i*(col+1);
        a2=original+j*col+i;
        if(*a2){
          f=*a1 / *a2;
          mtx_gauss_mulsub(a1, a2, col-i, f);
        }
      }
    }
  }

  /* 3. output the matrix */
  /* 3a convert the floatbuf to an atombuf; */
  float2matrix(x->atombuffer, original);

  /* 3c output the atombuf; */
  matrix_bang(x);
}

static void *mtx_gauss_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_gauss_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;

  return (x);
}
void mtx_gauss_setup(void)
{
  mtx_gauss_class = class_new(gensym("mtx_gauss"), (t_newmethod)mtx_gauss_new, 
				(t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_gauss_class, matrix_bang);
  class_addmethod(mtx_gauss_class, (t_method)mtx_gauss_matrix, gensym("matrix"), A_GIMME, 0);

}

void iemtx_gauss_setup(void){
  mtx_gauss_setup();
}
