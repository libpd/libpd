/* -------------------------- multi  ------------------------------------------ */
/*                                                                              */
/* Like '*', but calculates output whenever _any_ of the inlets changes.        */
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

static char *version = "multi v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct multi
{
  t_object x_ob;
  t_inlet  *x_inleft;               /* leftmost inlet */
  t_inlet  *x_inright;              /* right inlet */
  t_outlet *x_outlet;               /* result */
  t_int    x_numvalues;             /* number of values / inlets */

  t_float  x_multivalue[MAXSIZE];

} t_multi;

static void multi_bang(t_multi *x)
{
	int i;
	t_float result = x->x_multivalue[0];
	for(i = 1; i < x->x_numvalues; i++)
		result *= x->x_multivalue[i];
	outlet_float(x->x_outlet, result);
}

static void multi_float(t_multi *x, t_floatarg f)
{
	x->x_multivalue[0] = f;
	multi_bang(x);	/* calculate result */
}

static void multi_ft1(t_multi *x, t_floatarg f)
{
	x->x_multivalue[1] = f;
	multi_bang(x);	/* calculate result */
}

static t_class *multi_class;

static void *multi_new(t_symbol *s, t_int argc, t_atom* argv)
{
	int i;

    t_multi *x = (t_multi *)pd_new(multi_class);
    x->x_inright = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	for(i = 2; i < argc; i++)	/* create additional inlets, if any */
	{
		floatinlet_new(&x->x_ob, &x->x_multivalue[i]);
	}
	x->x_outlet = outlet_new(&x->x_ob, gensym("float"));

	for(i = 0; i < argc; i++)
	{
		x->x_multivalue[i] = atom_getfloatarg(i, argc, argv);;
	}
	x->x_numvalues = i;

    return (void *)x;
}

#ifndef MAXLIB
void multi_setup(void)
{
    multi_class = class_new(gensym("multi"), (t_newmethod)multi_new,
    	0, sizeof(t_multi), 0, A_GIMME, 0);
    class_addfloat(multi_class, multi_float);
    class_addmethod(multi_class, (t_method)multi_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addbang(multi_class, (t_method)multi_bang);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_multi_setup(void)
{
    multi_class = class_new(gensym("maxlib_multi"), (t_newmethod)multi_new,
    	0, sizeof(t_multi), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)multi_new, gensym("multi"), A_GIMME, 0);
    class_addfloat(multi_class, multi_float);
    class_addmethod(multi_class, (t_method)multi_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addbang(multi_class, (t_method)multi_bang);
    class_sethelpsymbol(multi_class, gensym("maxlib/multi-help.pd"));
}
#endif
