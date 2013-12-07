/* declarations for the pin~ object */

typedef struct _pin_tilde
{
     t_object x_obj;     
	 float p_prob;		
	 float p_ticktime;	 
	 int p_outchannel;
	 t_clock* p_clock;
	 long p_numticks;
	 int p_normalized_prob;
	 float x_f;
} t_pin_tilde;

t_int *pin_tilde_perform(t_int *w);
static void pin_tilde_dsp(t_pin_tilde *x, t_signal **sp);
static void pin_tilde_free(t_pin_tilde *x);
static void *pin_tilde_new(t_floatarg prob , t_floatarg tick);
static void pin_tilde_float(t_pin_tilde* x, t_float n);
static void pin_tilde_ticktime( t_pin_tilde* x, t_float tick );
static void pin_tilde_prob( t_pin_tilde* x, t_float prob );
static void pin_tilde_tick(t_pin_tilde *x);
	
