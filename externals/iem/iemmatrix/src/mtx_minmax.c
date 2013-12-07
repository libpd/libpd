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

static t_class *mtx_minmax_class;
static t_symbol *row_sym;
static t_symbol *col_sym;
static t_symbol *col_sym2;

typedef struct _MTXminmax_ MTXminmax;
struct _MTXminmax_
{
  t_object x_obj;
  int size;
  int outsize;
  int mode;
  int operator_minimum; /* 1 if we are [mtx_min], 0 if we are [mtx_max] */

  t_outlet *min_outlet;
  t_outlet *max_outlet;

  t_atom *minlist_out;
  t_atom *maxlist_out;
};

static void deleteMTXMinMax (MTXminmax *mtx_minmax_obj) 
{
  if (mtx_minmax_obj->maxlist_out)
    freebytes (mtx_minmax_obj->maxlist_out, sizeof(t_atom)*(mtx_minmax_obj->size));
  if (mtx_minmax_obj->minlist_out)
    freebytes (mtx_minmax_obj->minlist_out, sizeof(t_atom)*(mtx_minmax_obj->size));
}

static void mTXSetMinMaxMode (MTXminmax *mtx_minmax_obj, t_symbol *m_sym)
{
  int mode=0;
  if(gensym("row")==m_sym)
    mode=1;
  else if((gensym("col")==m_sym) || (gensym("column")==m_sym) || (gensym(":")==m_sym))
    mode=2;

  mtx_minmax_obj->mode = mode;
}

static void *newMTXMinMax (t_symbol *s)
{
  MTXminmax *mtx_minmax_obj = (MTXminmax *) pd_new (mtx_minmax_class);

  mtx_minmax_obj->mode=0;

  mtx_minmax_obj->operator_minimum = 1;

  mtx_minmax_obj->min_outlet = outlet_new (&mtx_minmax_obj->x_obj, gensym("matrix"));
  mtx_minmax_obj->max_outlet = outlet_new (&mtx_minmax_obj->x_obj, gensym("matrix"));

  if((NULL!=s)&&(&s_!=s)&&(NULL!=s->s_name))
    mTXSetMinMaxMode (mtx_minmax_obj, s);

  return ((void *) mtx_minmax_obj);
} 

static void mTXMinMaxBang (MTXminmax *mtx_minmax_obj)
{
  if (mtx_minmax_obj->maxlist_out) 
    outlet_list(mtx_minmax_obj->max_outlet, gensym("list"),
                    mtx_minmax_obj->outsize, mtx_minmax_obj->maxlist_out);
  if (mtx_minmax_obj->minlist_out) 
    outlet_list(mtx_minmax_obj->min_outlet, gensym("list"), 
                    mtx_minmax_obj->outsize, mtx_minmax_obj->minlist_out);
}

static void minmaxList (int n, t_atom *x, t_float*min, t_float*max)
{
  t_float min_=atom_getfloat(x);
  t_float max_=min_;
  t_float f;
  for (;n--;x++) {
    f = atom_getfloat(x);
    min_ = (min_ < f)?min_:f;
    max_ = (max_ > f)?max_:f;
  }
  *max=max_;
  *min=min_;
}

static void minmaxListStep (int n, const int step, t_atom *x, t_float*min, t_float*max)
{
  t_float min_=atom_getfloat(x);
  t_float max_=min_;
  t_float f;
  for (;n--;x+=step) {
    f = atom_getfloat(x);
    min_ = (min_ < f)?min_:f;
    max_ = (max_ > f)?max_:f;
  }
  *max=max_;
  *min=min_;
}

static void minmaxListColumns (const int rows, const int columns, t_atom *x, 
                               t_atom *ap_min, t_atom *ap_max)
{
  int count;
  t_float min, max;
  for (count=0; count < columns; count++, x++, ap_min++, ap_max++) {
    minmaxListStep (rows, columns, x, &min, &max);
    SETFLOAT(ap_min,min);
    SETFLOAT(ap_max,max);
  }
}
static void minmaxListRows (int rows, int columns, t_atom *x, 
                            t_atom *ap_min, t_atom*ap_max)
{
  int count;
  t_float min, max;
  for (count=0; count < rows; count++, x+=columns, ap_min++, ap_max++) {
    minmaxList (columns, x, &min, &max);
    SETFLOAT(ap_min, min);
    SETFLOAT(ap_max,max);
  }
}
static void mTXMinMaxMatrix (MTXminmax *mtx_minmax_obj, t_symbol *s, 
                             int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  t_atom *maxlist_out = mtx_minmax_obj->maxlist_out;
  t_atom *minlist_out = mtx_minmax_obj->minlist_out;
  int elements_out;
  
  /* size check */
  if (!size) {
    post("mtx_minmax: invalid dimensions");
    return;
  }
  else if ((argc-2)<size) {
    post("mtx_minmax: sparse matrix not yet supported: use \"mtx_check\"");
    return;
  }
   
  if (size != mtx_minmax_obj->size) {
    if (!minlist_out)
      minlist_out = (t_atom *) getbytes (sizeof (t_atom) * size);
    else
      minlist_out = (t_atom *) resizebytes (minlist_out,
                                            sizeof (t_atom) * (mtx_minmax_obj->size),
                                            sizeof (t_atom) * size);
    if (!maxlist_out)
      maxlist_out = (t_atom *) getbytes (sizeof (t_atom) * size);
    else
      maxlist_out = (t_atom *) resizebytes (maxlist_out,
                                            sizeof (t_atom) * (mtx_minmax_obj->size),
                                            sizeof (t_atom) * size);
  }

  mtx_minmax_obj->size = size;
  mtx_minmax_obj->minlist_out = minlist_out;
  mtx_minmax_obj->maxlist_out = maxlist_out;
  
  /* main part */
  
  switch(mtx_minmax_obj->mode){
  case 1:
    elements_out = rows;
    minmaxListRows (rows, columns, argv, minlist_out, maxlist_out);
    break;
  case 2:
    elements_out = columns;
    minmaxListColumns (rows, columns, argv, minlist_out, maxlist_out);
    break;
  default:
    elements_out = 1;
    minmaxListRows (1, size, argv, minlist_out, maxlist_out);
  }
  mtx_minmax_obj->outsize = elements_out;
  maxlist_out = mtx_minmax_obj->maxlist_out;
  minlist_out = mtx_minmax_obj->minlist_out;
  
  mTXMinMaxBang(mtx_minmax_obj);
}

void mtx_minmax_setup (void)
{
  mtx_minmax_class = class_new (
                                gensym("mtx_minmax"),
                                (t_newmethod) newMTXMinMax,
                                (t_method) deleteMTXMinMax,
                                sizeof (MTXminmax),
                                CLASS_DEFAULT, A_DEFSYM, 0);

  class_addbang (mtx_minmax_class, (t_method) mTXMinMaxBang);
  class_addmethod (mtx_minmax_class, (t_method) mTXMinMaxMatrix, gensym("matrix"), A_GIMME,0);
  class_addmethod (mtx_minmax_class, (t_method) mTXSetMinMaxMode, gensym("mode"), A_DEFSYMBOL ,0);



  row_sym = gensym("row");
  col_sym = gensym("col");
  col_sym2 = gensym("column");
}

void iemtx_minmax_setup(void){
  mtx_minmax_setup();
}
