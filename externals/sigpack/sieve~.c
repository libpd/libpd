/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ sieve~ ----------------------------- */
/* sift samples */

static t_class *sieve_tilde_class;

typedef struct _sieve_tilde
{
    t_object x_obj;
	t_sample x_mode;
	t_sample x_sample;
	t_sample x_last;
	float x_f;
} t_sieve_tilde;

static void *sieve_tilde_new(t_floatarg mode, t_floatarg sample)
{
    t_sieve_tilde *x = (t_sieve_tilde *)pd_new(sieve_tilde_class);
	x->x_mode = mode;
	x->x_sample = sample;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_mode);
	floatinlet_new(&x->x_obj, &x->x_sample);
	x->x_last = 0;
	x->x_f = 0;
	x->x_mode = 0;
	
	if(mode) x->x_mode = mode;
	if(x->x_mode > 1) x->x_mode = 1;
	if(x->x_mode < 0) x->x_mode = 0;
    return (x);
}

static float round(float in)
{
	float y, round;
	int temp;
	{
		round = in * 10;
		temp = round;
		y = temp * 0.1;
	}
	return y;
}

static t_int *sieve_tilde_perform(t_int *w)
{
	t_sieve_tilde *x = (t_sieve_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f;
	int mode = x->x_mode;
    while (n--)
    {
		f = *in++;
		switch(mode){
		case(0):
			if (round(f) != round(x->x_sample))
			{
				*out++ = f;
				x->x_last = f;
			}
			else
			{
				*out++ = x->x_last;
			}
			break;
		case(1):
			if (round(f) == round(x->x_sample))
			{
				*out++ = f;
				x->x_last = f;
			}
			else
			{
				*out++ = x->x_last;
			}
		}
    }
    return (w+5);
}

static void sieve_tilde_dsp(t_sieve_tilde *x, t_signal **sp)
{
    dsp_add(sieve_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void sieve_tilde_setup(void)
{
    sieve_tilde_class = class_new(gensym("sieve~"), (t_newmethod)sieve_tilde_new, 0,
    	sizeof(t_sieve_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(sieve_tilde_class, t_sieve_tilde, x_f);
    class_addmethod(sieve_tilde_class, (t_method)sieve_tilde_dsp, gensym("dsp"), 0);
}
