#ifdef NT
#include "stdafx.h"
#include <io.h>
#endif
#include "m_pd.h"
#include <stdlib.h>
#include<time.h>
#include "prob.h"

/**
*	The prob object is designed to generate random events 
*	with a given probability - essentially every clock tick
*	it looks to see if it should generate an event or not
*/

static t_class *prob_class;

/**
*	clock tick - do a bang and wait the next 
*	time delay in the list
*/

static void prob_tick(t_prob *x)
{
	if ( x->x_running )
	{
		if ( prob_event( x) )
		{
			outlet_bang(x->x_obj.ob_outlet);
		}
		clock_delay(x->x_clock, x->x_time );		
	}
}

/**
*	prob event looks to see if we should generate and event or not
*/

static int prob_event( t_prob* x )
{
	int ret = 0;
	 if ( rand() < x->x_probability * RAND_MAX)
	 {
		ret = 1;
	 }
	 return ret;
}

/**
*	a bang causes a reset to the start of the bar - used to 
*	synchronize multiple prob's. If the rhythm is not 
*	running it is started
*/

static void prob_bang(t_prob *x)
{
	x->x_running = 1;
	clock_delay(x->x_clock, x->x_time );		
}

/**
*	a stop message turns us off
*/

static void prob_stop(t_prob *x)
{
	x->x_running = 0;
}

/**
*	free our clock and our timer array
*/

static void prob_free(t_prob *x)
{
    clock_free(x->x_clock);	
}

/*
*	make a new prob - we can provide a list of times 
*	so read these in too
*/

static void *prob_new(t_float t , t_float probability )
{
	t_prob *x = (t_prob *)pd_new(prob_class);		
    prob_set_time( x , t );
	prob_set_probability( x , probability );
	x->x_running=0;
	// make us some ins and outs
    x->x_clock = clock_new(x, (t_method)prob_tick);
    outlet_new(&x->x_obj, gensym("bang"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("time"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("probability"));	
    return (x);
}

/**
*	set the probability
*/

static void prob_set_probability( t_prob *x, t_float f )
{
	if ( f < 0 )
	{
		post("probability must be between 0 and 1 ");
		f = 0;
	}
	else if( f > 1 )
	{
		post("probability must be between 0 and 1 ");
		f = 1;
	}
	x->x_probability = f;	
}


/**
*	set the time 
*/

static void prob_set_time( t_prob *x, t_float f )
{
	x->x_time = f;	
}

/**
*	make a new one and setup all of our messages
*/

 void prob_setup(void)
{
	 srand( (unsigned) time( NULL ) );
    prob_class = class_new(gensym("prob"), (t_newmethod)prob_new,
    	(t_method)prob_free, sizeof(t_prob), 0, A_DEFFLOAT , A_DEFFLOAT, 0);    
    class_addbang(prob_class, prob_bang);
    class_addmethod(prob_class, (t_method)prob_stop, gensym("stop"), 0);
	class_addmethod(prob_class, (t_method)prob_bang, gensym("start"), 0);    
	class_addmethod(prob_class, (t_method)prob_set_probability, gensym("probability" ), A_FLOAT, 0);    	
	class_addmethod(prob_class, (t_method)prob_set_time, gensym("time" ), A_FLOAT, 0);    	
	
}

