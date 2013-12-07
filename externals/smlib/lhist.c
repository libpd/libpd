#include "defines.h"

/*--------------- lhist ---------------*/

static t_class *lhist_class;

typedef struct _lhist
{
    t_object x_obj;
	t_float m_lo;
	t_float m_hi;
	t_float m_scale;
	t_float m_c_leak;
	t_float m_leak;
	int m_nbins;
//	int m_n_observations;
	t_float *m_lhist;
} t_lhist;

static void lhist_setHalfDecay(t_lhist *x, t_float halfDecayTime)
{
	x->m_c_leak=(t_float)powf(.5,(1.0f/halfDecayTime));
	x->m_leak=1.0f-x->m_c_leak;
}

static void lhist_perform_float(t_lhist *x, t_float f)
{
	int j;
	j=(int)(.5+(f-x->m_lo)*x->m_scale);
	j=(j>0)?(j<x->m_nbins?j:x->m_nbins-1):0; // limit without IF
	x->m_lhist[j]++;
//	x->m_n_observations++;
}

static void lhist_perform_list(t_lhist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i,j;
	for (i = 0; i < argc; i++)
	{
		j=(int)(.5f+(atom_getfloat(&argv[i])-x->m_lo)*x->m_scale);
		j=(j>0)?(j<x->m_nbins?j:x->m_nbins-1):0; // limit without IF
		x->m_lhist[j]++;
	}
//	x->m_n_observations+=argc;
}

static void lhist_leak(t_lhist *x)
{
	int i;
	t_float *f;
	t_float sc;
	f=x->m_lhist;
    sc=x->m_c_leak;
	i=x->m_nbins;
	while(i--)
      *f++*=sc;
}

static void lhist_bang(t_lhist *x)
{
	int i,n;
	t_float *f;
	t_atom *ap,*app;
	n=x->m_nbins;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;
	i=n;
	f=x->m_lhist;

    while(i--){
      SETFLOAT(app, *f);
	  f++;
      app++;
    }
	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap,n);
}

static void lhist_relative(t_lhist *x)
{
	int i,n;
	t_float *f;
	t_float invn,sum;
	t_atom *ap,*app;

	n=x->m_nbins;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;
	i=x->m_nbins;
	f=x->m_lhist;
	sum=0.0f;
    while(i--) sum+=*f++;
	invn=1.0f/(1e-10f+sum);

	i=x->m_nbins;
	f=x->m_lhist;

    while(i--){
      SETFLOAT(app, (*f*invn));
	  f++;
      app++;
    }
	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap,n);
}

static void lhist_clear(t_lhist *x)
{
	int i;
	t_float *f;
	f=x->m_lhist;
	for (i=0;i<x->m_nbins;i++)
		*f++=0.0f;
//	x->m_n_observations=0;
}

static void lhist_set(t_lhist *x, t_float lo, t_float hi, t_float nbins)
{
	if (nbins<1)
	{
		nbins=1;
		logpost(x, 2, "[lhist] minimum number of bins is 1");
	}
	if (hi<=lo)
	{
		logpost(x, 2, "[lhist] higher bound (%g) must be greater than lower bound (%g)",
                hi, lo);	
		hi=lo+1.0f;
	}
	freebytes(x->m_lhist, x->m_nbins);
	x->m_hi=hi;
	x->m_lo=lo;
	x->m_nbins=(int)nbins;
	x->m_scale=(t_float)x->m_nbins/(hi-lo);
    x->m_lhist = (t_float*)getbytes(sizeof(t_float)*x->m_nbins);

	lhist_clear(x);
}

static void *lhist_new(t_float lo, t_float hi, t_float nbins, t_float decay)
{
	t_lhist *x=(t_lhist *)pd_new(lhist_class);
	outlet_new(&x->x_obj, gensym("list"));
	lhist_setHalfDecay(x,decay);
	x->m_nbins=0;
	x->m_lhist=0;
	lhist_set(x, lo, hi, nbins);
	return (void *)x;
}

static void lhist_free(t_lhist *x)
{
	freebytes(x->m_lhist, x->m_nbins);
}

void lhist_setup(void)
{
    lhist_class = class_new(gensym("lhist"),
    	(t_newmethod)lhist_new, (t_method)lhist_free,
		sizeof(t_lhist), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,A_DEFFLOAT,0);

	class_addmethod(lhist_class, (t_method)lhist_clear, gensym("clear"),0);
	class_addmethod(lhist_class, (t_method)lhist_bang, gensym("absolute"),0);
	class_addmethod(lhist_class, (t_method)lhist_relative, gensym("relative"),0);
	class_addmethod(lhist_class, (t_method)lhist_leak, gensym("leak"),0);
	class_addmethod(lhist_class, (t_method)lhist_setHalfDecay,
    	gensym("decay"), A_DEFFLOAT, NULL);

    class_addlist(lhist_class, (t_method)lhist_perform_list);
    class_addfloat(lhist_class, (t_method)lhist_perform_float);
    class_addbang(lhist_class, (t_method)lhist_bang);
}

