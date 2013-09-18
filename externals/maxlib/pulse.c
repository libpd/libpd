/* --------------------------- pong  ------------------------------------------ */
/*                                                                              */
/* A more accurate replacement for the tempo object.                            */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Based on pulse for Max written by James McCartney.                           */
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

/* pulse.c ---- a more accurate replacement for the tempo object   */
/*				updated for CW 68K / PPC summer 96 -RD             */
/*              written for Max by James McCartney                 */

#include "m_pd.h"
#include <stdio.h>

static char *version = "pulse v0.1b, written by James McCartney for Max <james@clyde.as.utexas.edu>\n"
                       "             ported to Pd by Olaf Matthes <olaf.matthes@gmx.de>";

/* Pulse object data structure */
typedef struct pulse
{
	t_object p_ob;		
	t_clock *p_clock;
	t_outlet *p_out1;			/* outlet */
	t_outlet *p_out2;			/* outlet */
	t_int p_onoff, p_changenumer, p_changedenom;
	t_int p_tempo, p_durnumer, p_durdenom, p_maxbeats, p_count;
	double p_starttime, p_endtime, p_startremain, p_endremain, p_mspbquotient;
	t_int p_newdurnumer, p_newdurdenom;
	t_int p_mspbnumer, p_mspbdenom, p_mspbremainder;
} Pulse;

static t_class *pulse_class;

static void durcalc(Pulse *x)
{
	/* recalc duration */
	x->p_mspbnumer = 240000 * x->p_durnumer;
	if (x->p_tempo * x->p_durdenom != 0) /* bug fix by Frank Barknecht */
		x->p_mspbdenom = x->p_tempo * x->p_durdenom;
	x->p_mspbquotient = x->p_mspbnumer / x->p_mspbdenom;
	x->p_mspbremainder = x->p_mspbnumer % x->p_mspbdenom;
	if (x->p_mspbquotient < 5) {
		x->p_mspbquotient = 5;
		x->p_mspbremainder = 0;
	}
}

static void pulse_onoff(Pulse *x, t_floatarg f)
{
	int i = (int)f;
	if (i && !x->p_onoff) {
		x->p_onoff = 1;
		x->p_count = 0;
		outlet_float(x->p_out1, x->p_count);
		if (x->p_changedenom) {
			x->p_durdenom = x->p_newdurdenom;
			x->p_changedenom = 0;
		} 
		if (x->p_changenumer) {
			x->p_durnumer = x->p_newdurnumer;
			x->p_changenumer = 0;
		}
		durcalc(x);
		x->p_startremain = 0;
		x->p_endremain = x->p_mspbremainder;
		x->p_starttime = clock_getlogicaltime();
		x->p_endtime = x->p_starttime + x->p_mspbquotient;
		// clock_set(x->p_clock, x->p_endtime);
		clock_delay(x->p_clock, x->p_mspbquotient);
	} else if (i==0 && x->p_onoff) {
		x->p_onoff = 0;
		clock_unset(x->p_clock);
	}
}

static void pulse_bang(Pulse *x)
{
	if (!x->p_onoff) {
		x->p_onoff = 1;
		x->p_count = 0;
		outlet_float(x->p_out1, x->p_count);
		if (x->p_changedenom) {
			x->p_durdenom = x->p_newdurdenom;
			x->p_changedenom = 0;
		} 
		if (x->p_changenumer) {
			x->p_durnumer = x->p_newdurnumer;
			x->p_changenumer = 0;
		}
		durcalc(x);
		x->p_startremain = 0;
		x->p_endremain = x->p_mspbremainder;
		x->p_starttime = clock_getlogicaltime();
		x->p_endtime = x->p_starttime + x->p_mspbquotient;
		clock_set(x->p_clock, x->p_endtime);
	} else {
		x->p_onoff = 0;
		clock_unset(x->p_clock);
	}
}

/* clock tick routine */
static void pulse_tick(Pulse *x)
{
	x->p_count ++;
	if ((x->p_maxbeats > 0) && (x->p_count >= x->p_maxbeats)) {	/* turn off time */
		x->p_onoff = 0;
		outlet_bang(x->p_out2);
	} else {
		outlet_float(x->p_out1, x->p_count);
		x->p_startremain = x->p_endremain;	/* save in case we have to re do it */
		if (x->p_changenumer || x->p_changedenom) {		/* duration changed */
			if (x->p_changedenom) {
				/* this statement may cause a slight drift of (1/(tempo*denom) msecs) */
				x->p_startremain = (x->p_startremain * x->p_newdurdenom + (x->p_durdenom>>1))
					/x->p_durdenom;
				x->p_durdenom = x->p_newdurdenom;
				x->p_changedenom = 0;
			} 
			if (x->p_changenumer) {
				x->p_durnumer = x->p_newdurnumer;
				x->p_changenumer = 0;
			}
			durcalc(x);
		}
		x->p_endremain = x->p_startremain + x->p_mspbremainder;
		x->p_starttime = x->p_endtime;
		x->p_endtime = x->p_starttime + x->p_mspbquotient;
		if (x->p_endremain >= x->p_mspbdenom) {		
			x->p_endremain -= x->p_mspbdenom;
			x->p_endtime ++;
		}
		// clock_set(x->p_clock, x->p_endtime);
		clock_delay(x->p_clock, x->p_mspbquotient); 
	}
}

