/* --------------------- vthreshold ----------------------------- */

#include "defines.h"
#include "memory.h"

static t_class *vthreshold_class;

typedef struct _vthreshold
{
    t_object x_obj;
    t_outlet *x_outlet1;    	/* bang out for high thresh */
    t_outlet *x_outlet2;    	/* bang out for low thresh */
    int *x_state;    			/* 1 = high, 0 = low */
	int x_n;
    t_float x_hithresh;	    	/* value of high vthreshold */
    t_float x_lothresh;	    	/* value of low vthreshold */
    t_float x_hideadtime;	    	/* hi dead */
    t_float x_lodeadtime;	    	/* lo dead */
} t_vthreshold;

    /* "set" message to specify vthresholds and dead times */
static void vthreshold_set(t_vthreshold *x,
    t_floatarg hithresh, t_floatarg hideadtime,
    t_floatarg lothresh, t_floatarg lodeadtime,
	t_floatarg nf)
{
    if (lothresh > hithresh)
    	lothresh = hithresh;
    x->x_hithresh = hithresh;
    x->x_hideadtime = hideadtime;
    x->x_lothresh = lothresh;
    x->x_lodeadtime = lodeadtime;
    freebytes(x->x_state,x->x_n);
	x->x_n=(int)nf;
    x->x_state = (int *)getbytes(sizeof(int)*x->x_n);
	memset(x->x_state , 0, x->x_n);
}

static t_vthreshold *vthreshold_new(t_floatarg hithresh,
    t_floatarg hideadtime, t_floatarg lothresh, t_floatarg lodeadtime, t_floatarg n)
{
    t_vthreshold *x = (t_vthreshold *)
    	pd_new(vthreshold_class);
    x->x_state = 0;		/* low state */
    x->x_outlet1 = outlet_new(&x->x_obj, gensym("float"));
    x->x_outlet2 = outlet_new(&x->x_obj, gensym("float"));
    vthreshold_set(x, hithresh, hideadtime, lothresh, lodeadtime, n);
    return (x);
}

static void vthreshold_free(t_vthreshold *x)
{
	freebytes(x->x_state,x->x_n);
}

static void vthreshold_perform(t_vthreshold *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	int *state;

	state=x->x_state;
	if (argc>x->x_n) argc=x->x_n;
	for (i=0;i<argc;i++)
	{
		t_float f;
		f=atom_getfloat(argv++);

		if (*state<0)
		{
			if (f>x->x_hithresh)
			{
				outlet_float(x->x_outlet1, (t_float)i); // on
				*state=1;
			}
		}
		else 
		{
			if (f<x->x_lothresh)
			{
				outlet_float(x->x_outlet2, (t_float)i); // off
				*state=-1;
			}
		}
		state++;
	}
}


static void vthreshold_ff(t_vthreshold *x)
{
    freebytes(x->x_state,x->x_n);
}

void vthreshold_setup( void)
{
    vthreshold_class = class_new(gensym("vthreshold"),
    	(t_newmethod)vthreshold_new, (t_method)vthreshold_free,
	sizeof(t_vthreshold), 0,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addlist(vthreshold_class, (t_method)vthreshold_perform);   
	class_addmethod(vthreshold_class, (t_method)vthreshold_set,
    	gensym("set"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
//    class_addmethod(vthreshold_class, (t_method)vthreshold_ft1,
//    	gensym("ft1"), A_FLOAT, 0);
}
