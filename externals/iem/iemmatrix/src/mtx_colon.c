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

static t_class *mtx_colon_class;

typedef struct _MTXColon_ MTXColon;
struct _MTXColon_
{
   t_object x_obj;
   int size;

   t_atom *list_out;
   t_outlet *list_outlet;
};

static void deleteMTXColon (MTXColon *mtx_colon_obj) 
{
   if (mtx_colon_obj->list_out)
      freebytes (mtx_colon_obj->list_out, sizeof(t_atom)*(mtx_colon_obj->size+2));
}

static void *newMTXColon (t_symbol *s, int argc, t_atom *argv)
{
   MTXColon *mtx_colon_obj = (MTXColon *) pd_new (mtx_colon_class);

   mtx_colon_obj->list_outlet = outlet_new (&mtx_colon_obj->x_obj, gensym("matrix"));
   return ((void *) mtx_colon_obj);
} 

static void mTXColonBang (MTXColon *mtx_colon_obj)
{
   if (mtx_colon_obj->list_out) 
      outlet_anything(mtx_colon_obj->list_outlet, gensym("matrix"), 
	    mtx_colon_obj->size+2, mtx_colon_obj->list_out);
}

static void mTXColonList (MTXColon *mtx_colon_obj, t_symbol *s,
      int argc, t_atom *argv)
{
   int size;
   t_float startval;
   t_float stopval;
   t_float step;
   t_atom *list_out = mtx_colon_obj->list_out;
   if (argc == 3) {
      startval = atom_getfloat(argv++);
      step = atom_getfloat(argv++);
      stopval = atom_getfloat(argv++);
   }
   else if (argc == 2) {
      startval = atom_getfloat(argv++);
      stopval = atom_getfloat(argv++);
      step = 1.0f;
   }
   else {
      post("mtx_colon: wrong number of input arguments");
      return;
   }
      
   size = (int)((stopval- startval + step) / step);
   if (size) {
      if (size!=mtx_colon_obj->size) {
	 if (list_out) 
	    list_out = (t_atom *) resizebytes (list_out,
		  sizeof(t_atom)*(mtx_colon_obj->size+2),
		  sizeof(t_atom)*(size+2));
	 else
	    list_out = (t_atom*) getbytes (sizeof(t_atom)*(size+2));
	 mtx_colon_obj->size = size;
      }
      mtx_colon_obj->list_out = list_out;

      SETFLOAT (&list_out[0],1.0f);
      SETFLOAT (&list_out[1],(t_float)size);
      list_out += 2;
      for (;size--;list_out++,startval+=step)
	 SETFLOAT(list_out,startval);

      mTXColonBang (mtx_colon_obj);
   }
}

static void mTXColonMtx (MTXColon *mtx_colon_obj, t_symbol *s,
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_ptr = argv;
   t_atom *list_out = mtx_colon_obj->list_out;
 
   if (!size) {
      post("mtx_colon: invalid matrix dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_colon: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   else if (!list_out) {
      list_out = (t_atom*) getbytes (sizeof (t_atom) * (size+2));
   }
   else if (size != mtx_colon_obj->size)
   {
      list_out = (t_atom*) resizebytes (list_out, 
	    sizeof (t_atom) * (mtx_colon_obj->size+2),
	    sizeof (t_atom) * (size+2));
   }
   mtx_colon_obj->list_out = list_out;
   mtx_colon_obj->size = size;

   list_out+=2;
   while (size--)
      *list_out++ = *list_ptr++;

   list_out = mtx_colon_obj->list_out;
   size = mtx_colon_obj->size;

   SETSYMBOL(list_out, gensym("matrix"));
   SETFLOAT(list_out, 1);
   SETFLOAT(&list_out[1], size);
   mTXColonBang (mtx_colon_obj);
}

void mtx_colon_setup (void)
{
   mtx_colon_class = class_new 
      (gensym("mtx_colon"),
       (t_newmethod) newMTXColon,
       (t_method) deleteMTXColon,
       sizeof (MTXColon),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_colon_class, (t_method) mTXColonBang);
   class_addmethod (mtx_colon_class, (t_method) mTXColonMtx, gensym("matrix"), A_GIMME, 0);
   class_addlist (mtx_colon_class, (t_method) mTXColonList);
   class_addcreator ((t_newmethod) newMTXColon, gensym("mtx_:"), A_GIMME, 0);

}

void iemtx_colon_setup(void){
  mtx_colon_setup();
}
