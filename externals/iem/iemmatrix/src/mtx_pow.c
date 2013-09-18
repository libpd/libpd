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

/* mtx_.^ */
/* LATER: do a mtx_pow, mtx_^ */

static t_class *mtx_powelement_class, *mtx_powscalar_class;

static void mtx_powelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_pow: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_pow: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    post("matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = powf(atom_getfloat(argv++),atom_getfloat(m2++));
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_powelement_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("power by what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, powf(f,atom_getfloat(ap2++)));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void mtx_powscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    post("mtx_pow: crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = powf(atom_getfloat(argv++),factor);
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_powscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = x->f;

  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = powf(atom_getfloat(argv++),factor);
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_pow_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_pow : extra arguments ignored");
  if (argc) {
    /* scalar powision */
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_powscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    /* element powision */
    t_matrix *x = (t_matrix *)pd_new(mtx_powelement_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->col = x->row = 0;
    x->atombuffer = 0;
    return(x);
  }
}

void mtx_pow_setup(void)
{
  mtx_powelement_class = class_new(gensym("mtx_.^"), (t_newmethod)mtx_pow_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_powelement_class, (t_method)mtx_powelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_powelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_powelement_class, mtx_powelement_float);
  class_addbang  (mtx_powelement_class, mtx_binmtx_bang);

  mtx_powscalar_class = class_new(gensym("mtx_.^"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addmethod(mtx_powscalar_class, (t_method)mtx_powscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_powscalar_class, mtx_powscalar_list);
  class_addbang  (mtx_powscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_powelement_class, gensym("mtx_pow-help"));
  class_sethelpsymbol(mtx_powscalar_class, gensym("mtx_pow-help"));
}

void iemtx_pow_setup(void)
{
  mtx_pow_setup();
}
