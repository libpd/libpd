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

static t_class *mtx_sort_class;
static t_symbol *row_sym;
static t_symbol *col_sym;
static t_symbol *col_sym2;

typedef struct _MTXSort_ MTXSort;
struct _MTXSort_
{
   t_object x_obj;
   int rows;
   int columns;
   int size;
   t_symbol *sort_mode;
   int sort_direction;

   t_outlet *list_outlet1;
   t_outlet *list_outlet2;

   t_atom *list_out1;
   t_atom *list_out2;
   t_atom *list_in;
   t_float *x;
   t_float *i;
};

static void deleteMTXSort (MTXSort *mtx_sort_obj) 
{
   if (mtx_sort_obj->list_out1)
      freebytes (mtx_sort_obj->list_out1, sizeof(t_atom)*(mtx_sort_obj->size+2));
   if (mtx_sort_obj->list_out2)
      freebytes (mtx_sort_obj->list_out2, sizeof(t_atom)*(mtx_sort_obj->size+2));
   if (mtx_sort_obj->x)
      freebytes (mtx_sort_obj->x, sizeof(t_float)*(mtx_sort_obj->size));
   /*
     if (mtx_sort_obj->y)
       freebytes (mtx_sort_obj->y, sizeof(t_float)*(mtx_sort_obj->size));
   */
   if (mtx_sort_obj->i)
      freebytes (mtx_sort_obj->i, sizeof(t_float)*(mtx_sort_obj->size));
}

static void mTXSetSortDirection (MTXSort *mtx_sort_obj, t_float s_dir)
{
   int direction = (int) s_dir;
   mtx_sort_obj->sort_direction = (direction==-1)?direction:1;
}

static void mTXSetSortMode (MTXSort *mtx_sort_obj, t_symbol *m_sym)
{
   mtx_sort_obj->sort_mode = m_sym;
}

static void *newMTXSort (t_symbol *s, int argc, t_atom *argv)
{
   MTXSort *mtx_sort_obj = (MTXSort *) pd_new (mtx_sort_class);
   
   /* defaults: */
   mTXSetSortMode (mtx_sort_obj, gensym(":"));
   mTXSetSortDirection (mtx_sort_obj, 1.0f);
   if (argc>=1) {
      if (argv[0].a_type == A_SYMBOL) {
	 mTXSetSortMode (mtx_sort_obj, atom_getsymbol (argv));
	 if (argc>=2) {
	    if (argv[1].a_type != A_SYMBOL)
	       mTXSetSortDirection (mtx_sort_obj, atom_getfloat (argv+1));
	    else
	       post("mtx_sort: 2nd arg ignored. supposed to be float");
	 }
      }
      else {
	 mTXSetSortDirection (mtx_sort_obj, atom_getfloat (argv));
	 if (argc>=2) {
	    if (argv[1].a_type == A_SYMBOL)
	       mTXSetSortMode (mtx_sort_obj, atom_getsymbol (argv+1));
	    else
	       post("mtx_sort: 2nd arg ignored. supposed to be symbolic, e.g. \"row\", \"col\", \":\"");
	 }
      }
   }

   mtx_sort_obj->list_outlet1 = outlet_new (&mtx_sort_obj->x_obj, gensym("matrix"));
   mtx_sort_obj->list_outlet2 = outlet_new (&mtx_sort_obj->x_obj, gensym("matrix"));
   return ((void *) mtx_sort_obj);
} 

