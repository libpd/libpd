/* --------------------------  step  ------------------------------------------ */
/*                                                                              */
/* Step to a new value in N milliseconds (similar to line).                     */
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

/* -------------------------- step ------------------------------ */
static char *version = "step v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

static t_class *step_class;

typedef struct _step
{
    t_object x_obj;
    t_clock *x_clock;
    double x_targettime;
    t_float x_targetval;
    double x_prevtime;
    t_float x_setval;
    int x_gotinlet;
    t_float x_grain;        /* time interval for output */
	t_float x_step;			/* step size for output */
	t_float x_steptime;     /* length for one step */
	t_int   x_stepcall;
    double x_1overtimediff;
    double x_in1val;
} t_step;

static void step_tick(t_step *x)
{
	t_float outvalue;
    double timenow = clock_getsystime();
    double msectogo = - clock_gettimesince(x->x_targettime);
    if (msectogo < 1E-9)
    {
    	outlet_float(x->x_obj.ob_outlet, x->x_targetval);
    }
    else
    {
		if(x->x_setval < x->x_targetval)
		{		/* count upwards */
			outvalue = x->x_setval + x->x_stepcall * x->x_step;
		}
		else
		{		/* count downwards */
			outvalue = x->x_setval - x->x_stepcall * x->x_step;
		}
    	outlet_float(x->x_obj.ob_outlet, outvalue);
    	clock_delay(x->x_clock, (x->x_steptime > msectogo ? msectogo : x->x_steptime));
    }
	x->x_stepcall++;
}

static void step_float(t_step *x, t_float f)
{
    double timenow = clock_getsystime();
    if (x->x_gotinlet && x->x_in1val > 0 && x->x_step != 0 && f != x->x_setval)
    {
    	if (timenow > x->x_targettime) x->x_setval = x->x_targetval;
    	else x->x_setval = x->x_setval + x->x_1overtimediff *
    	    (timenow - x->x_prevtime)
    	    * (x->x_targetval - x->x_setval);
    	x->x_prevtime = timenow;
    	x->x_targetval = f;		/* where to end */
		x->x_stepcall = 0;
			/* how long does it take ? */
    	x->x_targettime = clock_getsystimeafter(x->x_in1val);
		if(x->x_setval < x->x_targetval)
		{
			x->x_steptime = x->x_in1val / (int)((x->x_targetval - x->x_setval) / x->x_step);
		}
		else
		{
			x->x_steptime = x->x_in1val / (int)((x->x_setval - x->x_targetval) / x->x_step);
		}
		// post("steptime %g", x->x_steptime);
    	step_tick(x);
    	x->x_gotinlet = 0;
    	x->x_1overtimediff = 1./ (x->x_targettime - timenow);
			/* call tick function */
    	clock_delay(x->x_clock, x->x_steptime);
    
    }
    else
    {
    	clock_unset(x->x_clock);
    	x->x_targetval = x->x_setval = f;
    	outlet_float(x->x_obj.ob_outlet, f);
    }
    x->x_gotinlet = 0;
}

static void step_ft1(t_step *x, t_floatarg g)
{
    x->x_in1val = g;
    x->x_gotinlet = 1;
}

static void step_ft2(t_step *x, t_floatarg g)
{
	if (g <= 0) g = 1;
	x->x_step = g;
    x->x_gotinlet = 1;
}

static void step_stop(t_step *x)
{
    x->x_targetval = x->x_setval;
    clock_unset(x->x_clock);
}

static void step_free(t_step *x)
{
    clock_free(x->x_clock);
}

static void *step_new(t_floatarg f, t_floatarg step, t_floatarg grain)
{
    t_step *x = (t_step *)pd_new(step_class);
    x->x_targetval = x->x_setval = f;
    x->x_gotinlet = 0;
    x->x_1overtimediff = 1;
    x->x_clock = clock_new(x, (t_method)step_tick);
    x->x_targettime = x->x_prevtime = clock_getsystime();
    if (grain <= 0) grain = 20;
    x->x_grain = grain;
    if (step <= 0) step = 1;
    x->x_step = step;
    outlet_new(&x->x_obj, gensym("float"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft2"));
    return (x);
}

#ifndef MAXLIB
void step_setup(void)
{
    step_class = class_new(gensym("step"), (t_newmethod)step_new,
    	(t_method)step_free, sizeof(t_step), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(step_class, (t_method)step_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(step_class, (t_method)step_ft2, gensym("ft2"), A_FLOAT, 0);
    class_addmethod(step_class, (t_method)step_stop, gensym("stop"), 0);
    class_addfloat(step_class, (t_method)step_float);
	
    logpost(NULL, 4, version);
}
#else
void maxlib_step_setup(void)
{
    step_class = class_new(gensym("maxlib_step"), (t_newmethod)step_new,
    	(t_method)step_free, sizeof(t_step), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)step_new, gensym("step"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(step_class, (t_method)step_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(step_class, (t_method)step_ft2, gensym("ft2"), A_FLOAT, 0);
    class_addmethod(step_class, (t_method)step_stop, gensym("stop"), 0);
    class_addfloat(step_class, (t_method)step_float);
	class_sethelpsymbol(step_class, gensym("maxlib/step-help.pd"));
}
#endif
