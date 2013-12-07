/* ----------------------------  deny  --------------------------------------- */
/*                                                                              */
/* Lets only floats/symbols through that are denyed to do so.                  */
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

static char *version = "deny v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct deny
{
	t_object x_obj;	
	t_outlet *x_out;
	t_atom   *x_elem;		// list of elemets that are denyed to pass
	t_int    x_numelem;		// the number of elemetns in our deny-list
} t_deny;

	/* we got a symbol... */
static void deny_symbol(t_deny *x, t_symbol *s)
{
	int i, deny = 0;
	for(i = 0; i < x->x_numelem; i++)
	{
		if(x->x_elem[i].a_type == A_SYMBOL)	// compare with all symbols in our list
			if(atom_getsymbolarg(i, x->x_numelem, x->x_elem) == s)
			{
				deny = 1;
				return;
			}
	}
	if(!deny)outlet_symbol(x->x_out, s);
}

	/* we got a float... */
static void deny_float(t_deny *x, t_floatarg f)
{
	int i, deny = 0;
	for(i = 0; i < x->x_numelem; i++)
	{
		if(x->x_elem[i].a_type == A_FLOAT)	// compare with all floats in our list
			if(atom_getfloatarg(i, x->x_numelem, x->x_elem) == f)
			{
				deny = 1;	// input is in deny-list
				return;
			}
	}
	if(!deny)outlet_float(x->x_out, f);
}

static t_class *deny_class;

static void deny_free(t_deny *x)
{
	freebytes(x->x_elem, x->x_numelem*sizeof(t_atom));
}

static void *deny_new(t_symbol *s, int argc, t_atom *argv)
{
    t_deny *x = (t_deny *)pd_new(deny_class);

	x->x_numelem = argc;	// get the number of elements
	x->x_elem = getbytes(argc*sizeof(t_atom));
	memcpy(x->x_elem, argv, argc*sizeof(t_atom));

	x->x_out = outlet_new(&x->x_obj, gensym("anything"));

	// post("deny: got %d elements", x->x_numelem);

	return (x);
}

#ifndef MAXLIB
void deny_setup(void)
{
	/* the object's class: */
    deny_class = class_new(gensym("deny"), (t_newmethod)deny_new,
    	(t_method)deny_free, sizeof(t_deny), 0, A_GIMME, 0);
#else
void maxlib_deny_setup(void)
{
	/* the object's class: */
    deny_class = class_new(gensym("maxlib_deny"), (t_newmethod)deny_new,
    	(t_method)deny_free, sizeof(t_deny), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)deny_new, gensym("deny"), A_GIMME, 0);
#endif
    class_addsymbol(deny_class, deny_symbol);
    class_addfloat(deny_class, deny_float);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(deny_class, gensym("maxlib/deny-help.pd"));
#endif
}
