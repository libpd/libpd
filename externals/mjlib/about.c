#ifdef NT
#include "stdafx.h"
#include <io.h>
#endif

#include "m_pd.h"
#include <stdlib.h>
#include <time.h>
#include "about.h"

/**
*	The about object is designed to output a number that is 
*	"about" the same as its input. A percentage error factor
*	gives the deviation.
*/

static t_class *about_class;

/**
*	a float causes a number that is within x_err range 
*	of the input number
*/

static void about_float(t_about *x , t_float f)
{
	float errp = (( (float) rand() / (float) RAND_MAX) *  ( x->x_err * 2));
	float tenp =  ((errp - x->x_err)/100)  * f;
	//float ep = (errp/100) * f;
	//float correction = ep - tenp;
	//float outf = f + correction;
	float outf = f + tenp;
	outlet_float( x->x_obj.ob_outlet , outf );
}


/*
*	make a new about it takes one parameter - the percentage error
*/

static void *about_new( t_float f )
{
	t_about *x = (t_about *)pd_new(about_class);			
    about_set_err( x , f );		
	// make us some ins and outs    
    outlet_new(&x->x_obj, gensym("float"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("error"));
	
    return (x);
}

static void about_free(t_about *x)
{
	// nothing doing here for now
}

/**
*	set the error factor
*/

static void about_set_err( t_about *x, t_float f )
{
	x->x_err = f;
}

/**
*	make a new one and setup all of our messages
*/

 void about_setup(void)
{
	 srand( (unsigned) time( NULL ) );
    about_class = class_new(gensym("about"), (t_newmethod)about_new,
    	(t_method)about_free, sizeof(t_about), 0, A_DEFFLOAT , 0);    
    class_addfloat( about_class, about_float );
    class_addmethod(about_class, (t_method)about_set_err, gensym("error" ), A_FLOAT, 0);    	
	
}

