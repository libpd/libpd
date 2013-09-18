/* -------------------------- temperature ------------------------------------- */
/*                                                                              */
/* Calculates temperature: number of 'events' within N milliseconds.            */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/              */
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
#include <math.h>

static char *version = "temperature v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct temperature
{
  t_object x_ob;
  t_clock  *x_clock;
  t_outlet *x_outfloat;             /* output the temperature */
  t_int    x_index;                 /* the number of elements to average */
  t_int    x_time;

} t_temperature;

static void temperature_tick(t_temperature *x)
{
	outlet_float(x->x_outfloat, x->x_index);
	x->x_index = 0;
	clock_delay(x->x_clock, x->x_time);
}

static void temperature_float(t_temperature *x, t_floatarg f)
{
	x->x_index++;	/* just count number of 'events' */
}

static void temperature_anything(t_temperature *x, t_symbol *s, int argc, t_atom *argv)
{
	x->x_index++;	/* just count number of 'events' */
}

static void temperature_time(t_temperature *x, t_floatarg f)
{
	x->x_time = (t_int)f;
	if(x->x_time < 1) x->x_time = 1;
	clock_unset(x->x_clock);
	clock_delay(x->x_clock, x->x_time);
}

static void temperature_reset(t_temperature *x)
{
	x->x_index = 0;
    post("temperature: reset");
}

static void temperature_free(t_temperature *x)
{
	clock_free(x->x_clock);
}

static t_class *temperature_class;

static void *temperature_new(t_floatarg f)
{
    t_temperature *x = (t_temperature *)pd_new(temperature_class);
	inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("time"));
	x->x_outfloat = outlet_new(&x->x_ob, gensym("float"));
	x->x_clock = clock_new(x, (t_method)temperature_tick);

	x->x_time = (t_int)f;
	if(x->x_time < 1)
	{
		x->x_time = 1;
		post("temperature: set time to %d ms", x->x_time);
	}
	x->x_index = 0;
	clock_delay(x->x_clock, x->x_time);
    return (void *)x;
}

#ifndef MAXLIB
void temperature_setup(void)
{
    temperature_class = class_new(gensym("temperature"), (t_newmethod)temperature_new,
    	(t_method)temperature_free, sizeof(t_temperature), 0, A_DEFFLOAT, 0);
    class_addmethod(temperature_class, (t_method)temperature_reset, gensym("reset"), 0);
    class_addfloat(temperature_class, temperature_float);
	class_addmethod(temperature_class, (t_method)temperature_time, gensym("time"), A_FLOAT, 0);
	class_addanything(temperature_class, temperature_anything);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_temperature_setup(void)
{
    temperature_class = class_new(gensym("maxlib_temperature"), (t_newmethod)temperature_new,
    	(t_method)temperature_free, sizeof(t_temperature), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)temperature_new, gensym("temperature"), A_DEFFLOAT, 0);
    class_addmethod(temperature_class, (t_method)temperature_reset, gensym("reset"), 0);
    class_addfloat(temperature_class, temperature_float);
	class_addmethod(temperature_class, (t_method)temperature_time, gensym("time"), A_FLOAT, 0);
	class_addanything(temperature_class, temperature_anything);
    class_sethelpsymbol(temperature_class, gensym("maxlib/temperature-help.pd"));
}
#endif
