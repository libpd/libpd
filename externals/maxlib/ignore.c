/* ------------------------- ignore ------------------------------------------- */
/*                                                                              */
/* Ignores input that is followed by next value faster than N ms.               */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
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

static char *version = "ignore v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct ignore
{
  t_object x_ob;
  t_clock  *x_clock;
  t_inlet  *x_intime;
  t_outlet *x_outfloat;
  t_float  x_input;
  t_float  x_lastinput;
  t_int    x_limit;                 /* indicates if input is 'blocked' (1) */
  t_int    x_time;                  /* the time in ms */
} t_ignore;

static void ignore_tick(t_ignore *x)
{
	x->x_limit = 0;
		/* output in case nothing has changed */
	if(x->x_lastinput == x->x_input)
		outlet_float(x->x_outfloat, x->x_lastinput);
}

static void ignore_float(t_ignore *x, t_floatarg f)
{
	x->x_input = f;
	if(!x->x_limit)
	{
		x->x_limit = 1;		                /* ignore input within next N ms */
		clock_delay(x->x_clock, x->x_time); /* start clock */
	}
	else	/* ignore / start clock again */
	{
		x->x_lastinput = x->x_input;        /* save current as last valid value */
		clock_unset(x->x_clock);            /* throw out last clock */
		clock_delay(x->x_clock, x->x_time); /* start new clock */
	}
}

static void ignore_time(t_ignore *x, t_floatarg f)
{
	x->x_time = (t_int)f;
}

static void ignore_reset(t_ignore *x)
{
	x->x_limit = 0;
    post("ignore: reset");
}

static void ignore_free(t_ignore *x)
{
	clock_free(x->x_clock);
}

static t_class *ignore_class;

static void *ignore_new(t_floatarg f)
{
    t_ignore *x = (t_ignore *)pd_new(ignore_class);
	x->x_intime = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("time"));
	x->x_outfloat = outlet_new(&x->x_ob, gensym("float"));
    x->x_clock = clock_new(x, (t_method)ignore_tick);

	x->x_time = (t_int)f;
	x->x_lastinput = 0;

    return (void *)x;
}

#ifndef MAXLIB
void ignore_setup(void)
{
    ignore_class = class_new(gensym("ignore"), (t_newmethod)ignore_new,
    	(t_method)ignore_free, sizeof(t_ignore), 0, A_DEFFLOAT, 0);
    class_addmethod(ignore_class, (t_method)ignore_reset, gensym("reset"), 0);
    class_addmethod(ignore_class, (t_method)ignore_time, gensym("time"), A_FLOAT, 0);
    class_addfloat(ignore_class, ignore_float);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_ignore_setup(void)
{
    ignore_class = class_new(gensym("maxlib_ignore"), (t_newmethod)ignore_new,
    	(t_method)ignore_free, sizeof(t_ignore), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)ignore_new, gensym("ignore"), A_DEFFLOAT, 0);
    class_addmethod(ignore_class, (t_method)ignore_reset, gensym("reset"), 0);
    class_addmethod(ignore_class, (t_method)ignore_time, gensym("time"), A_FLOAT, 0);
    class_addfloat(ignore_class, ignore_float);
    class_sethelpsymbol(ignore_class, gensym("maxlib/ignore-help.pd"));
}
#endif

