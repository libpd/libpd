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

/* mtx_sum */
/* column-wise sum
 */
static t_class *mtx_sum_class;
static void mtx_sum_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  int n;

  if(row*col>argc-2)post("mtx_sum: sparse matrices not yet supported : use \"mtx_check\"");
  else {
    t_atom *ap = (t_atom *)getbytes(col * sizeof(t_atom)), *dummy=ap;

    for(n=0;n<col;n++, dummy++){
      int i=row;
      t_float f=0.f;
      t_atom*ap2=argv+n;
      while(i--){
	f+=atom_getfloat(ap2+col*i);
      }
      SETFLOAT(dummy, f);
    }

    outlet_list(x->x_obj.ob_outlet, gensym("prod"), col, ap);

    freebytes(ap, (col * sizeof(t_atom)));
  }
}
static void mtx_sum_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv){
  t_float f=0.f;
  while(argc--)f+=atom_getfloat(argv++);
  outlet_float(x->x_obj.ob_outlet, f);
}


static void *mtx_sum_new(void)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_sum_class);
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;

  return (x);
}
void mtx_sum_setup(void)
{
  mtx_sum_class = class_new(gensym("mtx_sum"), (t_newmethod)mtx_sum_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist  (mtx_sum_class, mtx_sum_list);
  class_addmethod(mtx_sum_class, (t_method)mtx_sum_matrix, gensym("matrix"), A_GIMME, 0);

}
void iemtx_sum_setup(void){
  mtx_sum_setup();
}
