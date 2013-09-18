/* ------------------------- gestalt   ---------------------------------------- */
/*                                                                              */
/* Find the 'gestalt' of the MIDI input.                                        */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
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
#include <math.h>
#include <stdio.h>
#ifndef _WIN32
#include <stdlib.h>
#endif

static char *version = "gestalt v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct gestalt
{
  t_object x_ob;
  t_inlet  *x_invelocity;             /* inlet for velocity */
  t_outlet *x_outgestalt;             /* calculated 'gestalt'-value */

  t_float  x_lastpitch;
  t_float  x_velocity;

  t_float  x_reftime;

  double   x_lastontime;

} t_gestalt;

static void gestalt_ft1(t_gestalt *x, t_floatarg f)
{
	x->x_velocity = f;
}

static void gestalt_ft2(t_gestalt *x, t_floatarg f)
{
	if(f > 0.0) x->x_reftime = f;
}

static void gestalt_float(t_gestalt *x, t_floatarg f)
{

	int interval, pitch, gestalt;
	double ontime = clock_getlogicaltime();
	
	if(x->x_velocity)	/* only process note-ons */
	{

		pitch = (t_int)f;
		if(pitch < 1) pitch = 0;
		if(pitch > 127) pitch = 127;

		interval = abs(pitch - x->x_lastpitch);
		gestalt = (clock_gettimesince(x->x_lastontime)/x->x_reftime) + interval;

		x->x_lastpitch = pitch;
		x->x_lastontime = ontime;

			/* output values from right to left */
		outlet_float(x->x_outgestalt, gestalt);
	}
}

static t_class *gestalt_class;

static void *gestalt_new(t_floatarg f)
{
    t_gestalt *x = (t_gestalt *)pd_new(gestalt_class);
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft2"));
	x->x_outgestalt = outlet_new(&x->x_ob, gensym("float"));

	x->x_lastontime = clock_getlogicaltime();

	x->x_reftime = f;
	if(x->x_reftime < 1) x->x_reftime = 1;

    return (void *)x;
}

#ifndef MAXLIB
void gestalt_setup(void)
{
    gestalt_class = class_new(gensym("gestalt"), (t_newmethod)gestalt_new,
    	0, sizeof(t_gestalt), 0, A_DEFFLOAT, 0);
    class_addfloat(gestalt_class, gestalt_float);
	class_addmethod(gestalt_class, (t_method)gestalt_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(gestalt_class, (t_method)gestalt_ft2, gensym("ft2"), A_FLOAT, 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_gestalt_setup(void)
{
    gestalt_class = class_new(gensym("maxlib_gestalt"), (t_newmethod)gestalt_new,
    	0, sizeof(t_gestalt), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)gestalt_new, gensym("gestalt"), A_DEFFLOAT, 0);
    class_addfloat(gestalt_class, gestalt_float);
	class_addmethod(gestalt_class, (t_method)gestalt_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(gestalt_class, (t_method)gestalt_ft2, gensym("ft2"), A_FLOAT, 0);
    class_sethelpsymbol(gestalt_class, gensym("maxlib/gestalt-help.pd"));
}
#endif

