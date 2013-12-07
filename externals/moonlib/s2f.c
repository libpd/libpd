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

#include "m_pd.h"
#include <stdlib.h>

typedef struct _s2f
{
    t_object x_obj;
} t_s2f;

t_class *s2f_class;

void s2f_setup(void);

static void s2f_symbol(t_s2f *x,t_symbol *sym)
{
    if(!sym->s_name) return;

    outlet_float(x->x_obj.ob_outlet,(t_float)strtod(sym->s_name,0));
}

static void *s2f_new(void)
{
    t_s2f *x = (t_s2f *)pd_new(s2f_class);
    outlet_new(&x->x_obj, &s_float);
    return (void *)x;
}

void s2f_setup(void)
{
    s2f_class = class_new(gensym("s2f"),(t_newmethod)s2f_new,
                          0, sizeof(t_s2f), 0, 0);

    class_addsymbol(s2f_class, s2f_symbol);
}

