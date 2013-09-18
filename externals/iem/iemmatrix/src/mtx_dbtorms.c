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

#define LOGTEN 2.302585092994

/* mtx_dbtorms: B=log(A); B[n,m]=e^A[n,m]  */

static t_class *mtx_dbtorms_class;

static void mtx_dbtorms_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  int n = argc-2;

  if (argc<2){    post("mtx_dbtorms: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_dbtorms: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f=atom_getfloat(argv++);
    t_float v=0;
    f=(f>485)?485:f;
    v=(f<=0)?0:exp((LOGTEN*0.05) * (f-100.));
    SETFLOAT(m, (v<0)?0:v);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}

static void mtx_dbtorms_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;

  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    t_float f=atom_getfloat(argv++);
    t_float v=0;
    f=(f>485)?485:f;
    v=(f<=0)?0:exp((LOGTEN*0.05) * (f-100.));
    SETFLOAT(m, (v<0)?0:v);
    m++;
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_dbtorms_new(t_symbol *s)
{
  /* element log */
  t_matrix *x = (t_matrix *)pd_new(mtx_dbtorms_class);
  outlet_new(&x->x_obj, 0);
  x->col = x->row = 0;
  x->atombuffer = 0;
  return(x);
}

void mtx_dbtorms_setup(void)
{
  mtx_dbtorms_class = class_new(gensym("mtx_dbtorms"), (t_newmethod)mtx_dbtorms_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_dbtorms_class, (t_method)mtx_dbtorms_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_dbtorms_class, mtx_dbtorms_list);
  class_addbang  (mtx_dbtorms_class, mtx_binmtx_bang);


}

void iemtx_dbtorms_setup(void)
{
  mtx_dbtorms_setup();
}
