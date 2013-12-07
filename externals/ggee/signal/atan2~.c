/* (C) Guenter Geiger <geiger@epy.co.at> */



#include "math.h"
#include <m_pd.h>

/* ----------------------------- atan2 ----------------------------- */
static t_class *atan2_class;


#define INVTWOPI 0.15915494f

typedef struct _atan2
{
    t_object x_obj;
} t_atan2;

static void *atan2_new(t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) post("+~: extra arguments ignored");
    {
	t_atan2 *x = (t_atan2 *)pd_new(atan2_class);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	outlet_new(&x->x_obj, &s_signal);
	return (x);
    }
}

t_int *atan2_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *in2 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);

    int n = (int)(w[4]);
    while (n--) *out++ = (t_float) atan2(*in1++,*in2++) *INVTWOPI; 
    return (w+5);
}

t_int *atan2_perf8(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *in2 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
    	float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
    	float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

    	float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
    	float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

    	out[0] = (t_float) atan2(f0,g0)*INVTWOPI;
	out[1] = (t_float) atan2(f1,g1)*INVTWOPI;
	out[2] = (t_float) atan2(f2,g2)*INVTWOPI;
	out[3] = (t_float) atan2(f3,g3)*INVTWOPI;
    	out[4] = (t_float) atan2(f4,g4)*INVTWOPI;
	out[5] = (t_float) atan2(f5,g5)*INVTWOPI;
	out[6] = (t_float) atan2(f6,g6)*INVTWOPI;
	out[7] = (t_float) atan2(f7,g7)*INVTWOPI;
    }
    return (w+5);
}

void dsp_add_atan2(t_sample *in1, t_sample *in2, t_sample *out, int n)
{
    if (n&7)
    	dsp_add(atan2_perform, 4, in1, in2, out, n);
    else	
    	dsp_add(atan2_perf8, 4, in1, in2, out, n);
}

static void atan2_dsp(t_atan2 *x, t_signal **sp)
{
    dsp_add_atan2(sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void atan2_tilde_setup(void)
{
    atan2_class = class_new(gensym("atan2~"), (t_newmethod)atan2_new, 0,
    	sizeof(t_atan2), 0, A_GIMME, 0);
    class_addmethod(atan2_class, nullfn, gensym("signal"), 0);
    class_addmethod(atan2_class, (t_method)atan2_dsp, gensym("dsp"), 0);
}
