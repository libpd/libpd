/* -------------------------  mlife  ------------------------------------------ */
/*                                                                              */
/* A linear cellular automata object for PureData.                              */
/* Based on 'mlife' by pauld@koncon.nl                                          */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */
#include "m_pd.h"


static char *version = "mlife v0.1, a linear cellular automata object for Pd\n"
                       "            written by Olaf Matthes <olaf.matthes@gmx.de>";

#undef DEBUG
//#define DEBUG


#define INTSIZE sizeof(unsigned int) * 8
#define LONGSIZE sizeof(unsigned long) * 8
#define DEFAULT_DIE_LO 2
#define DEFAULT_DIE_HI 3
#define DEFAULT_N_SIZE 3	

#define MAXSIZE 1024

#include <stdio.h>
#include <stdlib.h>

/* -------------------- random stuff -------------------- */
static union {
	unsigned long		next;
	struct {
		unsigned short		: 1;
		unsigned short		n : 15;
	}					bits;
} seed = { 1 };


/*
 *  rand - pseudo-random number generator
 *
 */

static int my_rand(void)
{
	seed.next = seed.next * 1103515245 + 12345;
	return(seed.bits.n);
}


/*
 *  srand - seed pseudo-random number generator
 *
 */

static void my_srand(unsigned n)
{
	seed.next = n;
}
/* --------------------------------------------------------- */

//
// Maxlife object data structure
//
typedef struct maxlife
{
	t_object ml_ob;				// must begin every object 
	t_int universe[MAXSIZE];	// array of cells - alive and dead	
	t_outlet *out[MAXSIZE];		// outlets 
	t_int size;					// size of the CA field/world
	t_int view_start;			// Start of viewport
	t_int view_size;			// length of viewport and number of outlets
	t_int rule_die_lo;			// death if less than this
	t_int rule_die_hi;			// death if greater then this
	t_int neighbourhood_size;	// # of cells either side to check
	t_int closed;				// closed universe if true
} t_maxlife;

//
// Function prototypes for our methods and functions
//
static t_class *mlife_class;		// global variable that points to the Maxlife class

//
// ml_nextgeneration
// Step through the array, applying the rules and reset each cell
// accordingly.  For each cell:
//		- 	Check the number of neighbours (watch for "closed") 
//			using neighbourhood_size
//
//		-	If neighbours < rule_die_lo the cell is cleared (0)
//
//		-	If neighbours > rule_die_hi the cell is cleared (0)
//
//		-	Else the cell is filled (1)
//
//  not called by Pd itself
//
static void ml_nextgeneration(t_maxlife *mlp)
{
	register long i, j, k;
	register long size, neighbourhood_size, max_neighbours, min_neighbours, neighbours;
	register int closed, out_of_bounds;
	
	// get the important info a little closer to hand
	size = mlp->size;
	closed = mlp->closed;
	neighbourhood_size = mlp->neighbourhood_size;
	max_neighbours = mlp->rule_die_hi;
	min_neighbours = mlp->rule_die_lo;
	
#ifdef DEBUG
	post("mlife:next_generation called, vars n_size=%ld, n_max=%ld, n_min=%ld", 
		neighbourhood_size, max_neighbours, min_neighbours);
#endif
	// for each cell...
	for(i=0; i<size; i++)
	{
		neighbours = 0L;	// reset count of neighbours
		
		// for each neighbourhood, count the neighbours
		for(j = i-neighbourhood_size; j <= i+neighbourhood_size; j++)
		{
			k = j;	// k is the index into the array, decoupled from j
			// don't go outside our array, or wrap if closed
			if(closed)
			{
				if(j < 0)
					k = size + j;		// j is a negative number.  size-1?
				if(j > size-1L)
					k = j - size - 1L;	// not size-1 ???

				if(j != i)			// skip our own location in this roundup
					if(mlp->universe[k])	// if there's a neighbour inc count
						neighbours++;
			}
			else		// not closed
			{
				out_of_bounds = 0;
				if(k < 0L)			// start of array
				{
					out_of_bounds = 1;
					k = 0L;
				}
				if(k > size-1L)
				{
					out_of_bounds = 1;
					k = size-1L;	// end of array
				}
			
				if((j != i)	&& !out_of_bounds)	// skip our own location in this roundup
					if(mlp->universe[k])	// if there's a neighbour inc count
						neighbours++;
			}
			
		}	// end of neighbour search
		
		// based on number of neighbours, fill or clear this cell (i)
		if((neighbours < min_neighbours) || (neighbours > max_neighbours))
			mlp->universe[i] = 0;
		else
			mlp->universe[i] = 1;
	}
}

//
// method to set the die_lo number
//
static void ml_set_die_lo(t_maxlife *mlp, t_floatarg die_lo)
{
	mlp->rule_die_lo = (t_int)die_lo;
}

