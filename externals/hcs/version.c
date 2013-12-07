/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting the version of Pd-extended                             */
/* (it gets the version at compile time, so it will show the version of Pd   */
/* that is was compiled against)                                             */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2006, 2010 Hans-Christoph Steiner                           */
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

#include <m_pd.h>

static char *version = "1.3";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *version_class;

typedef struct _version {
	  t_object            x_obj;
} t_version;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void version_output(t_version* x)
{
	DEBUG(post("version_output"););
	int major, minor, bugfix;
	t_atom version_data[6];
	sys_getversion(&major, &minor, &bugfix);
	SETFLOAT(version_data, (t_float) major);
	SETFLOAT(version_data + 1, (t_float) minor);
	SETFLOAT(version_data + 2, (t_float) bugfix);
	SETSYMBOL(version_data + 3, gensym(PD_TEST_VERSION));
	SETSYMBOL(version_data + 4, gensym(__DATE__));
	SETSYMBOL(version_data + 5, gensym(__TIME__));
	
	outlet_list(x->x_obj.ob_outlet, &s_list, 6, version_data);
}


static void *version_new(t_symbol *s) 
{
	DEBUG(post("version_new"););

	t_version *x = (t_version *)pd_new(version_class);

	outlet_new(&x->x_obj, &s_list);
	
	return (x);
}

void version_setup(void) 
{
	DEBUG(post("version_setup"););
	version_class = class_new(gensym("version"), 
								  (t_newmethod)version_new, 
								  0,
								  sizeof(t_version), 
								  0, 
								  0);
	/* add inlet datatype methods */
	class_addbang(version_class,(t_method) version_output);
}
