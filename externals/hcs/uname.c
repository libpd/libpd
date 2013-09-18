/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting uname info                                             */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2006,2010 Hans-Christoph Steiner                            */
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

/* sadly, there is no uname in Windows, Cygwin has it tho */
#ifndef _WIN32

#include <m_pd.h>
#include <stdlib.h>

/* sadly, there is no uname in Windows, Cygwin has it tho */
#ifdef _WIN32
# include <windows.h>
# include <winsock2.h>
#else
# include <sys/utsname.h>
#endif

static char *version = "1.4";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *uname_class;

typedef struct _uname {
	  t_object            x_obj;
} t_uname;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void uname_output(t_uname* x)
{
	DEBUG(post("uname_output"););

#ifdef _WIN32
// TODO fake uname for Windows
//    http://msdn.microsoft.com/en-us/library/ms724429(VS.85).aspx 
#else
	struct utsname utsname_struct;
	t_atom uname_data[5];
	
	if ( uname(&utsname_struct) > -1 )
	{
		SETSYMBOL(uname_data, gensym(utsname_struct.sysname));
		SETSYMBOL(uname_data + 1, gensym(utsname_struct.nodename));
		SETSYMBOL(uname_data + 2, gensym(utsname_struct.release));
		SETSYMBOL(uname_data + 3, gensym(utsname_struct.version));
		SETSYMBOL(uname_data + 4, gensym(utsname_struct.machine));
	
		outlet_anything(x->x_obj.ob_outlet,
						atom_gensym(uname_data),
						4,
						uname_data + 1);
	}
#endif /* _WIN32 */
}


static void *uname_new(t_symbol *s) 
{
	DEBUG(post("uname_new"););

	t_uname *x = (t_uname *)pd_new(uname_class);

	outlet_new(&x->x_obj, &s_symbol);
	
	return (x);
}

void uname_setup(void) 
{
	DEBUG(post("uname_setup"););
	uname_class = class_new(gensym("uname"), 
								  (t_newmethod)uname_new, 
								  0,
								  sizeof(t_uname), 
								  0, 
								  0);
	/* add inlet datatype methods */
	class_addbang(uname_class,(t_method) uname_output);
	
}


#endif /* NOT _WIN32 */
