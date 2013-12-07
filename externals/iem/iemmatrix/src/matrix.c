/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) IOhannes m zmölnig, forum::für::umläute
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

/*
  matrix : basic object : create and store matrices
  mtx    : alias for matrix
*/

#include "iemmatrix.h"
#include <stdio.h>
#ifdef _WIN32
/* or should we use the security enhanced _snprintf_s() ?? */
# define snprintf _snprintf
#endif


/* -------------------- matrix ------------------------------ */
/*
  G.Holzmann: see iemmatrix_utility.c for the functions
              you don't find here ... ;)
*/

static t_class *matrix_class;

static void matrix_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;

  if (argc<2){
    post("matrix : corrupt matrix passed");
    return;
  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){
    post("matrix : corrupt matrix passed");
    return;
  }
  if (row*col > argc-2){
    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }

  matrix_matrix2(x, s, argc, argv);
  matrix_bang(x);
}

static void matrix_float(t_matrix *x, t_float f)
{
  matrix_set(x, f);
  matrix_bang(x);
}

static void matrix_size(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;

  switch(argc) {
  case 0: /* size */
    if (x->row*x->col)
       outlet_list(x->x_obj.ob_outlet, gensym("size"), 2, x->atombuffer);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }
}

/* ------------- file I/O ------------------ */

static void matrix_read(t_matrix *x, t_symbol *filename)
{
  t_binbuf *bbuf = binbuf_new();
  t_atom *ap;
  int n;

  if (binbuf_read_via_path(bbuf, filename->s_name, canvas_getdir(x->x_canvas)->s_name, 0))
    pd_error(x,"matrix: failed to read %128s", filename->s_name);

  ap=binbuf_getvec(bbuf);
  n =binbuf_getnatom(bbuf)-1;
  
  if ((ap->a_type == A_SYMBOL) && 
      (!strcmp(ap->a_w.w_symbol->s_name,"matrix") || !strcmp(ap->a_w.w_symbol->s_name,"#matrix")) ){
    matrix_matrix2(x, gensym("matrix"), n, ap+1);
  }

  binbuf_free(bbuf);
}

static void matrix_write(t_matrix *x, t_symbol *filename)
{
  t_atom *ap=x->atombuffer+2;
  int rows = x->row, cols = x->col;
  FILE *f=0;

  /* open file */
  if (!(f = sys_fopen(filename->s_name, "w"))) {
    pd_error(x,"matrix : failed to open %128s", filename->s_name);
  } else {
    char *text=(char *)getbytes(sizeof(char)*MAXPDSTRING);
    int textlen;

    /* header:
     * we now write "#matrix" instead of "matrix",
     * so that these files can easily read by other 
     * applications such as octave
     */
    snprintf(text, MAXPDSTRING, "#matrix %d %d\n", rows, cols);
    text[MAXPDSTRING-1]=0;
    textlen = strlen(text);
    if (fwrite(text, textlen*sizeof(char), 1, f) < 1) {
      pd_error(x,"matrix : failed to write %128s", filename->s_name); goto end;
    }

    while(rows--) {
      int c = cols;
      while (c--) {
	t_float val = atom_getfloat(ap++);
	snprintf(text, MAXPDSTRING, "%.15f ", val);
        text[MAXPDSTRING-1]=0;
	textlen=strlen(text);
	if (fwrite(text, textlen*sizeof(char), 1, f) < 1) {
	  pd_error(x,"matrix : failed to write %128s", filename->s_name); goto end;
	}
      }
      if (fwrite("\n", sizeof(char), 1, f) < 1) {
	pd_error(x, "matrix : failed to write %128s", filename->s_name); goto end;
      }
    }
    freebytes(text, sizeof(char)*MAXPDSTRING);
  }

 end:
  /* close file */
  if (f) fclose(f);
}

static void matrix_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* like matrix, but without col/row information, so the previous size is kept */
  int row=x->row, col=x->col;

  if(!row*col){
    post("matrix : unknown matrix dimensions");
    return;
  }
  if (argc<row*col){
    post("matrix: sparse matrices not yet supported : use \"mtx_check\" !");
    return;
  }
  
  memcpy(x->atombuffer+2, argv, row*col*sizeof(t_atom));
  matrix_bang(x);
}

static void *matrix_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(matrix_class);
  int row, col;


  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
  outlet_new(&x->x_obj, 0);

  x->atombuffer   = 0;
  x->x_canvas = canvas_getcurrent();

  switch (argc) {
  case 0:
    row = col = 0;
    break;
  case 1:
    if (argv->a_type == A_SYMBOL) {
      matrix_read(x, argv->a_w.w_symbol);
      return(x);
    }
    row = col = atom_getfloat(argv);
    break;
  default:
    row = atom_getfloat(argv++);
    col = atom_getfloat(argv++);
  }

  if(row*col){
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }

  return (x);
}

void matrix_setup(void)
{
  matrix_class = class_new(gensym("matrix"), (t_newmethod)matrix_new, 
			   (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)matrix_new, gensym("mtx"), A_GIMME, 0);
  class_addcreator((t_newmethod)matrix_new, gensym("iemmatrix"), A_GIMME, 0);

  /* the core : functions for matrices */
  class_addmethod  (matrix_class, (t_method)matrix_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_matrix2, gensym(""), A_GIMME, 0);
 
  /* the basics : functions for creation */
  class_addmethod  (matrix_class, (t_method)matrix_size, gensym("size"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_eye, gensym("eye"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_diag, gensym("diag"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_ones, gensym("ones"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_zeros, gensym("zeros"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_egg, gensym("egg"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_diegg, gensym("diegg"), A_GIMME, 0);
 
  /* the rest : functions for anything */
  class_addbang    (matrix_class, matrix_bang);
  class_addfloat   (matrix_class, matrix_float);
  class_addlist    (matrix_class, matrix_list);
  class_addmethod  (matrix_class, (t_method)matrix_row, gensym("row"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_col, gensym("column"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_col, gensym("col"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_element, gensym("element"), A_GIMME, 0);

  /* the file functions */
  class_addmethod  (matrix_class, (t_method)matrix_write, gensym("write"), A_SYMBOL, 0);
  class_addmethod  (matrix_class, (t_method)matrix_read , gensym("read") , A_SYMBOL, 0);
}

void iemtx_matrix_setup(void){
  matrix_setup();
}

void mtx_matrix_setup(void){
  matrix_setup();
}

void iematrix_setup(void){
  matrix_setup();
}
