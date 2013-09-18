#include "defines.h"

/*--------------- vlavg -----------------------*/
/* leaky average of each element in a list     */
/* arguments: [halfdecay] */

static t_class *vlavg_class;

typedef struct _vlavg
{
    t_object x_obj;
	t_float m_c_leak;
	t_float m_leak;
	t_float *m_avg;
	int m_n;
} t_vlavg;


static void vlavg_perform(t_vlavg *x, t_symbol *s, int argc, t_atom *argv)
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
		if (x->m_avg)
			freebytes(x->m_avg,x->m_n);
		x->m_avg=(t_float*)getbytes(argc*sizeof(t_float));
		for(i=0;i<argc;i++)
			x->m_avg[i]=0.0f;
		x->m_n=argc;
	}

	fp=x->m_avg;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		*fp = *fp * m_c_leak + f * m_leak;
		SETFLOAT(app, *fp);
		app++;
		fp++;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void vlavg_setHalfDecay(t_vlavg *x, t_floatarg halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0f/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void *vlavg_new(t_float halfDecayTime)
{
	t_vlavg *x=(t_vlavg *)pd_new(vlavg_class);
	outlet_new(&x->x_obj, gensym("list"));
	vlavg_setHalfDecay(x, halfDecayTime);
	x->m_n=0;
	x->m_avg=0;
	return (void *)x;
}

static void vlavg_free(t_vlavg *x)
{
	freebytes(x->m_avg,x->m_n);
}

void vlavg_setup(void)
{
    vlavg_class = class_new(gensym("vlavg"),
    	(t_newmethod)vlavg_new, (t_method)vlavg_free,
		sizeof(t_vlavg), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, 0);
	class_addmethod(vlavg_class, (t_method)vlavg_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);
    class_addlist(vlavg_class, (t_method)vlavg_perform);
}
