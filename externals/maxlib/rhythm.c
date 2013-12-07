/* --------------------------- rhythm  ---------------------------------------- */
/*                                                                              */
/* Detect the beats per minute of a MIDI stream.                                */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Based on code written by Robert Rowe.                                        */
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
#include <math.h>
#ifndef _WIN32
#include <stdlib.h>
#endif

#define ALPHA 10
#define ADAPT_ARRAY_SIZE  1000

#ifndef M_PI
#define M_PI 3.14159265358979
#endif
#ifndef TWO_PI
#define TWO_PI 2.0*M_PI
#endif

static char *version = "rhythm v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct rhythm
{
  t_object x_ob;
  t_clock  *x_tick;
  t_outlet *x_out_bpm;              /* beats per minute */
  t_outlet *x_out_period;           /* beats in milliseconds */
  t_outlet *x_out_pulse;
  t_int    x_print;                 /* switch printing to console window on / off */
  t_int    x_ticking;               /* indicates if clock is ticking or not */

  t_int    x_model;					/* algorhythm to use: 0 - Large & Kolen, 1 - Toiviainen */
  t_float  x_long_term[ADAPT_ARRAY_SIZE];
  t_float  x_short_term[ADAPT_ARRAY_SIZE];
  t_float  x_phi_at_pulse;			/* phase at latest pulse */
  t_float  x_phiVel_at_pulse;		/* phase velocity */

  t_float  x_adapt;
  t_float  x_errFunc;				/* error function */
  t_float  x_etaLong;				/* strength of long-term adaptation */
  t_float  x_etaShort;				/* strength of short-term adaptation */
  t_float  x_gamma;					/* gain parameter */
  double   x_lastIoi;				/* last inter-onset interval */
  double   x_lastPulseTime;			/* time of last pulse */
  t_float  x_output;				/* current output value of the oscillator */
  t_float  x_phi;					/* phase */
  double   x_expected;				/* estimated time of arrival */
  t_float  x_period;
  t_float  x_periodStrength;
  t_float  x_phaseStrength;
  double   x_startTime;

  t_int    x_pitch;
  t_int    x_velo;
	/* helpers needed to do the time calculations */
  double   x_last_input;
} t_rhythm;

/* --------------- rhythm  stuff ------------------------------------------------ */
	/* bang at the rhythm's pulse */
static void rhythm_tick(t_rhythm *x)
{
	outlet_bang(x->x_out_pulse);
	clock_delay(x->x_tick, x->x_period);
}

static t_float rhythm_get_adapt_long(t_rhythm *x, t_float arg)
{
	int address;
	if (arg > 1.0)
		address = ADAPT_ARRAY_SIZE - 1;
	else if (arg < -1.0)
		address = ADAPT_ARRAY_SIZE - 1;
	else
		address = abs((int)(arg*1000.0));
	return x->x_long_term[address];
}

static t_float rhythm_get_adapt_short(t_rhythm *x, t_float arg)
{
	int address;
	if (arg > 1.0)
		address = ADAPT_ARRAY_SIZE - 1;
	else if (arg < -1.0)
		address = ADAPT_ARRAY_SIZE - 1;
	else
		address = abs((int)(arg*1000.0));
	return x->x_short_term[address];
}


	/* Large & Kolen adaptation model */
static void rhythm_large(t_rhythm *x, t_int pulse, double time)
{
	while (time > (x->x_expected+(x->x_period/2)))		// move the expectation point
		x->x_expected += x->x_period;						// to be within one period of onset
	x->x_phi = (t_float)(time - x->x_expected) / x->x_period;		// calculate phi

	if (pulse) {								// if this was an onset
		x->x_adapt     = x->x_gamma * (cos(TWO_PI*x->x_phi)-1.0);
		x->x_adapt     = 1.0 / cosh(x->x_adapt);
		x->x_adapt    *= x->x_adapt;
		x->x_adapt    *= sin(TWO_PI*x->x_phi);
		x->x_adapt    *= (x->x_period / TWO_PI);
		x->x_period   += (x->x_periodStrength*x->x_adapt);		// update period
		x->x_expected += (x->x_phaseStrength *x->x_adapt);		// and phase
		x->x_phi       = (t_float)(time - x->x_expected) / x->x_period;
	}

	x->x_output = 1+tanh(x->x_gamma*(cos(TWO_PI*x->x_phi)-1.0)); // oscillator output
}
	/* Toiviainen adaptation model */
