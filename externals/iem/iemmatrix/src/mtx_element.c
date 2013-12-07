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


/* mtx_element */
static t_class *mtx_element_class;

static void mtx_element_list2(t_matrix *x, t_floatarg f1, t_floatarg f2)
{
  int r = f1, c= f2;
  if(r<0)r=0;
  if(c<0)c=0;
  x->current_row = r;
  x->current_col = c;
}
static void mtx_element_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
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
static void mtx_element_float(t_matrix *x, t_floatarg f)
{
  if(x->current_col>x->col || x->current_row>x->row){
    pd_error(x,"mtx_element: element position exceeds matrix dimensions");
    return;
  }
  if(x->current_row == 0 && x->current_col == 0){
    matrix_set(x, f);
    matrix_bang(x);
    return;
  }
  if(x->current_row*x->current_col)SETFLOAT(x->atombuffer+1+(x->current_row-1)*x->col+x->current_col, f);
  else {
    t_atom *ap=x->atombuffer+2;
    int count;
    if (!x->current_col){
      ap+=x->col*(x->current_row-1);
      count=x->col;
      while(count--)SETFLOAT(&ap[count], f);
    } else {
      ap+=x->current_col-1;
      count=x->row;
      while(count--)SETFLOAT(&ap[count*x->col], f);
    }
  }
  matrix_bang(x);
}

static void *mtx_element_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_element_class);
  int i, j, q;
  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  x->current_row=x->current_col=0;
  x->col=x->row=0;
  x->atombuffer=0;
  switch (argc) {
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
  case 4:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    q = atom_getfloat(argv++);if(q<0)q=0;
    x->current_row=q;
    q = atom_getfloat(argv++);if(q<0)q=0;
    x->current_col=q;
    break;
  default:;
  }
  return (x);
}
void mtx_element_setup(void)
{
  mtx_element_class = class_new(gensym("mtx_element"), (t_newmethod)mtx_element_new, 
				(t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_element_class, matrix_bang);
  class_addfloat (mtx_element_class, mtx_element_float);
  class_addmethod(mtx_element_class, (t_method)mtx_element_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_element_class, (t_method)mtx_element_list2, gensym(""), A_FLOAT, A_FLOAT, 0);

}


void iemtx_element_setup(void)
{
  mtx_element_setup();
}
