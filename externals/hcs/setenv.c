/* sets an environment variable                                              */
/*                                                                           */
/* Copyright (c) 2008 Hans-Christoph Steiner <hans@eds.org>                  */
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

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#else
#include <stdlib.h>
#endif

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *setenv_class;

typedef struct _setenv {
	t_object            x_obj;
    t_symbol*           x_variable_name;
    t_int               x_overwrite;
	t_outlet*           x_data_outlet;
} t_setenv;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void setenv_output(t_setenv* x)
{
	DEBUG(post("setenv_output"););
    char *envvar_value;
    if(x->x_variable_name != &s_)
    {
        envvar_value = getenv(x->x_variable_name->s_name);
        if(envvar_value)
            outlet_symbol(x->x_data_outlet, gensym(envvar_value));
        else
            pd_error(x, "[setenv]: The environment variable %s is not defined.", x->x_variable_name->s_name);
    }
    else
        pd_error(x, "[setenv]: no environment variable name is set.");
}


static void setenv_symbol(t_setenv* x, t_symbol *s)
{
	DEBUG(post("setenv_output"););
#ifdef _WIN32
    SetEnvironmentVariable(x->x_variable_name->s_name, s->s_name);
#else
    if(x->x_overwrite == 0)
        post("[setenv]: not in overwrite mode.");
    setenv(x->x_variable_name->s_name, s->s_name, x->x_overwrite);
#endif
    setenv_output(x);
}

static void setenv_anything(t_setenv* x, t_symbol* s, int argc, t_atom* argv)
{
    t_binbuf *argument_binbuf = binbuf_new();
    char *argument_buffer;
    int buffer_length;
    if(s != &s_list)
    {
        t_atom selector;
        SETSYMBOL(&selector, s);
        binbuf_add(argument_binbuf, 1, &selector);
    }
    binbuf_add(argument_binbuf, argc, argv);
    binbuf_gettext(argument_binbuf, &argument_buffer, &buffer_length);
    binbuf_free(argument_binbuf);
    argument_buffer[buffer_length] = 0;
    setenv_symbol(x, gensym(argument_buffer));
}


static void *setenv_new(t_symbol* s, t_float f) 
{
	DEBUG(post("setenv_new"););
	t_setenv *x = (t_setenv *)pd_new(setenv_class);

    symbolinlet_new(&x->x_obj, &x->x_variable_name);
	x->x_data_outlet = outlet_new(&x->x_obj, &s_symbol);

    x->x_overwrite = (t_int) f;
    x->x_variable_name = s;

	return (x);
}


void setenv_setup(void) 
{
	DEBUG(post("setenv_setup"););
	setenv_class = class_new(gensym("setenv"), 
                             (t_newmethod)setenv_new, 
                             0,
                             sizeof(t_setenv), 
                             0,
                             A_DEFSYMBOL, 
                             A_DEFFLOAT, 
                             0);
	/* add inlet datatype methods */
	class_addbang(setenv_class, (t_method)setenv_output);
	class_addsymbol(setenv_class, (t_method)setenv_symbol);
	class_addanything(setenv_class, (t_method)setenv_anything);
}

