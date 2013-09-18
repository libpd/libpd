// sigpack
// for
// pure-data
// by weiss
// www.weiss-archiv.de

#include "m_pd.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

// ------------------------ wavewrap~ ----------------------------- 
// sinus wavewrapper. produces an unusual distortion effect
// code from swh_plugins by steve harris www.plugin.org.uk  

static t_class *wavewrap_tilde_class;

typedef struct _wavewrap_tilde
{
    t_object x_obj;
	t_sample x_wrap;
	float x_f;
} t_wavewrap_tilde;

static void *wavewrap_tilde_new(t_floatarg wrap)
{
    t_wavewrap_tilde *x = (t_wavewrap_tilde *)pd_new(wavewrap_tilde_class);
	x->x_wrap = wrap;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_wrap);
	x->x_f = 0;
	if (wrap) x->x_wrap = wrap;
	else x->x_wrap = 0;
    return (x);
}

static t_int *wavewrap_tilde_perform(t_int *w)
{
	t_wavewrap_tilde *x = (t_wavewrap_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
	float coef = x->x_wrap * M_PI;
	if (coef < 0.05f) {
		coef = 0.05f;
	}

    while (n--)
    {
		f = *in++;
		value = sin(f * coef);
		*out++ = value;
    }
    return (w+5);
}

static void wavewrap_tilde_dsp(t_wavewrap_tilde *x, t_signal **sp)
{
    dsp_add(wavewrap_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void wavewrap_tilde_setup(void)
{
    wavewrap_tilde_class = class_new(gensym("wavewrap~"), (t_newmethod)wavewrap_tilde_new, 0,
    	sizeof(t_wavewrap_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(wavewrap_tilde_class, t_wavewrap_tilde, x_f);
    class_addmethod(wavewrap_tilde_class, (t_method)wavewrap_tilde_dsp, gensym("dsp"), 0);
}
