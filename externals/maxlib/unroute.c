/* --------------------------  unroute   ------------------------------------- */
/*                                                                              */
/* Opposit to croute.                                                           */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Get source at http://www.akustische-kunst.org/puredata/                      */
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

#define MAX_INLET 256

static char *version = "unroute v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct unroute
{
  t_object x_obj;
  t_outlet *x_outlet;
  t_int    x_ninstance;
  t_atom   x_id;
} t_unroute;

typedef struct proxy
{
	t_object obj;
	t_int index;		/* number of proxy inlet(s) */
	t_atom id;
	t_unroute *x;		/* we'll put the other struct in here */
} t_proxy;

static void unroute_any(t_unroute *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom at[MAXPDSTRING];

	for(i = 0; i < argc; i++)
	{
		at[i + 2] = argv[i];
	}
	argc += 2;
	SETSYMBOL(at+1, s);

	at[0] = x->x_id;	/* prepend id of that inlet */

	outlet_list(x->x_outlet, NULL, argc, at);
}

static void unroute_list(t_unroute *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom at[MAXPDSTRING];

	for(i = 0; i < argc; i++)
	{
		at[i + 1] = argv[i];
	}
	argc++;

	at[0] = x->x_id;	/* prepend id of that inlet */

	outlet_list(x->x_outlet, NULL, argc, at);
}

static void unroute_float(t_unroute *x, t_floatarg f)
{
	t_atom list[2];

	list[0] = x->x_id;	/* prepend id of that inlet */
	SETFLOAT(list+1, f);
	outlet_list(x->x_outlet, NULL, 2, list);
}

static void unroute_input(t_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
	t_unroute *x = (t_unroute *)(p->x);
	int i;
	t_atom at[MAXPDSTRING];

	if(s == gensym("list"))
	{
		for(i = 0; i < argc; i++)
		{
			at[i + 1] = argv[i];
		}
		argc++;
	}
	else
	{
		for(i = 0; i < argc; i++)
		{
			at[i + 2] = argv[i];
		}
		argc += 2;
		SETSYMBOL(at+1, s);
	}

	at[0] = p->id;	/* prepend id for that inlet */

	outlet_list(x->x_outlet, NULL, argc, at);
}


static t_class *unroute_class;
static t_class *proxy_class;

static void *unroute_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;

    t_unroute *x = (t_unroute *)pd_new(unroute_class);
	t_proxy *inlet[MAX_INLET];

	x->x_ninstance	= argc;
	x->x_id = argv[0];

	for(i = 0; i < x->x_ninstance - 1; i++)
	{
		inlet[i] = (t_proxy *)pd_new(proxy_class);
		inlet[i]->x = x;		/* make x visible to the proxy inlets */
		inlet[i]->index = i;	/* remember our number */
		inlet[i]->id = argv[i+1];
			/* the inlet we're going to create belongs to the object 
		       't_unroute' but the destination is the instance 'i'
		       of the proxy class 't_proxy'                           */
		inlet_new(&x->x_obj, &inlet[i]->obj.ob_pd, 0,0);
	}
	x->x_outlet = outlet_new(&x->x_obj, gensym("float"));

    return (void *)x;
}

#ifndef MAXLIB
void unroute_setup(void)
{
    unroute_class = class_new(gensym("unroute"), (t_newmethod)unroute_new,
    	0, sizeof(t_unroute), 0, A_GIMME, 0);
#else
void maxlib_unroute_setup(void)
{
    unroute_class = class_new(gensym("maxlib_unroute"), (t_newmethod)unroute_new,
    	0, sizeof(t_unroute), 0, A_GIMME, 0);
#endif
		/* a class for the proxy inlet: */
	proxy_class = class_new(gensym("maxlib_unroute_proxy"), NULL, NULL, sizeof(t_proxy),
		CLASS_PD|CLASS_NOINLET, A_NULL);

	class_addanything(proxy_class, unroute_input);

    class_addfloat(unroute_class, unroute_float);
    class_addlist(unroute_class, unroute_list);
    class_addanything(unroute_class, unroute_any);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)unroute_new, gensym("unroute"), A_GIMME, 0);
    class_sethelpsymbol(unroute_class, gensym("maxlib/unroute-help.pd"));
#endif
}