/* deal with tempo change */
static void pulse_tempo(Pulse *x, t_floatarg t)
{
	double time, msecdur, tickdur, fracremain;
	t_int fracnumer, fracquotient, oldtempo;
	oldtempo = x->p_tempo;
	x->p_tempo = (t<5) ? 5 : ((t>500) ? 500 : t);
	if (x->p_onoff) {
		/* calculate fraction of the beat we have done */
		time = clock_getlogicaltime();
		if (time != x->p_endtime) {	
			/* if pulse_tempo is called as a result of a call from pulse_tick 
				(call chain from outlet_float())
				then this stuff doesn't need to be done (time will == x->p_endtime) 
			*/
			msecdur = time - x->p_starttime;
			tickdur = msecdur * x->p_mspbdenom - x->p_startremain;
			fracnumer = (t_int)(x->p_mspbnumer - tickdur);
		
			durcalc(x);
			
			/* calculate end time */
			fracquotient = fracnumer / x->p_mspbdenom;
			fracremain = fracnumer % x->p_mspbdenom;
	
			x->p_endtime = time + fracquotient;
			x->p_endremain = fracremain;
			
			/* recalculate starttime so future tempo changes work */
			x->p_starttime = x->p_endtime - x->p_mspbquotient;
			x->p_startremain = x->p_mspbdenom - x->p_mspbremainder + fracremain;
			if (x->p_mspbremainder > fracremain) {
				x->p_startremain = x->p_mspbdenom - x->p_mspbremainder + fracremain;
				x->p_starttime --;
			} else {
				x->p_startremain = fracremain - x->p_mspbremainder;
			}
			clock_unset(x->p_clock);
			clock_set(x->p_clock, x->p_endtime);
			// clock_delay(x->p_clock, fracquotient);
		}
	}
}

static void pulse_numer(Pulse *x, t_floatarg n)
{
	int i = (t_int)n;
	if(i >= 0)
	{
		if (x->p_onoff) {
			if (x->p_durnumer != i) {
				x->p_changenumer = 1;
				x->p_newdurnumer = i;
			}
		} else {
			x->p_durnumer = i;
		}
	}
}

static void pulse_denom(Pulse *x, t_floatarg n)
{
	int i = (t_int)n;
	if(i >= 0)
	{
		if (x->p_onoff) {
			if (x->p_durdenom != i) {
				x->p_changedenom = 1;
				x->p_newdurdenom = i;
			}
		} else {
			x->p_durdenom = i;
		}
	}
}

static void pulse_beat(Pulse *x, t_floatarg n)
{
	int i = (t_int)n;
	if(i >= 0)
	{
		x->p_maxbeats = i;
	}
}


static void pulse_free(Pulse *x)
{
	clock_free(x->p_clock);
}


/* function run to create a new instance of the Pulse class */
static void *pulse_new(t_floatarg t, t_floatarg n, t_floatarg d, t_floatarg b)
{
	Pulse *x;
	
	x = (Pulse *)pd_new(pulse_class);	/* allocates memory and sticks in an inlet */
		
	inlet_new(&x->p_ob, &x->p_ob.ob_pd, gensym("float"), gensym("tempo"));
	inlet_new(&x->p_ob, &x->p_ob.ob_pd, gensym("float"), gensym("numer"));
	inlet_new(&x->p_ob, &x->p_ob.ob_pd, gensym("float"), gensym("denom"));
	inlet_new(&x->p_ob, &x->p_ob.ob_pd, gensym("float"), gensym("beat"));
	x->p_out1 = outlet_new(&x->p_ob, gensym("float"));
	x->p_out2 = outlet_new(&x->p_ob, gensym("float"));
	x->p_clock = clock_new(x, (t_method)pulse_tick);
	x->p_tempo = (t==0) ? 120 : ((t<5) ? 5 : ((t>500) ? 500 : t));
	x->p_durnumer = (n<=0) ? 1 : n;
	x->p_durdenom = (d<=0) ? 4 : d;
	x->p_maxbeats = (b<=0) ? 0 : b;
	x->p_changenumer = 0;
	x->p_changedenom = 0;
	x->p_onoff = 0;

	return (x);					/* always return a copy of the created object */
}

#ifndef MAXLIB
void pulse_setup(void)
{
    pulse_class = class_new(gensym("pulse"), (t_newmethod)pulse_new,
    	(t_method)pulse_free, sizeof(Pulse), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(pulse_class, (t_method)pulse_beat, gensym("beat"), A_FLOAT, 0);
	class_addmethod(pulse_class, (t_method)pulse_denom, gensym("denom"), A_FLOAT, 0);
    class_addmethod(pulse_class, (t_method)pulse_numer, gensym("numer"), A_FLOAT, 0);
    class_addmethod(pulse_class, (t_method)pulse_tempo, gensym("tempo"), A_FLOAT, 0);
    class_addfloat(pulse_class, pulse_onoff);
    class_addbang(pulse_class, pulse_bang);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_pulse_setup(void)
{
    pulse_class = class_new(gensym("maxlib_pulse"), (t_newmethod)pulse_new,
    	(t_method)pulse_free, sizeof(Pulse), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)pulse_new, gensym("pulse"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(pulse_class, (t_method)pulse_beat, gensym("beat"), A_FLOAT, 0);
	class_addmethod(pulse_class, (t_method)pulse_denom, gensym("denom"), A_FLOAT, 0);
    class_addmethod(pulse_class, (t_method)pulse_numer, gensym("numer"), A_FLOAT, 0);
    class_addmethod(pulse_class, (t_method)pulse_tempo, gensym("tempo"), A_FLOAT, 0);
    class_addfloat(pulse_class, pulse_onoff);
    class_addbang(pulse_class, pulse_bang);
    class_sethelpsymbol(pulse_class, gensym("maxlib/pulse-help.pd"));
}
#endif
