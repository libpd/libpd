/* --------------------------- pong  ------------------------------------------ */
/*                                                                              */
/* A virtual bouncing ball.                                                     */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Based on pong (for Max) version 1.5 written by Richard Dudas.                */
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

static char *version = "pong v0.1, ported by Olaf Matthes <olaf.matthes@gmx.de>\n"
                       "           written for Max by Richard Dudas";

typedef struct pong
{
	t_object x_obj;		
	t_outlet *p_bounceout;
	t_outlet *p_handout;
	t_outlet *p_velout;
	t_outlet *p_heightout;
	t_clock  *p_klok;

	t_int    p_ms;  			// ms count
	t_float  p_time;			// current time div by warp
	t_float  p_timegrain;		// timegrain in seconds
	t_float  p_timegrainin;		// timegrain in ms
	t_float  p_warp;			// timewarp in ms
	
	t_float  p_dinit;			// init distance
	t_float  p_vinit;			// init velocity
	t_float  p_ainit;			// base acceleration def -100
	t_float  p_damping;			// realtime multiplicative damping
	t_float  p_dhand;			// virtual hand distance
	t_float  p_force;			// force of hand 1.0 = no force
 
	t_float  p_accel;			// current accel value
	t_float  p_vi;				// current velocity
	t_float  p_di;				// current distance
	t_float  p_dt;				// distance out
	t_float  p_dtprev;			// previous distance out for accel computation
	t_float  p_vt;				// velocity out
	t_int    p_prevchg;			// for logical transition
} t_pong;

/* ---------------------------------------------------------- */
/* ---- this stuff is mainly for the timer, on off etc... --- */
/* ---------------------------------------------------------- */

static void pong_reset(t_pong *x)
{
	x->p_di = x->p_dinit;
	x->p_dt = x->p_dinit;			// added
	x->p_dtprev = x->p_dinit;		// added
	x->p_vi = x->p_vinit;
	x->p_vt = x->p_vinit;			// added
	x->p_ms = 0;
	x->p_time = 0.;
	x->p_ainit = -100.;				// added, but currently disabled
	x->p_accel = -100.;				// reactivated (?)
	x->p_damping = 1.; 				// i.e. no initial damping
	x->p_prevchg = 0;				// added
	
/*	x->p_ms = 0;
	x->p_time = 0.;
	x->p_timegrain = 0.05;  // if ms grain = 50
	x->p_timegrainin = 50;
	
	x->p_vinit = 0.;
	x->p_dinit = 100.;	
	x->p_ainit = -100.;
	x->p_damping = 1.; 	// i.e. no initial damping 
	x->p_dhand = 100.;
	x->p_force = 1.;  // i.e. hand does nothing initially
	
	x->p_accel = -100.;
	x->p_vi = 0.;
	x->p_di = 100.;
	
	x->p_dt = 100.;			// changed from 0 to 100 
	x->p_dtprev = 100.;		// changed from 0 to 100 
	x->p_vt = 0.;
	x->p_prevchg = 0;
*/
}

/* ---------------------------------------------------------- */

static void pong_timein(t_pong *x, t_floatarg n)
{
	int thischg;
	
	x->p_time = n / x->p_warp;
	
	x->p_dt = ((x->p_accel * (x->p_time*x->p_time)) + (x->p_time * x->p_vi) + x->p_di);
	x->p_vt = ((x->p_dt - x->p_dtprev) / x->p_timegrain);
	
	if (x->p_dt < 0.)
	{
		x->p_dt *= -1.;
		
		x->p_di = 0.;
		x->p_vi = x->p_vt * (-1. * x->p_damping);  // -1 will eventually be a damping variable
		//post("vel at bounce %f", x->p_vi); 
		outlet_bang(x->p_bounceout);
		x->p_ms = 0;
		//x->p_dtprev= 0.;
	}
	//else
		x->p_dtprev = x->p_dt;
	
	/* ---------------------------------- virtual hand below ------------ */
	
	if (x->p_dt > x->p_dhand)  // presuming the hand is initially equal to the dinit
		thischg = 1;
	else
		thischg = 0;
		
	if (thischg != x->p_prevchg)
	{
		x->p_ms = 0;
		x->p_vi = x->p_vt;
		x->p_di = x->p_dhand;
		
		if (thischg == 0)
		{
			x->p_accel = -100.;   					// x->p_ainit in lieu of -100.
			outlet_float(x->p_handout, 0);
		}
		else
		{
			x->p_accel = (x->p_force * -100.);     // x->p_ainit in lieu of -100.
			outlet_float(x->p_handout, 1);
		}
	}
	
	x->p_prevchg = thischg;
	outlet_float(x->p_velout, x->p_vt);
	outlet_float(x->p_heightout, x->p_dt);
}

static void pong_onoff(t_pong *x, t_floatarg n)
{
	if (n != 0)
		clock_delay(x->p_klok, 0);
	else
		clock_unset(x->p_klok);
}

/* ---------------------------------------------------------- */

