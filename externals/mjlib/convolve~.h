/* declarations for the pin~ object */

typedef struct _convolve_tilde
{
     t_object x_obj;     
	 float p_prob;		
	 float p_ticktime;	 
	 int p_outchannel;
	 t_clock* p_clock;
	 long p_numticks;
	 int p_normalized_prob;
	 float x_f;
} t_convolve_tilde;

t_int *convolve_tilde_perform(t_int *w);
static void convolve_tilde_dsp(t_convolve_tilde *x, t_signal **sp);
static void convolve_tilde_free(t_convolve_tilde *x);
static void *convolve_tilde_new(t_floatarg prob , t_floatarg tick);
static void convolve_tilde_float(t_convolve_tilde* x, t_float n);
static void convolve_tilde_ticktime( t_convolve_tilde* x, t_float tick );
static void convolve_tilde_prob( t_convolve_tilde* x, t_float prob );
static void convolve_tilde_tick(t_convolve_tilde *x);
	