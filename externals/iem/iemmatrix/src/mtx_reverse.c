/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) 2005, Franz Zotter
 * Copyright (c) 2006, IOhannes m zmölnig
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */


#include "iemmatrix.h"

static t_class *mtx_reverse_class;

typedef struct _MTXreverse_ MTXreverse;
struct _MTXreverse_
{
  t_object x_obj;
  int size;
  int reverse_mode; /* 0=col; 1=row */

  t_outlet *list_outlet;
  t_atom *list_out;
  t_atom *list_in;
};

static void deleteMTXreverse (MTXreverse *mtx_reverse_obj) 
{
   if (mtx_reverse_obj->list_out)
      freebytes (mtx_reverse_obj->list_out, sizeof(t_atom)*(mtx_reverse_obj->size+2));
}
static void mTXSetReverseMode (MTXreverse *mtx_reverse_obj, t_symbol *c_mode)
{
  char c=*c_mode->s_name;
  switch(c){
  case 'c': case 'C': case ':': /* "column" */
    mtx_reverse_obj->reverse_mode = 1;
    break;
  case 'r': case 'R': /* "row" */
    mtx_reverse_obj->reverse_mode = 0;
    break;
  case 'e': case 'E': case '.': /* "element" just revert the whole matrix as if it was a list */
    mtx_reverse_obj->reverse_mode = -1;
    break;
  default:
    error("mtx_reverse: invalid mode '%s'", c_mode->s_name);
    break;
  }
}

static void *newMTXreverse (t_symbol *s, int argc, t_atom *argv)
{
   MTXreverse *mtx_reverse_obj = (MTXreverse *) pd_new (mtx_reverse_class);
   if(argc&&(A_SYMBOL==argv->a_type))
     mTXSetReverseMode (mtx_reverse_obj, atom_getsymbol (argv));
   else
     mTXSetReverseMode (mtx_reverse_obj, gensym(":"));

   mtx_reverse_obj->list_outlet = outlet_new (&mtx_reverse_obj->x_obj, gensym("matrix"));
   return ((void *) mtx_reverse_obj);
} 

static void mTXreverseBang (MTXreverse *mtx_reverse_obj)
{
   if (mtx_reverse_obj->list_out) 
      outlet_anything(mtx_reverse_obj->list_outlet, gensym("matrix"), 
	    mtx_reverse_obj->size+2, mtx_reverse_obj->list_out);
}

static void copyList (int n, t_atom *x, t_atom *y)
{
   for (;n--;)  
      *y++ = *x++;
}
static void reverseList (int n, t_atom *y)
{
   t_atom *read = y+n-1;
   t_atom tmp;
   n >>= 1;
   while(n-->0) { 
      tmp = *y;
      *y++ = *read;
      *read-- = tmp;
   }
}
static void reverseListStep (int n, int step, t_atom *y)
{
   t_atom *read = y;
   t_atom tmp;
   n /= step;
   y += (n-1) * step;
   n >>= 1;
   for (;n--; y-=step, read+=step) { 
      tmp = *y;
      *y = *read;
      *read = tmp;
   }
}

static void mTXreverseMatrix (MTXreverse *mtx_reverse_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_out = mtx_reverse_obj->list_out;
   int count;

   /* size check */
   if (!size) {
      error("mtx_reverse: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      error("mtx_reverse: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   
   if (size != mtx_reverse_obj->size) {
      if (!list_out)
	 list_out = (t_atom *) getbytes (sizeof (t_atom) * (size + 2));
      else
	 list_out = (t_atom *) resizebytes (list_out,
	       sizeof (t_atom) * (mtx_reverse_obj->size+2),
	       sizeof (t_atom) * (size + 2));
   }
   
   mtx_reverse_obj->size = size;
   mtx_reverse_obj->list_out = list_out;
   
   /* main part */
   list_out += 2;
   copyList (size, argv, list_out);

   if ((mtx_reverse_obj->reverse_mode == 0)) {
     for (count = columns; count--; list_out++)
       reverseListStep (size, columns, list_out);
   }
   else if (mtx_reverse_obj->reverse_mode == 1) {
     for (count = rows; count--; list_out += columns) 
       reverseList (columns, list_out);
   }
   else 
     reverseList (size, list_out); 
   
   list_out = mtx_reverse_obj->list_out;

   SETSYMBOL(list_out, gensym("matrix"));
   SETFLOAT(list_out, rows);
   SETFLOAT(&list_out[1], columns);
   outlet_anything(mtx_reverse_obj->list_outlet, gensym("matrix"), 
	 mtx_reverse_obj->size+2, list_out);
}

void mtx_reverse_setup (void)
{
   mtx_reverse_class = class_new 
      (gensym("mtx_reverse"),
       (t_newmethod) newMTXreverse,
       (t_method) deleteMTXreverse,
       sizeof (MTXreverse),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_reverse_class, (t_method) mTXreverseBang);
   class_addmethod (mtx_reverse_class, (t_method) mTXreverseMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_reverse_class, (t_method) mTXSetReverseMode, gensym("mode"), A_DEFSYMBOL,0);
}

void iemtx_reverse_setup(void){
  mtx_reverse_setup();
}
