#ifdef _WIN32
#include "stdafx.h"
#include <io.h>
#endif
#include "m_pd.h"
#include <stdlib.h>
#include<time.h>
#include <string.h>
#include <stdio.h>
#include "n2m.h"

char* notes_up[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
int octaveoffset[11]  = {  0 , 12, 24 ,36, 48, 60, 72, 84, 96, 108 ,120 };

/**
*	The n2m object is designed to output a midi note number 
*	in response to a note name of the form <note name><octave>
*	for example C5 or D#3.
*/

static t_class *n2m_class;

/*
*	make a new n2m it takes one parameter - the percentage error
*/

static void *n2m_new( t_float f )
{
	t_n2m *x = (t_n2m *)pd_new(n2m_class);			    
	// make us an output for the note number
    outlet_new(&x->x_obj, gensym("float"));	
    return (x);
}

static void n2m_free(t_n2m *x)
{
	// nothing doing here for now
}

/**
*	set the error factor
*/

static void n2m_notename(  t_n2m *x, t_symbol *s, int ac, t_atom *av )
{
	char buf[255];	
	char note[255];
	int octave;
	int i;
	for(  i = 0 ; i < ac ; i++ )
	{
		atom_string( &av[i] , buf, 255 );
		//post("Symbol [%s]", buf );
		splitsym( buf , note , &octave );
		//post( "Which is %s note and %u octave and midi value %d" , note , octave , midilookup( note , octave ) );
		outlet_float( x->x_obj.ob_outlet , midilookup( note , octave ) );
	}	
}

/**
*	make a new one and setup all of our messages
*/

 void n2m_setup(void)
{
	 n2m_class = class_new(gensym("n2m"), (t_newmethod)n2m_new,
    	(t_method)n2m_free, sizeof(t_n2m), 0, 0);    
    class_addmethod(n2m_class, (t_method)n2m_notename, gensym("note" ), A_GIMME, 0);    	
	
}

/**
*	splitsym takes a note symbol and splits it into note and octave
*	if note not specified it defaults to C and if octave not specified 
*	it defaults to 4
*/

static void splitsym( char* buf , char* note, int* octave )
{
	int i,j;
	int split = -1;
	for( i = 0 ; buf[i] != 0 ; i++ )
	{
		if ( ( buf[i] >= '0' ) && ( buf[i] <= '9'))
		{
			split = i;
			if ( i > 0 )
			{
				for( j=0; j < i;j++) { note[j] = buf[j]; }
				note[i] = 0;
			}
			else
			{	
				note[0] = 'C';
				note[1] = 0;
			}
			sscanf( buf + i , "%u" , octave );
			break;
		}
	}
	if ( split == -1 )
	{
		i = 0;
		while( buf[i] != 0 )
		{
			note[i] = buf[i];
			i++;
		}
		note[i] = 0;
		*octave = 4;
	}
}

/**
*	return a midi note value for a given note name and octave 
*/

static int midilookup( char* note , int octave )
{
	int i,j;
	int nnum = 4;
	for( i = 0 ; i < 12 ; i++ )
	{
#ifdef NT
/* stricmp() is not an ANSI or ISO standard C function */
		if ( stricmp( note , notes_up[i]) == 0)
		{
			nnum = i;
			break;
		}
#else
/* replacing with a ANSI function, but it'll now be case sensitive */
		if ( strcmp( note , notes_up[i]) == 0)
		{
			nnum = i;
			break;
		}
#endif
	}
	return octaveoffset[octave + 1 ] + nnum;
}

