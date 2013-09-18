#include "defines.h"

/*--------------- vpow ----------------*/

static t_class *vpow_class;

typedef struct _vpow
{
    t_object x_obj;
	t_float m_y;
} t_vpow;


static void vpow_perform(t_vpow *x, t_symbol *s, int argc, t_atom *argv)
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
		t_float x=atom_getfloat(argv++);
		if (x>0) 
			x=(t_float)powf(x,y);
		else
			x=-1000.;
		SETFLOAT(app, x);
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void *vpow_new(t_float y)
{
	t_vpow *x=(t_vpow *)pd_new(vpow_class);

    floatinlet_new(&x->x_obj, &x->m_y);

	outlet_new(&x->x_obj, gensym("list"));
	if (y==0.0f)
		y=1.0f;
	x->m_y=y;
	return (void *)x;
}

void vpow_setup(void)
{
    vpow_class = class_new(gensym("vpow"),
    	(t_newmethod)vpow_new, 0,
		sizeof(t_vpow), 
		CLASS_DEFAULT,
	    A_DEFFLOAT,A_DEFFLOAT,0);
    class_addlist(vpow_class, (t_method)vpow_perform);
}

