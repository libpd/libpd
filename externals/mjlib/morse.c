#ifdef NT
#include "stdafx.h"
#include <io.h>
#endif

#include "m_pd.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "morse.h"

/**
*	The morse object is designed to translate messages into 
*	morse code. There are two outlets - a dot outlet and a dash 
*	outlet (from left to right). In addition there is an end of 
*	current message outlet. Each character in the current message 
*	is emited upon reciept of a bang allowing external control over the 
*	timing. A dot lasts one bang - a dash lasts three bangs, the space
*	between dots and dashes is one bang, the space between characters is 
*	three bangs, the space between words is seven bangs
*	it currently only does digits and numbers
*/

static t_class *morse_class;
char* morseletter[] = { 
		".-",
	"-...",
	"-.-.",
	"-..",
	".",
	"..-.",
	"--.",
	"....",
	"..",
	".---",
	"-.-",
	".-..",
	"--",
	"-.",
	"---",
	".--.",
	"--.-",
	".-.",
	"...",
	"-",
	"..-",
	"...-",
	".--",
	"-..-",
	"-.--",
	"--..",
};
char	* morsedigit[] = {
	"-----",
	".----",
	"..---",
	"...--",
	"....-",
	".....",
	"-....",
	"--...",
	"---..",
	"----.",
};
	
char* wordspace = "X";


/**
*	a bang causes a reset to the start of the bar - used to 
*	synchronize multiple morse's. If the rhythm is not 
*	running it is started
*/


static void morse_bang(t_morse *x)
{
	if( x->x_spaceticks > 0 )
	{
		//post("Tick");
		x->x_spaceticks--;
	}
	else
	{
		if ( x->x_curmsg != NULL)
		{
			if ( x->x_curmsg->idx != x->x_curmsg->length )
			{
				if( x->x_curmsg->msg == wordspace )
				{
					//post("Doing wordspace");
					x->x_spaceticks =6;
					x->x_curmsg->idx = 0;
					x->x_curmsg = x->x_curmsg->next;
				}
				else
				{
					//post("Doing %c" , x->x_curmsg->msg[ x->x_curmsg->idx++ ]);
					if( x->x_curmsg->msg[ x->x_curmsg->idx ] == '.')
					{
						outlet_bang( x->x_dot );
						x->x_spaceticks = 1;
					}
					else
					{
						outlet_bang( x->x_dash );
						x->x_spaceticks = 3;
					}
					x->x_curmsg->idx++ ;
					
				}
			}
			else
			{
				if( x->x_curmsg->next != NULL )
				{
					if( x->x_curmsg->next->msg != wordspace )
					{
							//post( "Doing space" );				
							x->x_spaceticks =2;
					}	
					else if ( x->x_curmsg->next->next == NULL )
					{
						//post("message end");
						outlet_bang( x->x_end );
						x->x_curmsg->idx = 0;
						x->x_spaceticks = 0;
						x->x_curmsg = x->x_curmsg->next;
					}
				}		
				x->x_curmsg->idx = 0;
				x->x_curmsg = x->x_curmsg->next;
			}
		}
	}
}

static void morse_rewind( t_morse *x)
{
	x->x_curmsg->idx = 0;
	x->x_spaceticks = 0;
	x->x_curmsg = x->x_msg;
}


/**
*	free our clock and our timer array
*/

static void morse_free(t_morse *x)
{
	outlet_free( x->x_dot );
	outlet_free( x->x_dash );
	outlet_free( x->x_end );
	if( x->x_msg != NULL )
	{
		morse_freemsg( x->x_msg );	
	}
}

static void morse_freemsg( morse_msglet* msg)
{
	if ( msg->next != NULL )
	{
		morse_freemsg( msg->next );
	}
	freebytes( (void*) msg , sizeof( morse_msglet) );
}


/*
*	make a new morse - we can provide a list of times 
*	so read these in too
*/

