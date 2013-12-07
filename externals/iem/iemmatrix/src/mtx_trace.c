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

/* mtx_trace */
static t_class *mtx_trace_class;
typedef struct _mtx_trace
{
  t_object x_obj;
  t_float trace;
} t_mtx_trace;
static void mtx_trace_bang(t_mtx_trace *x)
{
  outlet_float(x->x_obj.ob_outlet, x->trace);
}
static void mtx_trace_matrix(t_mtx_trace *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  int length=(col<row)?col:row;
  t_float trace = 0;
  if(row*col>argc-2)post("mtx_trace: sparse matrices not yet supported : use \"mtx_check\"");
  else while(length--)trace+=atom_getfloat(argv+length*(col+1));
  x->trace=trace;
  mtx_trace_bang(x);
}
static void *mtx_trace_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mtx_trace *x = (t_mtx_trace *)pd_new(mtx_trace_class);
  outlet_new(&x->x_obj, 0);
  x->trace=0;
  return (x);
}
void mtx_trace_setup(void)
{
  mtx_trace_class = class_new(gensym("mtx_trace"), (t_newmethod)mtx_trace_new, 
			      0, sizeof(t_mtx_trace), 0, A_GIMME, 0);
  class_addbang  (mtx_trace_class, mtx_trace_bang);
  class_addmethod(mtx_trace_class, (t_method)mtx_trace_matrix, gensym("matrix"), A_GIMME, 0);

}

void iemtx_trace_setup(void)
{
  mtx_trace_setup();
}