static void mTXSortBang (MTXSort *mtx_sort_obj)
{
   if (mtx_sort_obj->list_out2) 
      outlet_anything(mtx_sort_obj->list_outlet2, gensym("matrix"), 
	    mtx_sort_obj->size+2, mtx_sort_obj->list_out2);
   if (mtx_sort_obj->list_out1) 
      outlet_anything(mtx_sort_obj->list_outlet1, gensym("matrix"), 
	    mtx_sort_obj->size+2, mtx_sort_obj->list_out1);
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
static void readFloatFromListModulo (int n, int m, t_atom *l, t_float *f) 
{
   t_atom *ptr = l;
   int count1, count2;
   n /= m;
   count1 = m;
   while (count1--) 
      for (count2 = n, ptr = l++; count2--; ptr += m, f++) 
	 *f = atom_getfloat (ptr);
}
static void writeFloatIntoListModulo (int n, int m, t_atom *l, t_float *f) 
{
   t_atom *ptr = l;
   int count1, count2;
   n /= m;
   count1 = m;
   while (count1--) 
      for (count2 = n, ptr = l++; count2--; ptr += m, f++) 
	 SETFLOAT(ptr,*f);
}
static void sortVector (int n, t_float *x, t_float *i, int direction)
{
   int step = n;
   int size = n;
   int k, loops = 1;
   int i_tmp;
   t_float x_tmp;

   switch (direction) {
      case -1:
	 while (step > 1) {
	    step = (step % 2)?(step+1)/2:step/2;
	    k = loops;
	    loops += 2;
	    while(k--) { /* there might be some optimization in here */
	       for (n=0; n<(size-step); n++) 
		  if (x[n] < x[n+step]) {
		     i_tmp = i[n];
		     x_tmp = x[n];
		     x[n]        = x[n+step];
		     x[n+step]   = x_tmp;
		     i[n]        = i[n+step];
		     i[n+step]   = i_tmp;
		  }
	    }
	 }
	 break;
      default:
      case 1:
	 while (step > 1) {
	    step = (step % 2)?(step+1)/2:step/2;
	    k = loops;
	    loops += 2;
	    while(k--) { /* there might be some optimization in here */
	       for (n=0; n<(size-step); n++) 
		  if (x[n] > x[n+step]) {
		     i_tmp = i[n];
		     x_tmp = x[n];
		     x[n]        = x[n+step];
		     x[n+step]   = x_tmp;
		     i[n]        = i[n+step];
		     i[n+step]   = i_tmp;
		  }
	    }
	 }
   }
}

/*
static void indexingVector (int n, int m, int dimension, t_float *i)
{
   int count;
   int count2;
   int idx = n;
   t_float *ptr;
   i += n;
   switch (dimension) {
      case 2:
	 n /= m;
	 for (count = m; count--;) {
	    ptr = --i;
	    for (count2 = n; count2--; ptr -= m) 
	       *ptr = idx--;
	 }
	 break;
      default:
      case 1:
	 for (; idx;)
	    *--i = idx--;
   }
}
*/
static void indexingVector (int n, int m, t_symbol *sort_mode, t_float *i)
{
   int count;
   int count2;
   int idx = n;
   t_float *ptr;
   i += n;
   if ((sort_mode == col_sym)||(sort_mode == col_sym2)) {
	 n /= m;
	 for (count = m; count--;) {
	    ptr = --i;
	    for (count2 = n; count2--; ptr -= m) 
	       *ptr = idx--;
	 }
   }
   else {
	 for (; idx;)
	    *--i = idx--;
   }
}


static void mTXSortMatrix (MTXSort *mtx_sort_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_ptr = argv;
   t_atom *list_out1 = mtx_sort_obj->list_out1;
   t_atom *list_out2 = mtx_sort_obj->list_out2;
   t_float *x = mtx_sort_obj->x;
   t_float *i = mtx_sort_obj->i;
   int count;

   /* size check */
   if (!size) {
      post("mtx_sort: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_sort: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   else if ((!x)||(!list_out1)||(!list_out2)/*||(!y)*/) {
      if (!x)
	 x = (t_float *) getbytes (sizeof (t_float) * (size));
      /*if (!y)
       *   y = (t_float *) getbytes (sizeof (t_float) * (size));
       */
      if (!i)
	 i = (t_float *) getbytes (sizeof (t_float) * (size));
      if (!list_out1)
	 list_out1 = (t_atom *) getbytes (sizeof (t_atom) * (size+2));
      if (!list_out2)
	 list_out2 = (t_atom *) getbytes (sizeof (t_atom) * (size+2));
   }
   else if (size != mtx_sort_obj->size) {
      x = (t_float *) resizebytes (x,
                                   sizeof (t_float) * (mtx_sort_obj->size),
                                   sizeof (t_float) * (size));
      i = (t_float *) resizebytes (i,
                                   sizeof (t_float) * (mtx_sort_obj->size),
                                   sizeof (t_float) * (size));
      list_out1 = (t_atom *) resizebytes (list_out1,
                                          sizeof (t_atom) * (mtx_sort_obj->size+2),
                                          sizeof (t_atom) * (size + 2));
      list_out2 = (t_atom *) resizebytes (list_out2,
                                          sizeof (t_atom) * (mtx_sort_obj->size+2),
                                          sizeof (t_atom) * (size + 2));
   }

   mtx_sort_obj->list_out1 = list_out1;
   mtx_sort_obj->list_out2 = list_out2;
   mtx_sort_obj->x = x;
   mtx_sort_obj->i = i;
   mtx_sort_obj->size = size;
   mtx_sort_obj->rows = rows;
   mtx_sort_obj->columns = columns;

   /* generating indexing vector */
   indexingVector (size, columns, mtx_sort_obj->sort_mode, i);

   /* main part */
   /* reading matrix from inlet */
   if ((mtx_sort_obj->sort_mode == col_sym)||
	(mtx_sort_obj->sort_mode == col_sym2)) {
      readFloatFromListModulo (size, columns, list_ptr, x);
      columns = mtx_sort_obj->rows;
      rows = mtx_sort_obj->columns;
   }
   else
      readFloatFromList (size, list_ptr, x);
   
   /* calculating sort */
   if ((mtx_sort_obj->sort_mode != col_sym) &&
	 (mtx_sort_obj->sort_mode != col_sym2) &&
	 (mtx_sort_obj->sort_mode != row_sym))
      sortVector (size,x,i,mtx_sort_obj->sort_direction);
   else
      for (count = rows; count--;x+=columns,i+=columns)
	 sortVector (columns,x,i,mtx_sort_obj->sort_direction);
   x = mtx_sort_obj->x;
   i = mtx_sort_obj->i;

   /* writing matrix to outlet */
   if ((mtx_sort_obj->sort_mode == col_sym)||
	 (mtx_sort_obj->sort_mode == col_sym2)) {
      columns = mtx_sort_obj->columns;
      rows = mtx_sort_obj->rows;
      writeFloatIntoListModulo (size, columns, list_out1+2, x);
      writeFloatIntoListModulo (size, columns, list_out2+2, i);
   }
   else {
      writeFloatIntoList (size, list_out1+2, x);
      writeFloatIntoList (size, list_out2+2, i);
   }

   /* writing indices */
   SETSYMBOL(list_out2, gensym("matrix"));
   SETFLOAT(list_out2, rows);
   SETFLOAT(&list_out2[1], columns);
   outlet_anything(mtx_sort_obj->list_outlet2, gensym("matrix"), 
	 mtx_sort_obj->size+2, list_out2);
   /* writing sorted values */
   SETSYMBOL(list_out1, gensym("matrix"));
   SETFLOAT(list_out1, rows);
   SETFLOAT(&list_out1[1], columns);
   outlet_anything(mtx_sort_obj->list_outlet1, gensym("matrix"), 
	 mtx_sort_obj->size+2, list_out1);
}

void mtx_sort_setup (void)
{
   mtx_sort_class = class_new 
      (gensym("mtx_sort"),
       (t_newmethod) newMTXSort,
       (t_method) deleteMTXSort,
       sizeof (MTXSort),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_sort_class, (t_method) mTXSortBang);
   class_addmethod (mtx_sort_class, (t_method) mTXSortMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_sort_class, (t_method) mTXSetSortMode, gensym("mode"), A_DEFSYMBOL,0);
   class_addmethod (mtx_sort_class, (t_method) mTXSetSortDirection, gensym("direction"), A_DEFFLOAT,0);

   row_sym = gensym("row");
   col_sym = gensym("col");
   col_sym2 = gensym("column");
}

void iemtx_sort_setup(void){
  mtx_sort_setup();
}