//
// method to set the die_hi number
//
static void ml_set_die_hi(t_maxlife *mlp, t_floatarg die_hi)
{
	mlp->rule_die_hi = (t_int)die_hi;
}

//
// method to set the die_lo number
//
static void ml_set_neighbourhood(t_maxlife *mlp, t_floatarg n_size)
{
	mlp->neighbourhood_size = (t_int)n_size;
}

//
// bang method outputs bangs for filled cells within the view port
//
static void ml_bang(t_maxlife *mlp)	// argument is a pointer to an instance
{	
	register long i, view_start;
	
#ifdef DEBUG
	post("mlife:ml_bang called, sending bangs");
#endif

	view_start = mlp->view_start;

	// loop through the outlets right->left sending bangs if alive
	for(i=view_start+mlp->view_size-2; i>=view_start-1; i--)
	{
		// send a bang out the appropriate outlet 
		if(mlp->universe[i])
			outlet_bang(mlp->out[i-view_start+1]);
	}

	ml_nextgeneration(mlp);
}

//
// int method outputs ints for ALL cells in the view port (1=filled, 0=not)
//
static void ml_int(t_maxlife *mlp, t_floatarg dummy)
{	
	t_int i, view_start;
	
#ifdef DEBUG
	post("mlife:ml_int method called");
#endif

	view_start = mlp->view_start;

	// loop through the outlets right -> left sending ints
	for(i = view_start + mlp->view_size - 2; i >= view_start - 1; i--)
	{
		//outlet_int(mlp->out[i-view_start+1], mlp->universe[i]);
		if(mlp->universe[i] == 1)
			outlet_float(mlp->out[i-view_start+1], 1);
		else if(mlp->universe[i] == 0)
			outlet_float(mlp->out[i-view_start+1], 0);
		else
			error("mlife: corrupted data in universe[] array!");
	}

	ml_nextgeneration(mlp);
}


//
// method to print out the array
//
static void ml_display(t_maxlife *mlp)
{
	register long i;
	char s[MAXSIZE];
	
#ifdef DEBUG	
	post("mlife: display method called");
#endif

	for(i = 0; i < mlp->size; i++)			// print the universe array
	{
		//s[i] = itoa(mlp->universe[i]); // my very primitive itoa()
		if(mlp->universe[i])
			s[i] = '1';
		else 
			s[i] = '0';
	}
	s[mlp->size] = '\0';	// null terminate the string
	post("%s", s);
}

//
// method to fill the array with a number
//
static void ml_fill(t_maxlife *mlp, t_floatarg fill_no)
{
	t_int  n;
	register long i, j;
	
	for(i=mlp->size-1; i >= 0; i--)	// fill the universe array from the back
	{
		n = (t_int)fill_no;

		for(j=(long)INTSIZE; j>0; j--, i--, n>>=1)
		{
			if(i < 0L)
			{
				return;
			}
			if(n & 01)
				mlp->universe[i] = 1;
			else
				mlp->universe[i] = 0;
		}
	}
}

//
// method to fill the array with a random number
//
static void ml_randfill(t_maxlife *mlp)
{
	unsigned int s, rnum;
	register unsigned int n;
	register long i, j;
	
#ifdef DEBUG	
	post("mlife: randfill method called");
#endif

	s = (unsigned int)clock_getlogicaltime();	// set seed to a new number
	my_srand(s);					// reseed the 'random' generator 
	rnum = (unsigned int)my_rand();

	for(i=mlp->size - 1; i>=0; i--)		// fill the universe array from the back
	{
		n = rnum;

		for(j=(long)INTSIZE; j>0; j--, i--, n>>=1)
		{
			if(i < 0L)
			{
				return;
			}
			if(n & 01)
				mlp->universe[i] = 1;
			else
				mlp->universe[i] = 0;
		}
	}
}

//
// method to seed the array with a number
//
static void ml_seed(t_maxlife *mlp, t_floatarg start, t_floatarg fill_no)
{
	t_int n;
	register long i, st, end;
	
#ifdef DEBUG	
	post("mlife: seed method called");
#endif

	st = (t_int)start;
	n = (t_int)fill_no;

	if(st+(t_int)INTSIZE > mlp->size)
		i = mlp->size - 1;
	else
		i = st+(long)INTSIZE - 1;

	// init the universe array from the back i>=start?
	for(; i >= start - 1; i--, n>>=1)
	{
		if(n & 01)
			mlp->universe[i] = 1;
		else
			mlp->universe[i] = 0;
	}
}

