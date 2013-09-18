/*
 * copyright Steve Harris, Ben Saylor
 * see GPL.txt
 */

#include <math.h>
#include <string.h>
#include "m_pd.h"

#ifdef _MSC_VER
#define inline __inline
#define M_PI 3.14159265358979323846
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

// Number of filter oversamples
#define F_R 3

// Denormalise floats, only actually needed for PIII and very recent PowerPC
#define FLUSH_TO_ZERO(fv) (((*(unsigned int*)&(fv))&0x7f800000)==0)?0.0f:(fv)

/* pd's samplerate */
float fs;

static t_class *svf_class;

typedef struct _svf
{
	t_object x_obj;
        float f;     // 2.0*sin(PI*fs/(fc*r));
        float q;     // 2.0*cos(pow(q, 0.1)*PI*0.5);
        float qnrm;  // sqrt(m/2.0f+0.01f);
        float h;     // high pass output
        float b;     // band pass output
        float l;     // low pass output
        float p;     // peaking output (allpass with resonance)
        float n;     // notch output
        float *op;   // pointer to output value
} t_svf;

/* Store data in SVF struct, takes the sampling frequency, cutoff frequency
   and Q, and fills in the structure passed */
//static inline void setup_svf(sv_filter *sv, float fs, float fc, float q, int t) {
static inline void setup_svf(t_svf *sv, float fc, float q) {
        sv->f = 2.0f * sin(M_PI * fc / (float)(fs * F_R));
        sv->q = 2.0f * cos(pow(q, 0.1f) * M_PI * 0.5f);
        sv->qnrm = sqrt(sv->q/2.0+0.01);
}

/* Run one sample through the SV filter. Filter is by andy@vellocet */
static inline float run_svf(t_svf *sv, float in) {
        float out;
        int i;

        in = sv->qnrm * in ;
        for (i=0; i < F_R; i++) {
                // only needed for pentium chips
                if(PD_BIGORSMALL(in)) in = 0.;
                if(PD_BIGORSMALL(sv->l)) sv->l = 0.;
        // OLD VERSION
        //in = FLUSH_TO_ZERO(in);
        //sv->l = FLUSH_TO_ZERO(sv->l);
	  // new versions, thanks to Damon Chaplin, inserted by Ed Kelly, not yet working!!!
	  //in  = ((int)in & 0x7f800000)==0?0.0f:in;
	   //sv->l = ((int)sv->l & 0x7f800000)==0?0.0f:sv->l;
                // very slight waveshape for extra stability
                sv->b = sv->b - sv->b * sv->b * sv->b * 0.001f;

                // regular state variable code here
                // the notch and peaking outputs are optional
                sv->h = in - sv->l - sv->q * sv->b;
                sv->b = sv->b + sv->f * sv->h;
                sv->l = sv->l + sv->f * sv->b;
                sv->n = sv->l + sv->h;
                sv->p = sv->l - sv->h;

                out = *(sv->op);
                in = out;
        }

        return out;
}

static void svf_setstate_LP(t_svf *sv)
{
	sv->op = &(sv->l);
}

static void svf_setstate_HP(t_svf *sv)
{
	sv->op = &(sv->h);
}

static void svf_setstate_BP(t_svf *sv)
{
	sv->op = &(sv->b);
}

static void svf_setstate_BR(t_svf *sv)
{
	sv->op = &(sv->n);
}

static void svf_setstate_AP(t_svf *sv)
{
	sv->op = &(sv->p);
}

static t_int *svf_perform(t_int *w)
{
	t_svf *obj = (t_svf *)(w[1]);
	t_float *in   = (t_float *)(w[2]);
	t_float *freq = (t_float *)(w[3]);
	t_float *q    = (t_float *)(w[4]);
	t_float *res  = (t_float *)(w[5]);
	t_float *out  = (t_float *)(w[6]);
	int n = (int)(w[7]);
	while (n--) {
		float f = *(in++);
		setup_svf(obj, *(freq++), *(q++));
		*(out++) = run_svf(obj, f + ((obj->b) * (*(res++))));
	}
	return (w+8);
}

static void svf_dsp(t_svf *x, t_signal **sp)
{
	dsp_add(svf_perform, 7, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[0]->s_n);
}

static void *svf_new(t_symbol *s, int argc, t_atom *argv)
{
	char string[11];
	t_svf *x = (t_svf *)pd_new(svf_class);

	svf_setstate_LP(x);
	if (argc > 0) {
		atom_string(argv, string, 10);
		if (!strcmp(string, "high"))
			svf_setstate_HP(x);
		if (!strcmp(string, "band"))
			svf_setstate_BP(x);
		if (!strcmp(string, "notch"))
			svf_setstate_BR(x);
		if (!strcmp(string, "peak"))
			svf_setstate_AP(x);
	}

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

void svf_tilde_setup(void)
{
	fs = sys_getsr();
	svf_class = class_new(gensym("svf~"), (t_newmethod)svf_new, 0, sizeof(t_svf), 0, A_GIMME, 0);
	class_addmethod(svf_class, nullfn, gensym("signal"), 0);
	class_addmethod(svf_class, (t_method)svf_dsp, gensym("dsp"), 0);
	class_addmethod(svf_class, (t_method)svf_setstate_LP, gensym("low"), 0);
	class_addmethod(svf_class, (t_method)svf_setstate_HP, gensym("high"), 0);
	class_addmethod(svf_class, (t_method)svf_setstate_BP, gensym("band"), 0);
	class_addmethod(svf_class, (t_method)svf_setstate_BR, gensym("notch"), 0);
	class_addmethod(svf_class, (t_method)svf_setstate_AP, gensym("peak"), 0);
}
