/* --------------------------  dist  ------------------------------------------ */
/*                                                                              */
/* Distributes incoming data to a changeable list of receive objects.           */
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
/*
   connect <symbol list>: store receive objects in list of receivers
   disconnect <symbol list>: remove objects from list of receivers
   clear: clear list of receivers
   send <anything>: send anything to all receives named in the list of receivers  */

#include "m_pd.h"

#include <string.h>
#include <stdio.h>

#define MAX_REC 64		            /* maximum number of receive objects */
#define MAX_ARG 32		            /* maximum number of arguments to pass on */

static char *version = "dist v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

static t_class *dist_class;

typedef struct _dist
{
    t_object x_obj;
    t_symbol *x_sym[MAX_REC];       /* names of receiving objects */
	t_int    x_rec;                 /* current number of receiving objects */
	t_int    x_verbose;             /* set to 0 to turn off detailed output in Pd window */
} t_dist;

static void dist_bang(t_dist *x)
{
	int i;

	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) pd_bang(x->x_sym[i]->s_thing);
	}
}

static void dist_float(t_dist *x, t_float f)
{
	int i;

	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) pd_float(x->x_sym[i]->s_thing, f);
	}
}

static void dist_symbol(t_dist *x, t_symbol *s)
{
	int i;

	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) pd_symbol(x->x_sym[i]->s_thing, s);
	}
}

static void dist_pointer(t_dist *x, t_gpointer *gp)
{
	int i;

	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) pd_pointer(x->x_sym[i]->s_thing, gp);
	}
}

static void dist_list(t_dist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;

	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) pd_list(x->x_sym[i]->s_thing, s, argc, argv);
	}
}

static void dist_anything(t_dist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;

	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) typedmess(x->x_sym[i]->s_thing, s, argc, argv);
	}
}

	/* send 'anything' to receiver */
static void dist_send(t_dist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom av[MAX_ARG];		/* the 'new' t_atom without first element */
	t_int ac = argc - 1;    /* the 'new' number of arguments */

	if(ac > MAX_ARG)
	{
		post("dist: too many arguments!");
		return;
	}

	for(i = 1; i < argc; i++)
	{
		av[i - 1] = argv[i];	/* just copy, don't care about types */
	}
		/* send only argument-part to receivers */
	for(i = 0; i <= x->x_rec; i++)
	{
		if (x->x_sym[i]->s_thing) pd_forwardmess(x->x_sym[i]->s_thing, argc, argv);
	}
}

static void dist_connect(t_dist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i, j;
	int exist;
	t_symbol *name;
		/* just append every new receive-name to end of list */
	for(i = 0; i < argc; i++)
	{
		exist = 0;
		if(x->x_rec == MAX_REC - 1)
		{
			post("dist: too many connections in use!");
			return;
		}
		name = atom_getsymbolarg(i, argc, argv);
		for(j = 0; j <= x->x_rec; j++)
		{	
				/* check if the name already exists */
			if(x->x_sym[j] == name)
			{
				post("dist: \"%s\" already exists in list of receivers", name->s_name);
				exist = 1;	/* indicate that it _does_ exist */
			}
		}	
			/* add it in case it's a new one */
		if(!exist)
		{
			x->x_rec++;
			x->x_sym[x->x_rec] = name;
			if(x->x_verbose)post("dist: \"%s\" added to list of receivers", x->x_sym[x->x_rec]->s_name);
		}
	}
}

static void dist_disconnect(t_dist *x, t_symbol *s, int argc, t_atom *argv)
{
		/* need to rearrange list in order to get rid of empty entries */
	int i, j, k;
	int done;
	t_symbol *name;

	for(i = 0; i < argc; i++)
	{
		name = atom_getsymbolarg(i, argc, argv);	/* the one we're going to remove */
		done = 0;                                   /* not yet removed */
		for(j = 0; j <= x->x_rec; j++)              /* search for it... */
		{
			if(x->x_sym[j] == name)
			{
				x->x_rec--;
				if(x->x_verbose)post("dist: \"%s\" removed from list of receivers", x->x_sym[j]->s_name);
				x->x_sym[j] = NULL;	/* delete entry */
					/* rearrange list now: move entries to close the gap */
				for(k = j; k <= x->x_rec; k++)
				{
					x->x_sym[k] = x->x_sym[k + 1];
				}
				done = 1;							/* removed successfully */
			}
		}
		if(!done)post("dist: \"%s\" not in list of receivers, ignored", name->s_name);
	}
}

static void dist_clear(t_dist *x)
{
	int i;

	for(i = 0; i < MAX_REC; i++)
	{
		x->x_sym[i] = NULL;
	}
	x->x_rec = -1;
}

static void dist_print(t_dist *x)
{
	int i;

	if(x->x_rec == 0)
	{
		post("dist: there is one object in receiver list:");
	} 
	else if(x->x_rec > 0)
	{
		post("dist: there are %d objects in receiver list:", x->x_rec + 1);
	} 
	else
	{
		post("dist: there are no objects in receiver list");
		return;
	}

	for(i = 0; i <= x->x_rec; i++)
	{
		post("      \"%s\"", x->x_sym[i]->s_name);
	}
}

static void *dist_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;

    t_dist *x = (t_dist *)pd_new(dist_class);

	x->x_rec = -1;
	x->x_verbose = 1;	/* display info on connect/disconnect */
	for(i = 0; i < argc; i++)
	{
		x->x_sym[i] = atom_getsymbolarg(i, argc, argv);
		x->x_rec++;
	}
    return (x);
}

#ifndef MAXLIB
void dist_setup(void)
{
    dist_class = class_new(gensym("dist"), (t_newmethod)dist_new, 0,
    	sizeof(t_dist), 0, A_GIMME, 0);
#else
void maxlib_dist_setup(void)
{
    dist_class = class_new(gensym("maxlib_dist"), (t_newmethod)dist_new, 0,
    	sizeof(t_dist), 0, A_GIMME, 0);
#endif
    class_addcreator((t_newmethod)dist_new, gensym("d"), A_GIMME, 0);
    class_addbang(dist_class, dist_bang);
    class_addfloat(dist_class, dist_float);
    class_addsymbol(dist_class, dist_symbol);
    class_addpointer(dist_class, dist_pointer);
    class_addlist(dist_class, dist_list);
	class_addmethod(dist_class, (t_method)dist_connect, gensym("connect"), A_GIMME, 0);
	class_addmethod(dist_class, (t_method)dist_disconnect, gensym("disconnect"), A_GIMME, 0);
	class_addmethod(dist_class, (t_method)dist_clear, gensym("clear"), 0);
	class_addmethod(dist_class, (t_method)dist_print, gensym("print"), 0);
	class_addmethod(dist_class, (t_method)dist_send, gensym("send"), A_GIMME, 0);
    class_addanything(dist_class, dist_anything);
#ifndef MAXLIB
	
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)dist_new, gensym("dist"), A_GIMME, 0);
	class_sethelpsymbol(dist_class, gensym("maxlib/dist-help.pd"));
#endif
}
