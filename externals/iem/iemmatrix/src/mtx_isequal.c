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
  mtx_isequal
*/

/* mtx_isequal */
static t_class *mtx_isequal_class, *mtx_isequalscalar_class;

static void mtx_isequalscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  
  t_float offset=x->f;
  t_atom *buf;
  t_atom *ap=argv+2;

  if(argc<2){post("mtx_isequal: crippled matrix");return; }

  while(n--){
    if(atom_getfloat(ap)!=offset) {
      outlet_float(x->x_obj.ob_outlet, (t_float)0);
      return;
    }
    ap++;
  }
  outlet_float(x->x_obj.ob_outlet, (t_float)1);
}
static void mtx_isequalscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *ap=argv;
  t_float offset = x->f;

  while(n--){
    if(atom_getfloat(ap)!=offset) {
      outlet_float(x->x_obj.ob_outlet, (t_float)0);
      return;
    }
    ap++;
  }
  outlet_float(x->x_obj.ob_outlet, (t_float)1);
}

static void mtx_isequal_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_isequal: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_isequal: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  if ((col!=x->m2.col)||(row!=x->m2.row)){
    outlet_float(x->x_obj.ob_outlet, (t_float)0);
    return;
  }

  while(n--){
    if(atom_getfloat(m1++)!=atom_getfloat(m2++)) {
      outlet_float(x->x_obj.ob_outlet, (t_float)0);
      return;
    }
  }
  
  outlet_float(x->x_obj.ob_outlet, (t_float)1);
}
static void mtx_isequal_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ 
    outlet_float(x->x_obj.ob_outlet, (t_float)0);
    return; 
  }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);

  n=row2*col2;


  while(n--){
    if(atom_getfloat(ap)!=f) {
      outlet_float(x->x_obj.ob_outlet, (t_float)0);
      return;
    }
    ap++;
  }
  outlet_float(x->x_obj.ob_outlet, (t_float)1);
}
static void *mtx_isequal_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_isequal : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_isequalscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_isequal_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
  }
}

void mtx_isequal_setup(void)
{
  mtx_isequal_class = class_new(gensym("mtx_isequal"), (t_newmethod)mtx_isequal_new, (t_method)mtx_binmtx_free,
                            sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_isequal_class, (t_method)mtx_isequal_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_isequal_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_isequal_class, mtx_isequal_float);
  class_addbang  (mtx_isequal_class, mtx_binmtx_bang);

  mtx_isequalscalar_class = class_new(gensym("mtx_isequal"), 0, (t_method)mtx_binscalar_free,
                                  sizeof(t_mtx_binscalar), 0, 0);
  class_addmethod(mtx_isequalscalar_class, (t_method)mtx_isequalscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_isequalscalar_class, mtx_isequalscalar_list);
  class_addbang  (mtx_isequalscalar_class, mtx_binscalar_bang);



}

void iemtx_isequal_setup(void)
{
  mtx_isequal_setup();
}
