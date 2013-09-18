/* ------------------------  remote  ------------------------------------------ */
/*                                                                              */
/* Send data to receive obejct <name>.                                          */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.de/puredata/                       */
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
#include <stdio.h>

#define MAX_REC 64		            /* maximum number of receive objects */
#define MAX_ARG 64		            /* maximum number of arguments to pass on */

static char *version = "remote v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";

static t_class *remote_class;

typedef struct _remote
{
    t_object x_obj;
	t_symbol *x_prefix;
	t_int    x_prepend;
} t_remote;

	/* send 'anything' to receiver */
static void remote_anything(t_remote *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom av[MAX_ARG];		/* the 'new' t_atom without first element */
	t_int ac = argc - 1;    /* the 'new' number of arguments */
	char mysym[MAXPDSTRING];
	t_symbol *target;

	if(argc < 1)			/* need <name> <data> */
	{
		post("remote: too few arguments!");
		return;
	}
	if(ac > MAX_ARG)
	{
		post("remote: too many arguments!");
		return;
	}

	for(i = 1; i < argc; i++)
	{
		av[i - 1] = argv[i];	/* just copy, don't care about types */
	}
		/* send only argument-part to receivers */
	if(x->x_prepend)
	{
		sprintf(mysym,"%s%s", x->x_prefix->s_name, s->s_name);
		target = gensym(mysym);
		if (target->s_thing) pd_forwardmess(target->s_thing, argc, argv);
	}
	else
		if (s->s_thing) pd_forwardmess(s->s_thing, argc, argv);
}

static void *remote_new(t_symbol *s)
{
    t_remote *x = (t_remote *)pd_new(remote_class);

	x->x_prefix = s;
	if(x->x_prefix) x->x_prepend = 1;
	else x->x_prepend = 0;

    return (x);
}

#ifndef MAXLIB
void remote_setup(void)
{
    remote_class = class_new(gensym("remote"), (t_newmethod)remote_new, 0,
    	sizeof(t_remote), 0, A_DEFSYM, 0);
    class_addanything(remote_class, remote_anything);
	
    logpost(NULL, 4, version);
}
#else
void maxlib_remote_setup(void)
{
    remote_class = class_new(gensym("maxlib_remote"), (t_newmethod)remote_new, 0,
    	sizeof(t_remote), 0, A_DEFSYM, 0);
	class_addcreator((t_newmethod)remote_new, gensym("remote"), A_DEFSYM, 0);
    class_addanything(remote_class, remote_anything);
	class_sethelpsymbol(remote_class, gensym("maxlib/remote-help.pd"));
}
#endif
