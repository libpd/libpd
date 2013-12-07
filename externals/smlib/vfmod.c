#include "defines.h"

/*--------------- vfmod ----------------*/
/* floating point modulo of a vector */


static t_class *vfmod_class;

typedef struct _vfmod
{
    t_object x_obj;
	t_float m_y;
} t_vfmod;


static void vfmod_perform(t_vfmod *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_float y;
	t_atom *ap,*app;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	y=x->m_y;
	if (y==0.0f)
		y=1.0f;
	for (i = 0; i < argc; i++)
	{
		SETFLOAT(app, (t_float)fmod(atom_getfloat(argv++),y));
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
	freebytes(ap,argc);
}

static void *vfmod_new(t_float y)
{
	t_vfmod *x=(t_vfmod *)pd_new(vfmod_class);

    floatinlet_new(&x->x_obj, &x->m_y);

	outlet_new(&x->x_obj, gensym("list"));
	if (y==0.0f)
		y=1.0f;
	x->m_y=y;
	return (void *)x;
}

void vfmod_setup(void)
{
    vfmod_class = class_new(gensym("vfmod"),
    	(t_newmethod)vfmod_new, 0,
		sizeof(t_vfmod), 
		CLASS_DEFAULT,
	    A_DEFFLOAT,A_DEFFLOAT,0);
    class_addlist(vfmod_class, (t_method)vfmod_perform);
}

