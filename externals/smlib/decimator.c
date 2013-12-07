#include "defines.h"

/*--------------- decimator ---------------*/

static t_class *decimator_class;

typedef struct _decimator
{
    t_object x_obj;
	int m_state;
	int m_factor;
} t_decimator;

static void decimator_perform(t_decimator *x, t_float in)
{
	if (!x->m_state)
	{
		outlet_float(x->x_obj.ob_outlet, in);
		x->m_state=x->m_factor;
	}
	else
	{
		x->m_state--;
	}

}

static void decimator_setFactor(t_decimator *x, t_float factor)
{
	x->m_factor=(int)factor - 1;
}

static void decimator_clear(t_decimator *x)
{
	x->m_state=0;
}


static void *decimator_new(t_float factor)
{

	t_decimator *x=(t_decimator *)pd_new(decimator_class);
	outlet_new(&x->x_obj, gensym("float"));

	if (factor<1) factor=2;

	decimator_setFactor(x, factor);
	decimator_clear(x);
	return (void *)x;
}


void decimator_setup(void)
{
    decimator_class = class_new(gensym("decimator"),
    	(t_newmethod)decimator_new, 0,
		sizeof(t_decimator), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
    class_addfloat(decimator_class, (t_method)decimator_perform);
	class_addmethod(decimator_class, (t_method)decimator_clear,
    	gensym("clear"), A_GIMME, NULL);
	class_addmethod(decimator_class, (t_method)decimator_setFactor,
    	gensym("factor"), A_DEFFLOAT, NULL);
}

