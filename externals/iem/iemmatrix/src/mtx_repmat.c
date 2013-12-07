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

static t_class *mtx_repmat_class;

typedef struct _MTXrepmat_ MTXrepmat;
struct _MTXrepmat_
{
   t_object x_obj;
   int size;
   int repeat_rows;
   int repeat_cols;

   t_outlet *list_outlet;

   t_atom *list_out;
};

static void deleteMTXRepmat (MTXrepmat *mtx_repmat_obj) 
{
   if (mtx_repmat_obj->list_out)
      freebytes (mtx_repmat_obj->list_out, sizeof(t_atom)*(mtx_repmat_obj->size+2));
}
static void mTXRepmatList (MTXrepmat *mtx_repmat_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   if (argc!=2) {
      post("mtx_repmat: there have to be exactly 2 arguments");
      return;
   }
   mtx_repmat_obj->repeat_rows = atom_getint(argv++);
   mtx_repmat_obj->repeat_cols = atom_getint(argv);
}


static void *newMTXRepmat (t_symbol *s, int argc, t_atom *argv)
{
   MTXrepmat *mtx_repmat_obj = (MTXrepmat *) pd_new (mtx_repmat_class);
   mtx_repmat_obj->repeat_cols = 1;
   mtx_repmat_obj->repeat_rows = 1;

   if (argc) 
      mTXRepmatList (mtx_repmat_obj, gensym("list"), argc, argv);
  
   mtx_repmat_obj->list_outlet = outlet_new (&mtx_repmat_obj->x_obj, gensym("matrix"));
   inlet_new(&mtx_repmat_obj->x_obj, &mtx_repmat_obj->x_obj.ob_pd, gensym("list"),gensym(""));
   return ((void *) mtx_repmat_obj);
} 

static void mTXRepmatBang (MTXrepmat *mtx_repmat_obj)
{
   if (mtx_repmat_obj->list_out) 
      outlet_anything(mtx_repmat_obj->list_outlet, gensym("matrix"), 
	    mtx_repmat_obj->size+2, mtx_repmat_obj->list_out);
}

static void copyList (int n, t_atom *x, t_atom *y)
{
   while (n--)
      *y++=*x++;
}

static void writeRepeatIntoMatrix (int repeat_rows, int repeat_cols, int rows, int columns, t_atom *x, t_atom *y)
{
   int row_cnt;
   int col_cnt;
   int new_col = columns * repeat_cols;
   t_atom *ptr = y;

   /* writing each row repeatedly (repeat_col times) into output array */
   /* so that : row1#1 row1#2 ... row1#RN | ... | rowN#1 rowN#2 ... rowN#RN */
   for (row_cnt=rows;row_cnt--;x+=columns) 
      for(col_cnt=repeat_cols;col_cnt--;ptr+=columns) 
	 copyList (columns, x, ptr);

   /* repeating the above written long lines repeat row_repeat times in output array */
   for (;--repeat_rows;) 
      for (row_cnt=rows;row_cnt--;y+=new_col,ptr+=new_col) 
	 copyList (new_col, y, ptr);
   
}
static void mTXRepmatMatrix (MTXrepmat *mtx_repmat_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int rep_rows = mtx_repmat_obj->repeat_rows;
   int rep_cols = mtx_repmat_obj->repeat_cols;
   int mrows = rows * rep_rows;
   int mcolumns = columns * rep_cols;
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_in = argv;
   t_atom *list_out = mtx_repmat_obj->list_out;

   /* size check */
   if (!size) {
      post("mtx_repmat: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_repmat: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }

   mrows = rows * rep_rows;
   mcolumns = columns * rep_cols;
   size = mrows * mcolumns;

   if (size != mtx_repmat_obj->size) {
      if (list_out) 
	 list_out = (t_atom*) resizebytes (list_out,
	       sizeof(t_atom)*(mtx_repmat_obj->size+2),
	       sizeof(t_atom)*(size+2));
      else
	 list_out = (t_atom*) getbytes (sizeof(t_atom)*(size+2));
      mtx_repmat_obj->list_out = list_out;
      mtx_repmat_obj->size = size;
   }
   /* main part */
   
   writeRepeatIntoMatrix (rep_rows, rep_cols, rows, columns,
	 list_in, list_out+2);
   SETFLOAT(list_out, mrows);
   SETFLOAT(&list_out[1], mcolumns);

   mTXRepmatBang (mtx_repmat_obj);
}

void mtx_repmat_setup (void)
{
   mtx_repmat_class = class_new 
      (gensym("mtx_repmat"),
       (t_newmethod) newMTXRepmat,
       (t_method) deleteMTXRepmat,
       sizeof (MTXrepmat),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_repmat_class, (t_method) mTXRepmatBang);
   class_addmethod (mtx_repmat_class, (t_method) mTXRepmatMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_repmat_class, (t_method) mTXRepmatList, gensym(""), A_GIMME,0);

}

void iemtx_repmat_setup(void){
  mtx_repmat_setup();
}
