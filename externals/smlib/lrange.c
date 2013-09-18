#include "defines.h"

/*--------------- lrange ---------------*/

static t_class *lrange_class;

typedef struct _lrange
{
    t_object x_obj;
	t_float m_min;
	t_float m_max;
	t_float m_c_leak;
	t_float m_leak;
} t_lrange;


static void lrange_perform(t_lrange *x, t_float in)
{
	x->m_max=(in > x->m_max) ? in : x->m_max * x->m_c_leak + in * x->m_leak;
	x->m_min=(in < x->m_min) ? in : x->m_min * x->m_c_leak + in * x->m_leak;
    outlet_float(x->x_obj.ob_outlet, x->m_max-x->m_min);
}

static void lrange_setHalfDecay(t_lrange *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0f/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void lrange_clear(t_lrange *x)
{
	x->m_max = - MAXFLOAT;
	x->m_min = MAXFLOAT;
}

static void *lrange_new( t_float halfDecayTime)
{
	t_lrange *x=(t_lrange *)pd_new(lrange_class);
	outlet_new(&x->x_obj, gensym("float"));

	lrange_setHalfDecay(x, halfDecayTime);
	lrange_clear(x);
	return (void *)x;
}


void lrange_setup(void)
{
    lrange_class = class_new(gensym("lrange"),
    	(t_newmethod)lrange_new, 0,
		sizeof(t_lrange), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
    class_addfloat(lrange_class, (t_method)lrange_perform);
	class_addmethod(lrange_class, (t_method)lrange_clear,
    	gensym("clear"), A_GIMME, NULL);
	class_addmethod(lrange_class, (t_method)lrange_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
}

