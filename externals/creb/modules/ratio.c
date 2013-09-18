/*
 *   ratio.c  - multiplies by 2^k such that output is between 1 and 2
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "m_pd.h"

#define SEQL 16

typedef struct{
} t_ratio_data;
 
typedef struct ratio
{
  t_object t_ob;
  t_outlet *x_out;
  t_ratio_data x_c;
} t_ratio;

static void ratio_float(t_ratio *x, t_floatarg f)
{
  f = (f<0)?(-f):(f);
  if (f)
    {
      while (f <  1.0) f *= 2.0;
      while (f >= 2.0) f *= 0.5;
    }
  outlet_float(x->x_out, f);

}

static void ratio_free(void)
{
}

t_class *ratio_class;

static void *ratio_new(void)
{
    t_ratio *x = (t_ratio *)pd_new(ratio_class);
    x->x_out = outlet_new(&x->t_ob, gensym("float"));
    return (void *)x;
}

void ratio_setup(void)
{
    ratio_class = class_new(gensym("ratio"), (t_newmethod)ratio_new,
    	(t_method)ratio_free, sizeof(t_ratio), 0, 0);
    class_addfloat(ratio_class, ratio_float);
}

