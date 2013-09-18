/* ------------------------- velocity ----------------------------------------- */
/*                                                                              */
/* Get velocity of input in digits per second.                                  */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Originally written for Max by Trond Lossius.                                 */
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

//	velocity.c: (1000*(x[n]-x[n-1]))/Æt
//	(C) Trond Lossius/BEK 2000
//	Last revision: 4/7 2000
//
//	Input:
//		float
//		int (converted to float)
//		set (set new value but no output)
//	Argument (optional, defaults to 0):
//		Initial stored value
//	Output:
//		float: Velocity as change per second (not ms)

#include "m_pd.h"

static char *version = "velocity v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct velocity
{
  t_object x_ob;
  t_outlet *x_out;
  t_float  x_xn1;               /* input (floats) */
  t_float  x_xn;
  double   x_lasttime;
} t_velocity;

static void velocity_bang(t_velocity *x)
{
	double thistime;
	t_float vel;
	
	thistime = clock_getlogicaltime();
	vel = (1000 * (x->x_xn - x->x_xn1) ) / (clock_gettimesince(x->x_lasttime));
	x->x_lasttime = thistime;
	outlet_float(x->x_out, vel);
}

static void velocity_float(t_velocity *x, t_floatarg f)
{
	x->x_xn1 = x->x_xn;
	x->x_xn = f;
	velocity_bang(x);
}

static void velocity_free(t_velocity *x)
{
	// nothing to do
}

static t_class *velocity_class;

static void *velocity_new(t_floatarg f)
{
    t_velocity *x = (t_velocity *)pd_new(velocity_class);
	x->x_out = outlet_new(&x->x_ob, gensym("float"));
	x->x_lasttime = clock_getlogicaltime();

    return (void *)x;
}

#ifndef MAXLIB
void velocity_setup(void)
{
    velocity_class = class_new(gensym("velocity"), (t_newmethod)velocity_new,
    	(t_method)velocity_free, sizeof(t_velocity), 0, A_DEFFLOAT, 0);
#else
void maxlib_velocity_setup(void)
{
    velocity_class = class_new(gensym("maxlib_velocity"), (t_newmethod)velocity_new,
    	(t_method)velocity_free, sizeof(t_velocity), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)velocity_new, gensym("velocity"), A_DEFFLOAT, 0);
#endif
    class_addfloat(velocity_class, velocity_float);
	class_addbang(velocity_class, velocity_bang);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(velocity_class, gensym("maxlib/velocity-help.pd"));
#endif
}

