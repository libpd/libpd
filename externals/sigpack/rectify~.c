/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ rectify~ ----------------------------- */
/* flips negative signal values to positive */

static t_class *rectify_tilde_class;

typedef struct _rectify_tilde
{
    t_object x_obj;
	float x_f;
} t_rectify_tilde;

static void *rectify_tilde_new(void)
{
    t_rectify_tilde *x = (t_rectify_tilde *)pd_new(rectify_tilde_class);
    outlet_new(&x->x_obj, gensym("signal"));
	x->x_f = 0;
    return (x);
}

static t_int *rectify_tilde_perform(t_int *w)
{
	t_rectify_tilde *x = (t_rectify_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
    while (n--)
    {
		f = *in++;
		if (f < 0)
			value = f * -1;
		else
			value = f;
		*out++ = value;
    }
    return (w+5);
}

static void rectify_tilde_dsp(t_rectify_tilde *x, t_signal **sp)
{
    dsp_add(rectify_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void rectify_tilde_setup(void)
{
    rectify_tilde_class = class_new(gensym("rectify~"), (t_newmethod)rectify_tilde_new, 0,
    	sizeof(t_rectify_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(rectify_tilde_class, t_rectify_tilde, x_f);
    class_addmethod(rectify_tilde_class, (t_method)rectify_tilde_dsp, gensym("dsp"), 0);
}
