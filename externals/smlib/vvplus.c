#include "defines.h"

/*--------------- vvplus ---------------*/
/* vector addition
*/

static t_class *vvplus_class;
static t_class *vvplus_scal_class;

typedef struct _vvplus
{
  t_object x_obj;

  t_int n1, n2;

  t_float *buf1, *buf2;

  t_float f;
} t_vvplus;


static void vvplus_lst2(t_vvplus *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float *fp;
  if (x->n2 != argc) {
    freebytes(x->buf2, x->n2 * sizeof(t_float));
    x->n2 = argc;
    x->buf2=(t_float *)getbytes(sizeof(t_float)*x->n2);
  };
  fp = x->buf2;
  while(argc--)*fp++=atom_getfloat(argv++);
}

static void vvplus_lst(t_vvplus *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float *fp;
  t_atom  *ap;
  int n;

  if (argc){
    if (x->n1 != argc) {
      freebytes(x->buf1, x->n1 * sizeof(t_float));
      x->n1 = argc;
      x->buf1=(t_float *)getbytes(sizeof(t_float)*x->n1);
    };
    fp = x->buf1;
    while(argc--)*fp++=atom_getfloat(argv++);
  }

  if (x->n1*x->n2==1){
    outlet_float(x->x_obj.ob_outlet, *x->buf1+*x->buf2);
    return;
  }
  if (x->n1==1){
    t_atom *a;
    int i = x->n2;
    t_float f = *x->buf1;
    fp = x->buf2;
    n = x->n2;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    while(i--){
      SETFLOAT(a, *fp+++f);
      a++;
    }
  } else if (x->n2==1){
    t_float f = *x->buf2;
    t_atom *a;
    int i = x->n1;
    n = x->n1;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    fp = x->buf1;
    while(i--){
      SETFLOAT(a, *fp+++f);
      a++;
    }
  } else {
    t_atom *a;
    int i;
    t_float *fp2=x->buf2;
    fp = x->buf1;
    n = x->n1;
    if (x->n1!=x->n2){
      post("scalar multiplication: truncating vectors to the same length");
      if (x->n2<x->n1)n=x->n2;
    }
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    i=n;
    while(i--){
      SETFLOAT(a, *fp+++*fp2++);
      a++;
    }
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), n, ap);
  freebytes(ap, sizeof(t_atom)*n);
}
static void vvplus_free(t_vvplus *x)
{
  freebytes(x->buf1, sizeof(t_float)*x->n1);
  freebytes(x->buf2, sizeof(t_float)*x->n2);
}

static void *vvplus_new(t_symbol *s, int argc, t_atom *argv)
{
  t_vvplus *x;

  if (argc-1){
    x = (t_vvplus *)pd_new(vvplus_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  } else x = (t_vvplus *)pd_new(vvplus_scal_class);

  outlet_new(&x->x_obj, 0);

  x->n1   =1;
  x->buf1 =(t_float*)getbytes(sizeof(t_float));
  *x->buf1=0;

  if (argc)vvplus_lst2(x, gensym("list"), argc, argv);
  else {
    x->n2   =1;
    x->buf2 =(t_float*)getbytes(sizeof(t_float));
    *x->buf2=0;
  }

  if (argc==1)floatinlet_new(&x->x_obj, x->buf2);

  return (x);
}

void vvplus_setup(void)
{
  vvplus_class = class_new(gensym("vvplus"), (t_newmethod)vvplus_new, 
			    (t_method)vvplus_free, sizeof(t_vvplus), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)vvplus_new, gensym("vv+"), A_GIMME, 0);
  class_addlist(vvplus_class, vvplus_lst);
  class_addmethod  (vvplus_class, (t_method)vvplus_lst2, gensym(""), A_GIMME, 0);
  vvplus_scal_class = class_new(gensym("vv+"), 0, (t_method)vvplus_free, 
				 sizeof(t_vvplus), 0, 0);
  class_addlist(vvplus_scal_class, vvplus_lst);
}
