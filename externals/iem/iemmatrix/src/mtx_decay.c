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

static t_class *mtx_decay_class;
static t_symbol *row_sym;
static t_symbol *col_sym;
static t_symbol *col_sym2;


typedef struct _MTXDecay_ MTXDecay;
struct _MTXDecay_
{
   t_object x_obj;
   int rows;
   int columns;
   int size;
   int decay_direction;
   t_symbol *decay_mode;
   t_float decay_parameter;

   t_outlet *list_outlet;

   t_atom *list_out;
   t_atom *list_in;
   t_float *x;
   t_float *y;
};

static void deleteMTXDecay (MTXDecay *mtx_decay_obj) 
{
   if (mtx_decay_obj->list_out)
      freebytes (mtx_decay_obj->list_out, sizeof(t_atom)*(mtx_decay_obj->size+2));
   if (mtx_decay_obj->x)
      freebytes (mtx_decay_obj->x, sizeof(t_float)*(mtx_decay_obj->size));
   if (mtx_decay_obj->y)
      freebytes (mtx_decay_obj->y, sizeof(t_float)*(mtx_decay_obj->size));
}

static void mTXSetDecayParameter (MTXDecay *mtx_decay_obj, t_float d_param)
{
   d_param = (d_param > 0.0f)?d_param:0.0f;
   d_param = (d_param < 1.0f)?d_param:1.0f;
   mtx_decay_obj->decay_parameter = d_param;
}
static void mTXSetDecayDirection (MTXDecay *mtx_decay_obj, t_float c_dir)
{
   int direction = (int) c_dir;
   mtx_decay_obj->decay_direction = (direction==-1)?direction:1;
}
static void mTXSetDecayMode (MTXDecay *mtx_decay_obj, t_symbol *c_mode)
{
   mtx_decay_obj->decay_mode = c_mode;
}

static void *newMTXDecay (t_symbol *s, int argc, t_atom *argv)
{
   MTXDecay *mtx_decay_obj = (MTXDecay *) pd_new (mtx_decay_class);
   int sym_count=0;
   int first_sym=argc;
   int n=0;

   mTXSetDecayMode (mtx_decay_obj, gensym(":"));
   mTXSetDecayDirection (mtx_decay_obj, 1);
   mTXSetDecayParameter (mtx_decay_obj, .9f);

   argc = ((argc<3)?argc:3);
   while (n < argc) {
     if (argv[n].a_type == A_SYMBOL) {
	sym_count++;
	first_sym = (first_sym<n)?first_sym:n;
     }
     n++;
   }
   if (sym_count >= 1) 
      mTXSetDecayMode (mtx_decay_obj, atom_getsymbol(argv+first_sym));
   if (sym_count > 1) {
      post("mtx_decay: args after pos %d ignored. supposed to be non-symbolic",first_sym);
      argc = first_sym+1;
   }
      
   switch (argc) {
      case 3:
	 if (first_sym < 2)
	    mTXSetDecayDirection (mtx_decay_obj, atom_getfloat (argv+2));
      case 2:
	 if (first_sym < 1) 
	    mTXSetDecayParameter (mtx_decay_obj, atom_getfloat (argv+1));
	 else if (first_sym > 1)
	    mTXSetDecayDirection (mtx_decay_obj, atom_getfloat(argv+1));
      case 1:
	 if (first_sym != 0)
	    mTXSetDecayParameter (mtx_decay_obj, atom_getfloat (argv));
   }

   mtx_decay_obj->list_outlet = outlet_new (&mtx_decay_obj->x_obj, gensym("matrix"));
   return ((void *) mtx_decay_obj);
} 

