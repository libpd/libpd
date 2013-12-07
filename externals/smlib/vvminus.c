#include "defines.h"

/*--------------- vvminus ---------------*/
/* vector substraction
*/

static t_class *vvminus_class;
static t_class *vvminus_scal_class;

typedef struct _vvminus
{
  t_object x_obj;

  t_int n1, n2;

  t_float *buf1, *buf2;

  t_float f;
} t_vvminus;


static void vvminus_lst2(t_vvminus *x, t_symbol *s, int argc, t_atom *argv)
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

static void vvminus_lst(t_vvminus *x, t_symbol *s, int argc, t_atom *argv)
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
    outlet_float(x->x_obj.ob_outlet, *x->buf1-*x->buf2);
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
      SETFLOAT(a, f-*fp++);
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
      SETFLOAT(a, *fp++-f);
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
      SETFLOAT(a, *fp++-*fp2++);
      a++;
    }
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), n, ap);
  freebytes(ap, sizeof(t_atom)*n);
}
static void vvminus_free(t_vvminus *x)
{
  freebytes(x->buf1, sizeof(t_float)*x->n1);
  freebytes(x->buf2, sizeof(t_float)*x->n2);
}

static void *vvminus_new(t_symbol *s, int argc, t_atom *argv)
{
  t_vvminus *x;

  if (argc-1){
    x = (t_vvminus *)pd_new(vvminus_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  } else x = (t_vvminus *)pd_new(vvminus_scal_class);

  outlet_new(&x->x_obj, 0);

  x->n1   =1;
  x->buf1 =(t_float*)getbytes(sizeof(t_float));
  *x->buf1=0;

  if (argc)vvminus_lst2(x, gensym("list"), argc, argv);
  else {
    x->n2   =1;
    x->buf2 =(t_float*)getbytes(sizeof(t_float));
    *x->buf2=0;
  }

  if (argc==1)floatinlet_new(&x->x_obj, x->buf2);

  return (x);
}

void vvminus_setup(void)
{
  vvminus_class = class_new(gensym("vvminus"), (t_newmethod)vvminus_new, 
			    (t_method)vvminus_free, sizeof(t_vvminus), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)vvminus_new, gensym("vv-"), A_GIMME, 0);
  class_addlist(vvminus_class, vvminus_lst);
  class_addmethod  (vvminus_class, (t_method)vvminus_lst2, gensym(""), A_GIMME, 0);
  vvminus_scal_class = class_new(gensym("vv-"), 0, (t_method)vvminus_free, 
				 sizeof(t_vvminus), 0, 0);
  class_addlist(vvminus_scal_class, vvminus_lst);
}
