/* ------------------------- subst   ------------------------------------------ */
/*                                                                              */
/* Performs 'self-similar' substitution of a given list of values.              */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
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


static char *version = "subst v0.1, self-similar substitution of rows (list or array)\n"
                       "            written by Olaf Matthes <olaf.matthes@gmx.de>";

#undef DEBUG
//#define DEBUG

#define MAXSIZE 1024

#include <stdio.h>
#include <stdlib.h>


//
// Maxlife object data structure
//
typedef struct subst
{
	t_object x_obj;				// must begin every object 
	t_outlet *x_outlist;		// outlet for the processed list 
	t_outlet *x_outlength;
	t_atom   x_row[MAXSIZE];	// row of values to get processed
	t_int    x_length;			// length of row
	t_int    x_order;			// size of the CA field/world
	t_symbol *x_array;			// name of array that holds the data
	t_garray *x_buf;			// content of that array
} t_subst;

//
// Function prototypes for our methods and functions
//
static t_class *subst_class;		// global variable that points to the Maxlife class

//
// get the sum of intervals from no a to no b
// Parameters: the row, it's length, intv a, intev b
//
static int sum_intv(t_atom *argv, int argc, int a, int b)
{
	int i;
	int summe = 0;			// sum of intervals

	if(a==b)
		return(0);			// same index
	if(atom_getintarg(a, argc, argv) == atom_getintarg(b, argc, argv))
		return(0);			// same value

	for(i=a;i<b;i++)		// for all values from a to b....
	{
		if(atom_getintarg(i + 1, argc, argv) > atom_getintarg(i, argc, argv))	// positive intv.
		{
			summe += ((atom_getintarg(i + 1, argc, argv) - atom_getintarg(i, argc, argv)) % 12);
		}
		else				// negative interval
		{
			summe -= ((atom_getintarg(i + 1, argc, argv) - atom_getintarg(i, argc, argv)) % 12);
		}
	}
	return(summe);
}
//----- Anzahl Partialreihen mit Interval d  -------------------------------
static int no_part(t_atom *argv, int argc, int a, int b, int d)		// nn
{
	int i,j,r = 0;

    if(a = b)return(0);

    for(i = a; i < b; i++)
    {
            for(j=a+1;j<b;j++)
            {
        	if(sum_intv(argv, argc, i, j) == d)
				r++;
            }
    }
    return(r);
}

//----- n-te Partialreihe der Länge l mit Interval d -----------------------
static int n_part_intv(t_atom *argv, int argc, int a, int b, int l, int d, int n) // nn
{
	int i,j, r = 0;
    if(n > no_part(argv, argc, a, b, d))
		return(-1);
    for(i = 1; i = (b - a); i++)
    {
        for(j = 1; j = b; j++)
        {
        	if(sum_intv(argv, argc, i, j) == d) 
				r++;
        }
    }
    return(r);
}
//----- Test, ob Partialreihe der Ordnung o mit Interval d existiert ----------
static int check_part_intv(t_atom *argv, int argc, int o, int d)
{
	int z;

    for(z = 0; z < argc - o; z++)
    {
            if(sum_intv(argv, argc, z, z + o) == d)
				return(z);	// Abstand von Reihenanfang
    }

    return(-1);
}

static int my_random(int range) {
	int ret = rand();
	ret = ret % range;	// limit to desired output range
	return(ret);
}

