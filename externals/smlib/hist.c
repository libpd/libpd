#include "defines.h"

/*--------------- hist ---------------*/


static t_class *hist_class;

typedef struct _hist
{
    t_object x_obj;
	t_float m_lo;
	t_float m_hi;
	t_float m_scale;
	int m_nbins;
	int m_n_observations;
	t_float *m_hist;
} t_hist;


static void hist_perform_float(t_hist *x, t_float f)
{
	int j;
	j=(int)(.5+(f-x->m_lo)*x->m_scale);
	j=(j>0)?(j<x->m_nbins?j:x->m_nbins-1):0; // limit without IF
	x->m_hist[j]++;
	x->m_n_observations++;
}

static void hist_perform_list(t_hist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i,j;
	for (i = 0; i < argc; i++)
	{
		j=(int)(.5f+(atom_getfloat(&argv[i])-x->m_lo)*x->m_scale);
		j=(j>0)?(j<x->m_nbins?j:x->m_nbins-1):0; // limit without IF
		x->m_hist[j]++;
	}
	x->m_n_observations+=argc;
}

static void hist_bang(t_hist *x)
{
	int i,n;
	t_float *f;
	t_atom *ap,*app;

	n=x->m_nbins;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;

	i=x->m_nbins;
	f=x->m_hist;

    while(i--){
      SETFLOAT(app, *f);
	  f++;
      app++;
    }
	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap, sizeof(t_atom)*n);
}

static void hist_relative(t_hist *x)
{
	int i,n;
	t_float *f;
	t_float invn;
	t_atom *ap,*app;
	n=x->m_nbins;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;

	invn=1.0f/(1e-10f+x->m_n_observations);

	i=n;
	f=x->m_hist;

    while(i--){
      SETFLOAT(app, (*f*invn));
	  f++;
      app++;
    }
	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap, sizeof(t_atom)*n);
}

static void hist_clear(t_hist *x)
{
	int i;
	t_float *f;
	f=x->m_hist;
	for (i=0;i<x->m_nbins;i++)
		*f++=0.0f;
	x->m_n_observations=0;
}


static void hist_set(t_hist *x, t_float lo, t_float hi, t_float nbins)
{
	if (nbins<1)
	{
		nbins=1;
		logpost(x, 2, "[hist] minimum number of bins is 1");
	}
	if (hi<=lo)
	{
		logpost(x, 2, "[hist] higher bound (%g) must be greater than lower bound (%g)",
                hi, lo);
		hi=lo+1.0f;
	}

	freebytes(x->m_hist, x->m_nbins);

	x->m_hi=hi;
	x->m_lo=lo;
	x->m_nbins=(int)nbins;
	x->m_scale=(t_float)x->m_nbins/(hi-lo);
    x->m_hist = (t_float*)getbytes(sizeof(t_float)*x->m_nbins);

	hist_clear(x);
}

static void *hist_new(t_float lo, t_float hi, t_float nbins)
{
	t_hist *x=(t_hist *)pd_new(hist_class);
	outlet_new(&x->x_obj, gensym("list"));
	x->m_hist=0;
	x->m_nbins=0;
	hist_set(x, lo, hi, nbins);
	return (void *)x;
}

static void hist_free(t_hist *x)
{
	freebytes(x->m_hist, x->m_nbins);
}

void hist_setup(void)
{
    hist_class = class_new(gensym("hist"),
    	(t_newmethod)hist_new, (t_method)hist_free,
		sizeof(t_hist), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,0);

	class_addmethod(hist_class, (t_method)hist_clear, gensym("clear"),0);
	class_addmethod(hist_class, (t_method)hist_bang, gensym("absolute"),0);
	class_addmethod(hist_class, (t_method)hist_relative, gensym("relative"),0);

	class_addmethod(hist_class, (t_method)hist_set, gensym("set"),A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,0);

    class_addlist(hist_class, (t_method)hist_perform_list);
    class_addfloat(hist_class, (t_method)hist_perform_float);
    class_addbang(hist_class, (t_method)hist_bang);
}

