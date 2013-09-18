// sigpack
// for
// pure-data
// by weiss
// www.weiss-archiv.de

#include "m_pd.h"
#include <math.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

// ------------------------ hardlimit~ ----------------------------- 
// brick hard limiter with residue mixer
// code from swh_plugins by steve harris www.plugin.org.uk 

static t_class *hardlimit_tilde_class;

typedef struct _hardlimit_tilde
{
    t_object x_obj;
	t_sample x_limit_db;//-50.0 - 0.0
	t_sample x_wet_gain;//0.0 - 1.0
	t_sample x_res_gain;//0.0 - 1.0
	float x_f;
} t_hardlimit_tilde;

static void *hardlimit_tilde_new(t_floatarg limitdb, t_floatarg wetgain, t_floatarg resgain)
{
    t_hardlimit_tilde *x = (t_hardlimit_tilde *)pd_new(hardlimit_tilde_class);
	x->x_limit_db = limitdb;
	x->x_wet_gain = wetgain;
	x->x_res_gain = resgain;
	outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_limit_db);
	floatinlet_new(&x->x_obj, &x->x_wet_gain);
	floatinlet_new(&x->x_obj, &x->x_res_gain);
	x->x_f = 0;
	if (limitdb) x->x_limit_db = limitdb;
	else x->x_limit_db = 0.0;
	if (wetgain) x->x_wet_gain = wetgain;
	else x->x_wet_gain = 0.0;
	if (resgain) x->x_res_gain = resgain;
	else x->x_res_gain = 0.0;
    return (x);
}

static t_int *hardlimit_tilde_perform(t_int *w)
{
	t_hardlimit_tilde *x = (t_hardlimit_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f;
	float limit_g, sign, data, residue;
    while (n--)
    {
		f = *in++;
		limit_g = pow(10, x->x_limit_db / 20);
		sign = f < 0.0 ? -1.0 : 1.0;
		data = f * sign;
		residue = data > limit_g ? data - limit_g : 0.0;
		data -= residue;
		*out++ = sign * (x->x_wet_gain * data + x->x_res_gain * residue);
    }
    return (w+5);
}

static void hardlimit_tilde_dsp(t_hardlimit_tilde *x, t_signal **sp)
{
    dsp_add(hardlimit_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void hardlimit_tilde_setup(void)
{
    hardlimit_tilde_class = class_new(gensym("hardlimit~"), (t_newmethod)hardlimit_tilde_new, 0,
    	sizeof(t_hardlimit_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(hardlimit_tilde_class, t_hardlimit_tilde, x_f);
    class_addmethod(hardlimit_tilde_class, (t_method)hardlimit_tilde_dsp, gensym("dsp"), 0);
}
