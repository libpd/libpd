#include "defines.h"

/*--------------- vclip ----------------*/
/* clips a vector */


static t_class *vclip_class;

typedef struct _vclip
{
    t_object x_obj;
	t_float m_lo;
	t_float m_hi;
} t_vclip;


static void vclip_perform(t_vclip *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_float lo,hi;
	t_atom *ap,*app;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	lo=x->m_lo;
	hi=x->m_hi;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		SETFLOAT(app, (f<lo?lo:(f>hi?hi:f)));
		app++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
	freebytes(ap,argc);
}

static void *vclip_new(t_float lo, t_float hi)
{
	t_vclip *x=(t_vclip *)pd_new(vclip_class);

    floatinlet_new(&x->x_obj, &x->m_lo);
    floatinlet_new(&x->x_obj, &x->m_hi);

	outlet_new(&x->x_obj, gensym("list"));
	x->m_hi=hi;
	x->m_lo=lo;
	return (void *)x;
}

void vclip_setup(void)
{
    vclip_class = class_new(gensym("vclip"),
    	(t_newmethod)vclip_new, 0,
		sizeof(t_vclip), 
		CLASS_DEFAULT,
	    A_DEFFLOAT,A_DEFFLOAT,0);
    class_addlist(vclip_class, (t_method)vclip_perform);
}

