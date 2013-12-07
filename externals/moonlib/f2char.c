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

typedef struct _f2char
{
    t_object x_obj;
} t_f2char;

t_class *f2char_class;

void f2char_setup(void);

static void f2char_float(t_f2char *x,t_floatarg f)
{
    char s[2]= {0};

    s[0]=(char)f;
    outlet_symbol(x->x_obj.ob_outlet,gensym(s));
}

static void *f2char_new(void)
{
    t_f2char *x = (t_f2char *)pd_new(f2char_class);
    outlet_new(&x->x_obj,&s_symbol);
    return (void *)x;
}

void f2char_setup(void)
{
    f2char_class = class_new(gensym("f2char"),(t_newmethod)f2char_new,
                             0, sizeof(t_f2char), 0, 0);

    class_addfloat(f2char_class, f2char_float);
}

