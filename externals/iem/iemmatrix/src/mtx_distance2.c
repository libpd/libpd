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
  mtx_distance2: gets the euclidian distances (squared) between 2 sets of n-dimensional vectors
*/

/* -­------------------------------------------------------------- */
/* matrix math */

/* mtx_distance2 */
static t_class *mtx_distance2_class;

static void mtx_distance2_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  int row2=0, col2=0;
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int i, j;

  if (argc<2){    
    post("mtx_distance2: crippled matrix");
    return;
  }
  if ((col<1)||(row<1)) {
    post("mtx_distance2: invalid dimensions");
    return;
  }
  if (col*row>argc-2){
    post("sparse matrix not yet supported : use \"mtx_check\"");
    return;
  }

  row2=x->m2.row;
  col2=x->m2.col;

  if (!(col2*row2)) {
    /* 2nd matrix is NULL; take the 1st matrix instead (distance between it's own vectors) */
    m2=argv+2;
    row2=row;
    col2=col;
  } else if (col!=col2){ 
    post("mtx_distance2: matrix dimensions do not match");
    return;
  }

  adjustsize(&x->m, row, row2);
  m = x->m.atombuffer+2;

  for(i=0; i<row; i++)
    for(j=0; j<row2; j++){
      t_float f=0.f;
      int c1=col*i;
      int c2=col*j;

      int n;
      for(n=0; n<col; n++){
	t_float val=atom_getfloat(&m1[c1+n])-atom_getfloat(&m2[c2+n]);
	f+=val*val;
      }
      SETFLOAT(m,f);
      m++;
    }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), row*row2+2, x->m.atombuffer);
}
static void *mtx_distance2_new(void)
{
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_distance2_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
}

void mtx_distance2_setup(void)
{
  mtx_distance2_class = class_new(gensym("mtx_distance2"), 
				  (t_newmethod)mtx_distance2_new, (t_method)mtx_binmtx_free,
				  sizeof(t_mtx_binmtx), 0, A_NULL, 0);
  class_addmethod(mtx_distance2_class, (t_method)mtx_distance2_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_distance2_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addbang  (mtx_distance2_class, mtx_binmtx_bang);


}

void iemtx_distance2_setup(void)
{
  mtx_distance2_setup();
}
