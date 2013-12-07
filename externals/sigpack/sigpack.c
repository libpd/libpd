#ifndef VERSION
#define VERSION "0.04"
#endif

#include <m_pd.h>


typedef struct _sigpack
{
     t_object x_obj;
} t_sigpack;

static t_class* sigpack_class;

void chop_tilde_setup();
void decimate_tilde_setup();
void diode_tilde_setup();
void foldback_tilde_setup();
void foldover_tilde_setup();
void freqdiv_tilde_setup();
void freqshift_tilde_setup();
void hardlimit_tilde_setup();
void harmgen_tilde_setup();
void impulse_tilde_setup();
void rectify_tilde_setup();
void round_tilde_setup();
void saturate_tilde_setup();
void shape_tilde_setup();
void sieve_tilde_setup();
void split_tilde_setup();
void transient_tilde_setup();
void ustep_tilde_setup();
void valverect_tilde_setup();
void vowel_tilde_setup();
void wavewrap_tilde_setup();

static void* sigpack_new(t_symbol* s) {
    t_sigpack *x = (t_sigpack *)pd_new(sigpack_class);
    return (x);
}

void sigpack_setup(void) 
{
    sigpack_class = class_new(gensym("sigpack"), (t_newmethod)sigpack_new, 0,
    	sizeof(t_sigpack), 0,0);

	 chop_tilde_setup();
	 decimate_tilde_setup();
	 diode_tilde_setup();
	 foldback_tilde_setup();
	 foldover_tilde_setup();
	 freqdiv_tilde_setup();
	 freqshift_tilde_setup();
	 hardlimit_tilde_setup();
	 harmgen_tilde_setup();
	 impulse_tilde_setup();
	 rectify_tilde_setup();
	 round_tilde_setup();
	 saturate_tilde_setup();
	 shape_tilde_setup();
	 sieve_tilde_setup();
	 split_tilde_setup();
	 transient_tilde_setup();
	 ustep_tilde_setup();
	 valverect_tilde_setup();
	 vowel_tilde_setup();
	 wavewrap_tilde_setup();
     
     post("sigpack"" "VERSION " | 12.2007 | www.weiss-archiv.de");
}
