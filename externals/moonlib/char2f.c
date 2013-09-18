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


typedef struct _char2f
{
    t_object x_obj;
} t_char2f;

t_class *char2f_class;

void char2f_setup(void);

static void char2f_symbol(t_char2f *x,t_symbol *sym)
{
    if(!sym->s_name) return;

    outlet_float(x->x_obj.ob_outlet,(t_float)sym->s_name[0]);
}

static void *char2f_new(void)
{
    t_char2f *x = (t_char2f *)pd_new(char2f_class);
    outlet_new(&x->x_obj, &s_float);
    return (void *)x;
}

void char2f_setup(void)
{
    char2f_class = class_new(gensym("char2f"),(t_newmethod)char2f_new,
                             0, sizeof(t_char2f), 0, 0);

    class_addsymbol(char2f_class, char2f_symbol);
}