static void rhythm_toiviainen(t_rhythm *x, t_int pulse, double time)
{
	t_float deltaTime, varPhi, adaptLong, adaptShort;

	/* if just starting, initialize phi */
	if(x->x_lastPulseTime < 0)
	{
		x->x_phi = x->x_phi_at_pulse + x->x_phiVel_at_pulse * ((t_float)(time-x->x_startTime) / 1000.0);
	}
	else
	{
		deltaTime  = time - x->x_lastPulseTime;
		varPhi     = (deltaTime/1000.0) * x->x_phiVel_at_pulse;
		adaptLong  = rhythm_get_adapt_long(x, varPhi);	// get long adaptation from table
		adaptShort = rhythm_get_adapt_short(x, varPhi);	// get short adaptation from table
		x->x_phi = x->x_phi_at_pulse + varPhi + x->x_errFunc * (x->x_etaLong*adaptLong + x->x_etaShort*adaptShort);
		if (pulse)									// change tempo if on pulse
			x->x_phiVel_at_pulse = x->x_phiVel_at_pulse * (1 + x->x_etaLong * x->x_errFunc * adaptShort);
	}

	if (pulse) {
		x->x_output        = 1+tanh(x->x_gamma*(cos(TWO_PI*x->x_phi)-1.0));
		x->x_errFunc       = x->x_output * (x->x_output - 2.0) * sin(TWO_PI * x->x_phi);
		x->x_phi_at_pulse  = x->x_phi;
	}

	x->x_period = 1000.0 / x->x_phiVel_at_pulse;		// update period
}

static void rhythm_move(t_rhythm *x, t_int pulse, double time)
{
	switch (x->x_model)	/* choose adaptation model */
	{
		case 0:		
			rhythm_large(x, pulse, time);
			break;

		case 1:
			rhythm_toiviainen(x, pulse, time);
			break;
	}
	
	if(x->x_ticking == 0)
	{
		x->x_ticking = 1;	/* prevent us from further calls */
		clock_delay(x->x_tick, 0);	/* start pulse bangs */
	}
}

	/* main processing function */
static void rhythm_float(t_rhythm *x, t_floatarg f)
{
	t_int velo = x->x_velo;
	double time = clock_gettimesince(x->x_last_input);
	x->x_pitch = (t_int)f;

	if(velo != 0)	/* note-on received */
	{
		if (x->x_startTime == 0) {
			x->x_startTime = time;
			return;
		}
		
		if (x->x_period < 2.0) {
			x->x_period = (t_float)(time - x->x_startTime);
			x->x_phiVel_at_pulse = 1000.0 / x->x_period;
		}

		rhythm_move(x, 1, time);

		if (x->x_lastPulseTime >= 0)
		{
			x->x_lastIoi = time - x->x_lastPulseTime;
		}
		x->x_lastPulseTime = time; 
		x->x_last_input = clock_getlogicaltime();

		outlet_float(x->x_out_period, x->x_period);
		outlet_float(x->x_out_bpm, 60000.0/x->x_period);
	}
	return;
}
	/* get velocity */
static void rhythm_ft1(t_rhythm *x, t_floatarg f)
{
	x->x_velo = (t_int)f;
}

	/* toggle printing on/off (not used right now!) */
static void rhythm_print(t_rhythm *x)
{
	if(x->x_print)x->x_print = 0;
	else x->x_print = 1;
}
	/* initialise array for Toiviainen adaptation model */
static void rhythm_calculate_adaptations(t_rhythm *x)
{
	int i;
	t_float f;

	for(i = 0; i < ADAPT_ARRAY_SIZE; i++)
	{
		f = (t_float)i/(t_float)ADAPT_ARRAY_SIZE;
		x->x_long_term[i] = f+(ALPHA*f*f/2.0+2.0*f+3.0/ALPHA)*exp(-ALPHA*f)-3.0/ALPHA;
		x->x_short_term[i] = 1.0-(ALPHA*ALPHA*f*f/2.0+ALPHA*f+1.0)*exp(-ALPHA*f);
	}
}

