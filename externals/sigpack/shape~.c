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

// ------------------------ shape~ ----------------------------- 
// this external reshapes the wave by an exponential function, inspiration was taken from the Nord module of the same name.
// code from swh_plugins by steve harris www.plugin.org.uk  

static t_class *shape_tilde_class;

typedef struct _shape_tilde
{
    t_object x_obj;
	t_sample x_shapep;
	float x_f;
} t_shape_tilde;

static void *shape_tilde_new(t_floatarg waveshape)
{
    t_shape_tilde *x = (t_shape_tilde *)pd_new(shape_tilde_class);
	x->x_shapep = waveshape;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_shapep);
	x->x_f = 0;
	if (waveshape) x->x_shapep = waveshape;
	else x->x_shapep = 0.0f;
    return (x);
}

static t_int *shape_tilde_perform(t_int *w)
{
	t_shape_tilde *x = (t_shape_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
	float shape = 0.0f;
	if (x->x_shapep < 1.0f && x->x_shapep < -1.0f) {
		shape = 1.0f;
	} else if (shape < 0) {
		shape = -1.0f / shape;
	} else {
		shape = x->x_shapep;
	}

    while (n--)
    {
		f = *in++;
		if (f < 0.0f) {
			value = -pow(-f, shape);
		} else {
			value = pow(f, shape);
		}
		*out++ = value;
    }
    return (w+5);
}

static void shape_tilde_dsp(t_shape_tilde *x, t_signal **sp)
{
    dsp_add(shape_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void shape_tilde_setup(void)
{
    shape_tilde_class = class_new(gensym("shape~"), (t_newmethod)shape_tilde_new, 0,
    	sizeof(t_shape_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(shape_tilde_class, t_shape_tilde, x_f);
    class_addmethod(shape_tilde_class, (t_method)shape_tilde_dsp, gensym("dsp"), 0);
}
