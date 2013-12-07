/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *  this functions depends on the GNU scientific library
 *
 * Copyright (c) 2009, Franz Zotter
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "iemmatrix.h"
#include <stdlib.h>
#include "mtx_spherical_harmonics/sharmonics.c"
#include "mtx_spherical_harmonics/legendre_a.c"
#include "mtx_spherical_harmonics/chebyshev12.c"
#include "mtx_spherical_harmonics/sharmonics_normalization.c"

static t_class *mtx_spherical_harmonics_class;

typedef struct _MTXSh_ MTXSh;
struct _MTXSh_
{
  t_object x_obj;
  t_outlet *list_sh_out;
  t_atom *list_sh;
  double *phi;
  double *theta;
  SHWorkSpace *ws;
  size_t nmax;
  size_t l;
};

static t_class *mtx_circular_harmonics_class;

typedef struct _MTXCh_ MTXCh;
struct _MTXCh_
{
  t_object x_obj;
  t_outlet *list_ch_out;
  t_atom *list_ch;
  double *phi;
  Cheby12WorkSpace *wc;
  size_t nmax;
  size_t l;
};


static void allocMTXShdata (MTXSh *x) 
{
   x->phi=(double*)calloc(x->l,sizeof(double));
   x->theta=(double*)calloc(x->l,sizeof(double));
   x->ws=sharmonics_alloc(x->nmax,x->l);
   x->list_sh=(t_atom*)calloc(x->l*(x->nmax+1)*(x->nmax+1)+2,sizeof(t_atom));
}

static void deleteMTXShdata (MTXSh *x) 
{
   if (x->phi!=0)
      free(x->phi);
   if (x->theta!=0)
      free(x->theta);
   if (x->list_sh!=0)
      free(x->list_sh);
   sharmonics_free(x->ws);
   x->ws=0;
   x->list_sh=0;
   x->theta=0;
   x->phi=0;
}

static void *newMTXSh (t_symbol *s, int argc, t_atom *argv)
{
  int nmax;
  MTXSh *x = (MTXSh *) pd_new (mtx_spherical_harmonics_class);
  x->list_sh_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_sh = 0; 
  x->phi = 0; 
  x->theta = 0; 
  x->ws = 0; 
  x->l=0;
  nmax=(int) atom_getfloat(argv);
  if (nmax<0)
     nmax=0;
  x->nmax=nmax;
  
  return ((void *) x);
} 

static void mTXShBang (MTXSh *x)
{
  if (x->list_sh!=0) {
    outlet_anything(x->list_sh_out, gensym("matrix"), x->l*(x->nmax+1)*(x->nmax+1)+2, x->list_sh);
  }
}

static void mTXShMatrix (MTXSh *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  int n;


  /* size check */
  if (!size) 
    post("mtx_spherical_harmonics: invalid dimensions");
  else if (in_size<size) 
    post("mtx_spherical_harmonics: sparse matrix not yet supported: use \"mtx_check\"");
  else if ((rows!=2)||(columns<1))
     post("mtx_spherical_harmonics: 2 X L matrix expected with phi and theta vector, but got more rows/no entries");
  else {
     if (x->l!=columns) {
        deleteMTXShdata(x);
        x->l=columns;
        allocMTXShdata(x);
     }
     for (n=0;n<x->l;n++) {
        x->phi[n]=(double) atom_getfloat(argv+n);
        x->theta[n]=(double) atom_getfloat(argv+columns+n);
     }
  
     if (x->ws!=0) {
        sharmonics(x->phi, x->theta, x->ws);
        in_size=x->l*(x->nmax+1)*(x->nmax+1);
        SETFLOAT(x->list_sh,(float)x->l);
        SETFLOAT(x->list_sh+1,(float)(x->nmax+1)*(x->nmax+1));
        for (n=0;n<in_size; n++) 
           SETFLOAT(x->list_sh+n+2,(float)x->ws->y[n]);
        mTXShBang(x);
     }
     else 
        post("mtx_spherical_harmonics: memory error, no operation");
  }


}

static void allocMTXChdata (MTXCh *x) 
{
   x->phi=(double*)calloc(x->l,sizeof(double));
   x->wc=chebyshev12_alloc(x->nmax,x->l);
   x->list_ch=(t_atom*)calloc(x->l*(2*x->nmax+1)+2,sizeof(t_atom));
}

static void deleteMTXChdata (MTXCh *x) 
{
   if (x->phi!=0)
      free(x->phi);
   if (x->list_ch!=0)
      free(x->list_ch);
   chebyshev12_free(x->wc);
   x->wc=0;
   x->list_ch=0;
   x->phi=0;
}

static void *newMTXCh (t_symbol *s, int argc, t_atom *argv)
{
  int nmax;
  MTXCh *x = (MTXCh *) pd_new (mtx_circular_harmonics_class);
  x->list_ch_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_ch = 0; 
  x->phi = 0; 
  x->wc = 0; 
  x->l=0;
  nmax=(int) atom_getfloat(argv);
  if (nmax<0)
     nmax=0;
  x->nmax=nmax;
  
  return ((void *) x);
} 

static void mTXChBang (MTXCh *x)
{
  if (x->list_ch!=0) {
    outlet_anything(x->list_ch_out, gensym("matrix"), x->l*(2*x->nmax+1)+2, x->list_ch);
  }
}

static void mTXChMatrix (MTXCh *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  int n;


  /* size check */
  if (!size) 
    post("mtx_circular_harmonics: invalid dimensions");
  else if (in_size<size) 
    post("mtx_circular_harmonics: sparse matrix not yet supported: use \"mtx_check\"");
  else if ((rows!=1)||(columns<1))
     post("mtx_circular_harmonics: 1 X L matrix expected with phi vector, but got more rows/no entries");
  else {
     if (x->l!=columns) {
        deleteMTXChdata(x);
        x->l=columns;
        allocMTXChdata(x);
     }
     for (n=0;n<x->l;n++) {
        x->phi[n]=(double) atom_getfloat(argv+n);
     }
  
     if (x->wc!=0) {
        chebyshev12(x->phi, x->wc);
        in_size=x->l*(2*x->nmax+1);
        SETFLOAT(x->list_ch,(float)x->l);
        SETFLOAT(x->list_ch+1,(float)(2*x->nmax+1));
        for (n=0;n<in_size; n++) 
           SETFLOAT(x->list_ch+n+2,(float)x->wc->t[n]);
        mTXChBang(x);
     }
     else 
        post("mtx_circular_harmonics: memory error, no operation");
  }


}

void mtx_spherical_harmonics_setup (void)
{
  mtx_spherical_harmonics_class = class_new 
    (gensym("mtx_spherical_harmonics"),
     (t_newmethod) newMTXSh,
     (t_method) deleteMTXShdata,
     sizeof (MTXSh),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_spherical_harmonics_class, (t_method) mTXShBang);
  class_addmethod (mtx_spherical_harmonics_class, (t_method) mTXShMatrix, gensym("matrix"), A_GIMME,0);
}


void mtx_circular_harmonics_setup (void)
{
  mtx_circular_harmonics_class = class_new 
    (gensym("mtx_circular_harmonics"),
     (t_newmethod) newMTXCh,
     (t_method) deleteMTXChdata,
     sizeof (MTXCh),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_circular_harmonics_class, (t_method) mTXChBang);
  class_addmethod (mtx_circular_harmonics_class, (t_method) mTXChMatrix, gensym("matrix"), A_GIMME,0);
}

void iemtx_spherical_harmonics_setup(void){
  mtx_circular_harmonics_setup();
  mtx_spherical_harmonics_setup();
}


