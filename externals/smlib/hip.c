#include "defines.h"

/*--------------- hip ---------------*/

typedef struct hipctl
{
    t_float c_x;
    t_float c_coef;
} t_hipctl;

typedef struct hip
{
    t_object x_obj;
    t_float x_sr;
    t_float x_hz;
    t_hipctl x_cspace;
    t_hipctl *x_ctl;
    t_float x_f;
} t_hip;

t_class *hip_class;

static void hip_ft1(t_hip *x, t_floatarg f);

static void *hip_new(t_floatarg f)
{
    t_hip *x = (t_hip *)pd_new(hip_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
    outlet_new(&x->x_obj, gensym("float"));
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x = 0;
    hip_ft1(x, f);
    x->x_f = 0;
    return (x);
}

static void hip_ft1(t_hip *x, t_floatarg f)
{
    if (f < 0.001) f = 10;
    x->x_hz = f;
    x->x_ctl->c_coef = 1 - f * (2 * 3.14159f);
    if (x->x_ctl->c_coef < 0) x->x_ctl->c_coef = 0;
}

static void hip_perform(t_hip *x, t_float in)
{
    t_hipctl *c = x->x_ctl;
    t_float last = c->c_x;
    t_float coef = c->c_coef;
	t_float out;

	t_float new = in + coef * last;
	out = new - last;
	last = new;

	/* NAN protect */
    if (!((last <= 0) || (last >= 0)))
    	last = 0;
    c->c_x = last;

    outlet_float(x->x_obj.ob_outlet, out);
}

static void hip_clear(t_hip *x, t_floatarg q)
{
    x->x_cspace.c_x = 0;
}

void hip_setup(void)
{
    hip_class = class_new(gensym("hip"), (t_newmethod)hip_new, 0,
	sizeof(t_hip), 0, A_DEFFLOAT, 0);
	class_addfloat(hip_class, (t_method)hip_perform);
    class_addmethod(hip_class, (t_method)hip_ft1,
    	gensym("ft1"), A_FLOAT, 0);
    class_addmethod(hip_class, (t_method)hip_clear, gensym("clear"), 0);
}