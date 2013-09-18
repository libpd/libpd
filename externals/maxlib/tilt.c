/* ------------------------- tilt --------------------------------------------- */
/*                                                                              */
/* Monitor input for changes.                                                   */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Inspired by code written by Trond Lossius.                                   */
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
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXSIZE 32

static char *version = "tilt v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct tilt
{
  t_object x_ob;
  t_outlet *x_out;                  /* result */
  t_clock  *x_clock;

  t_float  x_tilt;                  /* the result */
  t_float  x_start_tilt;
  t_float  x_t;
  t_float  x_sa;
  t_float  x_sb;
  t_float  x_offset;
  t_float  x_time;
  t_float  x_wait;
  t_float  x_hi_limit;
  t_float  x_low_limit;
  t_float  x_trip_point;
} t_tilt;

static void tilt_tick(t_tilt *x)
{
	x->x_sb = x->x_t - x->x_offset;
	if((x->x_sb - x->x_sa) > x->x_hi_limit)
	{
		x->x_sa = x->x_sb;
		clock_delay(x->x_clock, x->x_wait);
		return;
	}
	else
	{
		if((x->x_sb - x->x_sa) > x->x_trip_point)
		{
			outlet_bang(x->x_out);
			clock_delay(x->x_clock, x->x_wait);
			return;
		}
		if((x->x_sb - x->x_sa) < x->x_low_limit)
		{
			x->x_time++;
			if(x->x_time > 15)
			{
				x->x_start_tilt = x->x_sa;
				x->x_time = 0;
			}
		}
		if((x->x_sb - x->x_start_tilt) > x->x_tilt)
		{
			outlet_bang(x->x_out);
			clock_delay(x->x_clock, x->x_wait);
		}
		else
		{
			x->x_sa = x->x_sb;
			clock_delay(x->x_clock, x->x_wait);
			return;
		}
	}
}

static void tilt_float(t_tilt *x, t_floatarg f)
{
	x->x_t = f;
}

static void tilt_intv(t_tilt *x, t_floatarg f)
{
	x->x_wait = f;
}

static void tilt_tilt(t_tilt *x, t_floatarg f)
{
	x->x_tilt = f;
	post("tilt: set tilt to %g", x->x_tilt);
}

static void tilt_hi_limit(t_tilt *x, t_floatarg f)
{
	x->x_hi_limit = f;
	post("tilt: set high limit to %g", x->x_hi_limit);
}

static void tilt_low_limit(t_tilt *x, t_floatarg f)
{
	x->x_low_limit = f;
	post("tilt: set low limit to %g", x->x_low_limit);
}

static void tilt_trip_point(t_tilt *x, t_floatarg f)
{
	x->x_trip_point = f;
	post("tilt: set trip point to %g", x->x_trip_point);
}

static void tilt_free(t_tilt *x)
{
	clock_free(x->x_clock);
}

static t_class *tilt_class;

static void *tilt_new(t_floatarg f, t_floatarg f2)
{
	int i;

    t_tilt *x = (t_tilt *)pd_new(tilt_class);
	inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("intv"));
	x->x_out = outlet_new(&x->x_ob, gensym("float"));
	x->x_clock = clock_new(x, (t_method)tilt_tick);

	x->x_t = f;	/* set initial value */
	if(f2 > 4)
		x->x_wait = f2;
	else
		x->x_wait = 4000;
	x->x_offset = 0;
	x->x_sa = 0;
	x->x_sb = 0;
	x->x_time = 0;
	x->x_tilt = 0;
	x->x_start_tilt = x->x_sa = x->x_t - x->x_offset;
	x->x_hi_limit = x->x_low_limit = x->x_trip_point = 0;
	clock_delay(x->x_clock, x->x_wait);	/* wait 4 sec and start calculation */

	post("tilt: set interval to %g msec", x->x_wait);
    return (void *)x;
}

#ifndef MAXLIB
void tilt_setup(void)
{
    tilt_class = class_new(gensym("tilt"), (t_newmethod)tilt_new,
    	(t_method)tilt_free, sizeof(t_tilt), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(tilt_class, tilt_float);
	class_addmethod(tilt_class, (t_method)tilt_intv, gensym("intv"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_tilt, gensym("tilt"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_hi_limit, gensym("hi"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_low_limit, gensym("low"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_trip_point, gensym("trip"), A_FLOAT, 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_tilt_setup(void)
{
    tilt_class = class_new(gensym("maxlib_tilt"), (t_newmethod)tilt_new,
    	(t_method)tilt_free, sizeof(t_tilt), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)tilt_new, gensym("tilt"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(tilt_class, tilt_float);
	class_addmethod(tilt_class, (t_method)tilt_intv, gensym("intv"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_tilt, gensym("tilt"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_hi_limit, gensym("hi"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_low_limit, gensym("low"), A_FLOAT, 0);
	class_addmethod(tilt_class, (t_method)tilt_trip_point, gensym("trip"), A_FLOAT, 0);
    class_sethelpsymbol(tilt_class, gensym("maxlib/tilt-help.pd"));
}
#endif
