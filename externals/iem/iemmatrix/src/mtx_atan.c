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

/* mtx_atan: B=atan(A); */

static t_class *mtx_atan_class;

static void mtx_atan_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getint(argv++);
  int col=atom_getint(argv++);
  t_atom *m;
  int n = row*col;

  if (argc<2){    post("mtx_atan: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_atan: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++);
    SETFLOAT(m, (t_float)atanf(f));
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}

static void mtx_atan_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;

  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = (t_float)atanf(atom_getfloat(argv++));
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_atan_new(t_symbol *s)
{
  /* element atan */
  t_matrix *x = (t_matrix *)pd_new(mtx_atan_class);
  outlet_new(&x->x_obj, 0);
  x->col = x->row = 0;
  x->atombuffer = 0;
  return(x);
}

void mtx_atan_setup(void)
{
  mtx_atan_class = class_new(gensym("mtx_atan"), (t_newmethod)mtx_atan_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_atan_class, (t_method)mtx_atan_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_atan_class, mtx_atan_list);
  class_addbang  (mtx_atan_class, mtx_binmtx_bang);


}

void iemtx_atan_setup(void)
{
  mtx_atan_setup();
}