static void mTXDecayBang (MTXDecay *mtx_decay_obj)
{
   if (mtx_decay_obj->list_out) 
      outlet_anything(mtx_decay_obj->list_outlet, gensym("matrix"), 
	    mtx_decay_obj->size+2, mtx_decay_obj->list_out);
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

static void deCay (int n, t_float *x, t_float *y, t_float alpha)
{
   t_float decay = *x;
   t_float oneminusalpha = 1.0f-alpha;
   for (;n--; x++, y++) {
      decay = alpha * decay + oneminusalpha * *x;
      *y = decay = (decay < *x)? *x : decay;
   }
}
static void deCayReverse (int n, t_float *x, t_float *y, t_float alpha)
{
   t_float decay = *x;
   t_float oneminusalpha = 1.0f-alpha;
   for (;n--; x--, y--) {
      decay = alpha * decay + oneminusalpha * *x;
      *y = decay = (decay < *x)? *x : decay;
   }
}

static void mTXDecayMatrix (MTXDecay *mtx_decay_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_ptr = argv;
   t_atom *list_out = mtx_decay_obj->list_out;
   t_float *x = mtx_decay_obj->x;
   t_float *y = mtx_decay_obj->y;
   int count;

   /* size check */
   if (!size) {
      post("mtx_decay: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_decay: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   else if ((!x)||(!list_out)||(!y)) {
      if (!x)
	 x = (t_float *) getbytes (sizeof (t_float) * (size));
      if (!y)
	 y = (t_float *) getbytes (sizeof (t_float) * (size));
      if (!list_out)
	 list_out = (t_atom *) getbytes (sizeof (t_atom) * (size+2));
   }
   else if (size != mtx_decay_obj->size) {
      x = (t_float *) resizebytes (x,
	    sizeof (t_float) * (mtx_decay_obj->size),
	    sizeof (t_float) * (size));
      y = (t_float *) resizebytes (y,
	    sizeof (t_float) * (mtx_decay_obj->size),
	    sizeof (t_float) * (size));
      list_out = (t_atom *) resizebytes (list_out,
	    sizeof (t_atom) * (mtx_decay_obj->size+2),
	    sizeof (t_atom) * (size + 2));
   }
   mtx_decay_obj->size = size;
   mtx_decay_obj->rows = rows;
   mtx_decay_obj->columns = columns;
   mtx_decay_obj->list_out = list_out;
   mtx_decay_obj->x = x;
   mtx_decay_obj->y = y;

   /* main part */
   /* reading matrix from inlet */

   if ((mtx_decay_obj->decay_mode == col_sym) ||
	 (mtx_decay_obj->decay_mode == col_sym2)) {
      readFloatFromListModulo (size, columns, list_ptr, x);
      columns = mtx_decay_obj->rows;
      rows = mtx_decay_obj->columns;
   }
   else
      readFloatFromList (size, list_ptr, x);
   
   /* calculating decay */
   if (mtx_decay_obj->decay_direction == -1) {
      if ((mtx_decay_obj->decay_mode == col_sym) ||
	    (mtx_decay_obj->decay_mode == col_sym2) ||
	    (mtx_decay_obj->decay_mode == row_sym)) {
	 x += columns-1;
	 y += columns-1;
	 for (count = rows; count--; x += columns, y += columns)
	    deCayReverse (columns,x,y,mtx_decay_obj->decay_parameter);
      }
      else {
	 x += size-1;
	 y += size-1;
	 deCayReverse (size,x,y,mtx_decay_obj->decay_parameter);
      }
   }
   else {
      if ((mtx_decay_obj->decay_mode == col_sym) ||
	    (mtx_decay_obj->decay_mode == col_sym2) ||
	    (mtx_decay_obj->decay_mode == row_sym)) 
	 for (count = rows; count--; x += columns, y += columns)
	    deCay (columns,x,y,mtx_decay_obj->decay_parameter);
      else
	 deCay (size,x,y,mtx_decay_obj->decay_parameter);
   }
   x = mtx_decay_obj->x;
   y = mtx_decay_obj->y;

   /* writing matrix to outlet */
  if ((mtx_decay_obj->decay_mode == col_sym) ||
	 (mtx_decay_obj->decay_mode == col_sym2)) {
      columns = mtx_decay_obj->columns;
      rows = mtx_decay_obj->rows;
      writeFloatIntoListModulo (size, columns, list_out+2, y);
   }
   else
      writeFloatIntoList (size, list_out+2, y);

   SETSYMBOL(list_out, gensym("matrix"));
   SETFLOAT(list_out, rows);
   SETFLOAT(&list_out[1], columns);
   outlet_anything(mtx_decay_obj->list_outlet, gensym("matrix"), 
	 mtx_decay_obj->size+2, list_out);
}

void mtx_decay_setup (void)
{
   mtx_decay_class = class_new 
      (gensym("mtx_decay"),
       (t_newmethod) newMTXDecay,
       (t_method) deleteMTXDecay,
       sizeof (MTXDecay),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_decay_class, (t_method) mTXDecayBang);
   class_addmethod (mtx_decay_class, (t_method) mTXDecayMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_decay_class, (t_method) mTXSetDecayParameter, gensym("alpha"), A_DEFFLOAT,0);
   class_addmethod (mtx_decay_class, (t_method) mTXSetDecayMode, gensym("mode"), A_DEFSYMBOL,0);
   class_addmethod (mtx_decay_class, (t_method) mTXSetDecayDirection, gensym("direction"), A_DEFFLOAT,0);

   row_sym = gensym("row");
   col_sym = gensym("col");
   col_sym2 = gensym("column");
}

void iemtx_decay_setup(void){
  mtx_decay_setup();
}
