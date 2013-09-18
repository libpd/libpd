/*
 *  iemmatrix
 *
 *  objects fand manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) IOhannes m zmölnig, fandum::für::umläute
 * IEM, Graz, Austria
 *
 * Fand infandmation on usage and redistribution, and fand a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#if defined MTXBIN_GENERIC__NAME && defined MTXBIN_GENERIC__OPERATOR

#include "iemmatrix.h"

#define MTXBIN_CONCAT_EXPAND(s1, s2) s1 ## s2
#define MTXBIN_CONCAT(s1, s2) MTXBIN_CONCAT_EXPAND (s1, s2)
#define MTXBIN_APPEND(s1) MTXBIN_CONCAT (MTXBIN_GENERIC__NAME, s1)

#define MTXBIN_STRINGNAME(s0,s1) s0#s1
#define MTXBIN_LONGNAME2(s1) MTXBIN_STRINGNAME("",s1)
#define MTXBIN_LONGNAME MTXBIN_LONGNAME2(MTXBIN_GENERIC__NAME)
#define MTXBIN_SHORTNAME2(s1) MTXBIN_STRINGNAME ("mtx_",s1)
#define MTXBIN_SHORTNAME MTXBIN_SHORTNAME2 (MTXBIN_GENERIC__OPERATOR)

#define MTXBIN_HELPNAME2(s1) MTXBIN_STRINGNAME("",s1)
#ifdef MTXBIN_HELPNAME
# define MTXBIN_REALHELPNAME MTXBIN_HELPNAME2(MTXBIN_HELPNAME)
#else
# define MTXBIN_REALHELPNAME MTXBIN_HELPNAME2(MTXBIN_GENERIC__NAME)
#endif

#define MTXBIN_IEMSETUP_EXPAND(s1) ie ## s1 ## _setup
#define MTXBIN_IEMSETUP(s1)  MTXBIN_IEMSETUP_EXPAND (s1)


static t_class *MTXBIN_APPEND(_class ), *MTXBIN_APPEND(_scalarclass);

static void mtxbin_scalar_matrix (t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  int row=atom_getint(argv), col=atom_getint(argv+1);
#ifdef MTXBIN_GENERIC__INTEGEROP
  t_int offset=(t_int)x->f;
#else
  t_float offset=x->f;
#endif
  t_atom *buf;
  t_atom *ap=argv+2;

  if(argc<2){post( MTXBIN_SHORTNAME ": crippled matrix");return; }
  adjustsize(&x->m, row, col);

  buf=x->m.atombuffer+2;
  while(n--){
#ifdef MTXBIN_GENERIC__INTEGEROP
    buf->a_w.w_float = atom_getint(ap) MTXBIN_GENERIC__OPERATOR offset;
#else
    buf->a_w.w_float = atom_getfloat(ap) MTXBIN_GENERIC__OPERATOR offset;
#endif
    buf->a_type = A_FLOAT;
    buf++; ap++;
  }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtxbin_scalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
#ifdef MTXBIN_GENERIC__INTEGEROP
  t_int offset=x->f;
#else
  t_float offset=x->f;
#endif
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
#ifdef MTXBIN_GENERIC__INTEGEROP
    (m++)->a_w.w_float = atom_getint(argv++) MTXBIN_GENERIC__OPERATOR offset;
#else
    (m++)->a_w.w_float = atom_getfloat(argv++) MTXBIN_GENERIC__OPERATOR offset;
#endif
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void mtxbin_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getint(argv);
  int col=atom_getint(argv+1);
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post( MTXBIN_SHORTNAME ": crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post( MTXBIN_SHORTNAME ": invalid dimensions");    return;  }
  if (col*row>argc-2){    post( MTXBIN_SHORTNAME ":sparse matrix not yet suppandted : use \"mtx_check\"");    return;  }

  if (!(x->m2.col*x->m2.row)) {
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, argv);
    return;
  }

  if(x->m2.col==1&&x->m2.row==1)
    {
#ifdef MTXBIN_GENERIC__INTEGEROP
      t_int offset=atom_getint(m2);
#else
      t_float offset=atom_getfloat(m2);
#endif
      adjustsize(&x->m, row, col);
      m = x->m.atombuffer+2;

      while(n--){
#ifdef MTXBIN_GENERIC__INTEGEROP
        t_float f = (t_float)(atom_getint(m1) MTXBIN_GENERIC__OPERATOR offset);
#else
        t_float f = atom_getfloat(m1) MTXBIN_GENERIC__OPERATOR offset;
#endif
        SETFLOAT(m, f);
        m++; m1++;
      }
      outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
      return;
    }
  if(x->m2.row==1)
    {
      int c, r;
      adjustsize(&x->m, row, col);
      m = x->m.atombuffer+2; 
      for(r=0; r<row; r++){
        m2 = x->m2.atombuffer+2;
        for(c=0; c<col; c++){
#ifdef MTXBIN_GENERIC__INTEGEROP
          t_float f = (t_float)(atom_getint(m1) MTXBIN_GENERIC__OPERATOR atom_getint(m2));
#else
          t_float f = atom_getfloat(m1) MTXBIN_GENERIC__OPERATOR atom_getfloat(m2);
#endif
          m1++; m2++;
          SETFLOAT(m, f);
          m++;
        }
      }
      outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
      return;
    }
  if(x->m2.col==1)
    {
      int c, r;
      adjustsize(&x->m, row, col);
      m = x->m.atombuffer+2;

      for(r=0; r<row; r++){
#ifdef MTXBIN_GENERIC__INTEGEROP
        t_int offset = atom_getint(m2);
#else
        t_float offset = atom_getfloat(m2);
#endif

        for(c=0; c<col; c++){
#ifdef MTXBIN_GENERIC__INTEGEROP
          t_float f = (t_float)(atom_getint(m1) MTXBIN_GENERIC__OPERATOR offset);
#else
          t_float f = atom_getfloat(m1) MTXBIN_GENERIC__OPERATOR offset;
#endif
          SETFLOAT(m, f);
          m++; m1++;
        }
        m2++;
      }

      outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
      return;
    }


  if ((col!=x->m2.col)||(row!=x->m2.row)){ 
    post( MTXBIN_SHORTNAME ": matrix dimensions do not match");
    /* LATER SOLVE THIS */    
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
#ifdef MTXBIN_GENERIC__INTEGEROP
    t_float f = (t_float)(atom_getint(m1) MTXBIN_GENERIC__OPERATOR atom_getint(m2));
