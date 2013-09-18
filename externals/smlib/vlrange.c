#include "defines.h"

/*--------------- vlrange -----------------------*/
/* leaky range of each element in a list     */
/* arguments: [halfdecay] */

static t_class *vlrange_class;

typedef struct _vlrange
{
    t_object x_obj;
	t_float m_c_leak;
	t_float m_leak;
	t_float *m_min;
	t_float *m_max;
	int m_n;
} t_vlrange;


static void vlrange_perform(t_vlrange *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom *ap,*app;
	t_float *fmin, *fmax;
	t_float m_leak;
	t_float m_c_leak;
	m_leak=x->m_leak;
	m_c_leak=x->m_c_leak;

	if (argc!=x->m_n)
	{
		int i;
		if (x->m_min)
		{
			freebytes(x->m_min,x->m_n);
			freebytes(x->m_max,x->m_n);
		}
		x->m_min=(t_float*)getbytes(argc*sizeof(t_float));
		x->m_max=(t_float*)getbytes(argc*sizeof(t_float));
		for(i=0;i<argc;i++)
		{
			x->m_min[i]=0.0;
			x->m_max[i]=0.0;
		}
		x->m_n=argc;
	}

	fmin=x->m_min;
	fmax=x->m_max;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		*fmax =(f > *fmax ) ? f : *fmax *m_c_leak + f*m_leak;
		*fmin =(f < *fmin ) ? f : *fmin *m_c_leak + f*m_leak;
		SETFLOAT(app, *fmax-*fmin);
		app++;
		fmax++;
		fmin++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void vlrange_setHalfDecay(t_vlrange *x, t_floatarg halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void *vlrange_new(t_float halfDecayTime)
{
	t_vlrange *x=(t_vlrange *)pd_new(vlrange_class);
	outlet_new(&x->x_obj, gensym("list"));
	vlrange_setHalfDecay(x, halfDecayTime);
	x->m_n=0;
	x->m_min=0;
	x->m_max=0;
	return (void *)x;
}

static void vlrange_free(t_vlrange *x)
{
	freebytes(x->m_max,x->m_n);
	freebytes(x->m_min,x->m_n);
}

void vlrange_setup(void)
{
    vlrange_class = class_new(gensym("vlrange"),
    	(t_newmethod)vlrange_new, (t_method)vlrange_free,
		sizeof(t_vlrange), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
	class_addmethod(vlrange_class, (t_method)vlrange_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
    class_addlist(vlrange_class, (t_method)vlrange_perform);
}