static void pong_bang(t_pong *x)
{
	x->p_ms = 0;
	clock_delay(x->p_klok, 0);
}

static void pong_stop(t_pong *x)
{
	clock_unset(x->p_klok);
}

/* ---------------------------------------------------------- */

static void pong_tick(t_pong *x)
{
	clock_delay(x->p_klok, (t_int)x->p_timegrainin);
	pong_timein(x, x->p_ms);
	//outlet_float(x->p_heightout, (float)x->p_ms);
	x->p_ms = x->p_ms + x->p_timegrainin;
}

/* ---------------------------------------------------------- */

static void pong_tgrain(t_pong *x, t_floatarg n)
{
	x->p_timegrain = n / x->p_warp;
	x->p_timegrainin = n;
	post("timegrain %f", x->p_timegrain);
}

/* ---------------------------------------------------------- */

static void pong_warpin(t_pong *x, t_floatarg n)
{
	x->p_warp = n;
	x->p_timegrain = x->p_timegrainin / x->p_warp;
	post("timewarp %f ms = one sec", x->p_warp);
}

/* ---------------------------------------------------------- */
/* ----- these are to receive and store the init values ----- */
/* ---------------------------------------------------------- */

static void pong_initdist(t_pong *x, t_floatarg n)
{
	x->p_dinit = n;
}

static void pong_initvel(t_pong *x, t_floatarg n)
{
	x->p_vinit = n;
}

static void pong_damp(t_pong *x, t_floatarg n)
{
	x->p_damping = n;
}

/* ---------------------------------------------------------- */

static void pong_baseacc(t_pong *x, t_floatarg n)
{
	//post ("baseaccel currently disabled", 0);
	 x->p_ainit = n; 
	 x->p_accel = x->p_ainit; 
}

/* ---------------------------------------------------------- */

static void pong_hand(t_pong *x, t_floatarg n)
{
	x->p_dhand = n;
}

static void pong_force(t_pong *x, t_floatarg n)
{
	x->p_force = n;
}

/* ---------------------------------------------------------- */


static t_class *pong_class;

static void *pong_new(t_floatarg n)
{
    t_pong *x = (t_pong *)pd_new(pong_class);

	x->p_klok = clock_new(x, (t_method)pong_tick);
	
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("dist"));	// distance
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("velo"));	// velocity
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("damp"));	// damping
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("force"));	// hand force 1.0 = no force
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("hand"));	// virtual hand (distance)

	
	if (n > 0)
		x->p_warp = n;
	else
		x->p_warp = 1000.;
	
	x->p_ms = 0;
	x->p_time = 0.;
	x->p_timegrain = 0.05;  // if ms grain = 50
	x->p_timegrainin = 50.0;
	
	x->p_vinit = 0.;
	x->p_dinit = 100.;	
	x->p_ainit = -100.;
	x->p_damping = 1.; 	// i.e. no initial damping
	x->p_dhand = 100.;
	x->p_force = 1.;  	// i.e. hand does nothing initially
	
	x->p_accel = -100.;
	x->p_vi = 0.;
	x->p_di = 100.;

	x->p_dt = 100.;			// changed from 0 to 100
	x->p_dtprev = 100.;		// changed from 0 to 100
	x->p_vt = 0.;
	x->p_prevchg = 0;
	
	x->p_bounceout = outlet_new(&x->x_obj, gensym("bang"));
	x->p_handout = outlet_new(&x->x_obj, gensym("float"));
	x->p_velout = outlet_new(&x->x_obj, gensym("float"));
	x->p_heightout = outlet_new(&x->x_obj, gensym("float"));

	return (x);
}

#ifndef MAXLIB
void pong_setup(void)
{
    pong_class = class_new(gensym("pong"), (t_newmethod)pong_new,
    	0, sizeof(t_pong), 0, A_DEFFLOAT, 0);
#else
void maxlib_pong_setup(void)
{
    pong_class = class_new(gensym("maxlib_pong"), (t_newmethod)pong_new,
    	0, sizeof(t_pong), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)pong_new, gensym("pong"), A_DEFFLOAT, 0);
#endif
		/* method handlers for inlets */
	class_addmethod(pong_class, (t_method)pong_initdist, gensym("dist"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_initvel, gensym("velo"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_damp, gensym("damp"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_force, gensym("force"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_hand, gensym("hand"), A_FLOAT, 0);
		/* method handlers for other messages to first inlet */
	class_addmethod(pong_class, (t_method)pong_tgrain, gensym("timegrain"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_warpin, gensym("timewarp"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_baseacc, gensym("baseaccel"), A_FLOAT, 0);
	class_addmethod(pong_class, (t_method)pong_reset, gensym("reset"), 0);
	class_addmethod(pong_class, (t_method)pong_stop, gensym("stop"), 0);

    class_addfloat(pong_class, pong_onoff);
	class_addbang(pong_class, pong_bang);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(pong_class, gensym("maxlib/pong-help.pd"));
#endif
}
