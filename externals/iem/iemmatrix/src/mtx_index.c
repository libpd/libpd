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

static t_class *mtx_index_class;

typedef struct _MTXindex_ MTXindex;
struct _MTXindex_
{
   t_object x_obj;
   int index_size;
   int index_rows;
   int index_columns;
   t_float fill_value;
   int max_index;
   int *index_in;

   t_outlet *list_outlet;

   t_atom *list_out;
   t_atom *list_in;
};

static void deleteMTXIndex (MTXindex *mtx_index_obj) 
{
   if (mtx_index_obj->index_in)
      freebytes (mtx_index_obj->index_in, sizeof(int)*(mtx_index_obj->index_size+2));
   if (mtx_index_obj->list_out)
      freebytes (mtx_index_obj->list_out, sizeof(t_atom)*(mtx_index_obj->index_size+2));
}

static void *newMTXIndex (t_symbol *s, int argc, t_atom *argv)
{
   MTXindex *mtx_index_obj = (MTXindex *) pd_new (mtx_index_class);
   t_atom fill_atom;

   SETFLOAT(&fill_atom,0);
   switch ((argc>1)?1:argc) {
      case 1:
	 fill_atom = *argv;
   }
   if (atom_getsymbol(&fill_atom) == gensym("nan"))
#ifdef __WIN32__
      mtx_index_obj->fill_value = 0.0f;
#else
      mtx_index_obj->fill_value = 0.0f/0.0f;
#endif
   else 
      mtx_index_obj->fill_value = atom_getfloat(&fill_atom);
   
   mtx_index_obj->list_outlet = outlet_new (&mtx_index_obj->x_obj, gensym("matrix"));
   inlet_new(&mtx_index_obj->x_obj, &mtx_index_obj->x_obj.ob_pd, gensym("matrix"),gensym(""));

   error("[mtx_index]: this object is likely to change! not really for use yet");

   return ((void *) mtx_index_obj);
} 

static void mTXIndexBang (MTXindex *mtx_index_obj)
{
   if (mtx_index_obj->list_out) 
      outlet_anything(mtx_index_obj->list_outlet, gensym("matrix"), 
	    mtx_index_obj->index_size+2, mtx_index_obj->list_out);
}
/*
   static void copyList (int size, t_atom *x, t_atom *y)
   {
   while(size--)
 *y++=*x++;
 }
 */

static int copyAtomToIntegerArrayMax (int n, t_atom *x, int *y)
{
   int max = atom_getint(x);
   for (;n--;x++,y++) {
      *y = atom_getint (x);
      max = (*y > max)?*y:max;
   }
   return max;
}

static void setAtomListConstFloat (int n, t_atom *x, t_float f)
{
   for (;n--;x++)
      SETFLOAT(x,f);
}

static void writeIndexedValuesIntoList (int n, int *indx, t_atom *x, t_atom *y)
{
   for (;n--;indx++,y++)
      if (*indx)
	 *y = x[*indx-1];
}

static void mTXIndexRightMatrix (MTXindex *mtx_index_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_in = argv;
   t_atom *list_out = mtx_index_obj->list_out;
   int *index_in = mtx_index_obj->index_in;
   int max;

   /* size check */
   if (!size) {
      post("mtx_index: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_index: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   
   if (size != mtx_index_obj->index_size) {
      if (!index_in)
	 index_in = (int *) getbytes (sizeof (int) * (size + 2));
      else
	 index_in = (int *) resizebytes (index_in,
	       sizeof (int) * (mtx_index_obj->index_size+2),
	       sizeof (int) * (size + 2));
      if (!list_out)
	 list_out = (t_atom *) getbytes (sizeof (t_atom) * (size + 2));
      else
	 list_out = (t_atom *) resizebytes (list_out,
	       sizeof (t_atom) * (mtx_index_obj->index_size+2),
	       sizeof (t_atom) * (size + 2));
   }

   mtx_index_obj->index_size = size;
   mtx_index_obj->index_columns = columns;
   mtx_index_obj->index_rows = rows;
   mtx_index_obj->list_out = list_out;
   mtx_index_obj->index_in = index_in;

   max = copyAtomToIntegerArrayMax (size, list_in, index_in);
   mtx_index_obj->max_index = max;

}

static void mTXIndexMatrix (MTXindex *mtx_index_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_in = argv;
   t_atom *list_out = mtx_index_obj->list_out;
   int index_rows = mtx_index_obj->index_rows;
   int index_columns = mtx_index_obj->index_columns;
   int *indx = mtx_index_obj->index_in;

   /* size check */
   if (!size) {
      post("mtx_index: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_index: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   
   if (size < mtx_index_obj->max_index) {
      post("mtx_index: index exceeds matrix dimensions");
      return;
   }
   if ((!indx)||(mtx_index_obj->index_size == 0)) {
      post("mtx_index: index with what? no right matrix defined");
      return;
   }
   /* main part */
   list_out += 2;
   setAtomListConstFloat (mtx_index_obj->index_size, list_out, mtx_index_obj->fill_value);
   writeIndexedValuesIntoList (mtx_index_obj->index_size, indx,list_in,list_out);
   list_out = mtx_index_obj->list_out;
   SETSYMBOL(list_out, gensym("matrix"));
   SETFLOAT(list_out, index_rows);
   SETFLOAT(&list_out[1], index_columns);
   outlet_anything(mtx_index_obj->list_outlet, gensym("matrix"), 
	 mtx_index_obj->index_size+2, list_out);
}

void mtx_index_setup (void)
{
   mtx_index_class = class_new 
      (gensym("mtx_index"),
       (t_newmethod) newMTXIndex,
       (t_method) deleteMTXIndex,
       sizeof (MTXindex),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_index_class, (t_method) mTXIndexBang);
   class_addmethod (mtx_index_class, (t_method) mTXIndexMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_index_class, (t_method) mTXIndexRightMatrix, gensym(""), A_GIMME,0);
}

void iemtx_index_setup(void){
  mtx_index_setup();
}
