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
#include "mtx_spherical_harmonics/sph_radial.c"

static t_class *mtx_spherical_radial_class;

typedef struct _MTXSph_ MTXSph;
struct _MTXSph_
{
  t_object x_obj;
  t_outlet *list_h_re_out;
  t_outlet *list_h_im_out;
  t_atom *list_h_re;
  t_atom *list_h_im;
  double *kr;
  double *h_re;
  double *h_im;
  size_t nmax;
  size_t l;
};

static void allocMTXSphdata (MTXSph *x) 
{
   x->kr=(double*)calloc(x->l,sizeof(double));
   if (x->list_h_re_out!=0) {
      x->list_h_re=(t_atom*)calloc(x->l*(x->nmax+1)+2,sizeof(t_atom));
      x->h_re=(double*)calloc(x->l*(x->nmax+1),sizeof(double));
   }
   if (x->list_h_im_out!=0) {
      x->list_h_im=(t_atom*)calloc(x->l*(x->nmax+1)+2,sizeof(t_atom));
      x->h_im=(double*)calloc(x->l*(x->nmax+1),sizeof(double));
   }
}

static void deleteMTXSphdata (MTXSph *x) 
{
   if (x->kr!=0)
      free(x->kr);
   if (x->h_re!=0)
      free(x->h_re);
   if (x->h_im!=0)
      free(x->h_im);
   if (x->list_h_re!=0)
      free(x->list_h_re);
   if (x->list_h_im!=0)
      free(x->list_h_im);
   x->list_h_re=0;
   x->list_h_im=0;
   x->h_re=0;
   x->h_im=0;
   x->kr=0;
}

static void *newMTXSph (t_symbol *s, int argc, t_atom *argv)
{
  int nmax;
  char whichfunction = 'j';
  t_symbol *fsym;
  MTXSph *x = (MTXSph *) pd_new (mtx_spherical_radial_class);
  x->list_h_re = 0; 
  x->list_h_im = 0; 
  x->list_h_im_out = 0; 
  x->list_h_re_out = 0; 
  x->kr = 0; 
  x->h_re = 0; 
  x->h_im = 0; 
  x->l=0;
  fsym=atom_getsymbol(argv);
  if (fsym->s_name!=0)
     whichfunction=fsym->s_name[0];
  switch (whichfunction) {
     default:
     case 'j':
        x->list_h_re_out = outlet_new (&x->x_obj, gensym("matrix"));
	break;
     case 'h':
        x->list_h_re_out = outlet_new (&x->x_obj, gensym("matrix"));
     case 'y':
        x->list_h_im_out = outlet_new (&x->x_obj, gensym("matrix"));
  }
  nmax=(int) atom_getfloat(argv+1);
  if (nmax<0)
     nmax=0;
  x->nmax=nmax;
  
  return ((void *) x);
} 

static void mTXSphBang (MTXSph *x)
{
  if (x->list_h_im!=0) {
    outlet_anything(x->list_h_im_out, gensym("matrix"), x->l*(x->nmax+1)+2, x->list_h_im);
  }
  if (x->list_h_re!=0) {
    outlet_anything(x->list_h_re_out, gensym("matrix"), x->l*(x->nmax+1)+2, x->list_h_re);
  }
}

static void mTXSphMatrix (MTXSph *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int size = rows * columns;
  int in_size = argc-2;
  int n,ofs;


  /* size check */
  if (!size) 
    post("mtx_spherical_radial: invalid dimensions");
  else if (in_size<size) 
    post("mtx_spherical_radial: sparse matrix not yet supported: use \"mtx_check\"");
  else if ((rows!=1)||(columns<1))
     post("mtx_spherical_radial: 1 X L matrix expected with kr and h vector, but got more rows/no entries");
  else {
     if (x->l!=columns) {
        deleteMTXSphdata(x);
        x->l=columns;
        allocMTXSphdata(x);
     }
     for (n=0;n<x->l;n++) {
        x->kr[n]=(double) atom_getfloat(argv+n);
     }
  
     if (x->h_re!=0) 
	for (n=0,ofs=0;n<x->l;n++,ofs+=x->nmax+1)
	    sphBessel(x->kr[n], x->h_re+ofs, x->nmax);

     if (x->h_im!=0) 
	for (n=0,ofs=0;n<x->l;n++,ofs+=x->nmax+1)
	    sphNeumann(x->kr[n], x->h_im+ofs, x->nmax);
     
     if (x->h_re!=0) {
        SETFLOAT(x->list_h_re+1,(float)(x->nmax+1));
        SETFLOAT(x->list_h_re,(float)x->l);
	for (n=0;n<x->l*(x->nmax+1);n++)
           SETFLOAT(x->list_h_re+n+2,(float)x->h_re[n]);
     }
     
     if (x->h_im!=0) {
        SETFLOAT(x->list_h_im+1,(float)(x->nmax+1));
        SETFLOAT(x->list_h_im,(float)x->l);
	for (n=0;n<x->l*(x->nmax+1);n++)
           SETFLOAT(x->list_h_im+n+2,(float)x->h_im[n]);
     }

     mTXSphBang(x);
  }
}

void mtx_spherical_radial_setup (void)
{
  mtx_spherical_radial_class = class_new 
    (gensym("mtx_spherical_radial"),
     (t_newmethod) newMTXSph,
     (t_method) deleteMTXSphdata,
     sizeof (MTXSph),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_spherical_radial_class, (t_method) mTXSphBang);
  class_addmethod (mtx_spherical_radial_class, (t_method) mTXSphMatrix, gensym("matrix"), A_GIMME,0);
}

void iemtx_spherical_radial_setup(void){
  mtx_spherical_radial_setup();
}


