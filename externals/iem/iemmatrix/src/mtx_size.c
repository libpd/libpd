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

/* mtx_size */
static t_class *mtx_size_class;
typedef struct _mtx_size
{
  t_object x_obj;

  int      row;
  int      col;

  t_outlet *left, *right;
} t_mtx_size;

static void mtx_size_matrix(t_mtx_size *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc<2)return;
  outlet_float(x->right, atom_getfloat(argv+1));
  outlet_float(x->left,  atom_getfloat(argv));

}

static void *mtx_size_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mtx_size *x = (t_mtx_size *)pd_new(mtx_size_class);
  x->left  = outlet_new(&x->x_obj, 0);
  x->right = outlet_new(&x->x_obj, 0);

  return (x);
}
void mtx_size_setup(void)
{
  mtx_size_class = class_new(gensym("mtx_size"), (t_newmethod)mtx_size_new, 
			     0, sizeof(t_mtx_size), 0, A_GIMME, 0);
  class_addmethod(mtx_size_class, (t_method)mtx_size_matrix, gensym("matrix"), A_GIMME, 0);

}

void iemtx_size_setup(void){
  mtx_size_setup();
}
