/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) 2005, Franz Zotter
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "iemmatrix.h"
#include <stdlib.h>

static t_class *mtx_conv_class;

typedef struct _MTXConv_ MTXConv;
struct _MTXConv_
{
   t_object x_obj;
   int size;
   int rows;
   int columns;
   
   int rows_k;
   int columns_k;
   int size_k;
   
   int rows_y;
   int columns_y;
   int size_y;

   t_float **x;
   t_float *x_array;
   t_float **k;
   t_float *k_array;
   t_float **y;
   t_float *y_array;

   t_outlet *list_outlet;
   
   t_atom *list;
};

static void getTFloatMatrix (int rows, int columns, t_float ***mtx, t_float **array)
{
   int size = rows*columns;
   t_float *ptr;
   t_float **dptr;

   if (!size)
      return;
   
   if (*array=ptr=(t_float *)calloc(sizeof(t_float),size)) {
      if (*mtx=dptr=(t_float **)calloc(sizeof(t_float *),rows)) {
	 for(;rows-- ; ptr+=columns) {
	    *dptr++ = ptr;
	 }
      } else {
	 freebytes (*array,sizeof(t_float)*size);
	 array=0;
      }
   }
}

static void deleteTFloatMatrix (int rows, int columns, t_float ***mtx, t_float **array)
{
   int size = rows*columns;

   if (*mtx) 
      freebytes (*mtx, sizeof(t_float*) * columns);
   if (*array)
      freebytes (*array, sizeof(t_float) * size);
   *mtx=0;
   *array=0;
}


static void deleteMTXConv (MTXConv *mtx_conv_obj) 
{
   deleteTFloatMatrix (mtx_conv_obj->rows_k, mtx_conv_obj->columns_k, &mtx_conv_obj->k, &mtx_conv_obj->k_array);
   deleteTFloatMatrix (mtx_conv_obj->rows, mtx_conv_obj->columns, &mtx_conv_obj->x, &mtx_conv_obj->x_array);
   deleteTFloatMatrix (mtx_conv_obj->rows_y, mtx_conv_obj->columns_y, &mtx_conv_obj->y, &mtx_conv_obj->y_array);
   if (mtx_conv_obj->list)
      freebytes (mtx_conv_obj->list, sizeof(t_float) * (mtx_conv_obj->size_y + 2));
         
   mtx_conv_obj->list = 0;
}

static void *newMTXConv (t_symbol *s, int argc, t_atom *argv)
{
   MTXConv *mtx_conv_obj = (MTXConv *) pd_new (mtx_conv_class);
   mtx_conv_obj->list_outlet = outlet_new (&mtx_conv_obj->x_obj, gensym("matrix"));
   inlet_new(&mtx_conv_obj->x_obj, &mtx_conv_obj->x_obj.ob_pd, gensym("matrix"),gensym(""));
   mtx_conv_obj->size = 0;
   mtx_conv_obj->rows = 0;
   mtx_conv_obj->columns = 0;
   mtx_conv_obj->size_y = 0;
   mtx_conv_obj->rows_y = 0;
   mtx_conv_obj->columns_y = 0;
   mtx_conv_obj->size_k = 0;
   mtx_conv_obj->rows_k = 0;
   mtx_conv_obj->columns_k = 0;
   return ((void *) mtx_conv_obj);
} 

static void mTXConvBang (MTXConv *mtx_conv_obj)
{
   if (mtx_conv_obj->list) 
      outlet_anything(mtx_conv_obj->list_outlet, gensym("matrix"), mtx_conv_obj->size+2, mtx_conv_obj->list);
}

static void zeroFloatArray (int n, t_float *f)
{
   while (n--)
      *f++ = 0.0f;
}
static void zeroTFloatMatrix (t_float **mtx, int rows, int columns)
{
   while (rows--)
      zeroFloatArray (columns, *mtx++);
}

static void writeFloatIntoList (int n, t_atom *l, t_float *f) 
{
   for (;n--;f++, l++) 
      SETFLOAT (l, *f);
}
static void readFloatFromList (int n, t_atom *l, t_float *f) 
{
   while (n--) 
      *f++ = atom_getfloat (l++);
}
static void readMatrixFromList (int rows, int columns, t_atom *l, t_float **mtx) 
{
   int n,m;
   for (n=0;n<rows; n++)
      for (m=0;m<columns; m++)
	 mtx[n][m]=atom_getfloat (l++);
}
static void writeMatrixIntoList (int rows, int columns, t_atom *l, t_float **mtx)
{
   int n,m;
   for (n=0;n<rows; n++)
      for (m=0;m<columns; m++, l++)
	 SETFLOAT(l,mtx[n][m]);
}

