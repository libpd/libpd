#include "defines.h"

/*--------------- vlmin -----------------------*/
/* leaky minimum of each element in a list     */
/* arguments: [halfdecay] */

static t_class *vlmin_class;

typedef struct _vlmin
{
    t_object x_obj;
	t_float m_c_leak;
	t_float m_leak;
	t_float *m_min;
	int m_n;
} t_vlmin;


static void vlmin_perform(t_vlmin *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom *ap,*app;
	t_float *fp;
	t_float m_leak;
	t_float m_c_leak;
	m_leak=x->m_leak;
	m_c_leak=x->m_c_leak;

	if (argc!=x->m_n)
	{
		int i;
		if (x->m_min)
			freebytes(x->m_min,x->m_n);
		x->m_min=(t_float*)getbytes(argc*sizeof(t_float));
		for(i=0;i<argc;i++)
			x->m_min[i]=0.0;
		x->m_n=argc;
	}

	fp=x->m_min;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		*fp =(f < *fp ) ? f : *fp *m_c_leak + f*m_leak;
		SETFLOAT(app, *fp);
		app++;
		fp++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void vlmin_setHalfDecay(t_vlmin *x, t_floatarg halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0/halfDecayTime));
	x->m_leak=1.0-x->m_c_leak;
}

static void *vlmin_new(t_float halfDecayTime)
{
	t_vlmin *x=(t_vlmin *)pd_new(vlmin_class);
	outlet_new(&x->x_obj, gensym("list"));
	vlmin_setHalfDecay(x, halfDecayTime);
	x->m_n=0;
	x->m_min=0;
	return (void *)x;
}

static void vlmin_free(t_vlmin *x)
{
	freebytes(x->m_min,x->m_n);
}

void vlmin_setup(void)
{
    vlmin_class = class_new(gensym("vlmin"),
    	(t_newmethod)vlmin_new, (t_method)vlmin_free,
		sizeof(t_vlmin), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
	class_addmethod(vlmin_class, (t_method)vlmin_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
    class_addlist(vlmin_class, (t_method)vlmin_perform);
}
