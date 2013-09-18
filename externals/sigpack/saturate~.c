/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#include <math.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ saturate~ ----------------------------- */
/* signal soft saturation */
/* code from www.musicdsp.org posted by bram de jong */

static t_class *saturate_tilde_class;

typedef struct _saturate_tilde
{
    t_object x_obj;
	t_sample x_thresh;
	float x_f;
} t_saturate_tilde;

static void *saturate_tilde_new(t_floatarg thresh)
{
    t_saturate_tilde *x = (t_saturate_tilde *)pd_new(saturate_tilde_class);
	x->x_thresh = thresh;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_thresh);
	x->x_f = 0;
	if(thresh) x->x_thresh = thresh;
	else x->x_thresh = 1.;
    return (x);
}

static float sigmoid(float x)
{
	if(fabs(x) < 1)
		return x * (1.5f - 0.5f * x * x);
	else
		return x > 0.f ? 1.f : -1.f;
}

static t_int *saturate_tilde_perform(t_int *w)
{
	t_saturate_tilde *x = (t_saturate_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
	float t = x->x_thresh;
    while (n--)
    {
		f = *in++;
		if(fabs(f) < t)
			value = f;
		else
		{
			if(f > 0.f)
				value = t + (1.f - t) * sigmoid((f - t)/((1 - t) * 1.5f));
			else
				value = -(t + (1.f - t) * sigmoid((-f - t)/((1 - t) * 1.5f)));
		}
		*out++ = value;
    }
    return (w+5);
}

static void saturate_tilde_dsp(t_saturate_tilde *x, t_signal **sp)
{
    dsp_add(saturate_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void saturate_tilde_setup(void)
{
    saturate_tilde_class = class_new(gensym("saturate~"), (t_newmethod)saturate_tilde_new, 0,
    	sizeof(t_saturate_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(saturate_tilde_class, t_saturate_tilde, x_f);
    class_addmethod(saturate_tilde_class, (t_method)saturate_tilde_dsp, gensym("dsp"), 0);
	class_addmethod(saturate_tilde_class, (t_method)saturate_tilde_dsp, gensym("sigmoid"), A_FLOAT, 0);
}
