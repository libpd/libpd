/* ------------------------- speedlim ----------------------------------------- */
/*                                                                              */
/* Lets information through only every N milliseconds.                          */
/* Written by Krzysztof Czaja for his cyclone library.                          */
/* Modified to fit into maxlib by Olaf Matthes <olaf.matthes@gmx.de>.           */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* You should have received a copy of the GNU Lesser General Public             */
/* License along with this library; if not, write to the                        */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,                 */
/* Boston, MA  02111-1307, USA.                                                 */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

/* this is the original copyright notice: */

/* Copyright (c) 1997-2002 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */


#include <string.h>
#include "m_pd.h"

#define SPEEDLIM_INISIZE   32  /* LATER rethink */
#define SPEEDLIM_MAXSIZE  256  /* not used */

typedef struct _speedlim
{
    t_object     x_ob;
    int          x_open;
    t_float      x_delta;
    t_symbol    *x_selector;
    t_float      x_float;
    t_symbol    *x_symbol;
    t_gpointer  *x_pointer;
    int          x_size;    /* as allocated */
    int          x_natoms;  /* as used */
    t_atom      *x_message;
    t_atom       x_messini[SPEEDLIM_INISIZE];
    int          x_entered;
    t_clock     *x_clock;
} t_speedlim;

static t_class *speedlim_class;

/* a caller must check for nrequested > *sizep */
/* returns actual number of atoms: requested (success)
   or a default value of initial size (failure) */
/* the result is guaranteed to be >= min(nrequested, inisize) */
static int speedlim_grow(int nrequested, int *sizep, t_atom **bufp,
		int inisize, t_atom *bufini)
{
    int newsize = *sizep * 2;
    while (newsize < nrequested) newsize *= 2;
    if (*bufp == bufini)
	*bufp = (t_atom *)getbytes(newsize * sizeof(**bufp));
    else
	*bufp = (t_atom *)resizebytes(*bufp, *sizep * sizeof(**bufp),
				      newsize * sizeof(**bufp));
    if (*bufp)
	*sizep = newsize;
    else
    {
	*bufp = bufini;
	nrequested = *sizep = inisize;
    }
    return (nrequested);
}

static void speedlim_dooutput(t_speedlim *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_open = 0;     /* so there will be no reentrant calls of dooutput */
    x->x_entered = 1;  /* this prevents a message from being overridden */
    clock_unset(x->x_clock);
    if (s == &s_bang)
	outlet_bang(((t_object *)x)->ob_outlet);
    else if (s == &s_float)
	outlet_float(((t_object *)x)->ob_outlet, x->x_float);
    else if (s == &s_symbol && x->x_symbol)
    {
	/* if x_symbol is null, then symbol &s_ is passed
	   by outlet_anything() -> typedmess() */
	outlet_symbol(((t_object *)x)->ob_outlet, x->x_symbol);
	x->x_symbol = 0;
    }
    else if (s == &s_pointer && x->x_pointer)
    {
	/* LATER */
	x->x_pointer = 0;
    }
    else if (s == &s_list)
	outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
    else if (s)
	outlet_anything(((t_object *)x)->ob_outlet, s, ac, av);
    x->x_selector = 0;
    x->x_natoms = 0;
    if (x->x_delta > 0)
	clock_delay(x->x_clock, x->x_delta);
    else
	x->x_open = 1;
    x->x_entered = 0;
}

static void speedlim_tick(t_speedlim *x)
{
    if (x->x_selector)
	speedlim_dooutput(x, x->x_selector, x->x_natoms, x->x_message);
    else
	x->x_open = 1;
}

static void speedlim_anything(t_speedlim *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_open)
	speedlim_dooutput(x, s, ac, av);
    else if (s && s != &s_ && !x->x_entered)
    {
	if (ac > x->x_size)
	    /* MAXSIZE not used, not even a warning...
	       LATER consider clipping */
	    ac = speedlim_grow(ac, &x->x_size, &x->x_message,
			     SPEEDLIM_INISIZE, x->x_messini);
	x->x_selector = s;
	x->x_natoms = ac;
	if (ac)
	    memcpy(x->x_message, av, ac * sizeof(*x->x_message));
    }
}

static void speedlim_bang(t_speedlim *x)
{
    x->x_selector = &s_bang;
    speedlim_anything(x, x->x_selector, 0, 0);
}

static void speedlim_float(t_speedlim *x, t_float f)
{
    x->x_selector = &s_float;
    x->x_float = f;
    speedlim_anything(x, x->x_selector, 0, 0);
}

static void speedlim_symbol(t_speedlim *x, t_symbol *s)
{
    x->x_selector = &s_symbol;
    x->x_symbol = s;
    speedlim_anything(x, x->x_selector, 0, 0);
}

/* LATER gpointer */

static void speedlim_list(t_speedlim *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_selector = &s_list;
    speedlim_anything(x, x->x_selector, ac, av);
}

static void speedlim_ft1(t_speedlim *x, t_floatarg f)
{
    if (f < 0)
	f = 0;  /* redundant (and CHECKED) */
    x->x_delta = f;
    /* CHECKED: no rearming --
       if clock is set, then new delta value is not used until next tick */
}

static void speedlim_free(t_speedlim *x)
{
    if (x->x_message != x->x_messini)
	freebytes(x->x_message, x->x_size * sizeof(*x->x_message));
    if (x->x_clock)
	clock_free(x->x_clock);
}

static void *speedlim_new(t_floatarg f)
{
    t_speedlim *x = (t_speedlim *)pd_new(speedlim_class);
    x->x_open = 1;  /* CHECKED */
    x->x_delta = 0;
    x->x_selector = 0;
    x->x_float = 0;
    x->x_symbol = 0;
    x->x_pointer = 0;
    x->x_size = SPEEDLIM_INISIZE;
    x->x_natoms = 0;
    x->x_message = x->x_messini;
    x->x_entered = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_anything);
    x->x_clock = clock_new(x, (t_method)speedlim_tick);
    speedlim_ft1(x, f);
    return (x);
}

#ifndef MAXLIB
void speedlim_setup(void)
{
    speedlim_class = class_new(gensym("speedlim"), (t_newmethod)speedlim_new,
		(t_method)speedlim_free, sizeof(t_speedlim), 0, A_DEFFLOAT, 0);
#else
void maxlib_speedlim_setup(void)
{
    speedlim_class = class_new(gensym("maxlib_speedlim"), (t_newmethod)speedlim_new,
		(t_method)speedlim_free, sizeof(t_speedlim), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)speedlim_new, gensym("speedlim"), A_DEFFLOAT, 0);
#endif
    class_addbang(speedlim_class, speedlim_bang);
    class_addfloat(speedlim_class, speedlim_float);
    class_addsymbol(speedlim_class, speedlim_symbol);
    class_addlist(speedlim_class, speedlim_list);
    class_addanything(speedlim_class, speedlim_anything);
    class_addmethod(speedlim_class, (t_method)speedlim_ft1, gensym("ft1"), A_FLOAT, 0);
#ifndef MAXLIB
    
#else
    class_sethelpsymbol(speedlim_class, gensym("maxlib/speedlim-help.pd"));
#endif
}
