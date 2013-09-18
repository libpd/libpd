/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

// ------------------------ decimate~ ----------------------------- 
// signal decimation
// code from musicdsp.org posted by tobybear 

static t_class *decimate_tilde_class;

typedef struct _decimate_tilde
{
    t_object x_obj;
	t_sample x_rate;
	t_sample x_bits;
	float x_f;
} t_decimate_tilde;

static void *decimate_tilde_new(t_floatarg rate, t_floatarg bits)
{
    t_decimate_tilde *x = (t_decimate_tilde *)pd_new(decimate_tilde_class);
	x->x_rate = rate;
	x->x_bits = bits;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_rate);
	floatinlet_new(&x->x_obj, &x->x_bits);
	x->x_f = 0;
	if (rate) x->x_rate = rate;
	else x->x_rate = 0.5;
	if (bits) x->x_bits = bits;
	else x->x_bits = 16;
    return (x);
}

static t_int *decimate_tilde_perform(t_int *w)
{
	t_decimate_tilde *x = (t_decimate_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f;
	long int m=1<<(int)(x->x_bits-1);
	float y=0, cnt=0;
    while (n--)
    {
		f = *in++;
		cnt+=x->x_rate;
		if (cnt>=1)
		{
			cnt-=1;
			y=(long int)(f*m)/(float)m;
		}
		*out++ = y;
    }
    return (w+5);
}

static void decimate_tilde_dsp(t_decimate_tilde *x, t_signal **sp)
{
    dsp_add(decimate_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void decimate_tilde_setup(void)
{
    decimate_tilde_class = class_new(gensym("decimate~"), (t_newmethod)decimate_tilde_new, 0,
    	sizeof(t_decimate_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(decimate_tilde_class, t_decimate_tilde, x_f);
    class_addmethod(decimate_tilde_class, (t_method)decimate_tilde_dsp, gensym("dsp"), 0);
}
