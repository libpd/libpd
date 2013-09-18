#include "defines.h"

/*--------------- linspace ----------------*/
/* clips a vector */


static t_class *linspace_class;

typedef struct _linspace
{
    t_object x_obj;
	t_float m_lo;
	t_float m_hi;
	t_float m_n;
} t_linspace;


static void linspace_bang(t_linspace *x)
{
	int n;
	n=(int)x->m_n;
	if ((n<256)&&(n>1))
	{
		int i;
		t_float lo,step;
		t_atom *ap,*app;

		ap = (t_atom *)getbytes(sizeof(t_atom)*n);
		app=ap;
		lo=x->m_lo;
		step=(x->m_hi-lo)/(n-1);	
		for (i = 0; i < n; i++)
		{

			SETFLOAT(app, lo);
			app++;
			lo+=step;
		}
		outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
		freebytes(ap,n);
	}
}

static void linspace_float(t_linspace *x, t_float lo)
{
	x->m_lo=lo;
	linspace_bang(x);
}

static void *linspace_new(t_float lo, t_float hi, t_float n)
{
	t_linspace *x=(t_linspace *)pd_new(linspace_class);

    floatinlet_new(&x->x_obj, &x->m_hi);
    floatinlet_new(&x->x_obj, &x->m_n);

	outlet_new(&x->x_obj, gensym("list"));
	x->m_lo=lo;
	x->m_hi=hi;
	x->m_n=n;
	return (void *)x;
}

void linspace_setup(void)
{
    linspace_class = class_new(gensym("linspace"),
    	(t_newmethod)linspace_new, 0,
		sizeof(t_linspace), 
		CLASS_DEFAULT,
	    A_DEFFLOAT,A_DEFFLOAT,A_DEFFLOAT,0);
    class_addfloat(linspace_class, (t_method)linspace_float);
    class_addbang(linspace_class, (t_method)linspace_bang);
}

