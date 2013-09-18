/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ vowel~ ----------------------------- */
/* simple formant filter */
/* code from musicdsp.org posted by alex@smartelectronix.com */

static t_class *vowel_tilde_class;

typedef struct _vowel_tilde
{
    t_object x_obj;
	t_sample x_vowelnum;
	float x_f;
} t_vowel_tilde;

static void *vowel_tilde_new(t_floatarg vowelnum)
{
    t_vowel_tilde *x = (t_vowel_tilde *)pd_new(vowel_tilde_class);
	x->x_vowelnum = vowelnum;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_vowelnum);
	x->x_f = 0;
	if(vowelnum) x->x_vowelnum = vowelnum;
	else x->x_vowelnum = 0;
    return (x);
}

const double coeff[5][11]= {
	{ 8.11044e-06,
      8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289,
	  -151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471 ///A
	},
	{ 4.36215e-06,
      8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248, ///E
	  -149.6496211,87.78352223, -34.60687431, 8.282228154, -0.914150747
	},
	{ 3.33819e-06,
	  8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618,
	  -150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035 ///I
	},
	{ 1.13572e-06,
	  8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544, ///O
	  -154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233
	},
	{ 4.09431e-07,
	  8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141, ///U
	  -153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753
	}
};

static double memory[10] = {0,0,0,0,0,0,0,0,0,0};

float formant_filter (float in, int vowelnum)
{
	float res;
	res= (float) (coeff[vowelnum][0]*in +
		coeff[vowelnum][1]*memory[0] +
		coeff[vowelnum][2]*memory[1] +
		coeff[vowelnum][3]*memory[2] +
		coeff[vowelnum][4]*memory[3] +
		coeff[vowelnum][5]*memory[4] +
		coeff[vowelnum][6]*memory[5] +
		coeff[vowelnum][7]*memory[6] +
		coeff[vowelnum][8]*memory[7] +
		coeff[vowelnum][9]*memory[8] +
		coeff[vowelnum][10]*memory[9]);

	memory[9]=memory[8];
	memory[8]=memory[7];
	memory[7]=memory[6];
	memory[6]=memory[5];
	memory[5]=memory[4];
	memory[4]=memory[3];
	memory[3]=memory[2];
	memory[2]=memory[1];
	memory[1]=memory[0];
	memory[0]=(double)res;
	return res;
}

static t_int *vowel_tilde_perform(t_int *w)
{
	t_vowel_tilde *x = (t_vowel_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value;
    while (n--)
    {
		f = *in++;
		value = formant_filter(f, (int)x->x_vowelnum);
		*out++ = value;
    }
    return (w+5);
}

static void vowel_tilde_dsp(t_vowel_tilde *x, t_signal **sp)
{
    dsp_add(vowel_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void vowel_tilde_setup(void)
{
    vowel_tilde_class = class_new(gensym("vowel~"), (t_newmethod)vowel_tilde_new, 0,
    	sizeof(t_vowel_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(vowel_tilde_class, t_vowel_tilde, x_f);
    class_addmethod(vowel_tilde_class, (t_method)vowel_tilde_dsp, gensym("dsp"), 0);
	class_addmethod(vowel_tilde_class, (t_method)formant_filter, gensym("formant_filter"), A_GIMME, A_NULL); 
}
