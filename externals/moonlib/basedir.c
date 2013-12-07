#ifndef _WIN32
/*
Copyright (C) 2002 Antoine Rousseau

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*#include <m_imp.h>*/
#include "m_pd.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>

typedef struct _basedir
{
    t_object x_obj;
} t_basedir;

t_class *basedir_class;

void basedir_setup(void);

static void basedir_symbol(t_basedir *x,t_symbol *sym)
{
    static t_binbuf *binbuf=0;
    t_atom at[2];
    char *b,*d;
    int l;

    if(!sym->s_name) return;

    b=strdup(sym->s_name);
    d=strdup(sym->s_name);

    SETSYMBOL(&at[0],gensym(basename(b)));
    SETSYMBOL(&at[1],gensym(dirname(d)));

    outlet_list(x->x_obj.ob_outlet,0,2,at);
}


static void *basedir_new(void)
{
    t_basedir *x = (t_basedir *)pd_new(basedir_class);
    outlet_new(&x->x_obj, 0);
    return (void *)x;
}

void basedir_setup(void)
{
    basedir_class = class_new(gensym("basedir"),(t_newmethod)basedir_new,
                              0, sizeof(t_basedir), 0, 0);

    class_addsymbol(basedir_class, basedir_symbol);
}


#endif /* NOT _WIN32 */