static void *morse_new(t_symbol *s, int argc, t_atom *argv)
{
	float f;
	t_morse *x = (t_morse *)pd_new(morse_class);	    
	x->x_msg = NULL;
	// parse any settings
	if ( argc > 0 )
	{
		morse_message( x, s , argc , argv  );		
	}
	// make us some ins and outs    
    x->x_dot = outlet_new(&x->x_obj, gensym("dot"));
	x->x_dash = outlet_new(&x->x_obj, gensym("dash"));
	x->x_end = outlet_new(&x->x_obj, gensym("end"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("msg"));	
    return (x);
}

/**
*	set a message and free the old array
*/	

static void morse_message( t_morse *x, t_symbol *s, int ac, t_atom *av )
{
	int i,j,l;
	if ( x->x_msg != NULL )
	{
		morse_freemsg( x->x_msg );
		x->x_msg = NULL;				
		x->x_curmsg = NULL;
		x->x_spaceticks =0;
	}
	if ( ac > 0 )
	{
		char buf[256];
		for(  i = 0 ; i < ac ; i++ )
		{
			atom_string( &av[i] , buf, 255 );						
			l = strlen( buf );
#ifdef NT			
/* this is not part of ANSI or ISO standard C, 
	only Microsoft and Borland use it. */
			strlwr( buf );
#else
/* Probably needs a loop using tolower(char c) from ctype.h 
 * This way it'll just be case sensitive
 */
#endif
			for( j = 0 ; j < l ; j++ )
			{
				morse_add_msg_part( x , morse_lookup( buf[j] ));
			}
			morse_add_msg_part( x , wordspace );
		}			
		x->x_curmsg = x->x_msg;
		x->x_spaceticks =0;
	}
	else
	{
		// if there is no pattern it doens't do anything
		x->x_msg = NULL;				
		x->x_curmsg = NULL;
		x->x_spaceticks =0;
	}
}

/**
*	add a non null msg part onto the end of the message list - if its null
*	the lookup failed and we just ignore it
*/

static void morse_add_msg_part(  t_morse *x , char *msgpart )
{
	morse_msglet* idx;
	morse_msglet* nmsg;
	if ( msgpart != NULL )
	{
		idx  = x->x_msg;
		if ( idx == NULL )
		{
			nmsg = idx = (morse_msglet*) getbytes( sizeof( morse_msglet) );			
			x->x_msg = nmsg;
		}
		else
		{
			while( idx->next != NULL ) { idx = idx->next; }
			idx->next = nmsg =  (morse_msglet*) getbytes( sizeof( morse_msglet) );			
		}		
		nmsg->next = NULL;
		nmsg->idx = 0;
		if( msgpart == wordspace )
		{
			nmsg->length = -1;
		}
		else
		{
			nmsg->length = strlen( msgpart );
		}
		nmsg->msg = msgpart;
	}
}

/**
*	morse lookup returns a pointer to a character representation of the morse
*	code for a given character. If the character is not recognized then NULL 
*	is returned. DO NOT TRY TO FREE THE RETURNED POINTER - its part
*	of the array above
*/

static char *morse_lookup( char c )
{
	if( ( c>= 'a') && ( c<='z'))
	{
		return ( morseletter[ c - 'a'] );
	}
	else if( ( c>= '0') && ( c<='9'))
	{
		return ( morsedigit[ c - '0'] );
	}
	return NULL;
}

/**
*	make a new one and setup all of our messages
*/

 void morse_setup(void)
{
    morse_class = class_new(gensym("morse"), (t_newmethod)morse_new,
    	(t_method)morse_free, sizeof(t_morse), 0, A_GIMME, 0);    
    class_addbang(morse_class, morse_bang);
	class_addmethod(morse_class, (t_method)morse_message, gensym("msg" ), A_GIMME, 0);    	
	//class_addmethod(morse_class, (t_method)morse_set_time, gensym("timeinterval" ), A_FLOAT, 0);    	
	class_addmethod(morse_class, (t_method)morse_rewind,gensym("rewind"),0);
	//class_addmethod(morse_class, (t_method)morse_set_nonexclusive,gensym("nonexclusive"),0); 
	
}

