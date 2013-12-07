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

/* mtx_inverse */
static t_class *mtx_inverse_class;

static void mtx_inverse_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* maybe we should do this in double or long double ? */
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);

  int err=0;

  t_matrixfloat *original, *inverted;

  if(row*col+2>argc){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }

  /* reserve memory for outputting afterwards */
  adjustsize(x, col, row);

  /* 1. extract values of A to float-buf */
  original=matrix2float(argv);

  if (row==col){
    /* fine, the matrix is square */
    inverted=mtx_doInvert(original, row, &err);
  } else {
    /* we'll have to do the pseudo-inverse:
     * P=A'*inv(A*A') if row<col
     * P=inv(A'*A)*A' if col<row 
     */
    t_matrixfloat*transposed, *invertee;
    int inverteeCol=0;
    transposed=mtx_doTranspose(original, row, col);
    if(row>col){
      inverteeCol=col;
      invertee  =mtx_doMultiply(col, transposed, row, original, col);
      inverted  =mtx_doMultiply(col, mtx_doInvert(invertee, col, &err), col, transposed, row);
    } else {
      inverteeCol=row;
      invertee  =mtx_doMultiply(row, original, col, transposed, row);
      inverted  =mtx_doMultiply(col, transposed, row, mtx_doInvert(invertee, row, &err), row);
    }
    freebytes(transposed, sizeof(t_matrixfloat)*col*row);
    freebytes(invertee  , sizeof(t_matrixfloat)*inverteeCol*inverteeCol);
  }

  /* 3. output the matrix */
  /* 3a convert the floatbuf to an atombuf; */
  float2matrix(x->atombuffer, inverted);
  /* 3b destroy the buffers */
  freebytes(original, sizeof(t_matrixfloat)*row*col);

  if(err){
    outlet_bang(x->x_outlet);
    pd_error(x, "mtx_inverse: couldn't really invert the matrix !!! %d error%c", err, (err-1)?'s':0);
  }

  /* 3c output the atombuf; */
  matrix_bang(x);
}

static void *mtx_inverse_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_inverse_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;
  x->x_outlet=outlet_new(&x->x_obj, 0);

  return (x);
}
void mtx_inverse_setup(void)
{
  mtx_inverse_class = class_new(gensym("mtx_inverse"), (t_newmethod)mtx_inverse_new, 
                                (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_inverse_class, matrix_bang);
  class_addmethod(mtx_inverse_class, (t_method)mtx_inverse_matrix, gensym("matrix"), A_GIMME, 0);

}

void iemtx_inverse_setup(void){
  mtx_inverse_setup();
}
