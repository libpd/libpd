/* --------------------------------------------------------------------------*/
/*                                                                           */
/* converts a UID number to a user name symbol                               */
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
#include <stdlib.h>
#include <pwd.h>
#endif

#include <string.h>

static char *version = "$Revision: 1.3 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *passwd_class;

typedef struct _passwd {
	t_object            x_obj;
	t_float             x_uid;  
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_passwd;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void passwd_output(t_passwd *x)
{
	DEBUG(post("passwd_output"););
	struct passwd *passwd_pointer;
	t_atom output_data[6];

#ifdef _WIN32
	/* TODO: implement for Windows! */
#else
	if( x->x_uid < 0 )
	{
		post("[passwd]: ignoring bad username or UID less than zero");
		outlet_bang(x->x_status_outlet);
	}
	else
	{
		passwd_pointer = getpwuid((uid_t)x->x_uid);
		if( passwd_pointer != NULL )
		{
			SETSYMBOL(output_data, gensym(passwd_pointer->pw_passwd));
			SETFLOAT(output_data + 1, passwd_pointer->pw_uid);
			SETFLOAT(output_data + 2, passwd_pointer->pw_gid);
			SETSYMBOL(output_data + 3, gensym(passwd_pointer->pw_gecos));
			SETSYMBOL(output_data + 4, gensym(passwd_pointer->pw_dir));
			SETSYMBOL(output_data + 5, gensym(passwd_pointer->pw_shell));
			outlet_anything(x->x_data_outlet, gensym(passwd_pointer->pw_name), 
							6, output_data);
		}
		else
		{
			outlet_bang(x->x_status_outlet);
		}
	}
#endif /* _WIN32 */
}


static t_float get_uid_from_arguments(int argc, t_atom *argv)
{
	t_symbol *first_argument;
	t_float uid = -1;
	struct passwd *passwd_pointer;

	if(argc == 0) return(0);

	if(argc != 1)
		post("[passwd]: too many arguments (%d), ignoring all but the first", 
			 argc);

	first_argument = atom_getsymbolarg(0,argc,argv);
	if(first_argument == &s_) 
	{ // single float arg means UID #
		uid = atom_getfloatarg(0,argc,argv);
		if( uid < 0 )
		{
			error("[passwd]: UID less than zero not allowed (%d)", uid);
			return(-1);
		}
	}
	else
	{ // single symbol arg means username
		passwd_pointer = getpwnam(first_argument->s_name);
		if( passwd_pointer != NULL )
			return((t_float) passwd_pointer->pw_uid);
		else
			return(-1);
	}
	return(-1);
}


static void passwd_set(t_passwd *x, t_symbol *s, int argc, t_atom *argv)
{
    /* get rid of the unused variable warning with the if() statement */
	if( strcmp(s->s_name, "set") == 0 ) 
		x->x_uid = get_uid_from_arguments(argc, argv);
}


static void passwd_float(t_passwd *x, t_float f) 
{
	x->x_uid = f;
	passwd_output(x);
}

static void passwd_symbol(t_passwd *x, t_symbol *s) 
{
	t_atom argv[1];
	SETSYMBOL(argv, s);
	passwd_set(x, gensym("set"), 1, argv);
	passwd_output(x);
}


static void *passwd_new(t_symbol *s, int argc, t_atom *argv) 
{
	DEBUG(post("passwd_new"););

	t_passwd *x = (t_passwd *)pd_new(passwd_class);

    floatinlet_new(&x->x_obj, &x->x_uid);
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	passwd_set(x, gensym("set"), argc, argv);

	return (x);
}


void passwd_free(void) 
{
#ifdef _WIN32
#else
	endpwent();
#endif /* _WIN32 */	
}


void passwd_setup(void) 
{
	DEBUG(post("passwd_setup"););
	passwd_class = class_new(gensym("passwd"), 
								  (t_newmethod)passwd_new, 
								  0,
								  sizeof(t_passwd), 
								  0, 
								  A_GIMME, 
								  0);
	/* add inlet datatype methods */
	class_addbang(passwd_class, (t_method) passwd_output);
	class_addfloat(passwd_class, (t_method) passwd_float);
	class_addsymbol(passwd_class, (t_method) passwd_symbol);
	/* add inlet message methods */
	class_addmethod(passwd_class,
					(t_method) passwd_set,
					gensym("set"), 
					A_GIMME, 
					0);

    logpost(NULL, 4, "[passwd] %s",version);  
    logpost(NULL, 4, "\twritten by Hans-Christoph Steiner <hans@eds.org>");
    logpost(NULL, 4, "\tcompiled on "__DATE__" at "__TIME__ " ");
}

#endif /* NOT _WIN32 */
