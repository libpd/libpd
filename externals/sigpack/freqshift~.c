/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ freqshift~ ----------------------------- */
/* frequency shifter */
/* code from swh_plugins by steve harris www.plugins.org.uk */

#define SIN_T_SIZE 64
#define D_SIZE 256
#define NZEROS 200

static t_class *freqshift_tilde_class;

typedef struct _freqshift_tilde
{
    t_object x_obj;
	t_float x_shift;//[0 - 5000]
	float *x_delay;
	unsigned int x_dptr;
	t_float x_fs;
	t_float x_last_shift;
	t_float x_phi;
	float *x_sint;
	float x_f;
} t_freqshift_tilde;

static void *freqshift_tilde_new(t_floatarg shift)
{
	unsigned int i;

    t_freqshift_tilde *x = (t_freqshift_tilde *)pd_new(freqshift_tilde_class);
	//x->x_shift = shift;
    outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_shift);
	x->x_fs = sys_getsr();
	x->x_delay = (float *)getbytes(D_SIZE * sizeof(float));
	x->x_sint = (float *)getbytes(SIN_T_SIZE * sizeof(float));
	x->x_dptr = 0;
	x->x_phi = 0.0f;
	x->x_last_shift = 0.0f;
	x->x_f = 0;
	for (i = 0; i < SIN_T_SIZE; i++) {
	x->x_sint[i] = sin(2.0f * M_PI * (float)i / (float)SIN_T_SIZE);
	}
	if (shift) x->x_shift = shift;
	else x->x_shift = 0;
    return (x);
}

/* The non-zero taps of the Hilbert transformer */
static float xcoeffs[] = {
     +0.0008103736f, +0.0008457886f, +0.0009017196f, +0.0009793364f,
     +0.0010798341f, +0.0012044365f, +0.0013544008f, +0.0015310235f,
     +0.0017356466f, +0.0019696659f, +0.0022345404f, +0.0025318040f,
     +0.0028630784f, +0.0032300896f, +0.0036346867f, +0.0040788644f,
     +0.0045647903f, +0.0050948365f, +0.0056716186f, +0.0062980419f,
     +0.0069773575f, +0.0077132300f, +0.0085098208f, +0.0093718901f,
     +0.0103049226f, +0.0113152847f, +0.0124104218f, +0.0135991079f,
     +0.0148917649f, +0.0163008758f, +0.0178415242f, +0.0195321089f,
     +0.0213953037f, +0.0234593652f, +0.0257599469f, +0.0283426636f,
     +0.0312667947f, +0.0346107648f, +0.0384804823f, +0.0430224431f,
     +0.0484451086f, +0.0550553725f, +0.0633242001f, +0.0740128560f,
     +0.0884368322f, +0.1090816773f, +0.1412745301f, +0.1988673273f,
     +0.3326528346f, +0.9997730178f, -0.9997730178f, -0.3326528346f,
     -0.1988673273f, -0.1412745301f, -0.1090816773f, -0.0884368322f,
     -0.0740128560f, -0.0633242001f, -0.0550553725f, -0.0484451086f,
     -0.0430224431f, -0.0384804823f, -0.0346107648f, -0.0312667947f,
     -0.0283426636f, -0.0257599469f, -0.0234593652f, -0.0213953037f,
     -0.0195321089f, -0.0178415242f, -0.0163008758f, -0.0148917649f,
     -0.0135991079f, -0.0124104218f, -0.0113152847f, -0.0103049226f,
     -0.0093718901f, -0.0085098208f, -0.0077132300f, -0.0069773575f,
     -0.0062980419f, -0.0056716186f, -0.0050948365f, -0.0045647903f,
     -0.0040788644f, -0.0036346867f, -0.0032300896f, -0.0028630784f,
     -0.0025318040f, -0.0022345404f, -0.0019696659f, -0.0017356466f,
     -0.0015310235f, -0.0013544008f, -0.0012044365f, -0.0010798341f,
     -0.0009793364f, -0.0009017196f, -0.0008457886f, -0.0008103736f,
};

static float f_clamp(float x, float a, float b)
{
	const float x1 = fabs(x - a);
	const float x2 = fabs(x - b);

	x = x1 + a + b;
	x -= x2;
	x *= 0.5;

	return x;
}

static int f_round(t_float f) {
#if PD_FLOAT_PRECISION == 64
    return (int)lround(f);
#else
    return (int)lroundf(f);
#endif
}

