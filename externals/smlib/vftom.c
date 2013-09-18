#include "defines.h"

/*--------------- vftom ----------------*/

static t_class *vftom_class;

typedef struct _vftom
{
    t_object x_obj;
} t_vftom;

t_float ftom(t_float f)
{
    return (t_float)(f > 0 ? 17.3123405046 * log(.12231220585 * f) : -1500);
}

static void vftom_perform(t_vftom *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom *ap,*app;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;

	for (i = 0; i < argc; i++)
	{
		SETFLOAT(app, ftom(atom_getfloat(argv++)));
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void *vftom_new()
{
	t_vftom *x=(t_vftom *)pd_new(vftom_class);
	outlet_new(&x->x_obj, gensym("list"));
	return (void *)x;
}

void vftom_setup(void)
{
    vftom_class = class_new(gensym("vftom"),
    	(t_newmethod)vftom_new, 0,
		sizeof(t_vftom), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vftom_class, (t_method)vftom_perform);
}

