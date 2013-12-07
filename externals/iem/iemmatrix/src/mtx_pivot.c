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


/* mtx_pivot */
static t_class *mtx_pivot_class;

typedef struct _mtx_pivot
{
  t_object x_obj;

  t_matrix m;  /* the output matrix */
  t_matrix m_pre;  /* the pre -multiply matrix */
  t_matrix m_post; /* the post-multiply matrix */

  t_outlet *pivo, *pre, *post;

  t_int ascending;
  
} t_mtx_pivot;

static void mtx_pivot_matrix(t_mtx_pivot *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m_pre, *m_post;
  int i, j, k;
  int min_rowcol = (row<col)?row:col;
  t_matrixfloat *buffer, *buf;
  int *i_pre, *i_post, *i_buf;

  int pivot_row, pivot_col;

  int ascending=(x->ascending);

  if (argc<2){    
    post("mtx_pivot: crippled matrix");    
    return;  
  }
  if ((col<1)||(row<1)){
    post("mtx_pivot: invalid dimensions");    
    return;  
  }
  if (col*row>argc-2){
    post("sparse matrix not yet supported : use \"mtx_check\"");
    return;  
  }

  adjustsize(&x->m, row, col);
  adjustsize(&x->m_pre, row, row);
  adjustsize(&x->m_post,col, col);
  matrix_set(&x->m_pre, 0);
  matrix_set(&x->m_post, 0);

  buffer = matrix2float(argv);
  i_pre    = (int *)getbytes(sizeof(int)*row);
  i_post   = (int *)getbytes(sizeof(int)*col);

  /* clear pre&post matrices */
  i=row;
  i_buf=i_pre;
  while(i--)*i_buf++=row-i-1;
  i=col;
  i_buf=i_post;
  while(i--)*i_buf++=col-i-1;

  /* do the pivot thing */

  for (k=0; k<min_rowcol; k++){
    /* 1. find max_element */
    t_float tmp = fabsf(buffer[k*(1+col)]);
    pivot_row = pivot_col = k;

    for(i=k; i<row; i++){
      buf=buffer+col*i+k;

      j=col-k;
      while(j--){
	t_float f = fabsf(*buf++);
	if ((ascending && f>tmp) || (!ascending && f<tmp)) {
	  tmp=f;
	  pivot_row = i;
	  pivot_col = col-j-1;
	}
      }
    }
    /* 2. move tmp el to [k,k] */
    /* 2a swap rows */
    if (k-pivot_row) {
      t_matrixfloat *oldrow=buffer+col*k;
      t_matrixfloat *newrow=buffer+col*pivot_row;

      i=col;
      while(i--){
	t_matrixfloat f=*oldrow;
	*oldrow++=*newrow;
	*newrow++=f;
      }
      i=i_pre[k];
      i_pre[k]=i_pre[pivot_row];
      i_pre[pivot_row]=i;
    }
    /* 2b swap columns */
    if (k-pivot_col) {
      t_matrixfloat *oldcol=buffer+k;
      t_matrixfloat *newcol=buffer+pivot_col;

      i=row;
      while(i--){
	t_matrixfloat f=*oldcol;
	*oldcol=*newcol;
	*newcol=f;
	oldcol+=col;
	newcol+=col;
      }
      i=i_post[k];
      i_post[k]=i_post[pivot_col];
      i_post[pivot_col]=i;
    }
  }

  float2matrix(x->m.atombuffer, buffer);

  i=col;
  m_post = x->m_post.atombuffer+2;
  while(i--){
    SETFLOAT(m_post+i_post[i]*col+i, 1);
  }
  i=row;
  m_pre = x->m_pre.atombuffer+2;
  while(i--){
    SETFLOAT(m_pre+i_pre[i]+i*row, 1);
  }

  
  outlet_anything(x->post, gensym("matrix"), 2+col*col, x->m_post.atombuffer);
  outlet_anything(x->pre, gensym("matrix"), 2+row*row, x->m_pre.atombuffer);
  outlet_anything(x->pivo , gensym("matrix"), 2+row*col, x->m.atombuffer );
}

static void mtx_pivot_free(t_mtx_pivot *x)
{
  matrix_free(&x->m);
  matrix_free(&x->m_pre);
  matrix_free(&x->m_post);
}

static void *mtx_pivot_new(t_floatarg f)
{
  t_mtx_pivot *x = (t_mtx_pivot *)pd_new(mtx_pivot_class);

  x->pivo = outlet_new(&x->x_obj, 0);
  x->pre  = outlet_new(&x->x_obj, 0);
  x->post = outlet_new(&x->x_obj, 0);

  x->ascending = (f < 0.f)?0:1;

  x->m.atombuffer = x->m_pre.atombuffer = x->m_post.atombuffer = 0;
  x->m.row = x->m.col = x->m_pre.row = x->m_pre.col = x->m_post.row = x->m_post.col = 0;

  return(x);
}

void mtx_pivot_setup(void)
{
  mtx_pivot_class = class_new(gensym("mtx_pivot"), (t_newmethod)mtx_pivot_new, (t_method)mtx_pivot_free,
			      sizeof(t_mtx_pivot), 0, 
			      A_DEFFLOAT, 0);
  class_addmethod(mtx_pivot_class, (t_method)mtx_pivot_matrix, gensym("matrix"), A_GIMME, 0);


}

void iemtx_pivot_setup(void)
{
  mtx_pivot_setup();
}
