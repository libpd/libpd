#include "defines.h"

/*--------------- vstd ---------------*/

static t_class *vstd_class;

typedef struct _vstd
{
    t_object x_obj;
} t_vstd;


static void vstd_perform(t_vstd *x, t_symbol *s, int argc, t_atom *argv)
{
	t_float sumsq=0.0f;
	t_float sum=0.0f;
	int i;
	for (i = 0; i < argc; i++)
	{
		t_float tmp=atom_getfloat(&argv[i]);
		sumsq+= tmp*tmp;
		sum+=tmp;
	}
	sumsq/=argc;
	sum/=argc;
	outlet_float(x->x_obj.ob_outlet, (t_float)sqrtf(sumsq-sum*sum));
}

static void *vstd_new( t_float halfDecayTime)
{
	t_vstd *x=(t_vstd *)pd_new(vstd_class);
	outlet_new(&x->x_obj, gensym("float"));
	return (void *)x;
}

void vstd_setup(void)
{
    vstd_class = class_new(gensym("vstd"),
    	(t_newmethod)vstd_new, 0,
		sizeof(t_vstd), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vstd_class, (t_method)vstd_perform);
}