// this relies on type-punning, which is not allowed in C99 or 64-bit
#if 0
// Round float to int using IEEE int* hack
static int f_round(float f) {
        f += (3<<22);
        return *((int*)&f) - 0x4b400000;
}
#endif

// Cubic interpolation function
static float cube_interp(const float fr, const float inm1, const float
                                in, const float inp1, const float inp2)
{
	return in + 0.5f * fr * (inp1 - inm1 +
	 fr * (4.0f * inp1 + 2.0f * inm1 - 5.0f * in - inp2 +
	 fr * (3.0f * (in - inp1) - inm1 + inp2)));
}

static t_int *freqshift_tilde_perform(t_int *w)
{
	t_freqshift_tilde *x = (t_freqshift_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out1 = (t_float *)(w[3]);
	t_float *out2 = (t_float *)(w[4]);
    int n = (int)(w[5]);
	float f, hilb, rm1, rm2, frac_p;
	float shift_i = x->x_last_shift;
	float sample_count = sys_getblksize();
	unsigned int i;
	int int_p;
	const float shift_c = f_clamp(x->x_shift, 0.0f, 10000.0f);
	const float shift_inc = (shift_c - x->x_last_shift) / (float)sample_count;
	const float freq_fix = (float)SIN_T_SIZE / x->x_fs;
    while (n--)
    {
		f = *in++;
		x->x_delay[x->x_dptr] = f;
		/* Perform the Hilbert FIR convolution
		 * (probably FFT would be faster) */
		hilb = 0.0f;
	    for (i = 0; i <= NZEROS/2; i++) {
	        hilb += (xcoeffs[i] * x->x_delay[(x->x_dptr - i*2) & (D_SIZE - 1)]);
		}

	    /* Calcuate the table positions for the sine modulator */
	    int_p = f_round(floor(x->x_phi));

	    /* Calculate ringmod1, the transformed input modulated with a shift Hz
	     * sinewave. This creates a +180 degree sideband at source-shift Hz and
	     * a 0 degree sindeband at source+shift Hz */
	    frac_p = x->x_phi - int_p;
	    rm1 = hilb * cube_interp(frac_p, x->x_sint[int_p], x->x_sint[int_p+1],
	                             x->x_sint[int_p+2], x->x_sint[int_p+3]);

	    /* Calcuate the table positions for the cosine modulator */
	    int_p = (int_p + SIN_T_SIZE / 4) & (SIN_T_SIZE - 1);

	    /* Calculate ringmod2, the delayed input modulated with a shift Hz
	     * cosinewave. This creates a 0 degree sideband at source+shift Hz
	     * and a -180 degree sindeband at source-shift Hz */
	    rm2 = x->x_delay[(x->x_dptr - 100) & (D_SIZE - 1)] * cube_interp(frac_p,
	          x->x_sint[int_p], x->x_sint[int_p+1], x->x_sint[int_p+2], x->x_sint[int_p+3]);

		/* Output the sum and differences of the ringmods. The +/-180 degree
	     * sidebands cancel (more of less) and just leave the shifted
	     * components */
	    *out1++ = (rm2 - rm1) * 0.5f; /*downshifting*/
	    *out2++ = (rm2 + rm1) * 0.5f; /*upshifting*/

		x->x_dptr = (x->x_dptr + 1) & (D_SIZE - 1);
	    x->x_phi += shift_i * freq_fix;
	    while (x->x_phi > SIN_T_SIZE) {
			x->x_phi -= SIN_T_SIZE;
		}
		shift_i += shift_inc;
	}	
    return (w+6);
}

static void freqshift_tilde_dsp(t_freqshift_tilde *x, t_signal **sp)
{
    dsp_add(freqshift_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void freqshift_tilde_free(t_freqshift_tilde *x)
{
	if(x->x_delay)
		freebytes(x->x_delay, D_SIZE * sizeof(float));
	if(x->x_sint)
		freebytes(x->x_sint, SIN_T_SIZE + 4 * sizeof(float));
}

void freqshift_tilde_setup(void)
{
    freqshift_tilde_class = class_new(gensym("freqshift~"), (t_newmethod)freqshift_tilde_new, (t_method)freqshift_tilde_free,
    	sizeof(t_freqshift_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(freqshift_tilde_class, t_freqshift_tilde, x_f);
    class_addmethod(freqshift_tilde_class, (t_method)freqshift_tilde_dsp, gensym("dsp"), 0);
}
