#include "defines.h"

/*--------------- vsum ---------------*/

static t_class *vsum_class;

typedef struct _vsum
{
    t_object x_obj;
} t_vsum;


static void vsum_perform(t_vsum *x, t_symbol *s, int argc, t_atom *argv)
{
	t_float sum=0.;
	int i;
	for (i = 0; i < argc; i++)
	{
		sum+= atom_getfloat(&argv[i]);
	}
    outlet_float(x->x_obj.ob_outlet, sum);
}

static void *vsum_new()
{
	t_vsum *x=(t_vsum *)pd_new(vsum_class);
	outlet_new(&x->x_obj, gensym("float"));
	return (void *)x;
}

void vsum_setup(void)
{
    vsum_class = class_new(gensym("vsum"),
    	(t_newmethod)vsum_new, 0,
		sizeof(t_vsum), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vsum_class, (t_method)vsum_perform);
}

