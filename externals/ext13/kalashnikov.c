#include "ext13.h"
#include "m_pd.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* -------------------------- kalashnikov ------------------------------ */
static t_class *kalashnikov_class;

typedef struct _kalashnikov
{
    t_object x_obj;
    t_float x_f;
    t_outlet *x_out2;
    t_outlet *x_out3;
} t_kalashnikov;

static void *kalashnikov_new(t_floatarg f)
{
    t_kalashnikov *x = (t_kalashnikov *)pd_new(kalashnikov_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_bang);
    x->x_out3 = outlet_new(&x->x_obj, &s_float);     
    x->x_out2 = outlet_new(&x->x_obj, &s_bang);
    floatinlet_new(&x->x_obj, &x->x_f);
    return (x);
}

static void kalashnikov_bang(t_kalashnikov *x)
{
  int count;
    for (count=x->x_f;count>0;count--){       
       outlet_bang(x->x_obj.ob_outlet);
       outlet_float(x->x_out3, x->x_f-count);
  }
  outlet_bang(x->x_out2);
}

static void kalashnikov_float(t_kalashnikov *x, t_float f)
{
  int count;
  x->x_f=f;
  for (count=f;count>0;count--){
      outlet_bang(x->x_obj.ob_outlet);
      outlet_float(x->x_out3, f-count);      
  }
  outlet_bang(x->x_out2);
}

void kalashnikov_setup(void)
{
    kalashnikov_class = class_new(gensym("kalashnikov"), (t_newmethod)kalashnikov_new, 0,
    	sizeof(t_kalashnikov), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)kalashnikov_new, gensym("uzi"), A_DEFFLOAT, 0);
    class_addbang(kalashnikov_class, kalashnikov_bang);
    class_addfloat(kalashnikov_class, kalashnikov_float);
}
