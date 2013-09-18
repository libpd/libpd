/*
 *   fwarp.c  - converts a frequency to a "standard" tangent warped freq
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
#include <math.h>

#define SEQL 16

 
typedef struct fwarp
{
  t_object t_ob;
  t_outlet *x_out;
} t_fwarp;

static void fwarp_float(t_fwarp *x, t_floatarg f)
{
    t_float twopi = 2.0f * M_PI;
    t_float sr = sys_getsr();
    f /= sr;
    f = tan(twopi * f) / twopi;
    outlet_float(x->x_out, f * sr);
}

static void fwarp_free(void)
{
}

t_class *fwarp_class;

static void *fwarp_new(void)
{
    t_fwarp *x = (t_fwarp *)pd_new(fwarp_class);
    x->x_out = outlet_new(&x->t_ob, gensym("float"));
    return (void *)x;
}

void fwarp_setup(void)
{
    fwarp_class = class_new(gensym("fwarp"), (t_newmethod)fwarp_new,
    	(t_method)fwarp_free, sizeof(t_fwarp), 0, 0);
    class_addfloat(fwarp_class, fwarp_float);
}

