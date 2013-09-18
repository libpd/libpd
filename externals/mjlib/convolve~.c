#include "m_pd.h"
#ifdef NT 
#include "stdafx.h"
#include <io.h>
#endif
#include <stdlib.h>
#include<time.h>

#include "convolve~.h"

/* ------------------------ convolve_tilde~ ----------------------------- */

static t_class *convolve_tilde_class;

/**
*	the perform routine unpacks its parameters 
*	looks to see if time is zero (do channel prob 
*	everytime) if it is to chooses a channel.
*	the routine then copies everything in the input
*	to the choosen output
*/

t_int *convolve_tilde_perform(t_int *w)
{
     float *in = (float *)(w[1]);
    float *outl = (float *)(w[2]);
	float *outr = (float *)(w[3]);
    t_convolve_tilde*obj = (t_convolve_tilde *)(w[4]);
    int n = (t_int)(w[5]);



   return w+6;
}

/**
*	set up our dsp perform routine - it takes parameters
*	the input channel, the output channels ( left and right), 
*	the pin object and the number of samples in the array
*/

static void convolve_tilde_dsp(t_convolve_tilde *x, t_signal **sp)
{
   dsp_add(convolve_tilde_perform, 5,sp[0]->s_vec, sp[1]->s_vec , sp[2]->s_vec , x ,sp[0]->s_n);
}

/**
*	free up the tilde object - for now we only need 
*	to get rid of the clock
*/

static void convolve_tilde_free(t_convolve_tilde *x)
{
	clock_free( x->p_clock );
}

/**
*	make a new object - set up out internal variables 
*	and add our inlets and outlets
*/

static void *convolve_tilde_new(t_floatarg prob , t_floatarg tick)
{
	t_convolve_tilde *x = (t_convolve_tilde *)pd_new(convolve_tilde_class);
	if ( prob < 0 )
	{
		post("probability must be between 0 and 1 ");
		prob = 0;
	}
	else if( prob > 1 )
	{
		post("probability must be between 0 and 1 ");
		prob = 1;
	}
	else if (prob == 0 )
	{
		// note that prob defaullts to 0.5
		prob = 0.5;
	}
	x->p_prob = prob;
	x->p_normalized_prob = prob * RAND_MAX;
	// set up our clocks
	x->p_ticktime = tick;   
	x->p_clock = clock_new(x, (t_method) convolve_tilde_tick);
	if (x->p_ticktime > 0) 
	{
		clock_delay(x->p_clock, x->p_ticktime);
	}
	// start off with a random channel
	if  ( rand() < x->p_normalized_prob )
	{
		x->p_outchannel=0;
	}
	else
	{
		x->p_outchannel=1;
	}
     // set up our inlets
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("prob"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("tick"));
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));	
	return (x);
}

/**
*	ticktime has been set - we only care about ticks above
*	zero. 
*/

static void convolve_tilde_ticktime( t_convolve_tilde* x, t_float tick )
{
	x->p_ticktime = tick;
	if ( x->p_ticktime > 0 ) 
	{
		clock_delay(x->p_clock, x->p_ticktime);
	}
}

/**
*	allows the probability to be set - note that although 
*	we accept a probability between 0 and 1 we need to
*	normalize it becuase rand() produces a number between
*	0 and rand_max. We precalucluate the normalized 
*	number becuase we sometimes use it in the dsp routine 
*	(if tick is zero).
*/

static void convolve_tilde_prob( t_convolve_tilde* x, t_float prob )
{
	if ( prob < 0 )
	{
		post("probability must be between 0 and 1 ");
		prob = 0;
	}
	else if( prob > 1 )
	{
		post("probability must be between 0 and 1 ");
		prob = 1;
	}
	x->p_prob=prob;
	x->p_normalized_prob = prob * RAND_MAX;
}

/**
*	clock tick - choose a channel and wait again
*/

static void convolve_tilde_tick(t_convolve_tilde *x)
{
	if  ( rand() < x->p_normalized_prob )
	{
		x->p_outchannel=0;
	}
	else
	{
		x->p_outchannel=1;
	}
    if (x->p_ticktime > 0) 
	{
		clock_delay(x->p_clock, x->p_ticktime);
	}
}

/**
*	setup - add our methods and seed the random number generator
*/

void convolve_tilde_setup(void)
{	
	 srand( (unsigned) time( NULL ) );
    convolve_tilde_class = class_new(gensym("convolve~"), (t_newmethod) convolve_tilde_new, (t_method) convolve_tilde_free,
    	sizeof(t_convolve_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);    
    CLASS_MAINSIGNALIN( convolve_tilde_class, t_convolve_tilde, x_f);
    class_addmethod(convolve_tilde_class, (t_method) convolve_tilde_dsp, gensym("dsp"), (t_atomtype)0);    	
	class_addmethod(convolve_tilde_class, (t_method) convolve_tilde_ticktime, gensym("tick") , A_DEFFLOAT , (t_atomtype)0 );
	class_addmethod(convolve_tilde_class, (t_method) convolve_tilde_prob, gensym("prob") , A_DEFFLOAT , (t_atomtype)0 );
	
}

