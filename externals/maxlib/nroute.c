/* ------------------------- nroute  ------------------------------------------ */
/*                                                                              */
/* Route input according to Nth element.                                        */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Based on code found on the web.                                              */
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

/*
   inlet 1: anything to be routed
   inlet 2: anything to be matched to
   inlet 3: position to match
   out   1: input if match found
   out   2: input if match not found
*/

#include "m_pd.h"

static char *version = "nroute v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct nroute
{
	t_object x_obj;		
	t_outlet *out1;	
	t_outlet *out2;
	t_int    pos;
	t_atom   match;
} t_nroute;

typedef struct proxy
{
	t_object obj;
	t_int index;		/* number of proxy inlet(s) */
	t_nroute *x;		/* we'll put the other struct in here */
} t_proxy;


	/* this is the routine that actually does the routing / matching */
	/* it get's called by all other routines that get any input and  */
	/* even handles the second (proxy) inlet !                       */
static void nroute_any(t_nroute *x, t_symbol *s, int argc, t_atom *argv)
{
	if(s)
	{
		if (x->pos == 1 && x->match.a_type == A_SYMBOL && x->match.a_w.w_symbol == s)
			outlet_anything (x->out1,s,argc,argv);
		else if (x->pos > 1 && x->pos <= argc + 1 && 
			argv[x->pos-2].a_type == x->match.a_type && 
			argv[x->pos-2].a_w.w_float == x->match.a_w.w_float)
				outlet_anything (x->out1,s,argc,argv);
		else outlet_anything (x->out2,s,argc,argv);
	}
	else
	{
		if (x->pos > 0 && x->pos <= argc && 
			argv[x->pos-1].a_type == x->match.a_type && 
			argv[x->pos-1].a_w.w_float == x->match.a_w.w_float)
			outlet_list (x->out1,0,argc,argv);
		else outlet_list (x->out2,0,argc,argv);
	}
}

static void nroute_float(t_nroute *x, float f)
{
	t_atom a;
	
	SETFLOAT (&a,f);
	nroute_any(x,0,1,&a);
}

static void nroute_list(t_nroute *x, t_symbol *s, int argc, t_atom *argv)
{
	nroute_any(x,0,argc,argv);
}

static void nroute_setmatch(t_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
	t_nroute *x = (t_nroute *)(p->x);

	if(argc == 0)	/* have to match a symbol */
	{
		x->match.a_type = A_SYMBOL;
		x->match.a_w.w_symbol = s;
	}
	else	/* got a float */
	{
		if(argc > 1)
		{
			post("nroute: middle inlet accepts only (float,symbol) for match");
			return;
		}
		x->match.a_type = A_FLOAT;
		x->match.a_w.w_float = argv[0].a_w.w_float;
	}
}

static void nroute_setpos(t_nroute *x, t_floatarg f)
{
	x->pos = (t_int)f;
}

static t_class *nroute_class;
static t_class *proxy_class;

static void *nroute_new(t_symbol *s, int argc, t_atom *argv)
{
    t_nroute *x = (t_nroute *)pd_new(nroute_class);
	t_proxy *inlet = (t_proxy *)pd_new(proxy_class);	/* for the proxy inlet */

	inlet->x = x;	/* make x visible to the proxy inlets */

	x->pos = 1;
	x->match.a_type = A_NULL;
	if (argc > 2) { error ("nroute: extra arguments"); return 0; }
	if (argc > 1) {
		if (argv[1].a_type == A_FLOAT) x->pos = argv[1].a_w.w_float;
		else { post ("nroute: second argument must be (int) position"); return 0; }
	}
	if (argc > 0) {
		x->match.a_type = argv[0].a_type;
		x->match.a_w.w_float = argv[0].a_w.w_float;
	}
    inlet->index = 0;	/* we are going to create a proxy inlet no. 0 */
		/* it belongs to the object t_nroute but the destination is t_proxy */
    inlet_new(&x->x_obj, &inlet->obj.ob_pd, 0,0);
		/* and now a 'normal' third inlet */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("right"));
	x->out1 = outlet_new(&x->x_obj, gensym("list"));
	x->out2 = outlet_new(&x->x_obj, gensym("list"));
	return (x);
}

#ifndef MAXLIB
void nroute_setup(void)
{
	/* the object's class: */
    nroute_class = class_new(gensym("nroute"), (t_newmethod)nroute_new,
    	0, sizeof(t_nroute), 0, A_GIMME, 0);
#else
void maxlib_nroute_setup(void)
{
	/* the object's class: */
    nroute_class = class_new(gensym("maxlib_nroute"), (t_newmethod)nroute_new,
    	0, sizeof(t_nroute), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)nroute_new, gensym("nroute"), A_GIMME, 0);
#endif
	/* a class for the proxy inlet: */
	proxy_class = class_new(gensym("maxlib_nroute_proxy"), NULL, NULL, sizeof(t_proxy),
		CLASS_PD|CLASS_NOINLET, A_NULL);

	class_addmethod(nroute_class, (t_method)nroute_setpos, gensym("right"), A_FLOAT, 0);
    class_addfloat(nroute_class, nroute_float);
	class_addlist(nroute_class, nroute_list);
	class_addanything(nroute_class, nroute_any);
	class_addanything(proxy_class, nroute_setmatch);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(nroute_class, gensym("maxlib/nroute-help.pd"));
#endif
}
