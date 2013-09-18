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

/* mtx_row */
static t_class *mtx_row_class;

static void mtx_row_float(t_matrix *x, t_floatarg f)
{
  int i = f;
  if(i<0)i=0;
  x->current_row = i;
}
static void mtx_row_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;
  if (argc<2){    post("matrix : corrupt matrix passed");    return;  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){    post("matrix : corrupt matrix passed");    return;  }
  if (row*col > argc-2){    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");    return;  }
  matrix_matrix2(x, s, argc, argv);
  matrix_bang(x);
}
static void mtx_row_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==1){
    t_float f=atom_getfloat(argv);
    t_atom *ap=x->atombuffer+2+(x->current_row-1)*x->col;
    if (x->current_row>x->row){
      post("mtx_row : too high a row is to be set");
      return;
    }
    if (x->current_row){
      int n=x->col;
      while(n--){
	SETFLOAT(ap, f);
	ap++;
      }
    }
    matrix_bang(x);
    return;
  }

  if (argc<x->col){
    post("mtx_row : row length is too small for %dx%d-matrix", x->row, x->col);
    return;
  }
  if (x->current_row>x->row){
    post("mtx_row : too high a row is to be set");
    return;
  }
  if(x->current_row) {memcpy(x->atombuffer+2+(x->current_row-1)*x->col, argv, x->col*sizeof(t_atom));
  }  else {
    int r=x->row;
    while(r--)memcpy(x->atombuffer+2+r*x->col, argv, x->col*sizeof(t_atom));      
  }
  matrix_bang(x);
}
static void *mtx_row_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_row_class);
  int i, j, q;

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  x->current_row=0;
  x->col=x->row=0;
  x->atombuffer=0;
  switch (argc) {
  case 0:break;
  case 1:
    i = atom_getfloat(argv);
    if (i<0)i=0;
    if(i)adjustsize(x, i, i);
    matrix_set(x, 0);
    break;
  case 2:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    break;
  default:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    q = atom_getfloat(argv++);if(q<0)q=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    x->current_row=q;
  }
  return (x);
}
void mtx_row_setup(void)
{
  mtx_row_class = class_new(gensym("mtx_row"), (t_newmethod)mtx_row_new, 
			    (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_row_class, matrix_bang);
  class_addlist  (mtx_row_class, mtx_row_list);
  class_addmethod(mtx_row_class, (t_method)mtx_row_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_row_class, (t_method)mtx_row_float, gensym(""), A_FLOAT, 0);

}


void iemtx_row_setup(void)
{
  mtx_row_setup();
}
