#include "m_pd.h"
#ifdef NT 
#include "stdafx.h"
#include <io.h>
#endif
#include <stdlib.h>
#include<time.h>

#include "synapseA~.h"

/* ------------------------ synapseA_tilde~ ----------------------------- */

static t_class *synapseA_tilde_class;

/**
*	do an average and see if it is bigger than our threshold
*	then check for state change and output accordingly
*/

t_int *synapseA_tilde_perform(t_int *w)
{
	float *in = (float *)(w[1]);
	t_synapseA_tilde *x = (t_synapseA_tilde *)(w[2]);
	int n = (t_int)(w[3]);

	t_float buf = 0.;

	while (n--)
	{
		buf += *in++;
	}
	if ( buf*x->n_inv > x->x_threshold )
	{
		if( !x->x_state )
		{
			x->x_state = 1;
			outlet_float( x->x_obj.ob_outlet , x->x_state );
			outlet_bang( x->x_onbang );
		}
	}
	else
	{
		if( x->x_state )
		{
			x->x_state = 0;
			outlet_float( x->x_obj.ob_outlet , x->x_state );
			outlet_bang( x->x_offbang );
		}
	}
	return (w+4);
}

/**
*	set up our dsp perform routine - it takes parameters
*	the input channel, the output channels ( left and right), 
*	the pin object and the number of samples in the array
*/

static void synapseA_tilde_dsp(t_synapseA_tilde *x, t_signal **sp)
{
	x->n_inv=1./sp[0]->s_n;
   dsp_add(synapseA_tilde_perform, 3,sp[0]->s_vec , x ,sp[0]->s_n);
}

/**
*	free up the tilde object - for now we only need 
*	to get rid of the clock
*/

static void synapseA_tilde_free(t_synapseA_tilde *x)
{

}

static void synapseA_tilde_threshold(t_synapseA_tilde *x, t_float f )
{
	if ( f > 0 )
	{
		x->x_threshold = f;
	}
	else
	{
		post( "Threshold must be bigger than 0  - setting to 0");
		x->x_threshold = 0;
	}
}

/**
*	make a new object - set up out internal variables 
*	and add our inlets and outlets
*/

static void *synapseA_tilde_new(t_floatarg prob , t_floatarg tick)
{
     // set up our inlets
	t_synapseA_tilde *x = (t_synapseA_tilde *)pd_new(synapseA_tilde_class);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("threshold"));
	outlet_new(&x->x_obj, gensym("float"));
	x->x_onbang = outlet_new(&x->x_obj, gensym("bang"));
	x->x_offbang = outlet_new(&x->x_obj, gensym("bang"));
	return (x);
}


/**
*	setup - add our methods and seed the random number generator
*/

 void synapseA_tilde_setup(void)
{	
    synapseA_tilde_class = class_new(gensym("synapseA~"), (t_newmethod) synapseA_tilde_new, (t_method) synapseA_tilde_free,
    	sizeof(t_synapseA_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);    
    CLASS_MAINSIGNALIN( synapseA_tilde_class, t_synapseA_tilde, x_f);
    class_addmethod(synapseA_tilde_class, (t_method) synapseA_tilde_dsp, gensym("dsp"), (t_atomtype)0);    	
	class_addmethod(synapseA_tilde_class, (t_method) synapseA_tilde_threshold, gensym("threshold") , A_DEFFLOAT , (t_atomtype)0 );
	
}

