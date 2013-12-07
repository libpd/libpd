/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

// ------------------------ chop~ -----------------------------
// signal chopping modulator

static t_class *chop_tilde_class;

typedef struct _chop_tilde
{
    t_object x_obj;
	t_sample x_factor;
	float x_f;
} t_chop_tilde;

static void *chop_tilde_new(t_floatarg factor)
{
    t_chop_tilde *x = (t_chop_tilde *)pd_new(chop_tilde_class);
	x->x_factor = factor;
    outlet_new(&x->x_obj, gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	floatinlet_new(&x->x_obj, &x->x_factor);
	x->x_f = 0;
	if(factor) x->x_factor = factor;
	else x->x_factor = 0;
    return (x);
}

static t_int *chop_tilde_perform(t_int *w)
{
	t_chop_tilde *x = (t_chop_tilde *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
	t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    int n = (int)(w[5]);
	float f, m, value;
    while (n--)
    {
		f = *in1++;
		m = *in2++;
		if(m > 0.) value = f * x->x_factor;
		else value = f;
		*out++ = value;
    }
    return (w+6);
}

static void chop_tilde_dsp(t_chop_tilde *x, t_signal **sp)
{
    dsp_add(chop_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void chop_tilde_setup(void)
{
    chop_tilde_class = class_new(gensym("chop~"), (t_newmethod)chop_tilde_new, 0,
    	sizeof(t_chop_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(chop_tilde_class, t_chop_tilde, x_f);
    class_addmethod(chop_tilde_class, (t_method)chop_tilde_dsp, gensym("dsp"), 0);
}
