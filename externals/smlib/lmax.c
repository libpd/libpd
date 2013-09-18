#include "defines.h"

/*--------------- lmax ---------------*/

static t_class *lmax_class;

typedef struct _lmax
{
    t_object x_obj;
	t_float m_max;
	t_float m_leak;
	t_float m_c_leak;
} t_lmax;


static void lmax_perform(t_lmax *x, t_float in)
{
	x->m_max=(in > x->m_max) ? in : x->m_max * x->m_c_leak + in * x->m_leak;
    outlet_float(x->x_obj.ob_outlet, x->m_max);
}

static void lmax_setHalfDecay(t_lmax *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0/halfDecayTime));
	x->m_leak=1.0-x->m_c_leak;
}

static void lmax_clear(t_lmax *x)
{
	x->m_max= - MAXFLOAT;
}

static void *lmax_new( t_float halfDecayTime)
{
	t_lmax *x=(t_lmax *)pd_new(lmax_class);
	outlet_new(&x->x_obj, gensym("float"));

	lmax_setHalfDecay(x, halfDecayTime);
	lmax_clear(x);
	return (void *)x;
}


void lmax_setup(void)
{
    lmax_class = class_new(gensym("lmax"),
    	(t_newmethod)lmax_new, 0,
		sizeof(t_lmax), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
    class_addfloat(lmax_class, (t_method)lmax_perform);
	class_addmethod(lmax_class, (t_method)lmax_clear,
    	gensym("clear"), A_GIMME, NULL);
	class_addmethod(lmax_class, (t_method)lmax_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
}

