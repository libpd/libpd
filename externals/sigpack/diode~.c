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

/* ------------------------ diode~ ----------------------------- */
/* Mangles the signal as if it had been passed through a diode rectifier network.*/
/* code from swh_plugins by steve harris www.plugin.org.uk */

static t_class *diode_tilde_class;

typedef struct _diode_tilde
{
    t_object x_obj;
	t_sample x_mode;//0=none,1=halfWave,2=fullWave
	float x_f;
} t_diode_tilde;

static void *diode_tilde_new(t_floatarg mode)
{
    t_diode_tilde *x = (t_diode_tilde *)pd_new(diode_tilde_class);
	x->x_mode = mode;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_mode);
	x->x_f = 0;
	if(mode) x->x_mode = mode;
	else x->x_mode = 0;
    return (x);
}

static t_int *diode_tilde_perform(t_int *w)
{
	t_diode_tilde *x = (t_diode_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
    while (n--)
    {
		f = *in++;
		if(x->x_mode >= 0.0f && x->x_mode < 1.0f) {
			value = ((1.0f - x->x_mode) * f + (x->x_mode * (f > 0.0f ? f : 0.0f)));
		}
		else if (x->x_mode >= 1.0f && x->x_mode < 2.0f) {
			float fac = x->x_mode - 1.0f;
			value = ((1.0f - fac) * (f > 0 ? f : 0.0)) + (fac * fabs(f));
		}
		else if (x->x_mode >= 2) {
			float fac = x->x_mode < 3 ? x->x_mode - 2 : 1.0;
			value = (1.0 - fac) * fabs(f);
		}
		else {
			value = f;
		}
		*out++ = value;
    }
    return (w+5);
}

static void diode_tilde_dsp(t_diode_tilde *x, t_signal **sp)
{
    dsp_add(diode_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void diode_tilde_setup(void)
{
    diode_tilde_class = class_new(gensym("diode~"), (t_newmethod)diode_tilde_new, 0,
    	sizeof(t_diode_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(diode_tilde_class, t_diode_tilde, x_f);
    class_addmethod(diode_tilde_class, (t_method)diode_tilde_dsp, gensym("dsp"), 0);
}
