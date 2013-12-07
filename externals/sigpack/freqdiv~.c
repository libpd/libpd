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

/* ------------------------ freqdiv~ ----------------------------- */
/* frequency divider */
/* code from swh_plugins by steve harris www.plugins.org.uk */

static t_class *freqdiv_tilde_class;

typedef struct _freqdiv_tilde
{
    t_object x_obj;
	t_sample x_denominate;
	t_sample x_amp;
	float x_count;
	t_sample x_lamp;
	t_sample x_last;
	t_sample x_out;
	int x_zeroxs;
	float x_f;
} t_freqdiv_tilde;

static void *freqdiv_tilde_new(t_floatarg denominate)
{
    t_freqdiv_tilde *x = (t_freqdiv_tilde *)pd_new(freqdiv_tilde_class);
	x->x_denominate = denominate;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_denominate);
	x->x_f = 0;
	x->x_amp = 0;
	x->x_count = 0;
	x->x_lamp = 0;
	x->x_last = 0;
	x->x_out = 0;
	x->x_zeroxs = 0;
	if (denominate) x->x_denominate = denominate;
	else x->x_denominate = 1;
    return (x);
}

static t_int *freqdiv_tilde_perform(t_int *w)
{
	t_freqdiv_tilde *x = (t_freqdiv_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f;
	int den = (int)x->x_denominate;
    while (n--)
    {
		f = *in++;
		x->x_count += 1.0f;
		if ((f > 0.0f && x->x_last <= 0.0f) || (f < 0.0f && x->x_last >= 0.0))
		{
			x->x_zeroxs++;
			if (den == 1)
			{
				x->x_out = x->x_out > 0.0f ? -1.0f : 1.0f;
				x->x_lamp = x->x_amp / x->x_count;
				x->x_zeroxs = 0;
				x->x_count = 0;
				x->x_amp = 0;
			}
		}
		x->x_amp += fabs(f);
		if (den > 1 && (x->x_zeroxs % den) == den-1)
		{
			x->x_out = x->x_out > 0.0f ? -1.0f : 1.0f;
			x->x_lamp = x->x_amp / x->x_count;
			x->x_zeroxs = 0;
			x->x_count = 0;
			x->x_amp = 0;
		}
		x->x_last = f;
		*out++ = x->x_out * x->x_lamp;
    }
    return (w+5);
}

static void freqdiv_tilde_dsp(t_freqdiv_tilde *x, t_signal **sp)
{
    dsp_add(freqdiv_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void freqdiv_tilde_setup(void)
{
    freqdiv_tilde_class = class_new(gensym("freqdiv~"), (t_newmethod)freqdiv_tilde_new, 0,
    	sizeof(t_freqdiv_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(freqdiv_tilde_class, t_freqdiv_tilde, x_f);
    class_addmethod(freqdiv_tilde_class, (t_method)freqdiv_tilde_dsp, gensym("dsp"), 0);
}
