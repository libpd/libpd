#include "defines.h"

/*--------------- lavg ---------------*/

static t_class *lavg_class;

typedef struct _lavg
{
    t_object x_obj;
	t_float m_avg;
	t_float m_c_leak;
	t_float m_leak;
} t_lavg;


static void lavg_perform(t_lavg *x, t_float in)
{
	x->m_avg= x->m_avg * x->m_c_leak + in * x->m_leak;
    outlet_float(x->x_obj.ob_outlet, x->m_avg);
}

static void lavg_clear(t_lavg *x)
{
	x->m_avg=0.0;
}

static void lavg_setHalfDecay(t_lavg *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0/halfDecayTime));
	x->m_leak=1.0-x->m_c_leak;
}

static void *lavg_new( t_float halfDecayTime)
{
	t_lavg *x=(t_lavg *)pd_new(lavg_class);
	outlet_new(&x->x_obj, gensym("float"));

	lavg_setHalfDecay(x, halfDecayTime);
	return (void *)x;
}

void lavg_setup(void)
{
    lavg_class = class_new(gensym("lavg"),
    	(t_newmethod)lavg_new, 0,
		sizeof(t_lavg), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
    class_addfloat(lavg_class, (t_method)lavg_perform);
	class_addmethod(lavg_class, (t_method)lavg_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
	class_addmethod(lavg_class, (t_method)lavg_clear,
    	gensym("clear"), 0);
}

