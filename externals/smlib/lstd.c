#include "defines.h"

/*--------------- lstd ---------------*/

static t_class *lstd_class;

typedef struct _lstd
{
    t_object x_obj;
	t_float m_avg;
	t_float m_sum_squares;
	t_float m_std;
	t_float m_c_leak;
	t_float m_leak;
} t_lstd;


static void lstd_perform(t_lstd *x, t_float in)
{
	t_float tmp=x->m_avg-in;
	x->m_avg= x->m_avg * x->m_c_leak + in * x->m_leak;
	x->m_sum_squares=x->m_sum_squares * x->m_c_leak + x->m_leak*tmp*tmp;
	x->m_std=(t_float)sqrtf(x->m_sum_squares);
    outlet_float(x->x_obj.ob_outlet, x->m_std);
}

static void lstd_setHalfDecay(t_lstd *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0f/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void lstd_clear(t_lstd *x)
{
	x->m_sum_squares=0.0f;
	x->m_avg=0.0f;
}

static void *lstd_new( t_float halfDecayTime)
{
	t_lstd *x=(t_lstd *)pd_new(lstd_class);
	outlet_new(&x->x_obj, gensym("float"));

	lstd_setHalfDecay(x, halfDecayTime);
	return (void *)x;
}


void lstd_setup(void)
{
    lstd_class = class_new(gensym("lstd"),
    	(t_newmethod)lstd_new, 0,
		sizeof(t_lstd), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
    class_addfloat(lstd_class, (t_method)lstd_perform);
	class_addmethod(lstd_class, (t_method)lstd_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
}

