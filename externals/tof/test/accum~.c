


/* [accum~] by Thomas Ouellet Fredericks
 * License: If you use and die, it's not my fault
 * Based on [count~] (sickle/count.c) 
 * Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
*/

#include "m_pd.h"
#include <math.h>

/* TODO
- Maybe add a pause?
*/

typedef struct _accum_tilde
{
	t_object  	x_obj;
	t_float			f;
    t_float     	min;
    t_float     	max;
    int				wrap;
    //int    		limit;
    int    		on;
    int    		autoreset;
   double    	count; 
} t_accum_tilde;

static t_class *accum_tilde_class;

static void accum_tilde_wrap(t_accum_tilde *x, t_floatarg f) {
    x->wrap = (f != 0);
}
/*
static void accum_tilde_set(t_accum_tilde *x, t_floatarg f1, t_floatarg f2) {
	x->min=(f1<f2)?f1:f2;
    x->max=(f1>f2)?f1:f2;
    post("   min: %f \n   max: %f",x->min,x->max);
}
*/

static void accum_tilde_min(t_accum_tilde *x, t_floatarg f) {
	
	x->min = f;
	//accum_tilde_set(x,f,x->max);
	
}


static void accum_tilde_max(t_accum_tilde *x, t_floatarg f) {
	x->max = f;
	//accum_tilde_set(x,x->min,f);
   
}

static void accum_tilde_autoreset(t_accum_tilde *x, t_floatarg f)
{
    x->autoreset = (f != 0);
}

static void accum_tilde_bang(t_accum_tilde *x)
{
    x->count = x->min;
    x->on = 1;
}

/*
static void accum_tilde_float(t_accum_tilde *x, t_floatarg f)
{
    x->x_accum_tilde = x->x_min = (int)f;
    x->x_on = 1;
}
*/

/*
static void accum_tilde_list(t_accum_tilde *x, t_symbol *s, int ac, t_atom *av)
{
    int i;
    if (ac > 4) ac = 4;
    for (i = 0; i < ac; i++)
	if (av[i].a_type != A_FLOAT) break;
    switch (i)
    {
    case 4:
	count_autoreset(x, av[3].a_w.w_float);
    case 3:
	x->x_on = (av[2].a_w.w_float != 0);
    case 2:
	count_max(x, av[1].a_w.w_float);
    case 1:
	count_min(x, av[0].a_w.w_float);
    default:
	x->x_accum_tilde = x->x_min;
    }
}
*/
/*
static void accum_tilde_set(t_accum_tilde *x, t_symbol s, int ac, t_atom *av)
{
    if (av[0].a_type == A_FLOAT) count_min(x, av[0].a_w.w_float);
    if (av[1].a_type == A_FLOAT) count_max(x, av[1].a_w.w_float);
}
*/
static void accum_tilde_stop(t_accum_tilde *x)
{
    x->count = x->min;
    x->on = 0;
}

static t_int *accum_tilde_perform(t_int *w) {
	// point to x
    // point to number of samples
    // point to inlet~
    // point to outlet~
	
    t_accum_tilde *x = (t_accum_tilde *)(w[1]);
    int nblock = (int)(w[2]);
    t_float* in = (t_float *)(w[3]);
    t_float* out = (t_float *)(w[4]);
    double count = x->count;
    
    t_float min = (x->min < x->max)?x->min:x->max;
    t_float max = (x->max > x->min)?x->max:x->min;
    //post("   min: %f \n   max: %f",min,max);
    
    
    // Do not count if min == max
    if ( min == max)  {
		count = min;
		while (nblock--) *out++ = count;
	} else {
		if (x->on) {
			if ( x->wrap ) {
				// Possible wrapping
				while (nblock--) {
					count = count + *in;
					in++;
					if (count < min || count >= max) {
						count = fmod((count - min),(max - min));
						if (count < 0) {
							count = count + max ;
						} else {
							count = count + min;
						}
					}
					*out++ = count;
				}
			} else {
				// No wrapping
				while (nblock--) {
					count = count + *in;
					in++;
					*out++ = count;
				}
			}
		} else {	
			/* WRAP EVEN IF NOT ACCUMULATING ?
			if ( x->wrap && (count < min || count >= max)) {
				count = fmod((count - min),(max - min)); 
				if (count < 0) {
							count = count + max ;
						} else {
							count = count + min;
				}
			}
			*/
			while (nblock--) *out++ = count;
		}
	}
    x->count = count;
    
    return (w + 5);
}

static void accum_tilde_dsp(t_accum_tilde *x, t_signal **sp) {
    if (x->autoreset) accum_tilde_bang(x);
    // point to x
    // point to number of samples
    // point to inlet~
    // point to outlet~
    dsp_add(accum_tilde_perform, 4, x, sp[0]->s_n, sp[0]->s_vec,sp[1]->s_vec);
}

static void *accum_tilde_new(t_floatarg minval, t_floatarg maxval,
		       t_floatarg wrapflag, t_floatarg autoflag)
{
    t_accum_tilde *x = (t_accum_tilde *)pd_new(accum_tilde_class);
    
    x->wrap = 0;
    x->on = 0;
    
    accum_tilde_min(x, minval);
    accum_tilde_max(x, maxval);
    x->wrap = (wrapflag != 0);
    accum_tilde_autoreset(x, autoflag);
    x->count = x->min;
    
    
    
    //inlet_new((t_object *)x, &s_signal);
    
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("min"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("max"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("wrap"));
    outlet_new((t_object *)x, &s_signal);
    
    return (x);
}

void accum_tilde_setup(void)
{
    accum_tilde_class = class_new(gensym("accum~"),
			    (t_newmethod)accum_tilde_new, 0,
			    sizeof(t_accum_tilde), 0,
			    A_DEFFLOAT, A_DEFFLOAT,
			    A_DEFFLOAT, A_DEFFLOAT, 0);
			    
	CLASS_MAINSIGNALIN(accum_tilde_class, t_accum_tilde, f);
	 class_addmethod(accum_tilde_class, (t_method)accum_tilde_dsp, gensym("dsp"), 0);
    //sic_setup(count_class, count_dsp, SIC_NOMAINSIGNALIN);
    class_addbang(accum_tilde_class, accum_tilde_bang);
    //class_addfloat(accum_tilde_class, count_float);
    //class_addlist(accum_tilde_class, count_list);
    /*class_addmethod(count_class, (t_method)count_max,
		    gensym("ft1"), A_FLOAT, 0);*/
  
    class_addmethod(accum_tilde_class, (t_method)accum_tilde_autoreset,
		    gensym("autoreset"), A_FLOAT, 0);
    class_addmethod(accum_tilde_class, (t_method)accum_tilde_min,
		    gensym("min"), A_FLOAT, 0);
	class_addmethod(accum_tilde_class, (t_method)accum_tilde_max,
		    gensym("max"), A_FLOAT, 0);
    class_addmethod(accum_tilde_class, (t_method)accum_tilde_wrap,
		    gensym("wrap"), A_FLOAT, 0);
    //class_addmethod(count_class, (t_method)count_set, gensym("set"), A_GIMME, 0);
    class_addmethod(accum_tilde_class, (t_method)accum_tilde_stop, gensym("stop"), 0);
}
