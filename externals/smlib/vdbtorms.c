#include "defines.h"

/*--------------- vdbtorms ----------------*/

static t_class *vdbtorms_class;

typedef struct _vdbtorms
{
    t_object x_obj;
} t_vdbtorms;

static t_float decibelbtorms(t_float f)
{
    if (f <= 0)
    	return(0);
    else
    {
    	if (f > 485)
	    f = 485;
    }
    return (t_float)(exp((LOGTEN * 0.05) * (f-100.)));
}

static void vdbtorms_perform(t_vdbtorms *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom *ap,*app;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;

	for (i = 0; i < argc; i++)
	{
		SETFLOAT(app, decibelbtorms(atom_getfloat(argv++)));
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void *vdbtorms_new()
{
	t_vdbtorms *x=(t_vdbtorms *)pd_new(vdbtorms_class);
	outlet_new(&x->x_obj, gensym("list"));
	return (void *)x;
}

void vdbtorms_setup(void)
{
    vdbtorms_class = class_new(gensym("vdbtorms"),
    	(t_newmethod)vdbtorms_new, 0,
		sizeof(t_vdbtorms), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vdbtorms_class, (t_method)vdbtorms_perform);
}

