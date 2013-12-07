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

static t_class *mtx_slice_class;

typedef struct _MTXslice_ MTXslice;
struct _MTXslice_
{
  t_object x_obj;
  int slice_size;
  int slice_startcol;
  int slice_startrow;
  int slice_stopcol;
  int slice_stoprow;

  t_outlet *list_outlet;

  t_atom *list_out;
};

static void deleteMTXSlice (MTXslice *mtx_slice_obj) 
{
  if (mtx_slice_obj->list_out)
    freebytes (mtx_slice_obj->list_out, sizeof(t_atom)*(mtx_slice_obj->slice_size+2));
}

static void mTXSliceIndexList (MTXslice *mtx_slice_obj, t_symbol *s, 
			       int argc, t_atom *argv)
{
  int startcol;
  int startrow;
  int stopcol;
  int stoprow;
  t_symbol *endsym = gensym("end");

  if (argc<4) {
    post("mtx_slice: invalid index vector: <startrow><startcol><stoprow><stopcol>");
    return;
  }
  startrow = atom_getint(&argv[0]);
  startcol = atom_getint(&argv[1]);
  stoprow = atom_getint(&argv[2]);
  stopcol = atom_getint(&argv[3]);
  if (atom_getsymbol(&argv[0])==endsym) {
    startrow = -1;
  }
  if (atom_getsymbol(&argv[1])==endsym) {
    startcol = -1;
  }
  if (atom_getsymbol(&argv[2])==endsym) {
    stoprow = -1;
  }
  if (atom_getsymbol(&argv[3])==endsym) {
    stopcol = -1;
  }

  if (((startrow<1) && (atom_getsymbol(&argv[0])!=endsym)) || 
      ((startcol<1) && (atom_getsymbol(&argv[1])!=endsym))) {
    post("mtx_slice: row and column indices must be >0, or misused \"end\" keyword");
    return;
  }
   
  if (((startrow>stoprow) && (atom_getsymbol(&argv[2])!=endsym)) ||
      ((startcol>stopcol) && (atom_getsymbol (&argv[3])!=endsym))) {
    post("mtx_slice: start_index<stop_index for rows and columns, or misused \"end\" keyword");
    return;
  }

  mtx_slice_obj->slice_startrow = startrow;
  mtx_slice_obj->slice_startcol = startcol;
  mtx_slice_obj->slice_stoprow = stoprow;
  mtx_slice_obj->slice_stopcol = stopcol;
}

static void *newMTXSlice (t_symbol *s, int argc, t_atom *argv)
{
  MTXslice *mtx_slice_obj = (MTXslice *) pd_new (mtx_slice_class);
  if (argc==4)  
    mTXSliceIndexList (mtx_slice_obj, gensym("list"),argc,argv); 
  else {
    mtx_slice_obj->slice_startrow = 1;
    mtx_slice_obj->slice_startcol = 1;
    mtx_slice_obj->slice_stopcol = -1;
    mtx_slice_obj->slice_stoprow = -1;
  }
  mtx_slice_obj->list_outlet = outlet_new (&mtx_slice_obj->x_obj, gensym("matrix"));
  inlet_new(&mtx_slice_obj->x_obj, &mtx_slice_obj->x_obj.ob_pd, gensym("list"),gensym(""));
  return ((void *) mtx_slice_obj);
} 

static void mTXSliceBang (MTXslice *mtx_slice_obj)
{
  if (mtx_slice_obj->list_out) 
    outlet_anything(mtx_slice_obj->list_outlet, gensym("matrix"), 
		    mtx_slice_obj->slice_size+2, mtx_slice_obj->list_out);
}
/*
  static void copyList (int size, t_atom *x, t_atom *y)
  {
  while(size--)
  *y++=*x++;
  }
*/

static void writeVectorIntoList (int n, t_atom *x, t_atom *y)
{
  for (;n--;x++,y++) 
    *y = *x;
}

static void writeSliceIntoList (int slicerows, const int slicecols, int columns, t_atom *x, t_atom *y)
{
  for (;slicerows--;x+=columns,y+=slicecols)
    writeVectorIntoList(slicecols, x, y);
}

static void mTXSliceMatrix (MTXslice *mtx_slice_obj, t_symbol *s, 
			    int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int list_size = argc - 2;
  t_atom *list_in = argv;
  t_atom *list_out = mtx_slice_obj->list_out;
  int stopcol = mtx_slice_obj->slice_stopcol;
  int stoprow = mtx_slice_obj->slice_stoprow;
  int startrow = mtx_slice_obj->slice_startrow;
  int startcol = mtx_slice_obj->slice_startcol;
  int slicecols, slicerows, slicesize;

  /* size check */
  if (!size) {
    post("mtx_slice: invalid dimensions");
    return;
  }
  else if (list_size<size) {
    post("mtx_slice: sparse matrix not yet supported: use \"mtx_check\"");
    return;
  }
  startrow = (startrow==-1)?rows:startrow;
  startcol = (startcol==-1)?columns:startcol;
  stoprow = (stoprow==-1)?rows:stoprow;
  stopcol = (stopcol==-1)?columns:stopcol;
  if ((!startrow)||(!startcol)) {
    post("mtx_slice: indices must be >0");
    return;
  }
  if ((stopcol > columns) ||
      (stoprow > rows)) {
    post("mtx_slice: slice index exceeds matrix dimensions");
    return;
  }
  if ((stoprow<startrow) || (stopcol<startcol)) {
    post("mtx_slice: start_index<stop_index for rows and columns, or misused \"end\" keyword");
    return;
  }
  slicerows = stoprow-startrow+1;
  slicecols = stopcol-startcol+1;
  slicesize = slicerows*slicecols;

  /* main part */
  if (slicesize != mtx_slice_obj->slice_size) {
    if (!list_out)
      list_out = (t_atom *) getbytes (sizeof (t_atom) * (slicesize + 2));
    else
      list_out = (t_atom *) resizebytes (list_out,
					 sizeof (t_atom) * (mtx_slice_obj->slice_size+2),
					 sizeof (t_atom) * (slicesize + 2));
    mtx_slice_obj->slice_size = slicesize;
    mtx_slice_obj->list_out = list_out;
  }
  list_out += 2;
  list_in += columns * (startrow-1) + startcol-1;
  writeSliceIntoList (slicerows, slicecols,
		      columns, list_in,list_out);
  list_out = mtx_slice_obj->list_out;
  SETSYMBOL(list_out, gensym("matrix"));
  SETFLOAT(list_out, slicerows);
  SETFLOAT(&list_out[1], slicecols);
  outlet_anything(mtx_slice_obj->list_outlet, gensym("matrix"), 
		  slicesize+2, list_out);
}

void mtx_slice_setup (void)
{
  mtx_slice_class = class_new 
    (gensym("mtx_slice"),
     (t_newmethod) newMTXSlice,
     (t_method) deleteMTXSlice,
     sizeof (MTXslice),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_slice_class, (t_method) mTXSliceBang);
  class_addmethod (mtx_slice_class, (t_method) mTXSliceMatrix, gensym("matrix"), A_GIMME,0);
  class_addmethod (mtx_slice_class, (t_method) mTXSliceIndexList, gensym(""), A_GIMME,0);
}

void iemtx_slice_setup(void){
  mtx_slice_setup();
}
