/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ foldback~ ----------------------------- */
/* signal mirror */

static t_class *foldback_tilde_class;

typedef struct _foldback_tilde
{
    t_object x_obj;
	t_sample x_low;
	t_sample x_high;
	float x_f;
} t_foldback_tilde;

static void *foldback_tilde_new(t_floatarg low, t_floatarg high)
{
    t_foldback_tilde *x = (t_foldback_tilde *)pd_new(foldback_tilde_class);
	x->x_low = low;
	x->x_high = high;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_low);
	floatinlet_new(&x->x_obj, &x->x_high);
	x->x_f = 0;
	if(low) x->x_low = low;
	else x->x_low = -1;
	if(high) x->x_high = high;
	else x->x_high = 1;
    return (x);
}

static t_int *foldback_tilde_perform(t_int *w)
{
	t_foldback_tilde *x = (t_foldback_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
    while (n--)
    {
		f = *in++;
		if(f < x->x_low) value = (f - ((f - x->x_low) * 2));
		else if(f > x->x_high) value = (f - ((f - x->x_high) * 2));
		else value = f;
		*out++ = value;
    }
    return (w+5);
}

static void foldback_tilde_dsp(t_foldback_tilde *x, t_signal **sp)
{
    dsp_add(foldback_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void foldback_tilde_setup(void)
{
    foldback_tilde_class = class_new(gensym("foldback~"), (t_newmethod)foldback_tilde_new, 0,
    	sizeof(t_foldback_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(foldback_tilde_class, t_foldback_tilde, x_f);
    class_addmethod(foldback_tilde_class, (t_method)foldback_tilde_dsp, gensym("dsp"), 0);
}
