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

static t_class *mtx_find_class;
static t_symbol *row_sym;
static t_symbol *col_sym;
static t_symbol *col_sym2;
static t_symbol *mtx_sym;
static t_symbol *mtx_sym2;

typedef struct _MTXfind_ MTXfind;
struct _MTXfind_
{
   t_object x_obj;
   int size;
   int outsize;
   t_symbol *find_mode;
   int find_direction;

   t_outlet *list_outlet;

   t_atom *list_out;
   t_atom *list_in;
};

static void deleteMTXFind (MTXfind *mtx_find_obj) 
{
   if (mtx_find_obj->list_out)
      freebytes (mtx_find_obj->list_out, sizeof(t_atom)*(mtx_find_obj->size+2));
}

static void mTXSetFindDirection (MTXfind *mtx_find_obj, t_float c_dir)
{
   int direction = (int) c_dir;
   if ((direction != -1) && (direction != 1))
      direction = 1;
   mtx_find_obj->find_direction = direction;
}
/*
static void mTXSetFindDimension (MTXfind *mtx_find_obj, t_float c_dim)
{
   int dimension = (int) c_dim;
   dimension = (dimension > 0)?dimension:0;
   dimension = (dimension < 3)?dimension:3;
   mtx_find_obj->find_dimension = dimension;
}
*/
static void mTXSetFindMode (MTXfind *mtx_find_obj, t_symbol *c_dim)
{
   mtx_find_obj->find_mode = c_dim;
}

static void *newMTXFind (t_symbol *s, int argc, t_atom *argv)
{
   MTXfind *mtx_find_obj = (MTXfind *) pd_new (mtx_find_class);

   mTXSetFindMode (mtx_find_obj, gensym(":"));
   mTXSetFindDirection (mtx_find_obj, 1);
   if (argc>=1) {
      if (argv[0].a_type == A_SYMBOL) {
	 mTXSetFindMode (mtx_find_obj, atom_getsymbol (argv));
	 if (argc>=2) {
	    if (argv[1].a_type != A_SYMBOL)
	       mTXSetFindDirection (mtx_find_obj, atom_getfloat (argv+1));
	    else
	       post("mtx_find: 2nd arg ignored. supposed to be float");
	 }
      }
      else {
	 mTXSetFindDirection (mtx_find_obj, atom_getfloat (argv));
	 if (argc>=2) {
	    if (argv[1].a_type == A_SYMBOL)
	       mTXSetFindMode (mtx_find_obj, atom_getsymbol (argv+1));
	    else
	       post("mtx_find: 2nd arg ignored. supposed to be symbolic, e.g. \"row\", \"col\", \":\", \"mtx\"");
	 }
      }
   }
/*
   switch ((argc>2)?2:argc) {
      case 2:
	 c_dir = atom_getint(argv+1);
      case 1:
	 c_dim = atom_getint(argv);
   }
   mTXSetFindDimension (mtx_find_obj, (t_float) c_dim);
   mTXSetFindDirection (mtx_find_obj, (t_float) c_dir);
   */

   mtx_find_obj->list_outlet = outlet_new (&mtx_find_obj->x_obj, gensym("matrix"));

   error("[mtx_find]: this object is likely to change! not really for use yet");
   return ((void *) mtx_find_obj);
} 

static void mTXFindBang (MTXfind *mtx_find_obj)
{
   if (mtx_find_obj->list_out) 
      outlet_anything(mtx_find_obj->list_outlet, gensym("matrix"), 
	    mtx_find_obj->outsize+2, mtx_find_obj->list_out);
}
/*
static void copyList (int size, t_atom *x, t_atom *y)
{
   while(size--)
 *y++=*x++;
 }
 */
static int findPreviousNonZero (const int n, t_atom *x, int offset)
{
   x+=offset;
   for (; offset > n; offset--, x--) 
      if (atom_getfloat(x))
	 return offset;
   return -1;
}
static int findPreviousNonZeroStep (const int step, t_atom *x, int offset)
{
   x += offset;
   for (; offset >= 0; offset-=step, x-=step) 
      if (atom_getfloat(x))
	 return offset;
   return -1;
}
static int findNextNonZero (const int n, t_atom *x, int offset)
{
   x+=offset;
   for (; offset < n; offset++, x++) 
      if (atom_getfloat(x))
	 return offset;
   return -1;
}
static int findNextNonZeroStep (const int n, const int step, t_atom *x, int offset)
{
   x += offset;
   for (; offset < n; offset+=step, x+=step) 
      if (atom_getfloat(x))
	 return offset;
   return -1;
}

static void findFirstNonZeroRow (const int rows, const int columns, t_atom *x, t_atom *y)
{
   int offset;
   int pos;
   const int size = rows*columns;
   for (offset = 0; offset < size; y++, offset+=columns) {
      pos = findNextNonZero(offset+columns,x,offset)+1;
      SETFLOAT(y,(t_float)pos);
   }
}
static void findLastNonZeroRow (const int rows, const int columns, t_atom *x, t_atom *y)
{
   int offset;
   int pos;
   const int size = rows*columns;
   for (offset = columns-1; offset < size; y++, offset+=columns) {
      pos = findPreviousNonZero(offset-columns,x,offset)+1;
      SETFLOAT(y,(t_float)pos);
   }
}
static void findFirstNonZeroColumn (const int rows, const int columns, t_atom *x, t_atom *y)
{
   int offset;
   int pos;
   const int size = rows*columns;
   for (offset = 0; offset < columns; y++, offset++) {
      pos = findNextNonZeroStep(size,columns,x,offset)+1;
      SETFLOAT(y,(t_float)pos);
   }
}
static void findLastNonZeroColumn (const int rows, const int columns, t_atom *x, t_atom *y)
{
   int offset;
   int pos;
   const int size = rows*columns;
   for (offset = size-columns; offset < size; y++, offset++) {
      pos = findPreviousNonZeroStep(columns,x,offset)+1;
      SETFLOAT(y,(t_float)pos);
   }
}

