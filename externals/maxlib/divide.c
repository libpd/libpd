/* ------------------------- divide  ------------------------------------------ */
/*                                                                              */
/* Like '/', but calculates output whenever _any_ of the inlets changes.        */
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
#include <stdio.h>

#define MAXSIZE 32

static char *version = "divide v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct divide
{
  t_object x_ob;
  t_inlet  *x_inleft;               /* leftmost inlet */
  t_inlet  *x_inright;              /* right inlet */
  t_outlet *x_outlet;               /* result */
  t_int    x_numvalues;             /* number of values / inlets */

  t_float  x_dividevalue[MAXSIZE];

} t_divide;

static void divide_bang(t_divide *x)
{
	int i;
	t_float result = x->x_dividevalue[0];
	for(i = 1; i < x->x_numvalues; i++)
		result /= x->x_dividevalue[i];
	outlet_float(x->x_outlet, result);
}

static void divide_float(t_divide *x, t_floatarg f)
{
	x->x_dividevalue[0] = f;
	divide_bang(x);	/* calculate result */
}

static void divide_ft1(t_divide *x, t_floatarg f)
{
	x->x_dividevalue[1] = f;
	divide_bang(x);	/* calculate result */
}

static t_class *divide_class;

static void *divide_new(t_symbol *s, t_int argc, t_atom* argv)
{
	int i;

    t_divide *x = (t_divide *)pd_new(divide_class);
    x->x_inright = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	for(i = 2; i < argc; i++)	/* create additional inlets, if any */
	{
		floatinlet_new(&x->x_ob, &x->x_dividevalue[i]);
	}
	x->x_outlet = outlet_new(&x->x_ob, gensym("float"));

	for(i = 0; i < argc; i++)
	{
		x->x_dividevalue[i] = atom_getfloatarg(i, argc, argv);
	}
	x->x_numvalues = i;

    return (void *)x;
}

#ifndef MAXLIB
void divide_setup(void)
{
    divide_class = class_new(gensym("divide"), (t_newmethod)divide_new,
    	0, sizeof(t_divide), 0, A_GIMME, 0);
#else
void maxlib_divide_setup(void)
{
    divide_class = class_new(gensym("maxlib_divide"), (t_newmethod)divide_new,
    	0, sizeof(t_divide), 0, A_GIMME, 0);
#endif
    class_addfloat(divide_class, divide_float);
    class_addmethod(divide_class, (t_method)divide_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addbang(divide_class, (t_method)divide_bang);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)divide_new, gensym("divide"), A_GIMME, 0);
    class_sethelpsymbol(divide_class, gensym("maxlib/divide-help.pd"));
#endif
}

