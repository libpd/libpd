#include "m_pd.h"
#include <math.h>

static t_class *binshift_class;

typedef struct _binshift
{
  t_object x_obj;
  t_float x_f1;
  t_float x_f2;
  t_int x_t1;
  t_int x_t2;
} t_binshift;

/* ------------------ binop1:  +, -, *, / ----------------------------- */

static void *binshift1_new(t_class *floatclass, t_floatarg f)
{
    t_binshift *x = (t_binshift *)pd_new(floatclass);
    outlet_new(&x->x_obj, &s_float);
    floatinlet_new(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}


/* --------------------- rightshift ------------------------------- */

static t_class *binshift1_rshift_class;

static void *binshift1_rshift_new(t_floatarg f)
{
  return (binshift1_new(binshift1_rshift_class, f));
}

static void binshift1_rshift_bang(t_binshift *x)
{
  outlet_float(x->x_obj.ob_outlet, ((int)(x->x_f1) >> (int)(x->x_f2)));
}

static void binshift1_rshift_float(t_binshift *x, t_float f)
{
  outlet_float(x->x_obj.ob_outlet, ((int)(x->x_f1 = f) >> (int)(x->x_f2)));
}

/* --------------------- leftshift ------------------------------- */

static t_class *binshift1_lshift_class;

static void *binshift1_lshift_new(t_floatarg f)
{
  return (binshift1_new(binshift1_lshift_class, f));
}

static void binshift1_lshift_bang(t_binshift *x)
{
  outlet_float(x->x_obj.ob_outlet, ((int)(x->x_f1) << (int)(x->x_f2)));
}

static void binshift1_lshift_float(t_binshift *x, t_float f)
{
  outlet_float(x->x_obj.ob_outlet, ((int)(x->x_f1 = f) << (int)(x->x_f2)));
}

/* setup */

void binshift_setup(void)
{
  // post("binary shift");

    binshift1_rshift_class = class_new(gensym(">>"), (t_newmethod)binshift1_rshift_new, 0,
    	sizeof(t_binshift), 0, A_DEFFLOAT, 0);
    class_addbang(binshift1_rshift_class, binshift1_rshift_bang);
    class_addfloat(binshift1_rshift_class, (t_method)binshift1_rshift_float);

    binshift1_lshift_class = class_new(gensym("<<"), (t_newmethod)binshift1_lshift_new, 0,
    	sizeof(t_binshift), 0, A_DEFFLOAT, 0);
    class_addbang(binshift1_lshift_class, binshift1_lshift_bang);
    class_addfloat(binshift1_lshift_class, (t_method)binshift1_lshift_float);
}