//
// the substitution algorhythm
//
static int subst_calc(t_subst *x, int n)
{
	int i,j,k,l,o = x->x_order;
    int s = -1;
    int intv;
	t_atom newrow[MAXSIZE];
	t_garray *A = x->x_buf;
	int npoints;
	t_float *vec;

	if(x->x_length <= 1)
	{
		post("subst: need some data first!");
		return(-1);
	}
    srand((unsigned int)clock_getlogicaltime());

    if(n == -1)	// no interval given: choose one by chance
	{
		do
		{
			n = my_random(x->x_length - 1);						// choose interval
			intv = sum_intv(x->x_row, x->x_length, n, n + 1);	// size of this interval
		}
		while(intv == 0);				// ...until we found one that is not 0!
	}
	else intv = sum_intv(x->x_row, x->x_length, n, n + 1);

#ifdef DEBUG
    post("subst: substitution of %dth interval (%d halftones)", n+1, intv);
#endif

    /* for-Schleife für möglichst lange Substitutionen
    for(j=anzahlReihe(alteReihe);j>2;j--)				*/
    for(j = x->x_order; j < x->x_length; j++)	// prefer lower orders (min. 2)
    {							// search for possible order...
            s = check_part_intv(x->x_row, x->x_length, j, intv);
            if(s != -1)			// check if there is a partial row with the interval we want
            {
                o = j;			// save actual order, might be larger then x->x_order
                break;			// ... do it!
            }
            if(o == j)break;	// found one
    }

    for(i = 0; i < x->x_length; i++)
    {
        if(i <= n)		// just copy values before substitution
        {
            newrow[i] = x->x_row[i];
        }
        if((i == n) && (s != -1))	// now substitute
        {
            for(k=1;k<o;k++)		// well get order - 1 new values...
            {
                SETFLOAT(newrow + (i + k), (atom_getintarg((i + k - 1), 1024, newrow) 
					+ sum_intv(x->x_row, x->x_length, s+k-1, s+k)));
#ifdef DEBUG
                post("subst: new interval[%d]: %d ", k, sum_intv(x->x_row, x->x_length, s+k-1, s+k));
#endif
            }
            post("subst: replaced interval %d (%d halftones) with %d new intervals", n, intv, o);
        }
        else if((i == n) && (s == -1))	// not partial row found
        {
            o = 1;		// order is 1 -> now substitution
            newrow[i] = x->x_row[i];	// copy the next value of the row
            post("subst: coundn't find any partial rows to fit in!");
        }

        if(i>n)		// behind substitution
        {
            newrow[i+(o-1)] = x->x_row[i];	// copy rest or row
        }
    }

    //  copy stuff back...
    x->x_length = l = x->x_length + o - 1;
	for(i = 0; i < x->x_length; i++)
		x->x_row[i] = newrow[i];

	// write to array
	if(x->x_array)if (!(A = (t_garray *)pd_findbyclass(x->x_array, garray_class)))
		error("subst: %s: no such array", x->x_array->s_name);
	else if (!garray_getfloatarray(A, &npoints, &vec))
		error("subst: %s: bad template ", x->x_array->s_name);
	else
	{
		i = 0;

		if (l >= npoints)	// keep end of array
		{
			while(npoints--)
			{
				*vec++ = atom_getfloat(x->x_row + i);
				i++;
			}
		}
		else				// update 
		{
			npoints -= l;
			while (l--)
			{
				*vec++ = atom_getfloat(x->x_row + i);
				i++;
			}
			while (npoints--) *vec++ = 0;
		}
		garray_redraw(A);
	}

	// output stuff
	outlet_float(x->x_outlength, x->x_length);
	outlet_list(x->x_outlist, NULL, x->x_length, x->x_row);

    return(0);
}

static void subst_list(t_subst *x, t_symbol *s, int argc, t_atom *argv)
{	
	t_garray *b = x->x_buf;		/* make local copy of array */
	float *tab;                 /* we'll store notes in here */
	int items;
	int i;

	for(i = 0; i < argc; i++)
	{
		x->x_row[i] = argv[i];		// just copy input
	}
	x->x_length = argc;

}

//
// choose the array that holds the processed row (output!!)
//
void subst_set(t_subst *x, t_symbol *s)
{
	t_garray *b;
	
	x->x_array = s;

	if ((b = (t_garray *)pd_findbyclass(s, garray_class)))
	{
		post("subst: array set to \"%s\"", s->s_name);
		x->x_buf = b;
	} else {
		post("subst: no array \"%s\" (error %d)", s->s_name, b);
		x->x_buf = 0;
	}
}