static void rhythm_reset(t_rhythm *x)
{
	if(x->x_ticking)clock_unset(x->x_tick);
	x->x_ticking = 0;

	x->x_gamma          = 1.0;	/* default value for gain parameter */
	x->x_phi            = 0.0;
	x->x_output         = 1+tanh(x->x_gamma*(cos(TWO_PI*x->x_phi)-1.0));
	x->x_expected       = 0;
	x->x_lastIoi		= 0;
	x->x_lastPulseTime  = -1;
	x->x_period         = 1.0;
	x->x_periodStrength	= 0.2;
	x->x_phaseStrength	= 0.2;

	x->x_errFunc        = 0.0;
	x->x_etaLong		= 0.2;
	x->x_etaShort		= 0.2;
	x->x_phi_at_pulse   = 0.0;
	x->x_phiVel_at_pulse = 0.9;
	x->x_startTime		= 0;

	rhythm_calculate_adaptations(x);
}

static void rhythm_model(t_rhythm *x, t_floatarg f)
{
	if(f == 1)
	{
		x->x_model = 1;		/* Toiviainen model */
		rhythm_reset(x);
		post("rhythm: using \"Toiviainen\" adaptation model");
	}
	else
	{
		x->x_model = 0;		/* Large and Kolen model */
		rhythm_reset(x);
		post("rhythm: using \"Large and Kolen\" adaptation model");
	}
}

static t_class *rhythm_class;

static void rhythm_free(t_rhythm *x)
{
	clock_free(x->x_tick);
}

static void *rhythm_new(t_floatarg f)
{
    t_rhythm *x = (t_rhythm *)pd_new(rhythm_class);
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	x->x_out_bpm = outlet_new(&x->x_ob, gensym("float"));
	x->x_out_period = outlet_new(&x->x_ob, gensym("float"));
	x->x_out_pulse = outlet_new(&x->x_ob, gensym("bang"));
	x->x_tick = clock_new(x, (t_method)rhythm_tick);

	rhythm_reset(x);

	if(f == 1)
	{
		x->x_model = 1;		/* Toiviainen model */
		post("rhythm: using \"Toiviainen\" adaptation model");
	}
	else
	{
		x->x_model = 0;		/* Large and Kolen model */
		post("rhythm: using \"Large and Kolen\" adaptation model");
	}

    return (void *)x;
}

#ifndef MAXLIB
void rhythm_setup(void)
{
    rhythm_class = class_new(gensym("rhythm"), (t_newmethod)rhythm_new,
    	(t_method)rhythm_free, sizeof(t_rhythm), 0, A_DEFFLOAT, 0);
    class_addfloat(rhythm_class, rhythm_float);
	class_addmethod(rhythm_class, (t_method)rhythm_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(rhythm_class, (t_method)rhythm_model, gensym("model"), A_FLOAT, 0);
	class_addmethod(rhythm_class, (t_method)rhythm_reset, gensym("reset"), 0);
	class_addmethod(rhythm_class, (t_method)rhythm_print, gensym("print"), 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_rhythm_setup(void)
{
    rhythm_class = class_new(gensym("maxlib_rhythm"), (t_newmethod)rhythm_new,
    	(t_method)rhythm_free, sizeof(t_rhythm), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)rhythm_new, gensym("rhythm"), A_DEFFLOAT, 0);
    class_addfloat(rhythm_class, rhythm_float);
	class_addmethod(rhythm_class, (t_method)rhythm_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(rhythm_class, (t_method)rhythm_model, gensym("model"), A_FLOAT, 0);
	class_addmethod(rhythm_class, (t_method)rhythm_reset, gensym("reset"), 0);
	class_addmethod(rhythm_class, (t_method)rhythm_print, gensym("print"), 0);
    class_sethelpsymbol(rhythm_class, gensym("maxlib/rhythm-help.pd"));
}
#endif
