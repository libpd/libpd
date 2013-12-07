#ifdef NT
#include "stdafx.h"
#include <io.h>
#endif
#include "m_pd.h"
#include <stdlib.h>
#include <time.h>
#include "monorhythm.h"

/**
*	The monorhythm object is designed to help build polyrhythms. Given 
*	a time interval and a pattern it produces the pattern within the time
*	interval given. Thus if two where set going with the same time interval
*	the two patterns (assuming they where different) would play against
*	each other. 
*
*	this filename is spelt wrong 'cos I can't spell
*/

static t_class *monorhythm_class;

/**
*	clock tick - do a bang and wait the next 
*	time delay in the list
*/

static void monorhythm_tick(t_monorhythm *x)
{
	if ( x->t_running )
	{
		monorhythm_do_beat( x );
		clock_delay(x->x_clock, x->x_beattime );		
	}
}

static void monorhythm_do_beat( t_monorhythm* x )
{
	float beat;
	if ( x->x_idx == x->x_size )
	{
		x->x_idx = 0;
	
	}
	if ( x->x_idx == 0)
	{
		outlet_bang( x->x_sync );
	}
	beat = x->x_pattern[ x->x_idx++ ];	
	if ( beat > 1 )
	{
		if ( x->t_exclusive == 0 )
		{
			outlet_bang( x->x_bang );
		}
		outlet_bang( x->x_accent );
	}
	else if ( beat ==  1 )
	{
		outlet_bang( x->x_bang );
	}	
}


/**
*	a bang causes a reset to the start of the bar - used to 
*	synchronize multiple monorhythm's. If the rhythm is not 
*	running it is started
*/


static void monorhythm_bang(t_monorhythm *x)
{
    if ( x->x_beattime > 0 )
	{		
		monorhythm_restart( x );
	}
}

/**
*	reset the rhythm to start at the beginning
*/

static void monorhythm_restart(t_monorhythm *x)
{
	if ( x->x_beattime > 0 )
	{
		x->t_running = 1;
		x->x_idx = 0;
		monorhythm_do_beat( x );
		clock_delay(x->x_clock, x->x_beattime );
	}
}

/**
*	a stop message turns us off
*/

static void monorhythm_stop(t_monorhythm *x)
{
	x->t_running = 0;
}

/**
*	set exclusive mode
*/

static void monorhythm_set_exclusive(t_monorhythm *x)
{
	x->t_exclusive = 1;
}

/**
*	set nonexclusive mode
*/

static void monorhythm_set_nonexclusive(t_monorhythm *x)
{
	x->t_exclusive = 0;
}

/**
*	free our clock and our timer array
*/

static void monorhythm_free(t_monorhythm *x)
{
    clock_free(x->x_clock);
	free( x->x_pattern );	
}

/*
*	make a new monorhythm - we can provide a list of times 
*	so read these in too
*/

static void *monorhythm_new(t_symbol *s, int argc, t_atom *argv)
{
	float f;
	t_monorhythm *x = (t_monorhythm *)pd_new(monorhythm_class);	
    x->x_pattern = NULL;
	// parse any settings
	if ( argc > 0 )
	{
		f = atom_getfloat( &argv[0] );
		monorhythm_set_time( x , f );
		monorhythm_pattern_seq( x, s , argc - 1 , argv + 1 );		
	}
	x->t_running=0;
	x->t_exclusive = 0;
	// make us some ins and outs
    x->x_clock = clock_new(x, (t_method)monorhythm_tick);
    x->x_bang = outlet_new(&x->x_obj, gensym("bang"));
	x->x_accent = outlet_new(&x->x_obj, gensym("accent"));
	x->x_sync = outlet_new(&x->x_obj, gensym("sync"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("pattern"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("timeinterval"));	
    return (x);
}

/**
*	set a time sequence and free the old array
*/	

static void monorhythm_pattern_seq( t_monorhythm *x, t_symbol *s, int ac, t_atom *av )
{
	int i;
	if ( x->x_pattern != NULL )
	{
		free( x->x_pattern );
	}
	if ( ac > 0 )
	{
		x->x_pattern = (float *) malloc( ac * sizeof( float ));
		for( i = 0 ; i < ac ; i++ )
		{
			float t = atom_getfloat( &av[i] );
			x->x_pattern[i] = t;
		}
		x->x_size=ac;
		monorhythm_calculate_beat_interval( x );
	}
	else
	{
		// if there is no pattern it doens't do anything
		x->x_pattern = NULL;		
		x->x_size=0;		
		x->t_running = 0;
	}
	x->x_idx = 0;
}

/**
*	the time interval is divided by the number of beats that are 
*	going to happen in order to get the beat time. If this would 
*	be invallid for any reason it is set to 0 and the rhythm is stopped
*/

static void monorhythm_calculate_beat_interval( t_monorhythm *x )
{
	if ( ( x->x_size > 0 ) && ( x->x_time > 0 ))
	{
		x->x_beattime = x->x_time / x->x_size;
	}
	else
	{
		x->x_beattime = 0;
		x->t_running = 0;
	}
}

/**
*	set the time - recalculate the beat time
*/

static void monorhythm_set_time( t_monorhythm *x, t_float f )
{
	x->x_time = f;
	monorhythm_calculate_beat_interval( x );
}

/**
*	make a new one and setup all of our messages
*/

 void monorhythm_setup(void)
{
    monorhythm_class = class_new(gensym("monorhythm"), (t_newmethod)monorhythm_new,
    	(t_method)monorhythm_free, sizeof(t_monorhythm), 0, A_GIMME, 0);    
    class_addbang(monorhythm_class, monorhythm_bang);
    class_addmethod(monorhythm_class, (t_method)monorhythm_stop, gensym("stop"), 0);
	class_addmethod(monorhythm_class, (t_method)monorhythm_bang, gensym("start"), 0);    
	class_addmethod(monorhythm_class, (t_method)monorhythm_pattern_seq, gensym("pattern" ), A_GIMME, 0);    	
	class_addmethod(monorhythm_class, (t_method)monorhythm_set_time, gensym("timeinterval" ), A_FLOAT, 0);    	
	class_addmethod(monorhythm_class, (t_method)monorhythm_set_exclusive,gensym("exclusive"),0);
	class_addmethod(monorhythm_class, (t_method)monorhythm_set_nonexclusive,gensym("nonexclusive"),0); 
	
}

