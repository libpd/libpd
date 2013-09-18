#include "defines.h"

/*--------------- vrms ---------------*/

static t_class *vrms_class;

typedef struct _vrms
{
    t_object x_obj;
} t_vrms;


static void vrms_perform(t_vrms *x, t_symbol *s, int argc, t_atom *argv)
{
	t_float sum=0.;
	int i;
	for (i = 0; i < argc; i++)
	{
		t_float tmp=atom_getfloat(&argv[i]);
		sum+= tmp*tmp;
	}
	outlet_float(x->x_obj.ob_outlet, (t_float)sqrtf(sum/argc));
}

static void *vrms_new( t_float halfDecayTime)
{
	t_vrms *x=(t_vrms *)pd_new(vrms_class);
	outlet_new(&x->x_obj, gensym("float"));
	return (void *)x;
}

void vrms_setup(void)
{
    vrms_class = class_new(gensym("vrms"),
    	(t_newmethod)vrms_new, 0,
		sizeof(t_vrms), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vrms_class, (t_method)vrms_perform);
}

