#include "defines.h"

/*--------------- vdelta ----------------------------*/
/* differences between element in succesive vectors  */

static t_class *vdelta_class;

typedef struct _vdelta
{
    t_object x_obj;
	t_float m_c_leak;
	t_float m_leak;
	t_float *m_prev;
	int m_n;
} t_vdelta;


static void vdelta_perform(t_vdelta *x, t_symbol *s, int argc, t_atom *argv)
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
		if (x->m_prev)
			freebytes(x->m_prev,x->m_n);
		x->m_prev=(t_float*)getbytes(argc*sizeof(t_float));
		for(i=0;i<argc;i++)
			x->m_prev[i]=0.0f;
		x->m_n=argc;
	}

	fp=x->m_prev;
    ap = (t_atom *)getbytes(sizeof(t_atom)*argc);
	app=ap;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(argv++);
		SETFLOAT(app, f-*fp);
		app++;
		*fp++=f;
	}
	outlet_list(x->x_obj.ob_outlet,gensym("list"),argc,ap);
    freebytes(ap,argc);
}

static void *vdelta_new()
{
	t_vdelta *x=(t_vdelta *)pd_new(vdelta_class);
	outlet_new(&x->x_obj, gensym("list"));
	x->m_n=0;
	x->m_prev=0;
	return (void *)x;
}

static void vdelta_free(t_vdelta *x)
{
	freebytes(x->m_prev,x->m_n);
}

void vdelta_setup(void)
{
    vdelta_class = class_new(gensym("vdelta"),
    	(t_newmethod)vdelta_new, (t_method)vdelta_free,
		sizeof(t_vdelta), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vdelta_class, (t_method)vdelta_perform);
}
