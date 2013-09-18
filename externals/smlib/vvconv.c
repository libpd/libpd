#include "defines.h"

/*--------------- vvconv ---------------*/
/* vector convolution
*/

static t_class *vvconv_class;
static t_class *vvconv_scal_class;

typedef struct _vvconv
{
  t_object x_obj;

  t_int n1, n2;

  t_float *buf1, *buf2;

  t_float f;
} t_vvconv;


static void vvconv_lst2(t_vvconv *x, t_symbol *s, int argc, t_atom *argv)
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

static void vvconv_lst(t_vvconv *x, t_symbol *s, int argc, t_atom *argv)
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
    outlet_float(x->x_obj.ob_outlet, *x->buf1**x->buf2);
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
      SETFLOAT(a, *fp++*f);
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
      SETFLOAT(a, *fp++*f);
      a++;
    }
  } else {
    t_atom *a;
    int i,j,n1,n2;
	t_float *f;
	t_float *g;
    t_float *fp2=x->buf2;
    fp = x->buf1;
	n1=x->n1;
	n2=x->n2;
    n = n1 + n2 - 1;
    f = (t_float *)getbytes(sizeof(t_float)*n);
	g=f;
	for(i=0;i<n;i++)
		*g++=0.0f;
	for(i=0;i<n1;i++)
		for(j=0;j<n2;j++)
			f[i+j]+=fp[i]*fp2[j];

	g=f;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    i=n;
    while(i--){
      SETFLOAT(a, *g++);
      a++;
    }
    freebytes(f,sizeof(t_float)*n);
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), n, ap);
  freebytes(ap, sizeof(t_atom)*n);
}
static void vvconv_free(t_vvconv *x)
{
  freebytes(x->buf1, sizeof(t_float)*x->n1);
  freebytes(x->buf2, sizeof(t_float)*x->n2);
}

static void *vvconv_new(t_symbol *s, int argc, t_atom *argv)
{
  t_vvconv *x;

  if (argc-1){
    x = (t_vvconv *)pd_new(vvconv_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  } else x = (t_vvconv *)pd_new(vvconv_scal_class);

  outlet_new(&x->x_obj, 0);

  x->n1   =1;
  x->buf1 =(t_float*)getbytes(sizeof(t_float));
  *x->buf1=0;

  if (argc)vvconv_lst2(x, gensym("list"), argc, argv);
  else {
    x->n2   =1;
    x->buf2 =(t_float*)getbytes(sizeof(t_float));
    *x->buf2=0;
  }

  if (argc==1)floatinlet_new(&x->x_obj, x->buf2);

  return (x);
}

void vvconv_setup(void)
{
  vvconv_class = class_new(gensym("vvconv"), (t_newmethod)vvconv_new, 
			    (t_method)vvconv_free, sizeof(t_vvconv), 0, A_GIMME, 0);
  class_addlist(vvconv_class, vvconv_lst);
  class_addmethod  (vvconv_class, (t_method)vvconv_lst2, gensym(""), A_GIMME, 0);
  vvconv_scal_class = class_new(gensym("vv+"), 0, (t_method)vvconv_free, 
				 sizeof(t_vvconv), 0, 0);
  class_addlist(vvconv_scal_class, vvconv_lst);
}
