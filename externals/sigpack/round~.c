/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ round~ ----------------------------- */
/* simple rounder */

static t_class *round_tilde_class;

typedef struct _round_tilde
{
    t_object x_obj;
	t_sample x_coarse;
	float x_f;
} t_round_tilde;

static void *round_tilde_new(t_floatarg coarse)
{
    t_round_tilde *x = (t_round_tilde *)pd_new(round_tilde_class);
	x->x_coarse = coarse;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_coarse);
	x->x_f = 0;
	if(coarse) x->x_coarse = coarse;
	else x->x_coarse = 1;
    return (x);
}

static t_int *round_tilde_perform(t_int *w)
{
	t_round_tilde *x = (t_round_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, mult, value;
    while (n--)
    {
		f = *in++;
		mult = f * x->x_coarse;
		value = (int)mult / x->x_coarse;
		*out++ = value;
    }
    return (w+5);
}

static void round_tilde_dsp(t_round_tilde *x, t_signal **sp)
{
    dsp_add(round_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void round_tilde_setup(void)
{
    round_tilde_class = class_new(gensym("round~"), (t_newmethod)round_tilde_new, 0,
    	sizeof(t_round_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(round_tilde_class, t_round_tilde, x_f);
    class_addmethod(round_tilde_class, (t_method)round_tilde_dsp, gensym("dsp"), 0);
}
