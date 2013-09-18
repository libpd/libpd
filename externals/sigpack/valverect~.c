// sigpack
// for
// pure-data
// by weiss
// www.weiss-archive.de

#include "m_pd.h"
#include <math.h>
#include <string.h>
#include <stdint.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

// ------------------------ valverect~ ----------------------------- 
// valve rectifier
// code from swh_plugins by steve harris www.plugin.org.uk 

// 1.0 / ln(2)
#define LN2R 1.442695041f
/* Fast exponentiation function, y = e^x */
#define f_exp(x) f_pow2(x * LN2R) 

static t_class *valverect_tilde_class;

typedef struct _valverect_tilde
{
    t_object x_obj;
	t_sample x_sag;//sag level[0-1]
	t_sample x_dist_p;//distortion[0-1]
	unsigned int x_apos;
	float *x_avg;
	int x_avg_size;
	float x_avg_sizer;
	float x_avgs;
	float x_lp1tm1;
	float x_lp2tm1;
	float x_s_rate;
	float x_f;
} t_valverect_tilde;

static void *valverect_tilde_new(t_floatarg sag, t_floatarg dist)
{
    t_valverect_tilde *x = (t_valverect_tilde *)pd_new(valverect_tilde_class);
	x->x_sag = sag;
	x->x_dist_p = dist;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_sag);
	floatinlet_new(&x->x_obj, &x->x_dist_p);
	x->x_s_rate = sys_getsr();
	x->x_avg_size = x->x_s_rate / 9;//number of samples in averaging buffer
	x->x_avg_sizer = 9.0f / x->x_s_rate;//reciprocal of above
	x->x_avg = (float *)getbytes(x->x_avg_size * sizeof(float));//averaging buffer
	memset(x->x_avg, 0, x->x_avg_size * sizeof(float));
	x->x_avgs = 0.0f;//sum of samples in averaging buffer
	x->x_apos = 0;//position in averaging buffer
	x->x_lp1tm1 = 0.0f;//last value in lowpass 1
	x->x_lp2tm1 = 0.0f;//last value in lowpass 2
	x->x_f = 0;
	if (sag) x->x_sag = sag;
	else x->x_sag = 0.0;
	if (dist) x->x_dist_p = dist;
	else x->x_dist_p = 0.0;
    return (x);
}

/* Andrew Simper's pow(2, x) aproximation from the music-dsp list */

/* 32 bit "pointer cast" union */
typedef union {
        float f;
        int32_t i;
} ls_pcast32;

/* union version */
static inline float f_pow2(float x)
{
        ls_pcast32 *px, tx, lx;
        float dx;

        px = (ls_pcast32 *)&x; // store address of float as long pointer
        tx.f = (x-0.5f) + (3<<22); // temporary value for truncation
        lx.i = tx.i - 0x4b400000; // integer power of 2
        dx = x - (float)lx.i; // float remainder of power of 2

        x = 1.0f + dx * (0.6960656421638072f + // cubic apporoximation of 2^x
                   dx * (0.224494337302845f +  // for x in the range [0, 1]
                   dx * (0.07944023841053369f)));
        (*px).i += (lx.i << 23); // add integer power of 2 to exponent

        return (*px).f;
}

static t_int *valverect_tilde_perform(t_int *w)
{
	t_valverect_tilde *x = (t_valverect_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float abs, q, fx;
	const float dist = x->x_dist_p * 40.0f + 0.1f;
	float f;
    while (n--)
    {
		f = *in++;
		abs = fabs(f);
		if(abs > x->x_lp1tm1) {
			x->x_lp1tm1 = abs;
		}
		else {
			x->x_lp1tm1 = 0.9999f * x->x_lp1tm1 + 0.0001f * abs;
		}
		x->x_avgs -= x->x_avg[x->x_apos];
		x->x_avgs += x->x_lp1tm1;
		x->x_avg[x->x_apos] = x->x_lp1tm1;
		x->x_apos %= x->x_avg_size;

		x->x_lp2tm1 = 0.999f * x->x_lp2tm1 + x->x_avgs * x->x_avg_sizer * 0.001f;
		q = x->x_lp1tm1 * x->x_sag - x->x_lp2tm1 * 1.02f - 1.0f;
		if(q > -0.01f) {
			q = -0.01f;
		}
		else if(q < -1.0f) {
			q = -1.0f;
		}

		if(f == q) {
			fx = 1.0f / dist + q / (1.0f - f_exp(dist * q));
		}
		else {
			fx = (f - q) / (1.0f - f_exp(-dist * (f - q))) + q / (1.0f - f_exp(dist * q));
		}
		*out++ = fx;
    }
    return (w+5);
}

static void valverect_tilde_dsp(t_valverect_tilde *x, t_signal **sp)
{
    dsp_add(valverect_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void valverect_tilde_free(t_valverect_tilde *x)
{
	freebytes(x->x_avg, x->x_avg_size * sizeof(float));
}

void valverect_tilde_setup(void)
{
    valverect_tilde_class = class_new(gensym("valverect~"), (t_newmethod)valverect_tilde_new, (t_method)valverect_tilde_free,
    	sizeof(t_valverect_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(valverect_tilde_class, t_valverect_tilde, x_f);
    class_addmethod(valverect_tilde_class, (t_method)valverect_tilde_dsp, gensym("dsp"), 0);
}