//
// method to seed the array with a random number
//
static void ml_randseed(t_maxlife *mlp, t_floatarg start)
{
	unsigned long s, rnum;
	register unsigned long n;
	register long i, st;
	
#ifdef DEBUG	
	post("mlife: randseed method called, INTSIZE=%ld", (long)INTSIZE);
#endif
	//if((start < 1) || (start > mlp->size-(long)INTSIZE))
	if(start < 1)
	{
		error("Randseed start parameter must be between 1 and %ld", mlp->size);
		return;
	}

	s = (unsigned long)clock_getlogicaltime();	// set seed to a new number
	my_srand(s);					// reseed the 'random' generator 
	rnum = (unsigned long)my_rand();
	n = (unsigned int)rnum;
	st = start;

	if(st+(t_int)INTSIZE > mlp->size)
		i = mlp->size - 1;
	else
		i = st+(t_int)INTSIZE - 1;

	// init the universe array from the back
	for(; i>=st-1; i--, n>>=1)
	{
		if(n & 01)
			mlp->universe[i] = 1;
		else
			mlp->universe[i] = 0;

	}
}


//
// function to create an instance of the mlife class
//
static void *ml_new(t_floatarg size, t_floatarg view_start, t_floatarg view_size, t_floatarg closed)
{
	long i;
	t_maxlife *mlp = (t_maxlife *)pd_new(mlife_class);

	// check all args...
	if((size>MAXSIZE) || (size<1))
	{
		post("mlife: size argument must be between 1 and %ld", MAXSIZE);
		size = 1.0;
	}
	if(view_start < 1)
	{
		post("mlife: view_start argument must be between 1 and %ld", size);
		view_start = 1.0;
	}
	if((view_size < 1) || (view_size+view_start > size+1))
	{
		post("mlife: viewsize argument must be between 1 and %ld", size-view_start);
		view_size = 1.0;
	}


	// set up our structure
	mlp->size = (t_int)size;			
	mlp->view_start = (t_int)view_start;
	mlp->view_size = (t_int)view_size;
	mlp->rule_die_lo = DEFAULT_DIE_LO;			// 2
	mlp->rule_die_hi = DEFAULT_DIE_HI;			// 3
	mlp->neighbourhood_size = DEFAULT_N_SIZE;	// 3
	mlp->closed = (t_int)closed;
	for(i=0; i<MAXSIZE; i++)
		mlp->universe[i] = 0;
	
	// create outlets - last first!
	for(i = 0; i < mlp->view_size; i++)
		mlp->out[i] = outlet_new(&mlp->ml_ob, gensym("float"));
		
#ifdef DEBUG	
	post("mlife: finished building object");
	post("mlife: INTSIZE=%ld, LONGSIZE=%ld", (long)INTSIZE, (long)LONGSIZE);
#endif

	post("mlife: defaults are: lo=%ld, hi=%ld, nset=%ld", (long)DEFAULT_DIE_LO, (long)DEFAULT_DIE_HI, DEFAULT_N_SIZE);

	return(mlp);				// always return a copy of the created object 
}

static void ml_free(t_maxlife *mlp)
{
	long i;
	
#ifdef DEBUG	
	post("mlife:freeing outlet memory");
#endif
/*	for(i=mlp->view_size-1; i>=0; i--)
		freeobject(mlp->out[i]); */
}

#ifndef MAXLIB
void mlife_setup(void)
{
    mlife_class = class_new(gensym("mlife"), (t_newmethod)ml_new,
    	(t_method)ml_free, sizeof(t_maxlife), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_randfill, gensym("randfill"), 0);
	class_addmethod(mlife_class, (t_method)ml_fill, gensym("fill"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_set_die_lo, gensym("lo"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_set_die_hi, gensym("hi"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_set_neighbourhood, gensym("nset"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_randseed, gensym("randseed"), A_FLOAT, 0);
	class_addmethod(mlife_class, (t_method)ml_seed, gensym("seed"), A_FLOAT, A_FLOAT, 0);
	class_addmethod(mlife_class, (t_method)ml_display, gensym("display"), 0);
    class_addfloat(mlife_class, ml_int);
	class_addbang(mlife_class, ml_bang);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_mlife_setup(void)
{
    mlife_class = class_new(gensym("maxlib_mlife"), (t_newmethod)ml_new,
    	(t_method)ml_free, sizeof(t_maxlife), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)ml_new, gensym("mlife"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_randfill, gensym("randfill"), 0);
	class_addmethod(mlife_class, (t_method)ml_fill, gensym("fill"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_set_die_lo, gensym("lo"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_set_die_hi, gensym("hi"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_set_neighbourhood, gensym("nset"), A_FLOAT, 0);
    class_addmethod(mlife_class, (t_method)ml_randseed, gensym("randseed"), A_FLOAT, 0);
	class_addmethod(mlife_class, (t_method)ml_seed, gensym("seed"), A_FLOAT, A_FLOAT, 0);
	class_addmethod(mlife_class, (t_method)ml_display, gensym("display"), 0);
    class_addfloat(mlife_class, ml_int);
	class_addbang(mlife_class, ml_bang);
    class_sethelpsymbol(mlife_class, gensym("maxlib/mlife-help.pd"));
}
#endif
