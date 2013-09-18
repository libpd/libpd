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
  mtx_bspline: 
  this is only in the iemmatrix library since i have to make sure that there is an x-value
  for each y-value; this however enforces that for each point we have to define a point 
  in all dimensions;

  think: should we split this into 2 objects?
  - one for calculating the coefficients of the polynomial function for the bspline
  - another for calculating the value of a piecewise polyfuns
*/

static t_class *mtx_bspline_class;

typedef struct _mtx_spline
{
  t_object x_obj;
  t_outlet *x_outlet;

  int x_numpoints;
  int x_dimension;
  t_matrixfloat x_min, x_max;
  t_matrixfloat*x_x;
  t_matrixfloat**x_y, **x_u, **x_p;
  t_atom*x_result;
} t_mtx_spline;

static void mtx_bspline_resize(t_mtx_spline *x, int cols, int dim){
  int size=x->x_numpoints*sizeof(t_matrixfloat);
  int i=0;

  if(x->x_x)freebytes(x->x_x, size); x->x_x=0;

  for(i=0; i<x->x_dimension; i++){
    if(x->x_y&&x->x_y[i])freebytes(x->x_y[i], size); x->x_y[i]=0;
    if(x->x_u&&x->x_u[i])freebytes(x->x_u[i], size); x->x_u[i]=0;
    if(x->x_p&&x->x_p[i])freebytes(x->x_p[i], size); x->x_p[i]=0;
  }
  if(x->x_y)freebytes(x->x_y, x->x_dimension*sizeof(t_matrixfloat*)); x->x_y=0;
  if(x->x_u)freebytes(x->x_u, x->x_dimension*sizeof(t_matrixfloat*)); x->x_u=0;
  if(x->x_p)freebytes(x->x_p, x->x_dimension*sizeof(t_matrixfloat*)); x->x_p=0;

  if(x->x_result)freebytes(x->x_result, x->x_dimension*sizeof(t_atom)); x->x_p=0;

  if(dim<1)dim=1;
  if(cols<0)cols=0;

  x->x_numpoints = cols;
  x->x_dimension = dim;

  if(cols>0){
    size=x->x_numpoints*sizeof(t_matrixfloat);
    x->x_x = (t_matrixfloat*)getbytes(size);
    x->x_result = (t_atom*)getbytes(x->x_dimension*sizeof(t_atom));

    x->x_y = (t_matrixfloat**)getbytes(dim*sizeof(t_matrixfloat*));
    x->x_u = (t_matrixfloat**)getbytes(dim*sizeof(t_matrixfloat*));
    x->x_p = (t_matrixfloat**)getbytes(dim*sizeof(t_matrixfloat*));

    for(i=0; i<x->x_dimension; i++){
      x->x_y[i] = (t_matrixfloat*)getbytes(size);
      x->x_u[i] = (t_matrixfloat*)getbytes(size);
      x->x_p[i] = (t_matrixfloat*)getbytes(size);
    }
  }
}


static void mtx_bspline_matrix2(t_mtx_spline *X, t_symbol *s, int argc, t_atom *argv)
{
  int row=0;
  int col=0;

  t_matrixfloat *x, **y, **u, **p, *w, *d, *fp;
  t_matrixfloat*dummy;
  int i,j;
  int N;

  if (argc<2){    error("mtx_bspline: crippled matrix");    return;  }

  row=atom_getfloat(argv);
  col=atom_getfloat(argv+1);
  
  if ((col<2)||(row<3)) {    error("mtx_bspline: invalid dimensions");    return;  }
  if (col*row>argc-2){    error("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  col--;

  mtx_bspline_resize(X, row, col);

    /* 1st fill the matrix into the arrays */
  fp=matrix2float(argv);

  dummy=fp;
  x=X->x_x;
  y=X->x_y;
  u=X->x_u;
  p=X->x_p;

  for(i=0; i<row; i++){
    x[i]=*dummy++;
    for(j=0; j<col; j++){
      y[j][i]=*dummy++;
    }
  }
  X->x_min=x[0];
  X->x_max=x[row-1];


  w=(t_matrixfloat*)getbytes(X->x_numpoints*sizeof(t_matrixfloat));
  d=(t_matrixfloat*)getbytes(X->x_numpoints*sizeof(t_matrixfloat));

  N=row-1;

  for(j=0; j<col;j++){
    d[0]=0.0; d[1]=0.0;

    for(i=1; i<N; i++)
      d[i] = 2*(x[i+1]-x[i-1]);

    for(i=0; i<N; i++)
      u[j][i] = x[i+1]-x[i];

    for(i=1; i<N; i++)
      w[i]=6.0*((y[j][i+1]-y[j][i])/u[j][i]
		-(y[j][i]-y[j][i-1])/u[j][i-1]);

    /* now solve this tridiagonal matrix */

    for(i=1; i<N-1; i++)
      {
	w[i+1] -= w[i]*u[j][i]/d[i];
	d[i+1] -= u[j][i]*u[j][i]/d[i];
      }

    for(i=0; i<N; i++)p[j][i]=0.0;

    for(i=N-1; i>0; i--)
      p[j][i] = (w[i]-u[j][i]*p[j][i+1])/d[i];  
  }
}

static void mtx_bspline_list(t_mtx_spline *x, t_symbol *s, int argc, t_atom *argv)
{
  /* this should output a matrix, one row for each element of this list */
}
static void mtx_bspline_float(t_mtx_spline *X, t_float f)
{
  int i=0, j=0;
  int dim=X->x_dimension;
  t_matrixfloat *x=X->x_x, **y=X->x_y, **u=X->x_u, **p=X->x_p;
  t_atom*result=X->x_result;

  if(dim<1){
    outlet_float(X->x_outlet, 0.f);
    return;
  }

  if(f<X->x_min)f=X->x_min;
  if(f>X->x_max)f=X->x_max;

  while(f>x[i+1])i++;

  for(j=0; j<dim; j++){
    t_matrixfloat uji=u[j][i];
    t_matrixfloat t=(f-x[i])/uji;
    t_matrixfloat t3=t*t*t-t;
    t_matrixfloat t3i=(1-t)*(1-t)*(1-t)-(1-t);
    t_float ret=(t_float)(t*y[j][i+1]+(1-t)*y[j][i]+uji*uji*(p[j][i+1]*t3+p[j][i]*t3i)/6.0);
    SETFLOAT(result+j, ret);
  }
  outlet_list(X->x_outlet, 0, dim, result);

}
static void mtx_bspline_free(t_mtx_spline *x)
{
  mtx_bspline_resize(x, 0, 0);
}
static void *mtx_bspline_new(void)
{
  t_mtx_spline *x = (t_mtx_spline *)pd_new(mtx_bspline_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));

  x->x_numpoints=0;
  x->x_dimension=0;

  x->x_min=0.0; 
  x->x_max=0.0;

  x->x_x=0;
  x->x_y=x->x_u=x->x_p=0;
  x->x_result=0;

  x->x_outlet=outlet_new(&x->x_obj, 0);
  return(x);
}

void mtx_bspline_setup(void)
{
  mtx_bspline_class = class_new(gensym("mtx_bspline"), (t_newmethod)mtx_bspline_new, (t_method)mtx_bspline_free,
                            sizeof(t_mtx_spline), 0, A_NULL);
  /*  class_addmethod(mtx_bspline_class, (t_method)mtx_bspline_matrix, gensym("list"), A_GIMME, 0); */
  class_addmethod(mtx_bspline_class, (t_method)mtx_bspline_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_bspline_class, mtx_bspline_float);
}

void iemtx_bspline_setup(void)
{
  mtx_bspline_setup();
}
