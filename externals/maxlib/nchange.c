/* ------------------------- ignore ------------------------------------------- */
/*                                                                              */
/* A 'new' change object for more than just floats.                             */
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

#define A_LIST 254;
#define A_ANYTHING 255;

#include "m_pd.h"

// #include <string.h>
// #include <stdio.h>

/* -------------------------- nchange ------------------------------ */
static char *version = "nchange v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

static t_class *nchange_class;

typedef struct _nchange
{
    t_object x_obj;
    t_atom   x_a[256];
	int      x_c;
	int	x_type;
} t_nchange;

static void *nchange_new(t_symbol *s)
{
	int i;

    t_nchange *x = (t_nchange *)pd_new(nchange_class);

	if(s == gensym("s"))
	{
		x->x_type = A_SYMBOL;
	    outlet_new(&x->x_obj, &s_symbol);
	}
	else if(s == gensym("f"))
	{
		x->x_type = A_FLOAT;
	    outlet_new(&x->x_obj, &s_float);
	}
	else if(s == gensym("l"))
	{
		x->x_type = A_LIST;
	    outlet_new(&x->x_obj, &s_list);
	}
	else
	{
		x->x_type = A_ANYTHING;
	    outlet_new(&x->x_obj, &s_anything);
	}

    return (x);
}

static void nchange_bang(t_nchange *x)
{
    if (x->x_type == A_FLOAT)
		outlet_float(x->x_obj.ob_outlet, x->x_a->a_w.w_float);
    else if (x->x_type == A_SYMBOL)
		outlet_symbol(x->x_obj.ob_outlet, x->x_a->a_w.w_symbol);
    else
		outlet_list(x->x_obj.ob_outlet, NULL, x->x_c, x->x_a);
}

static void nchange_float(t_nchange *x, t_float f)
{
 	if (x->x_type == A_FLOAT)
	{
		if (f != x->x_a->a_w.w_float)
		{
    		// x->x_f = f;
			SETFLOAT(x->x_a, f);
			outlet_float(x->x_obj.ob_outlet, x->x_a->a_w.w_float);
		}
	}
}

static void nchange_symbol(t_nchange *x, t_symbol *s)
{
	if (x->x_type == A_SYMBOL)
	{
		if (s != x->x_a->a_w.w_symbol)
		{
    		// x->x_s = s;
			SETSYMBOL(x->x_a, s);
			outlet_symbol(x->x_obj.ob_outlet, x->x_a->a_w.w_symbol);
		}
	}
}

static void nchange_list(t_nchange *x, t_symbol *s, int argc, t_atom *argv)
{
	int i, change = 0;
	int c = argc;

	if(c == x->x_c)	// same number of elements
		for (i = 0; i < c; i++)
		{
		/*	if(x->x_a[i].a_type != argv[i].a_type)
			{
				change = 1;
				break;
			}	*/
			if (x->x_a[i].a_type == A_FLOAT)
			{
				if (argv[i].a_type != A_FLOAT || x->x_a[i].a_w.w_float != argv[i].a_w.w_float)
				{
					change = 1;
					break;
				}
			}
			else if (x->x_a[i].a_type == A_SYMBOL)
			{
				if (argv[i].a_type != A_SYMBOL || x->x_a[i].a_w.w_symbol != argv[i].a_w.w_symbol)
				{
					change = 1;
					break;
				}
			}
		}
	else change = 1;	// different number of elems.

    if (change)
    {
		x->x_c = c;
		for (i = 0; i < c; i++)	// same new list
		{
			x->x_a[i] = argv[i];
		}
		outlet_list(x->x_obj.ob_outlet, s, argc, argv);
    }
}

static void nchange_set(t_nchange *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;

	if (x->x_type == A_SYMBOL)
	{
		SETSYMBOL(x->x_a, argv->a_w.w_symbol);
	}
 	else if (x->x_type == A_FLOAT)
	{
		SETFLOAT(x->x_a, argv->a_w.w_float);
	}
	else	// list or anything
    {
		x->x_c = argc;
		for (i = 0; i < argc; i++)
		{
			x->x_a[i] = argv[i];
		}
    }
}

#ifndef MAXLIB
void nchange_setup(void)
{
    nchange_class = class_new(gensym("nchange"), (t_newmethod)nchange_new, 0,
    	sizeof(t_nchange), 0, A_SYMBOL, 0);
    class_addbang(nchange_class, nchange_bang);
    class_addfloat(nchange_class, nchange_float);
    class_addsymbol(nchange_class, nchange_symbol);
    class_addlist(nchange_class, nchange_list);
    class_addanything(nchange_class, nchange_list);
    class_addmethod(nchange_class, (t_method)nchange_set, gensym("set"), A_GIMME, 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_nchange_setup(void)
{
    nchange_class = class_new(gensym("maxlib_nchange"), (t_newmethod)nchange_new, 0,
    	sizeof(t_nchange), 0, A_SYMBOL, 0);
	class_addcreator((t_newmethod)nchange_new, gensym("nchange"), A_SYMBOL, 0);
    class_addbang(nchange_class, nchange_bang);
    class_addfloat(nchange_class, nchange_float);
    class_addsymbol(nchange_class, nchange_symbol);
    class_addlist(nchange_class, nchange_list);
    class_addanything(nchange_class, nchange_list);
    class_addmethod(nchange_class, (t_method)nchange_set, gensym("set"), A_GIMME, 0);
    class_sethelpsymbol(nchange_class, gensym("maxlib/nchange-help.pd"));
}
#endif
