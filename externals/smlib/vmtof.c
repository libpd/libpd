#include "defines.h"

/*--------------- vmtof ----------------*/

static t_class *vmtof_class;

typedef struct _vmtof
{
    t_object x_obj;
} t_vmtof;

t_float mtof(t_float f)
{
    if (f <= -1500) return(0);
    else if (f > 1499) return(mtof(1499));
    else return (t_float)(8.17579891564 * exp(.0577622650 * f));
}

static void vmtof_perform(t_vmtof *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom *ap,*app;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;

	for (i = 0; i < argc; i++)
	{
		SETFLOAT(app, mtof(atom_getfloat(argv++)));
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void *vmtof_new()
{
	t_vmtof *x=(t_vmtof *)pd_new(vmtof_class);
	outlet_new(&x->x_obj, gensym("list"));
	return (void *)x;
}

void vmtof_setup(void)
{
    vmtof_class = class_new(gensym("vmtof"),
    	(t_newmethod)vmtof_new, 0,
		sizeof(t_vmtof), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vmtof_class, (t_method)vmtof_perform);
}

