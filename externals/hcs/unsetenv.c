/* unsets an environment variable                                            */
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
static t_class *unsetenv_class;

typedef struct _unsetenv {
    t_object            x_obj;
    t_symbol*           x_variable_name;
    t_outlet*           x_data_outlet;
} t_unsetenv;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void unsetenv_bang(t_unsetenv* x)
{
    DEBUG(post("unsetenv_bang"););

#ifdef _WIN32
    if(x->x_variable_name != &s_)
    {
        char buf[MAXPDSTRING];
        strncpy(buf, x->x_variable_name->s_name, MAXPDSTRING - 1);
        strcat(buf, "=");
        putenv(buf);
    }
#else
    if(x->x_variable_name != &s_)
        unsetenv(x->x_variable_name->s_name);
#endif
}



static void unsetenv_symbol(t_unsetenv* x, t_symbol *s)
{
    DEBUG(post("unsetenv_bang"););
    x->x_variable_name = s;
}


static void *unsetenv_new(t_symbol* s) 
{
    DEBUG(post("unsetenv_new"););
    t_unsetenv *x = (t_unsetenv *)pd_new(unsetenv_class);

    symbolinlet_new(&x->x_obj, &x->x_variable_name);

    x->x_variable_name = s;

    return (x);
}


void unsetenv_setup(void) 
{
    DEBUG(post("unsetenv_setup"););
    unsetenv_class = class_new(gensym("unsetenv"), 
                             (t_newmethod)unsetenv_new, 
                             0,
                             sizeof(t_unsetenv), 
                             0,
                             A_DEFSYMBOL, 
                             0);
    /* add inlet datatype methods */
    class_addbang(unsetenv_class, (t_method)unsetenv_bang);
    class_addsymbol(unsetenv_class, (t_method)unsetenv_symbol);
}

