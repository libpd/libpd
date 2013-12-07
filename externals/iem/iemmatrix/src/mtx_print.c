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

typedef struct _mtxprint
{
  t_object x_obj;
  t_symbol *x_s;
} t_mtxprint;


/* mtx_print */
static t_class *mtx_print_class;
static void mtx_print(t_mtxprint *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  if (argc<2){
    post("mtx_print : crippled matrix");
    return;
  }
  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  if(row*col>argc-2){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  post("%s:", x->x_s->s_name);
  while(row--){
    postatom(col, argv);
    argv+=col;
    endpost();
  }
  endpost();
}
static void *mtx_print_new(t_symbol*s)
{
  t_mtxprint *x = (t_mtxprint *)pd_new(mtx_print_class);
  x->x_s=(s&&s!=&s_)?s:gensym("matrix");
  return (x);
}
void mtx_print_setup(void)
{
  mtx_print_class = class_new(gensym("mtx_print"), (t_newmethod)mtx_print_new, 
			      0, sizeof(t_mtxprint), 0, A_DEFSYM, 0);
  class_addmethod  (mtx_print_class, (t_method)mtx_print, gensym("matrix"), A_GIMME, 0);

}

void iemtx_print_setup(void){
  mtx_print_setup();
}

