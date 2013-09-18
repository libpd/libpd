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
#define HARMONICS 11

// ------------------------ harmgen~ -----------------------------
// harmonic generator
// code from swh_plugins by steve harris www.plugins.org.uk

static t_class *harmgen_tilde_class;

typedef struct _harmgen_tilde
{
    t_object x_obj;
	t_sample x_mag1;
	t_sample x_mag2;
	t_sample x_mag3;
	t_sample x_mag4;
	t_sample x_mag5;
	t_sample x_mag6;
	t_sample x_mag7;
	t_sample x_mag8;
	t_sample x_mag9;
	t_sample x_mag10;
	float x_itm;
	float x_otm;
	float x_f;
} t_harmgen_tilde;

static void *harmgen_tilde_new(t_floatarg mag1, t_floatarg mag2, t_floatarg mag3, t_floatarg mag4, t_floatarg mag5, t_floatarg mag6, t_floatarg mag7, t_floatarg mag8, t_floatarg mag9, t_floatarg mag10)
{
    t_harmgen_tilde *x = (t_harmgen_tilde *)pd_new(harmgen_tilde_class);
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_mag1);
	floatinlet_new(&x->x_obj, &x->x_mag2);
	floatinlet_new(&x->x_obj, &x->x_mag3);
	floatinlet_new(&x->x_obj, &x->x_mag4);
	floatinlet_new(&x->x_obj, &x->x_mag5);
	floatinlet_new(&x->x_obj, &x->x_mag6);
	floatinlet_new(&x->x_obj, &x->x_mag7);
	floatinlet_new(&x->x_obj, &x->x_mag8);
	floatinlet_new(&x->x_obj, &x->x_mag9);
	floatinlet_new(&x->x_obj, &x->x_mag10);
	x->x_f = 0;
	if(mag1) x->x_mag1 = mag1;
	else x->x_mag1 = 1;
	if(mag2) x->x_mag2 = mag2;
	else x->x_mag2 = 1;
	if(mag3) x->x_mag3 = mag3;
	else x->x_mag3 = 1;
	if(mag4) x->x_mag4 = mag4;
	else x->x_mag4 = 1;
	if(mag5) x->x_mag5 = mag5;
	else x->x_mag5 = 1;
	if(mag6) x->x_mag6 = mag6;
	else x->x_mag6 = 1;
	if(mag7) x->x_mag7 = mag7;
	else x->x_mag7 = 1;
	if(mag8) x->x_mag8 = mag8;
	else x->x_mag8 = 1;
	if(mag9) x->x_mag9 = mag9;
	else x->x_mag9 = 1;
	if(mag10) x->x_mag10 = mag10;
	else x->x_mag10 = 1;
    return (x);
}

/* Calculate Chebychev coefficents from partial magnitudes, adapted from
 * example in Num. Rec. */
void chebpc(float c[], float d[])
{
    int k, j;
    float sv, dd[HARMONICS];

    for (j = 0; j < HARMONICS; j++) {
        d[j] = dd[j] = 0.0;
    }

    d[0] = c[HARMONICS - 1];

    for (j = HARMONICS - 2; j >= 1; j--) {
        for (k = HARMONICS - j; k >= 1; k--) {
            sv = d[k];
            d[k] = 2.0 * d[k - 1] - dd[k];
            dd[k] = sv;
        }
        sv = d[0];
        d[0] = -dd[0] + c[j];
        dd[0] = sv;
    }

    for (j = HARMONICS - 1; j >= 1; j--) {
        d[j] = d[j - 1] - dd[j];
    }
    d[0] = -dd[0] + 0.5 * c[0];
}

static t_int *harmgen_tilde_perform(t_int *w)
{
	t_harmgen_tilde *x = (t_harmgen_tilde *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	unsigned long i;
	float mag_fix, y, f, value;
	float mag[HARMONICS] = {0.0f, x->x_mag1, x->x_mag2, x->x_mag3, x->x_mag4, x->x_mag5, x->x_mag6,
		x->x_mag7, x->x_mag8, x->x_mag9, x->x_mag10};
	float p[HARMONICS];

	// Normalise magnitudes
	mag_fix = (fabs(x->x_mag1) + fabs(x->x_mag2) + fabs(x->x_mag3) + fabs(x->x_mag4) +
	           fabs(x->x_mag5) + fabs(x->x_mag6) + fabs(x->x_mag7) + fabs(x->x_mag8) +
	           fabs(x->x_mag9) + fabs(x->x_mag10));
	if (mag_fix < 1.0f) {
	  mag_fix = 1.0f;
	} else {
	  mag_fix = 1.0f / mag_fix;
	}
	for (i=0; i<HARMONICS; i++) {
	  mag[i] *= mag_fix;
	}

	// Calculate polynomial coefficients, using Chebychev aproximation
	chebpc(mag, p);
    while (n--)
    {
		f = *in1++;

		// Calculate the polynomial using Horner's Rule
		y = p[0] + (p[1] + (p[2] + (p[3] + (p[4] + (p[5] + (p[6] + (p[7] +
			(p[8] + (p[9] + p[10] * f) * f) * f) * f) * f) * f) * f) * f) *
			f) * f;

		// DC offset remove (odd harmonics cause DC offset)
		x->x_otm = 0.999f * x->x_otm + y - x->x_itm;
		x->x_itm = y;
		*out++ = x->x_otm;
    }
    return (w+5);
}

static void harmgen_tilde_dsp(t_harmgen_tilde *x, t_signal **sp)
{
    dsp_add(harmgen_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void harmgen_tilde_setup(void)
{
    harmgen_tilde_class = class_new(gensym("harmgen~"), (t_newmethod)harmgen_tilde_new, 0,
    	sizeof(t_harmgen_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(harmgen_tilde_class, t_harmgen_tilde, x_f);
    class_addmethod(harmgen_tilde_class, (t_method)harmgen_tilde_dsp, gensym("dsp"), 0);
}
