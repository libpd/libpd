#include "defines.h"

/*--------------- lmin ---------------*/

static t_class *lmin_class;

typedef struct _lmin
{
    t_object x_obj;
	t_float m_min;
	t_float m_leak;
	t_float m_c_leak;
} t_lmin;


static void lmin_perform(t_lmin *x, t_float in)
{
	x->m_min=(in < x->m_min) ? in : x->m_min * x->m_c_leak + in * x->m_leak;
    outlet_float(x->x_obj.ob_outlet, x->m_min);
}

static void lmin_setHalfDecay(t_lmin *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0f/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void lmin_clear(t_lmin *x)
{
	x->m_min= MAXFLOAT;
}

static void *lmin_new( t_float halfDecayTime)
{
	t_lmin *x=(t_lmin *)pd_new(lmin_class);
	outlet_new(&x->x_obj, gensym("float"));

	lmin_setHalfDecay(x, halfDecayTime);
	lmin_clear(x);
	return (void *)x;
}


void lmin_setup(void)
{
    lmin_class = class_new(gensym("lmin"),
    	(t_newmethod)lmin_new, 0,
		sizeof(t_lmin), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
    class_addfloat(lmin_class, (t_method)lmin_perform);
	class_addmethod(lmin_class, (t_method)lmin_clear,
    	gensym("clear"), A_GIMME, NULL);
	class_addmethod(lmin_class, (t_method)lmin_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
}