static int findAllNonZeros (int n, t_atom *x, t_atom *y)
{
   int outsize = 0;
   int pos = 0;
   while ((pos = findNextNonZero(n,x,pos)) != -1) {
      pos++;
      SETFLOAT(y,(t_float)pos);
      y++;
      outsize++;
   }
   return outsize;
}

static void zeroFloatList (int n, t_atom *x)
{
   for (;n--;x++)
      SETFLOAT(x,0);
}
static void findReplaceNonZerosWithIndex (int n, t_atom *x, t_atom *y)
{
   int pos = 0;
   zeroFloatList(n,y);
   while ((pos = findNextNonZero(n,x,pos)) != -1) {
      SETFLOAT(y+pos,(t_float)pos+1);
      pos++;
   }
}

static void mTXFindMatrix (MTXfind *mtx_find_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_in = argv;
   t_atom *list_out = mtx_find_obj->list_out;
   int rows_out;
   int columns_out;

   /* size check */
   if (!size) {
      post("mtx_find: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_find: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   
   if (size != mtx_find_obj->size) {
      if (!list_out)
	 list_out = (t_atom *) getbytes (sizeof (t_atom) * (size + 2));
      else
	 list_out = (t_atom *) resizebytes (list_out,
	       sizeof (t_atom) * (mtx_find_obj->size+2),
	       sizeof (t_atom) * (size + 2));
   }

   mtx_find_obj->size = size;
   mtx_find_obj->list_out = list_out;

   /* main part */
   list_out += 2;
   rows_out = 1;
   if (mtx_find_obj->find_mode == row_sym) {
      if (mtx_find_obj->find_direction == -1)
	 findLastNonZeroRow (rows, columns, list_in, list_out);
      else
	 findFirstNonZeroRow (rows, columns, list_in, list_out);
      rows_out = rows;
      columns_out = 1;
   }
   else if ((mtx_find_obj->find_mode == col_sym)||
	 (mtx_find_obj->find_mode == col_sym2)) {
      if (mtx_find_obj->find_direction == -1)
	 findLastNonZeroColumn (rows, columns, list_in, list_out);
      else
	 findFirstNonZeroColumn (rows, columns, list_in, list_out);
      columns_out = columns;
      rows_out = 1;
   }
   else if ((mtx_find_obj->find_mode == mtx_sym)||
	 (mtx_find_obj->find_mode == mtx_sym2)) {
      findReplaceNonZerosWithIndex (size, list_in, list_out);
      rows_out = rows;
      columns_out = columns;
   }
   else {
      columns_out = findAllNonZeros (size, list_in, list_out); 
      rows_out = 1;
   }
   /*
   switch (mtx_find_obj->find_dimension) {
      case 0:
	 columns_out = findAllNonZeros (size, list_in, list_out); 
	 rows_out = 1;
	 break;
      case 3:
	 findReplaceNonZerosWithIndex (size, list_in, list_out);
	 rows_out = rows;
	 columns_out = columns;
	 break;
      case 2:
	 if (mtx_find_obj->find_direction == -1)
	    findLastNonZeroColumn (rows, columns, list_in, list_out);
	 else
	    findFirstNonZeroColumn (rows, columns, list_in, list_out);
	 columns_out = columns;
	 rows_out = 1;
	 break;
      case 1:
	 if (mtx_find_obj->find_direction == -1)
	    findLastNonZeroRow (rows, columns, list_in, list_out);
	 else
	    findFirstNonZeroRow (rows, columns, list_in, list_out);
	 rows_out = rows;
	 columns_out = 1;
	 break;
   }
   */
   mtx_find_obj->outsize = columns_out * rows_out;
   list_out = mtx_find_obj->list_out;

   SETSYMBOL(list_out, gensym("matrix"));
   SETFLOAT(list_out, rows_out);
   SETFLOAT(&list_out[1], columns_out);
   outlet_anything(mtx_find_obj->list_outlet, gensym("matrix"), 
	 mtx_find_obj->outsize+2, list_out);
}

void mtx_find_setup (void)
{
   mtx_find_class = class_new 
      (gensym("mtx_find"),
       (t_newmethod) newMTXFind,
       (t_method) deleteMTXFind,
       sizeof (MTXfind),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_find_class, (t_method) mTXFindBang);
   class_addmethod (mtx_find_class, (t_method) mTXFindMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_find_class, (t_method) mTXSetFindMode, gensym("mode"), A_DEFSYMBOL,0);
   class_addmethod (mtx_find_class, (t_method) mTXSetFindDirection, gensym("direction"), A_DEFFLOAT,0);

   row_sym = gensym("row");
   col_sym = gensym("col");
   col_sym2 = gensym("columns");
   mtx_sym = gensym("mtx");
   mtx_sym2 = gensym ("matrix");
}

void iemtx_find_setup(void){
  mtx_find_setup();
}
