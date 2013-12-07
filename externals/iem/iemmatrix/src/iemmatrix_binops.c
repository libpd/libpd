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

/* -------------------------------------------------------------- */
/* matrix math */


void mtx_bin_matrix2(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row = atom_getfloat(argv);
  int col = atom_getfloat(argv+1);
  if (argc<2){post("mtx_bin2: crippled matrix"); return;}
  if ((col<1)||(row<1)) {post("mtx_bin2: invalid dimensions %dx%d", row,col); return;}
  if (col*row+2>argc){ post("mtx_bin2: sparse matrix not yet supported : use \"mtx_check\""); return;}

  if (row*col!=x->m2.row*x->m2.col) {
    freebytes(x->m2.atombuffer, (x->m2.row*x->m2.col+2)*sizeof(t_atom));
    x->m2.atombuffer=copybytes(argv,(row*col+2)*sizeof(t_atom));
  }else memcpy(x->m2.atombuffer, argv, (row*col+2)*sizeof(t_atom));
  setdimen(&x->m2, row, col);
}

void mtx_binmtx_bang(t_mtx_binmtx *x)
{
  if((&x->m)&&(x->m.atombuffer))
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->m.col*x->m.row+2, x->m.atombuffer);
}


void mtx_binmtx_free(t_mtx_binmtx *x)
{
  matrix_free(&x->m);
  matrix_free(&x->m2);
}
void mtx_binscalar_bang(t_mtx_binscalar *x)
{
  if((&x->m)&&(x->m.atombuffer))
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->m.col*x->m.row+2, x->m.atombuffer);
}
void mtx_binscalar_free(t_mtx_binscalar *x)
{
  matrix_free(&x->m);
}


void iemtx_binops_setup(void)
{
}
