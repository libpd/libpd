#include "defines.h"

/*--------------- vabs ----------------*/
/* absolute values of a list of floats */


static t_class *vabs_class;

typedef struct _vabs
{
    t_object x_obj;
} t_vabs;


static void vabs_perform(t_vabs *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom *ap,*app;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;

	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		SETFLOAT(app, f>0?f:-f);
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
	freebytes(ap,argc);
}

static void *vabs_new()
{
	t_vabs *x=(t_vabs *)pd_new(vabs_class);
	outlet_new(&x->x_obj, gensym("list"));
	return (void *)x;
}

void vabs_setup(void)
{
    vabs_class = class_new(gensym("vabs"),
    	(t_newmethod)vabs_new, 0,
		sizeof(t_vabs), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vabs_class, (t_method)vabs_perform);
}

