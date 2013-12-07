/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ ustep~ ----------------------------- */
/* signal unity step function */

static t_class *ustep_tilde_class;

typedef struct _ustep_tilde
{
    t_object x_obj;
	t_sample x_mode;
	t_sample x_thres;
	float x_f;
} t_ustep_tilde;

static void *ustep_tilde_new(t_floatarg mode, t_floatarg thres)
{
    t_ustep_tilde *x = (t_ustep_tilde *)pd_new(ustep_tilde_class);
	x->x_mode = mode;
	x->x_thres = thres;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_mode);
	floatinlet_new(&x->x_obj, &x->x_thres);
	x->x_f = 0;
	if(mode) x->x_mode = mode;
	else x->x_mode = 0;
	if(x->x_mode > 1) x->x_mode = 1;
	if(x->x_mode < 0) x->x_mode = 0;
	if(thres) x->x_thres = thres;
	else x->x_thres = 0.5;
    return (x);
}

static t_int *ustep_tilde_perform(t_int *w)
{
	t_ustep_tilde *x = (t_ustep_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
	int mode = x->x_mode;
    while (n--)
    {
		f = *in++;
		switch(mode){
		case(0):
			if (f >= x->x_thres)
				value = 1;
			else
				value = 0;
			break;
		case(1):
			if (f >= x->x_thres)
				value = 1;
			else
				value = f;
		}
		*out++ = value;
    }
    return (w+5);
}

static void ustep_tilde_dsp(t_ustep_tilde *x, t_signal **sp)
{
    dsp_add(ustep_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void ustep_tilde_setup(void)
{
    ustep_tilde_class = class_new(gensym("ustep~"), (t_newmethod)ustep_tilde_new, 0,
    	sizeof(t_ustep_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(ustep_tilde_class, t_ustep_tilde, x_f);
    class_addmethod(ustep_tilde_class, (t_method)ustep_tilde_dsp, gensym("dsp"), 0);
}
