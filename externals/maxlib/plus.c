/* --------------------------- plus  ------------------------------------------ */
/*                                                                              */
/* Like '+', but calculates output whenever _any_ of the inlets changes.        */
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

static char *version = "plus v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct plus
{
  t_object x_ob;
  t_inlet  *x_inleft;               /* leftmost inlet */
  t_inlet  *x_inright;              /* right inlet */
  t_outlet *x_outlet;               /* result */
  t_int    x_numvalues;             /* number of values / inlets */

  t_float  x_plusvalue[MAXSIZE];

} t_plus;

static void plus_bang(t_plus *x)
{
	int i;
	t_float result = x->x_plusvalue[0];
	for(i = 1; i < x->x_numvalues; i++)
		result += x->x_plusvalue[i];
	outlet_float(x->x_outlet, result);
}

static void plus_float(t_plus *x, t_floatarg f)
{
	x->x_plusvalue[0] = f;
	plus_bang(x);	/* calculate result */
}

static void plus_ft1(t_plus *x, t_floatarg f)
{
	x->x_plusvalue[1] = f;
	plus_bang(x);	/* calculate result */
}

static t_class *plus_class;

static void *plus_new(t_symbol *s, t_int argc, t_atom* argv)
{
	int i;

    t_plus *x = (t_plus *)pd_new(plus_class);
    x->x_inright = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	for(i = 2; i < argc; i++)	/* create additional inlets, if any */
	{
		floatinlet_new(&x->x_ob, &x->x_plusvalue[i]);
	}
	x->x_outlet = outlet_new(&x->x_ob, gensym("float"));

	for(i = 0; i < argc; i++)
	{
		x->x_plusvalue[i] = atom_getfloatarg(i, argc, argv);;
	}
	x->x_numvalues = i;

    return (void *)x;
}

#ifndef MAXLIB
void plus_setup(void)
{
    plus_class = class_new(gensym("plus"), (t_newmethod)plus_new,
    	0, sizeof(t_plus), 0, A_GIMME, 0);
    class_addfloat(plus_class, plus_float);
    class_addmethod(plus_class, (t_method)plus_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addbang(plus_class, (t_method)plus_bang);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_plus_setup(void)
{
    plus_class = class_new(gensym("maxlib_plus"), (t_newmethod)plus_new,
    	0, sizeof(t_plus), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)plus_new, gensym("plus"), A_GIMME, 0);
    class_addfloat(plus_class, plus_float);
    class_addmethod(plus_class, (t_method)plus_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addbang(plus_class, (t_method)plus_bang);
    class_sethelpsymbol(plus_class, gensym("maxlib/plus-help.pd"));
}
#endif