static void mTXConvKernelMatrix (MTXConv *mtx_conv_obj, t_symbol *s, int argc,
      t_atom *argv)
{
   int rows_k = atom_getint (argv++);
   int columns_k = atom_getint (argv++);
   int in_size = argc-2;
   int size_k = rows_k * columns_k;

   if (!size_k) {
      post ("mtx_conv: invalid matrix dimensions!");
      return;
   }
   if (in_size < size_k) {
      post("mtx_conv: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }

   if ((rows_k != mtx_conv_obj->rows_k) || (columns_k != mtx_conv_obj->columns_k)) {
      if (mtx_conv_obj->k)
	 deleteTFloatMatrix (mtx_conv_obj->rows_k, mtx_conv_obj->columns_k,
	       &mtx_conv_obj->k, &mtx_conv_obj->k_array);
      getTFloatMatrix (rows_k, columns_k, &mtx_conv_obj->k, &mtx_conv_obj->k_array);
      if ((!mtx_conv_obj->k)||(!mtx_conv_obj->k_array)) {
	 post("mtx_conv: memory allocation failed!");
	 return;
      }
      mtx_conv_obj->rows_k = rows_k;
      mtx_conv_obj->columns_k = columns_k;
      mtx_conv_obj->size_k = size_k;

   }

   readMatrixFromList (rows_k, columns_k, argv, mtx_conv_obj->k);
}

static void convolveMtx (int rows, int columns, int rows_h, int columns_h, 
      t_float **x, t_float **h, t_float **y)
{
   int n,m,k,l;
   int rows_y=rows+rows_h-1;
   int cols_y=columns+columns_h-1;
   int n_max, m_max;
   zeroTFloatMatrix (y, rows_y, cols_y);


   for (k=0; k<rows_h; k++) {
      n_max=(rows_y<rows+k)?rows_y:rows+k;
      for (l=0; l<columns_h; l++) {
	 m_max=(cols_y<columns+l)?cols_y:columns+l;
	 for (n=k; n<n_max; n++) 
	    for (m=l; m<m_max; m++) 
	       y[n][m]+=x[n-k][m-l]*h[k][l];
      }
   }
}


static void mTXConvMatrix (MTXConv *mtx_conv_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int rows_k = mtx_conv_obj->rows_k;
   int columns_k = mtx_conv_obj->columns_k;
   int size_k = mtx_conv_obj->size_k;
   int in_size = argc-2;
   int rows_y;
   int columns_y;
   int size_y = mtx_conv_obj->size_y;
   t_atom *list_ptr = mtx_conv_obj->list;

   /* fftsize check */
   if (!size){
      post("mtx_conv: invalid dimensions");
      return;
   }  else if (in_size<size) {
      post("mtx_conv: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }  else if (!size_k) {
      post("mtx_conv: no valid filter kernel defined");
      return;
   }

   if ((mtx_conv_obj->rows != rows)||(mtx_conv_obj->columns != columns)) { 
     if (mtx_conv_obj->x)
       deleteTFloatMatrix (mtx_conv_obj->rows, mtx_conv_obj->columns, 
	     &mtx_conv_obj->x, &mtx_conv_obj->x_array);
     getTFloatMatrix (rows, columns, &mtx_conv_obj->x, &mtx_conv_obj->x_array);
     if ((!mtx_conv_obj->x)||(!mtx_conv_obj->x_array)) {
	post("mtx_conv: memory allocation failed!");
	return;
     }
     mtx_conv_obj->size = size;
     mtx_conv_obj->rows = rows;
     mtx_conv_obj->columns = columns;
   }
   rows_y = rows+rows_k-1;
   columns_y = columns+columns_k-1;
   if ((mtx_conv_obj->rows_y != rows_y)||(mtx_conv_obj->columns_y != columns_y)) { 
     size_y = rows_y * columns_y;      
     if (mtx_conv_obj->y)
        deleteTFloatMatrix (mtx_conv_obj->rows_y, mtx_conv_obj->columns_y,
                               &mtx_conv_obj->y, &mtx_conv_obj->y_array);
     getTFloatMatrix (rows_y, columns_y, &mtx_conv_obj->y, &mtx_conv_obj->y_array);
     if ((!mtx_conv_obj->y)||(!mtx_conv_obj->y_array)) {
	post("mtx_conv: memory allocation failed!");
	return;
     }
     mtx_conv_obj->size_y = size_y;
     mtx_conv_obj->rows_y = rows_y;
     mtx_conv_obj->columns_y = columns_y;

     if (list_ptr)
	list_ptr = (t_atom *) resizebytes (list_ptr, sizeof(t_atom) * (mtx_conv_obj->size_y+2),
                                          sizeof (t_atom) * (size_y+2));
     else
	list_ptr = (t_atom *) getbytes (sizeof (t_atom) * (size_y+2));
     mtx_conv_obj->list = list_ptr;
     if (!list_ptr) {
	post("mtx_conv: memory allocation failed!");
	return;
     }

   }
   /* main part */
   readMatrixFromList (rows, columns, argv, mtx_conv_obj->x); 

   convolveMtx (rows, columns, rows_k, columns_k, 
	 mtx_conv_obj->x, mtx_conv_obj->k, mtx_conv_obj->y);

   writeMatrixIntoList (rows_y, columns_y, list_ptr+2, mtx_conv_obj->y);
   SETSYMBOL(list_ptr, gensym("matrix"));
   SETFLOAT(list_ptr, rows_y);
   SETFLOAT(&list_ptr[1], columns_y);
   outlet_anything(mtx_conv_obj->list_outlet, gensym("matrix"), 
                   size_y+2, list_ptr);
}

void mtx_conv_setup (void)
{
   mtx_conv_class = class_new 
      (gensym("mtx_conv"),
       (t_newmethod) newMTXConv,
       (t_method) deleteMTXConv,
       sizeof (MTXConv),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_conv_class, (t_method) mTXConvBang);
   class_addmethod (mtx_conv_class, (t_method) mTXConvMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_conv_class, (t_method) mTXConvKernelMatrix, gensym(""), A_GIMME,0);

}

void iemtx_conv_setup(void){
  mtx_conv_setup();
}
