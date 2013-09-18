/* bangfilter: = % x plus sel 0
 * spaeter: % x plus sel y mit 2 arguments
 */
#include "m_pd.h"
#include <math.h>

static t_class *bfilt_class;

typedef struct _bfilt
{
    t_object x_obj;
    t_float x_f1;
    t_float x_f2;
} t_bfilt;

static void *bfilt_new(t_floatarg f)
{
    t_bfilt *x = (t_bfilt *)pd_new(bfilt_class);
    outlet_new(&x->x_obj, &s_bang);
    floatinlet_new(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}

static void bfilt_bang(t_bfilt *x)
{
    int n2 = x->x_f2, result;
    if (n2 < 0) n2 = -n2;
    else if (!n2) n2 = 1;
    result = ((int)(x->x_f1)) % n2;
    if (result == 0) //result += n2;
      {
	outlet_bang(x->x_obj.ob_outlet);
      }
	//outlet_float(x->x_obj.ob_outlet, (t_float)result);
}

static void bfilt_float(t_bfilt *x, t_float f)
{
    x->x_f1 = f;
    bfilt_bang(x);
}


void bfilt_setup()
{
    bfilt_class = class_new(gensym("bfilt"), (t_newmethod)bfilt_new, 0,
    	sizeof(t_bfilt), 0, A_DEFFLOAT, 0);
    class_addbang(bfilt_class, bfilt_bang);
    class_addfloat(bfilt_class, (t_method)bfilt_float);
}
