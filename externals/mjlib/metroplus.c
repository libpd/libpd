#ifdef NT
#include "stdafx.h"
#include <io.h>
#endif
#include "m_pd.h"

#include <stdlib.h>
#include<time.h>
#include "metroplus.h"

/**
*	The metroplus object is a more complex version of the metro 
*	object - it allows a list of time intervals to be given which are 
*	sequentially used thus giving a more compelex timing source 
*	than the metro object 
*
*	the code is based on the metro code from the pd source code
*/

static t_class *metroplus_class;
static t_class *metroplus_2_class;

/**
*	clock tick - do a bang and wait the next 
*	time delay in the list
*/

static void metroplus_tick(t_metroplus *x)
{
    x->x_hit = 0;
    outlet_bang(x->x_obj.ob_outlet);
   if (!x->x_hit) clock_delay(x->x_clock, metroplus_getNextDelay(x) );
}

/**
*	switch the metroplus object on or off
*/

static void metroplus_float(t_metroplus *x, t_float f)
{
    if (f != 0) metroplus_tick(x);
    else clock_unset(x->x_clock);
    x->x_hit = 1;
}

/**
*	a bang turns us on  - a start message also calls this function
*/

static void metroplus_bang(t_metroplus *x)
{
    metroplus_float(x, 1);
}

/**
*	a stop message turns us off
*/

static void metroplus_stop(t_metroplus *x)
{
    metroplus_float(x, 0);
}

/**
*	free our clock and our timer array
*/

static void metroplus_free(t_metroplus *x)
{
    clock_free(x->x_clock);
	free( x->x_times );
	pd_free( &( (t_metroplus*)x->x_shadow)->x_obj.ob_pd );
}

/**
*	get the next delay time in the list - wrap 
*	if we have run over the end
*/

static float metroplus_getNextDelay( t_metroplus *x )
{
	if ( x->x_idx == x->x_size )
	{
		x->x_idx = 0;
	}
	return x->x_times[ x->x_idx++ ];	
}

/*
*	make a new metroplus - we can provide a list of times 
*	so read these in too
*/

static void *metroplus_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
    t_metroplus *x = (t_metroplus *)pd_new(metroplus_class);
	t_metroplus *x1 = (t_metroplus *)pd_new(metroplus_2_class);
    x->x_shadow=x1;
	x1->x_shadow=x;
	x->x_times = NULL;
	// a silly little kludge - out time_seq method assumes it is accessed from x1 
	// so we have to pass a pointer to this so it can dereference it
	metroplus_time_seq( x->x_shadow , s , argc , argv );
    x->x_clock = clock_new(x, (t_method)metroplus_tick);
    outlet_new(&x->x_obj, gensym("bang"));
	inlet_new( &x->x_obj , &x1->x_obj.ob_pd,0,0);
    //inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("tseq"));
    return (x);
}

/**
*	set a time sequence and free the old array
*/	

static void metroplus_time_seq( t_metroplus *x1, t_symbol *s, int ac, t_atom *av )
{
	int i;
	t_metroplus *x = x1->x_shadow;
	if ( x->x_times != NULL )
	{
		free( x->x_times );
	}
	if ( ac > 0 )
	{
		x->x_times = (float *) malloc( ac * sizeof( float ));
		for( i = 0 ; i < ac ; i++ )
		{
			float t = atom_getfloat( &av[i] );
			x->x_times[i] = t > 0 ? t : 10;
		}
		x->x_size=ac;
	}
	else
	{
		x->x_times = (float *) malloc( sizeof( float ));
		x->x_times[0] = 10;		
		x->x_size=1;		
	}
	x->x_idx = 0;
	x->x_hit = 0;
}

static void metroplus_time_float( t_metroplus *x1, t_float f )
{
	int i;
	t_metroplus *x = x1->x_shadow;
	post("here with %f" , f );
	if ( x->x_times != NULL )
	{
		free( x->x_times );
	}
	x->x_times = (float *) malloc( sizeof( float ));
	x->x_times[0] = f > 0 ? f : 10;
	x->x_size=1;
	x->x_idx = 0;
	x->x_hit = 0;
}

/**
*	make a new one and setup all of our messages
*/

 void metroplus_setup(void)
{
    metroplus_class = class_new(gensym("metroplus"), (t_newmethod)metroplus_new,
    	(t_method)metroplus_free, sizeof(t_metroplus), 0, A_GIMME, 0);
    metroplus_2_class = class_new(gensym("metroplus (second inlet)"), 0,
    	0, sizeof(t_metroplus), CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(metroplus_class, metroplus_bang);
    class_addmethod(metroplus_class, (t_method)metroplus_stop, gensym("stop"), 0);
	class_addmethod(metroplus_class, (t_method)metroplus_bang, gensym("start"), 0);    	
	class_addmethod(metroplus_2_class, (t_method)metroplus_time_seq, gensym("list"),A_GIMME,0);
	class_addmethod(metroplus_2_class, (t_method)metroplus_time_float ,gensym("float"),A_FLOAT,0);
	
}

