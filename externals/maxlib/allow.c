/* ----------------------------  allow  --------------------------------------- */
/*                                                                              */
/* Lets only floats/symbols through that are allowed to do so.                  */
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
#include <string.h>

static char *version = "allow v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct allow
{
	t_object x_obj;	
	t_outlet *x_out;
	t_atom   *x_elem;		// list of elemets that are allowed to pass
	t_int    x_numelem;		// the number of elemetns in our allow-list
} t_allow;

	/* we got a symbol... */
static void allow_symbol(t_allow *x, t_symbol *s)
{
	int i;
	for(i = 0; i < x->x_numelem; i++)
	{
		if(x->x_elem[i].a_type == A_SYMBOL)	// compare with all symbols in our list
			if(atom_getsymbolarg(i, x->x_numelem, x->x_elem) == s)
			{
				outlet_symbol(x->x_out, s);
				return;
			}
	}			
}

	/* we got a float... */
static void allow_float(t_allow *x, t_floatarg f)
{
	int i;
	for(i = 0; i < x->x_numelem; i++)
	{
		if(x->x_elem[i].a_type == A_FLOAT)	// compare with all floats in our list
			if(atom_getfloatarg(i, x->x_numelem, x->x_elem) == f)
			{
				outlet_float(x->x_out, f);
				return;
			}
	}
}

static t_class *allow_class;

static void allow_free(t_allow *x)
{
	freebytes(x->x_elem, x->x_numelem*sizeof(t_atom));
}

static void *allow_new(t_symbol *s, int argc, t_atom *argv)
{
    t_allow *x = (t_allow *)pd_new(allow_class);

	x->x_numelem = argc;	// get the number of elements
	x->x_elem = getbytes(argc*sizeof(t_atom));
	memcpy(x->x_elem, argv, argc*sizeof(t_atom));

	x->x_out = outlet_new(&x->x_obj, gensym("anything"));

	// post("allow: got %d elements", x->x_numelem);

	return (x);
}

#ifndef MAXLIB
void allow_setup(void)
{
	/* the object's class: */
    allow_class = class_new(gensym("allow"), (t_newmethod)allow_new,
    	(t_method)allow_free, sizeof(t_allow), 0, A_GIMME, 0);
#else
void maxlib_allow_setup(void)
{
	/* the object's class: */
    allow_class = class_new(gensym("maxlib_allow"), (t_newmethod)allow_new,
    	(t_method)allow_free, sizeof(t_allow), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)allow_new, gensym("allow"), A_GIMME, 0);
#endif
    class_addsymbol(allow_class, allow_symbol);
    class_addfloat(allow_class, allow_float);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(allow_class, gensym("maxlib/allow-help.pd"));
#endif
}
