/* --------------------------------------------------------------------------*/
/*                                                                           */
/* This object outputs the global search path for finding objects using a    */
/* similar interface as [textfile] and [qlist].                               */
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

#include "m_pd.h"
#include "s_stuff.h"

#include <string.h>

/*
#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#else
#include <stdlib.h>
#endif
*/

static char *version = "$Revision: 1.3 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *classpath_class;

typedef struct _classpath {
	t_object            x_obj;
	t_namelist          *x_top;
	t_namelist          *x_current;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_classpath;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void classpath_output(t_classpath* x)
{
	DEBUG(post("classpath_output"););

/* TODO: think about using x->x_current->nl_next so that if [classlist] is at
 * the end of its list, and another element gets added to the global
 * classpath, [classpath] will output the new element on the next bang. */
	if(x->x_current)
	{
		outlet_symbol( x->x_data_outlet, gensym(x->x_current->nl_string) );
		x->x_current = x->x_current->nl_next;
	}
	else 
		outlet_bang(x->x_status_outlet);
}


static void classpath_rewind(t_classpath* x) 
{
	DEBUG(post("classpath_output"););
	char buffer[MAXPDSTRING];
	
	strncpy(buffer, sys_libdir->s_name, MAXPDSTRING - 6);
	strcat(buffer, "/extra");
	x->x_top = namelist_append(NULL, buffer, 0);
	x->x_top->nl_next = sys_searchpath;
	x->x_current = x->x_top;
}


static void classpath_add(t_classpath* x, t_symbol *s) 
{
	DEBUG(post("classpath_add"););
}


static void *classpath_new() 
{
	DEBUG(post("classpath_new"););
	t_classpath *x = (t_classpath *)pd_new(classpath_class);

	x->x_data_outlet = outlet_new(&x->x_obj, &s_symbol);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	classpath_rewind(x);

	return (x);
}

void classpath_free()
{
	// TODO: look into freeing the namelist
}


void classpath_setup(void) 
{
	DEBUG(post("classpath_setup"););
	classpath_class = class_new(gensym("classpath"), 
								  (t_newmethod)classpath_new, 
								  0,
								  sizeof(t_classpath), 
								  0, 
								  0);
	/* add inlet datatype methods */
	class_addbang(classpath_class,(t_method) classpath_output);
	
	/* add inlet message methods */
	class_addmethod(classpath_class,(t_method) classpath_rewind,
					gensym("rewind"), 0);
	class_addmethod(classpath_class,(t_method) classpath_add,gensym("add"), 
					A_DEFSYMBOL, 0);

    logpost(NULL, 4, "[classpath] %s",version);  
    logpost(NULL, 4, "\twritten by Hans-Christoph Steiner <hans@eds.org>");
    logpost(NULL, 4, "\tcompiled on "__DATE__" at "__TIME__ " ");
    logpost(NULL, 4, "\tcompiled against Pd version %d.%d.%d", PD_MAJOR_VERSION, 
            PD_MINOR_VERSION, PD_BUGFIX_VERSION);
}

