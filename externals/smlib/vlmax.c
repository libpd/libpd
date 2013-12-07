#include "defines.h"

/*--------------- vlmax -----------------------*/
/* leaky maximum of each element in a list     */
/* arguments: [halfdecay] */

static t_class *vlmax_class;

typedef struct _vlmax
{
    t_object x_obj;
	t_float m_c_leak;
	t_float m_leak;
	t_float *m_max;
	int m_n;
} t_vlmax;


static void vlmax_perform(t_vlmax *x, t_symbol *s, int argc, t_atom *argv)
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
		if (x->m_max)
			freebytes(x->m_max,x->m_n);
		x->m_max=(t_float*)getbytes(argc*sizeof(t_float));
		for(i=0;i<argc;i++)
			x->m_max[i]=0.0;
		x->m_n=argc;
	}

	fp=x->m_max;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		*fp =(f > *fp ) ? f : *fp *m_c_leak + f*m_leak;
		SETFLOAT(app, *fp);
		app++;
		fp++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void vlmax_setHalfDecay(t_vlmax *x, t_floatarg halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0/halfDecayTime));
	x->m_leak=1.0-x->m_c_leak;
}

static void *vlmax_new(t_float halfDecayTime)
{
	t_vlmax *x=(t_vlmax *)pd_new(vlmax_class);
	outlet_new(&x->x_obj, gensym("list"));
	vlmax_setHalfDecay(x, halfDecayTime);
	x->m_n=0;
	x->m_max=0;
	return (void *)x;
}

static void vlmax_free(t_vlmax *x)
{
	freebytes(x->m_max,x->m_n);
}

void vlmax_setup(void)
{
    vlmax_class = class_new(gensym("vlmax"),
    	(t_newmethod)vlmax_new, (t_method)vlmax_free,
		sizeof(t_vlmax), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
	class_addmethod(vlmax_class, (t_method)vlmax_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
    class_addlist(vlmax_class, (t_method)vlmax_perform);
}