//
// load row from array (input!!)
//
static void subst_load(t_subst *x, t_symbol *s)
{	
	t_garray *b;		/* make local copy of array */
	t_float *tab;                 /* the content itselfe */
	int items, i;

	if ((b = (t_garray *)pd_findbyclass(s, garray_class)))
	{
		post("subst: array set to \"%s\"", s->s_name);
	} else {
		post("subst: no array \"%s\" (error %d)", s->s_name, b);
		return;
	}

		// read from our array
	if (!garray_getfloatarray(b, &items, &tab))
	{
		post("subst: couldn't read from array!");
		return;
	}
	for(i = 0; i < items; i++)
	{
		SETFLOAT(x->x_row + i, tab[i]);		// copy array into x->x_row
	}
	x->x_length = items;
	post("subst: loaded %d values from array \"%s\"", items, s->s_name);
}

//
// substitute an interval choosen by chance
//
static void subst_bang(t_subst *x)
{	
	subst_calc(x, -1);
}

//
// substitute the Nth interval
//
static void subst_intv(t_subst *x, t_floatarg f)
{	
	int i = (int)f;
	if(i > x->x_length) i = x->x_length;
	subst_calc(x, i);
}

//
// set the minimum order of substitution
//
static void subst_set_order(t_subst *x, t_floatarg f)
{
	x->x_order = (t_int)f;
	if(x->x_order < 2)x->x_order = 2;
	post("subst: set order to %d", x->x_order);
}

//
// method to print out: but what?
//
static void subst_display(t_subst *x)
{
}

//
// function to create an instance of the subst class
//
static void *subst_new(t_symbol *s, int argc, t_atom *argv)
{
	long i;
	t_symbol *sym;
	t_subst *x = (t_subst *)pd_new(subst_class);
	// read in order...
	x->x_order = 3;
	if(argc == 1)
	{
		x->x_order = atom_getintarg(0, argc, argv);
	}
	else if(argc == 2)
	{
		sym = atom_getsymbolarg(0, argc, argv);
		x->x_order = atom_getintarg(1, argc, argv);
		subst_set(x, sym);
	}

	// create outlets
	x->x_outlist = outlet_new(&x->x_obj, gensym("list"));
	x->x_outlength = outlet_new(&x->x_obj, gensym("float"));
		
	return(x);				// always return a copy of the created object 
}

static void subst_free(t_subst *x)
{
	/* nothing to do */
}

#ifndef MAXLIB
void subst_setup(void)
{
    subst_class = class_new(gensym("subst"), (t_newmethod)subst_new,
    	(t_method)subst_free, sizeof(t_subst), 0, A_GIMME, 0);
    class_addmethod(subst_class, (t_method)subst_set_order, gensym("order"), A_FLOAT, 0);
    class_addmethod(subst_class, (t_method)subst_intv, gensym("interval"), A_FLOAT, 0);
	class_addmethod(subst_class, (t_method)subst_set, gensym("set"), A_SYMBOL, 0);
	class_addmethod(subst_class, (t_method)subst_load, gensym("load"), A_SYMBOL, 0);
	class_addmethod(subst_class, (t_method)subst_display, gensym("display"), 0);
	class_addlist(subst_class, subst_list);
	class_addbang(subst_class, subst_bang);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_subst_setup(void)
{
    subst_class = class_new(gensym("maxlib_subst"), (t_newmethod)subst_new,
    	(t_method)subst_free, sizeof(t_subst), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)subst_new, gensym("subst"), A_GIMME, 0);
    class_addmethod(subst_class, (t_method)subst_set_order, gensym("order"), A_FLOAT, 0);
    class_addmethod(subst_class, (t_method)subst_intv, gensym("interval"), A_FLOAT, 0);
	class_addmethod(subst_class, (t_method)subst_set, gensym("set"), A_SYMBOL, 0);
	class_addmethod(subst_class, (t_method)subst_load, gensym("load"), A_SYMBOL, 0);
	class_addmethod(subst_class, (t_method)subst_display, gensym("display"), 0);
	class_addlist(subst_class, subst_list);
	class_addbang(subst_class, subst_bang);
    class_sethelpsymbol(subst_class, gensym("maxlib/subst-help.pd"));
}
#endif
