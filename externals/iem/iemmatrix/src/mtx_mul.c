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

/*
  mtx_mul
  mtx_*
  mtx_.*
  mtx_./

  matrix multiplication

*/


/* mtx_mul */
static t_class *mtx_mul_class, *mtx_mulelement_class, *mtx_mulscalar_class;

static void mtx_mul_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap1=argv+2, *ap2=m2->atombuffer+2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  int row2, col2, n, r, c;

  if (!m2->atombuffer){ pd_error(x, "right-hand matrix is missing");            return; }
  if (argc<2){          pd_error(x, "crippled matrix");        return; }
  if ((col<1)||(row<1)){pd_error(x, "invalid dimensions");     return; }
  if (col*row>argc-2){  pd_error(x, "sparse matrix not yet supported : use \"mtx_check\""); return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
 
  if (col!=row2) {      pd_error(x, "matrix dimensions do not match !"); return;  }

  adjustsize(m, row, col2); 
  ap=m->atombuffer+2;

  for(r=0;r<row;r++)
    for(c=0;c<col2;c++) {
      t_matrixfloat sum = 0.f;
      for(n=0;n<col;n++)sum+=(t_matrixfloat)atom_getfloat(ap1+col*r+n)*atom_getfloat(ap2+col2*n+c);
      SETFLOAT(ap+col2*r+c,sum);
    }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}

static void mtx_mul_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ pd_error(x, "right-hand matrix is missing");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f*atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}

static void mtx_mulelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    pd_error(x, "crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    pd_error(x, "invalid dimensions");    return;  }
  if (col*row>argc-2){    pd_error(x, "sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    pd_error(x, "matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++)*atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}

static void mtx_mulscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    pd_error(x, "crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_mulscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_mul_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) error("[%s]: extra arguments ignored", s->s_name);
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_mulscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    if (s->s_name[4]=='.') {
      /* element mul */

      t_matrix *x = (t_matrix *)pd_new(mtx_mulelement_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
      outlet_new(&x->x_obj, 0);
      x->col = x->row = 0;
      x->atombuffer = 0;
      return(x);
    } else {
      t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_mul_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
      outlet_new(&x->x_obj, 0);
      x->m.col = x->m.row = x->m2.col = x->m2.row = 0;
      x->m.atombuffer = x->m2.atombuffer = 0;
      return (x);
    }
  }
}

void mtx_mul_setup(void)
{
  mtx_mul_class = class_new(gensym("mtx_mul"), (t_newmethod)mtx_mul_new, (t_method)mtx_binmtx_free,
                            sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_mul_new, gensym("mtx_*"), A_GIMME,0);
  class_addmethod(mtx_mul_class, (t_method)mtx_mul_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_mul_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_mul_class, mtx_mul_float);
  class_addbang  (mtx_mul_class, mtx_binmtx_bang);

  mtx_mulelement_class = class_new(gensym("mtx_.*"), (t_newmethod)mtx_mul_new, (t_method)mtx_binmtx_free,
                                   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_mulelement_class, (t_method)mtx_mulelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_mulelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_mulelement_class, mtx_mul_float);
  class_addbang  (mtx_mulelement_class, mtx_binmtx_bang);
  class_sethelpsymbol(mtx_mulelement_class, gensym("mtx_mul-help"));

  mtx_mulscalar_class = class_new(gensym("mtx_mul"), 0, (t_method)mtx_binscalar_free,
                                  sizeof(t_mtx_binscalar), 0, 0);

  class_addmethod(mtx_mulscalar_class, (t_method)mtx_mulscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_mulscalar_class, mtx_mulscalar_list);
  class_addbang  (mtx_mulscalar_class, mtx_binscalar_bang);
}


/* mtx_div */
static t_class *mtx_divelement_class, *mtx_divscalar_class;

static void mtx_divelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    pd_error(x, "crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    pd_error(x, "invalid dimensions");    return;  }
  if (col*row>argc-2){    pd_error(x, "sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    pd_error(x, "matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++)/atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_divelement_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ pd_error(x, "right-hand matrix missing");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f/atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void mtx_divscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = 1.0/x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    pd_error(x, "crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_divscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = 1.0/x->f;

  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_div_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) error("[%s] extra arguments ignored", s->s_name);
  if (argc) {
    /* scalar division */
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_divscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    /* element division */
    t_matrix *x = (t_matrix *)pd_new(mtx_divelement_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->col = x->row = 0;
    x->atombuffer = 0;
    return(x);
  }
}

void mtx_div_setup(void)
{
  mtx_divelement_class = class_new(gensym("mtx_./"), (t_newmethod)mtx_div_new, (t_method)mtx_binmtx_free,
                                   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_divelement_class, (t_method)mtx_divelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_divelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_divelement_class, mtx_divelement_float);
  class_addbang  (mtx_divelement_class, mtx_binmtx_bang);

  mtx_divscalar_class = class_new(gensym("mtx_./"), 0, (t_method)mtx_binscalar_free,
                                  sizeof(t_mtx_binscalar), 0, 0);
  class_addmethod(mtx_divscalar_class, (t_method)mtx_divscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_divscalar_class, mtx_divscalar_list);
  class_addbang  (mtx_divscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_divelement_class, gensym("mtx_mul-help"));
  class_sethelpsymbol(mtx_divscalar_class, gensym("mtx_mul-help"));
}

void iemtx_mul_setup(void)
{
  mtx_mul_setup();
  mtx_div_setup();
}