#else
    t_float f = atom_getfloat(m1) MTXBIN_GENERIC__OPERATOR atom_getfloat(m2);
#endif
    m1++; m2++;
    SETFLOAT(m, f);
    m++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtxbin_float(t_mtx_binmtx *x, t_float fval)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

#ifdef MTXBIN_GENERIC__INTEGEROP
  t_int offset=(t_int)fval;
#else
  t_float offset=fval;
#endif


  if (!m2->atombuffer){ post( MTXBIN_SHORTNAME ": operate on what ?");            return; }

  row2=atom_getint(m2->atombuffer);
  col2=atom_getint(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;
  while(n--){
    t_float f = 
#ifdef MTXBIN_GENERIC__INTEGEROP
                offset MTXBIN_GENERIC__OPERATOR atom_getint(ap2);
#else
                offset MTXBIN_GENERIC__OPERATOR atom_getfloat(ap2);
#endif
    SETFLOAT(ap, f);
    ap++; ap2++;
  }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void *mtxbin_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post( MTXBIN_SHORTNAME " : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(MTXBIN_APPEND(_scalarclass));
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(MTXBIN_APPEND(_class));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
  }
}

void MTXBIN_APPEND(_setup) (void)
{
  MTXBIN_APPEND(_class) = class_new(gensym(MTXBIN_LONGNAME), 
                                     (t_newmethod)mtxbin_new, 
                                     (t_method)mtx_binmtx_free,
                                     sizeof(t_mtx_binmtx), 0, A_GIMME, 0);

  class_addmethod(MTXBIN_APPEND(_class), (t_method)mtxbin_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(MTXBIN_APPEND(_class), (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (MTXBIN_APPEND(_class), mtxbin_float);
  class_addbang  (MTXBIN_APPEND(_class), mtx_binmtx_bang);

  MTXBIN_APPEND(_scalarclass) = class_new(gensym(MTXBIN_LONGNAME), 0, (t_method)mtx_binscalar_free,
                                  sizeof(t_mtx_binscalar), 0, 0);
  class_addmethod(MTXBIN_APPEND(_scalarclass),
                  (t_method)mtxbin_scalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (MTXBIN_APPEND(_scalarclass), mtxbin_scalar_list);
  class_addbang  (MTXBIN_APPEND(_scalarclass), mtx_binscalar_bang);

  class_sethelpsymbol(MTXBIN_APPEND(_class), gensym(MTXBIN_REALHELPNAME));
  class_sethelpsymbol(MTXBIN_APPEND(_scalarclass), gensym(MTXBIN_REALHELPNAME));

  class_addcreator((t_newmethod)mtxbin_new, gensym(MTXBIN_SHORTNAME), A_GIMME,0);
}

void MTXBIN_IEMSETUP(MTXBIN_GENERIC__NAME)  (void)
{
  MTXBIN_APPEND(_setup());
}
#else
# error no operation defined
#endif /* MTXBIN_GENERIC__NAME && MTXBIN_GENERIC__OPERATOR */
