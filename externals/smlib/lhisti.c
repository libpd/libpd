#include "defines.h"

/*--------------- lhisti ---------------*/
// ignores samples outside bins

static t_class *lhisti_class;

typedef struct _lhisti
{
    t_object x_obj;
	t_float m_lo;
	t_float m_hi;
	t_float m_scale;
	t_float m_c_leak;
	t_float m_leak;
	int m_nbins;
//	int m_n_observations;
	t_float *m_lhisti;
} t_lhisti;

static void lhisti_setHalfDecay(t_lhisti *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0f/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void lhisti_perform_float(t_lhisti *x, t_float f)
{
	int j;
	j=(int)(.5+(f-x->m_lo)*x->m_scale);
	if ((j>=0)&&(j<x->m_nbins))
		x->m_lhisti[j]++;
}

static void lhisti_perform_list(t_lhisti *x, t_symbol *s, int argc, t_atom *argv)
{
	int i,j;
	for (i = 0; i < argc; i++)
	{
		j=(int)(.5f+(atom_getfloat(&argv[i])-x->m_lo)*x->m_scale);
		if ((j>=0)&&(j<x->m_nbins))
			x->m_lhisti[j]++;
	}
//	x->m_n_observations+=argc;
}

static void lhisti_leak(t_lhisti *x)
{
	int i;
	t_float *f;
	t_float sc;
	f=x->m_lhisti;
    sc=x->m_c_leak;
	i=x->m_nbins;
	while(i--)
      *f++*=sc;
}

static void lhisti_bang(t_lhisti *x)
{
	int i,n;
	t_float *f;
	t_atom *ap,*app;
	n=x->m_nbins;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;
	i=n;
	f=x->m_lhisti;

    while(i--){
      SETFLOAT(app, *f);
	  f++;
      app++;
    }
	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap,n);
}

static void lhisti_relative(t_lhisti *x)
{
	int i,n;
	t_float *f;
	t_float invn,sum;
	t_atom *ap,*app;

	n=x->m_nbins;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;
	i=x->m_nbins;
	f=x->m_lhisti;
	sum=0.0f;
    while(i--) sum+=*f++;
	invn=1.0f/(1e-10f+sum);

	i=x->m_nbins;
	f=x->m_lhisti;

    while(i--){
      SETFLOAT(app, (*f*invn));
	  f++;
      app++;
    }
	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap,n);
}

static void lhisti_clear(t_lhisti *x)
{
	int i;
	t_float *f;
	f=x->m_lhisti;
	for (i=0;i<x->m_nbins;i++)
		*f++=0.0f;
//	x->m_n_observations=0;
}

static void lhisti_set(t_lhisti *x, t_float lo, t_float hi, t_float nbins)
{
	if (nbins<1)
	{
		nbins=1;
		logpost(x, 2, "[lhisti] number of bins is minimum 1");
	}
	if (hi<=lo)
	{
		post("[lhisti] higher bound (%g) must be greater than lower bound (%g)",
             hi, lo);	
		hi=lo+1.0f;
	}
	freebytes(x->m_lhisti, x->m_nbins);
	x->m_hi=hi;
	x->m_lo=lo;
	x->m_nbins=(int)nbins;
	x->m_scale=(t_float)x->m_nbins/(hi-lo);
    x->m_lhisti = (t_float*)getbytes(sizeof(t_float)*x->m_nbins);

	lhisti_clear(x);
}

static void *lhisti_new(t_float lo, t_float hi, t_float nbins, t_float decay)
{
	t_lhisti *x=(t_lhisti *)pd_new(lhisti_class);
	outlet_new(&x->x_obj, gensym("list"));
	lhisti_setHalfDecay(x,decay);
	x->m_nbins=0;
	x->m_lhisti=0;
	lhisti_set(x, lo, hi, nbins);
	return (void *)x;
}

static void lhisti_free(t_lhisti *x)
{
	freebytes(x->m_lhisti, x->m_nbins);
}

void lhisti_setup(void)
{
    lhisti_class = class_new(gensym("lhisti"),
    	(t_newmethod)lhisti_new, (t_method)lhisti_free,
		sizeof(t_lhisti), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,A_DEFFLOAT,0);

	class_addmethod(lhisti_class, (t_method)lhisti_clear, gensym("clear"),0);
	class_addmethod(lhisti_class, (t_method)lhisti_bang, gensym("absolute"),0);
	class_addmethod(lhisti_class, (t_method)lhisti_relative, gensym("relative"),0);
	class_addmethod(lhisti_class, (t_method)lhisti_leak, gensym("leak"),0);
	class_addmethod(lhisti_class, (t_method)lhisti_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);

    class_addlist(lhisti_class, (t_method)lhisti_perform_list);
    class_addfloat(lhisti_class, (t_method)lhisti_perform_float);
    class_addbang(lhisti_class, (t_method)lhisti_bang);
}

