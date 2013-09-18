/* --------------------------------------------------------------------------*/
/*                                                                           */
/* converts a GID number to a user name symbol                               */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2006 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#ifndef _WIN32 // this doesn't work on Windows (yet?)

#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#include <lm.h>
#else
#include <sys/types.h>
#include <stdlib.h>
#include <grp.h>
#endif

#include <string.h>

static char *version = "$Revision: 1.3 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *group_class;

typedef struct _group {
	t_object            x_obj;
	t_float             x_gid;  
/* output */
	t_atom              *output; // holder for a list of atoms to be outputted
	t_int               output_count;  // number of atoms in in x->output 
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_group;

/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

/* add one new atom to the list to be outputted */
static void add_atom_to_output(t_group *x, t_atom *new_atom)
{
    t_atom *new_atom_list;

	new_atom_list = (t_atom *)getbytes((x->output_count + 1) * sizeof(t_atom));
	memcpy(new_atom_list, x->output, x->output_count * sizeof(t_atom));
	freebytes(x->output, x->output_count * sizeof(t_atom));
	x->output = new_atom_list;
	memcpy(x->output + x->output_count, new_atom, sizeof(t_atom));
	++(x->output_count);
}

static void add_symbol_to_output(t_group *x, t_symbol *s)
{
	t_atom *temp_atom = getbytes(sizeof(t_atom));
	SETSYMBOL(temp_atom, s); 
	add_atom_to_output(x,temp_atom);
	freebytes(temp_atom,sizeof(t_atom));
}
		
static void add_float_to_output(t_group *x, t_float f)
{
	t_atom *temp_atom = getbytes(sizeof(t_atom));
	SETFLOAT(temp_atom, f);
	add_atom_to_output(x,temp_atom);
	freebytes(temp_atom,sizeof(t_atom));
}

static void reset_output(t_group *x)
{
	if(x->output)
	{
		freebytes(x->output, x->output_count * sizeof(t_atom));
		x->output = NULL;
		x->output_count = 0;
	}
}

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void group_output(t_group *x)
{
	DEBUG(post("group_output"););
	struct group *group_pointer;
	char **members;

#ifdef _WIN32
	/* TODO: implement for Windows! */
#else
	if( x->x_gid < 0 )
	{
		post("[group]: ignoring bad username or GID less than zero");
		outlet_bang(x->x_status_outlet);
	}
	else
	{
		group_pointer = getgrgid((gid_t)x->x_gid);
		if( group_pointer != NULL )
		{
			reset_output(x);
            /* group passwd just seems to be always blank, so omit it */
			/* add_symbol_to_output(x, gensym(group_pointer->gr_passwd)); */
			add_float_to_output(x, group_pointer->gr_gid);
			members = group_pointer->gr_mem;
			while(*members)
			{
				add_symbol_to_output(x, gensym( *(members) ));
				members++;
			}
			outlet_anything(x->x_data_outlet, gensym(group_pointer->gr_name), 
							x->output_count, x->output);
		}
		else
		{
			outlet_bang(x->x_status_outlet);
		}
	}
#endif /* _WIN32 */
}


static t_float get_gid_from_arguments(int argc, t_atom *argv)
{
	t_symbol *first_argument;
	t_float gid = -1;
	struct group *group_pointer;

	if(argc == 0) return(0);

	if(argc != 1)
		post("[group]: too many arguments (%d), ignoring all but the first", 
			 argc);

	first_argument = atom_getsymbolarg(0,argc,argv);
	if(first_argument == &s_) 
	{ // single float arg means GID #
		gid = atom_getfloatarg(0,argc,argv);
		if( gid < 0 )
		{
			error("[group]: GID less than zero not allowed (%d)", gid);
			return(-1);
		}
	}
	else
	{ // single symbol arg means username
		group_pointer = getgrnam(first_argument->s_name);
		if( group_pointer != NULL )
			return((t_float) group_pointer->gr_gid);
		else
			return(-1);
	}
	return(-1);
}


static void group_set(t_group *x, t_symbol *s, int argc, t_atom *argv)
{
    /* get rid of the unused variable warning with the if() statement */
	if( strcmp(s->s_name, "set") == 0 ) 
		x->x_gid = get_gid_from_arguments(argc, argv);
}


static void group_float(t_group *x, t_float f) 
{
	x->x_gid = f;
	group_output(x);
}

static void group_symbol(t_group *x, t_symbol *s) 
{
	t_atom argv[1];
	SETSYMBOL(argv, s);
	group_set(x, gensym("set"), 1, argv);
	group_output(x);
}


static void *group_new(t_symbol *s, int argc, t_atom *argv) 
{
	DEBUG(post("group_new"););

	t_group *x = (t_group *)pd_new(group_class);


    floatinlet_new(&x->x_obj, &x->x_gid);
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	group_set(x, gensym("set"), argc, argv);

	return (x);
}


void group_free(void) 
{
#ifdef _WIN32
#else
	endgrent();
#endif /* _WIN32 */	
}


void group_setup(void) 
{
	DEBUG(post("group_setup"););
	group_class = class_new(gensym("group"), 
								  (t_newmethod)group_new, 
								  0,
								  sizeof(t_group), 
								  0, 
								  A_GIMME, 
								  0);
	/* add inlet datatype methods */
	class_addbang(group_class, (t_method) group_output);
	class_addfloat(group_class, (t_method) group_float);
	class_addsymbol(group_class, (t_method) group_symbol);
	/* add inlet message methods */
	class_addmethod(group_class,
					(t_method) group_set,
					gensym("set"), 
					A_GIMME, 
					0);
    logpost(NULL, 4, "[group] %s",version);  
    logpost(NULL, 4, "\twritten by Hans-Christoph Steiner <hans@eds.org>");
    logpost(NULL, 4, "\tcompiled on "__DATE__" at "__TIME__ " ");
}

#endif /* NOT _WIN32 */
