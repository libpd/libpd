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

static t_atom _commaatom_;

typedef struct _comma
{
    t_object x_obj;
} t_comma;

t_class *comma_class;

void comma_setup(void);

static void comma_bang(t_comma *x)
{
    outlet_list(x->x_obj.ob_outlet, &s_list, 1, &_commaatom_);
}

static void *comma_new(void)
{
    t_comma *x = (t_comma *)pd_new(comma_class);
    outlet_new(&x->x_obj,&s_symbol);
    return (void *)x;
}

void comma_setup(void)
{
    comma_class = class_new(gensym("comma"),(t_newmethod)comma_new,
                            0, sizeof(t_comma), 0, 0);

    class_addbang(comma_class, comma_bang);
    SETCOMMA(&_commaatom_);
}

