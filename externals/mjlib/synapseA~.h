/* declarations for the pin~ object */

typedef struct _synapseA_tilde
{
     t_object x_obj;     	 
	 t_float x_f;
	 t_float x_threshold;
	 t_outlet *x_onbang;
	 t_outlet *x_offbang;
	 t_float n_inv;
	 t_float x_state;
} t_synapseA_tilde;

t_int *synapseA_tilde_perform(t_int *w);
static void synapseA_tilde_dsp(t_synapseA_tilde *x, t_signal **sp);
static void synapseA_tilde_free(t_synapseA_tilde *x);
static void *synapseA_tilde_new(t_floatarg prob , t_floatarg tick);
static void synapseA_tilde_float(t_synapseA_tilde* x, t_float n);
static void synapseA_tilde_threshold(t_synapseA_tilde *x, t_float f );

	
