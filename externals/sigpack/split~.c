/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ split~ ----------------------------- */
/* signal splitter */

static t_class *split_tilde_class;

typedef struct _split_tilde
{
    t_object x_obj;
    float x_f;
    t_sample x_thres;
} t_split_tilde;

static void *split_tilde_new(t_floatarg thres)
{
    t_split_tilde *x = (t_split_tilde *)pd_new(split_tilde_class);
    x->x_thres = thres;
    outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
    floatinlet_new(&x->x_obj, &x->x_thres);
    x->x_f = 0;
	if(thres) x->x_thres = thres;
	else x->x_thres = 0;
    return (x);
}

static t_int *split_tilde_perform(t_int *w)
{
    t_split_tilde *x = (t_split_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out1 = (t_float *)(w[3]);
	t_float *out2 = (t_float *)(w[4]);
    int n = (int)(w[5]);
    while (n--)
    {
    	float f = *in++;
    	if(f < x->x_thres)
		{
			*out1++ = f;
			*out2++ = 0;
		}
		else if(f >= x->x_thres)
		{
			*out1++ = 0;
			*out2++ = f;
		}
    }
    return (w+6);
}

static void split_tilde_dsp(t_split_tilde *x, t_signal **sp)
{
    dsp_add(split_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void split_tilde_setup(void)
{
    split_tilde_class = class_new(gensym("split~"), (t_newmethod)split_tilde_new, 0,
    	sizeof(t_split_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(split_tilde_class, t_split_tilde, x_f);
    class_addmethod(split_tilde_class, (t_method)split_tilde_dsp, gensym("dsp"), 0);
}
