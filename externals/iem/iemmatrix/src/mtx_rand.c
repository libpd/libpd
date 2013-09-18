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

/* mtx_rand */
static t_class *mtx_rand_class;

static void mtx_rand_seed(t_matrix *x, t_float f)
{
  x->current_row=f;
}
static int makeseed(void)
{
  static unsigned int random_nextseed = 1489853723;
  random_nextseed = random_nextseed * 435898247 + 938284287;
  return (random_nextseed & 0x7fffffff);
}
static void mtx_rand_random(t_matrix *x)
{
  long size = x->row * x->col;
  t_atom *ap=x->atombuffer+2;
  int val = x->current_row;
  while(size--)SETFLOAT(ap+size, ((float)(((val=val*435898247+382842987)&0x7fffffff)-0x40000000))*(float)(0.5/0x40000000)+0.5);
  x->current_row=val;
}

static void mtx_rand_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row = atom_getfloat(argv++);
  int col = atom_getfloat(argv++);

  if(!argv)return;
  if(argc==1)col=row;

  adjustsize(x, row, col);
  mtx_rand_random(x);
  matrix_bang(x);
}
static void mtx_rand_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  matrix_matrix2(x, s, argc, argv);
  mtx_rand_random(x);
  matrix_bang(x);
}
static void mtx_rand_bang(t_matrix *x)
{
  mtx_rand_random(x);
  matrix_bang(x);
}
static void *mtx_rand_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_rand_class);
  int row, col;
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;
  x->current_row=makeseed();

  if (argc) {
	row=atom_getfloat(argv);
	col=(argc>1)?atom_getfloat(argv+1):row;
	adjustsize(x, row, col);
	mtx_rand_random(x);
  }
  return (x);
}
void mtx_rand_setup(void)
{
  mtx_rand_class = class_new(gensym("mtx_rand"), (t_newmethod)mtx_rand_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addmethod(mtx_rand_class, (t_method)mtx_rand_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_rand_class, mtx_rand_list);
  class_addbang  (mtx_rand_class, mtx_rand_bang);

  class_addmethod(mtx_rand_class, (t_method)mtx_rand_seed, gensym("seed"), A_FLOAT, 0);

}

void iemtx_rand_setup(void){
  mtx_rand_setup();
}

