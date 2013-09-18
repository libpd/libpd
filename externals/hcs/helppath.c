/*
 * This object outputs the global search path for finding objects using a 
 * similar interface as [textfile].                                       
 *                                                                        
 * Copyright (c) 2007 Free Software Foundation
 *                                                                        
 * This program is free software; you can redistribute it and/or          
 * modify it under the terms of the GNU General Public License            
 * as published by the Free Software Foundation; either version 3         
 * of the License, or (at your option) any later version.                 
 *                                                                        
 * See file LICENSE for further informations on licensing terms.          
 *                                                                        
 * This program is distributed in the hope that it will be useful,        
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          
 * GNU General Public License for more details.                           

 * You should have received a copy of the GNU General Public License      
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.        
 */

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

static char *version = "$Revision: 1.1 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *helppath_class;

typedef struct _helppath {
	t_object            x_obj;
	t_namelist          *x_top;
	t_namelist          *x_current;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_helppath;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void helppath_output(t_helppath* x)
{
	DEBUG(post("helppath_output"););

/* TODO: think about using x->x_current->nl_next so that if [helppath] is at
 * the end of its list, and another element gets added to the global
 * helppath, [helppath] will output the new element on the next bang. */
	if(x->x_current)
	{
		outlet_symbol( x->x_data_outlet, gensym(x->x_current->nl_string) );
		x->x_current = x->x_current->nl_next;
	}
	else 
    {
        outlet_bang(x->x_status_outlet);
    }
}


static void helppath_reset(t_helppath* x) 
{
	DEBUG(post("helppath_reset"););

	x->x_current = x->x_top = sys_helppath;
}


static void helppath_add(t_helppath* x, t_symbol *s) 
{
	DEBUG(post("helppath_add"););
}


static void *helppath_new() 
{
	DEBUG(post("helppath_new"););
	t_helppath *x = (t_helppath *)pd_new(helppath_class);

	x->x_data_outlet = outlet_new(&x->x_obj, &s_symbol);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	helppath_reset(x);

	return (x);
}

void helppath_free()
{
	// TODO: look into freeing the namelist
}


void helppath_setup(void) 
{
	DEBUG(post("helppath_setup"););
	helppath_class = class_new(gensym("helppath"), 
								  (t_newmethod)helppath_new, 
								  0,
								  sizeof(t_helppath), 
								  0, 
								  0);
	/* add inlet datatype methods */
	class_addbang(helppath_class,(t_method) helppath_output);
	
	/* add inlet message methods */
	class_addmethod(helppath_class,(t_method) helppath_reset,
					gensym("reset"), 0);
	class_addmethod(helppath_class,(t_method) helppath_add,gensym("add"), 
					A_DEFSYMBOL, 0);

    logpost(NULL, 4, "[helppath] %s", version);  
    logpost(NULL, 4, "\tcompiled on "__DATE__" at "__TIME__ " ");
    logpost(NULL, 4, "\tcompiled against Pd version %d.%d.%d", PD_MAJOR_VERSION, 
            PD_MINOR_VERSION, PD_BUGFIX_VERSION);
}

