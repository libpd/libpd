/* --------------------------- beat  ------------------------------------------ */
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
#include <stdlib.h>
#include <string.h>

#define BEAT_LONG 1500			/* longest time we take into concideration (40 bpm) */
#define BEAT_SHORT 300			/* shortest time we take into concideration (200 bpm) */

static char *version = "beat v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct
{
	t_int	points;					/* number of points assigned to this theory */
	double  expect;					/* time of next expected hit */
	t_int	onbeat;					/* whether (1) or not (0) it was on the beat */
} beat_theory;

typedef struct						/* used for sorting theories */
{
	t_int points;
	t_int theory;
} beat_sort_record;

 
typedef struct beat
{
  t_object x_ob;
  t_clock  *x_clock;
  t_outlet *x_outbpm;               /* beat as MIDI note number */
  t_outlet *x_outms;                /* beat in milliseconds */
  t_outlet *x_outbeat;				/* send a bang whenever beat is 'on beat' */
  t_int    x_print;                 /* switch printing to console window on / off */

  t_int    x_num_beats;				/* number of beats we've received */
  double   x_beat_period;			/* time in ms until next expected beat / beat pulse */
  beat_theory	x_beats[BEAT_LONG];
  double   x_beatexpect;			/* expected time for next beat */
  t_int    x_on_beat;				/* indicate if last event was on beat */
  t_int    x_band_percent;

  t_int    x_pitch;
  t_int    x_velo;
	/* helpers needed to do the time calculations */
  double   x_this_input;
  double   x_last_input;
  double   x_lasttime;
  double   x_lastlasttime;
} t_beat;

/* ---------------- mathematical functions to work with doubles  -------------- */
static double double_abs(double value)
{
	if(value < 0)
		return (value * -1);
	else
		return (value);
}

/* --------------- beat  stuff ------------------------------------------------ */
	/* evaluate results: find theory that is the most likely one and
       print out internal data to console window if print is enabled */
static int beat_evaluate(t_beat *x)
{
	int i, j, K;
	char string[256];
	char info[40];
	beat_sort_record theories[BEAT_LONG], *sortp, R;
	int value;	/* the result of the sorting */

	for (i = 0; i < BEAT_LONG; i++)
	{		/* prepare sort records */
		sortp         = &(theories[i]);
		sortp->points = x->x_beats[i].points;
		sortp->theory = i;
	}
	for (j = 2; j < BEAT_LONG; j++)
	{		/* sort */
		i = j - 1;
		K = theories[j].points;
		R = theories[j];
		while (i > 0)
		{
			if (K >= theories[i].points)
			{
				theories[i+1] = R;
				break;
			}
			else
			{
				theories[i+1] = theories[i];
				i -= 1;
			}
		}
		if (i==0) theories[i+1] = R;
	}
		/* get leading result */
	sortp = &(theories[BEAT_LONG - 1]);
	value = sortp->theory;	/* get our resulting theory */

	if(x->x_print)
	{
		post("         0         1         2         3         4         R            E");
		*string = '\0';						/* print out five leading theories */
		sprintf(info, "%4g", x->x_this_input);
		strcat(string, info);
		for(i = 1; i < 6; i++)
		{
			sortp = &(theories[BEAT_LONG - i]);
			sprintf(info, " %4d[%3d]", (int) sortp->theory, (int) sortp->points);
			strcat(string, info);
		}
		sprintf(info, "  %g %g", clock_getlogicaltime(), x->x_beatexpect);
		strcat(string, info);
		post(string);
	}

	return value;
}

	/* reduce duration to fit into our processing window */
	/* some sort of 'double modulo'...                   */
static double beat_reduce_offset(double duration)
{
	double temp	= duration;
	int divisor = 2;					/* first try dividing by two */
	while (temp > BEAT_LONG)		    /* while duration is too long */
		temp = duration / divisor++;    /* divide by progressively higher divisors */
	return temp;				        /* return a value in bounds */
}

/*
 * beat_eligible: determine whether an event is eligible for consideration
 * as a beat theory
 */
static int beat_eligible(double candidate, int* offsets, int num_offsets)
{
	double diff;
	int i;

	if (candidate >= BEAT_LONG)		/* if too long try subharmonics */
		candidate = beat_reduce_offset(candidate);
	
		/* if candidate is close to one already found */
	for(i = 0; i < num_offsets; i++)
	{
		diff = double_abs((candidate - offsets[i]));
		if (diff < offsets[i]/20) {
			if (candidate > offsets[i])
				++offsets[i]; else			/* pull existing one */
			if (candidate < offsets[i])		/* toward new candidate */
				--offsets[i];
			return 0;						/* declare candidate ineligible */
		}
	}
	return candidate;						/* otherwise return legal candidate */
}

static void beat_float(t_beat *x, t_floatarg f)
{
	t_int velo = x->x_velo;
	int i, j, indx;
	int num_offsets, candidate;
	int low_limit, high_limit, width, deviation;
	int points, band, center_offset, period;
	beat_theory* t;
	int offsets[7];
	static int factors[10] = 
		{ 200, 50, 300, 150, 66, 400, 600, 133, 33, 75 };
	double now = clock_getlogicaltime();
	t_float outvalue;

	x->x_pitch = (t_int)f;
	x->x_this_input = clock_gettimesince(x->x_last_input);

	if(velo != 0)	/* note-on received */
	{
		if(++x->x_num_beats == 1)
		{
			goto time;					/* only one event, no beats yet */
		}

		num_offsets = 0;
		candidate  = beat_eligible(x->x_this_input, offsets, num_offsets);
		if(candidate)
			offsets[num_offsets++] = candidate;				/* offset[0] set to incoming offset */
 
		if(x->x_num_beats > 2)
		{								/* if three events */
				/* check previous for eligibility */
			candidate = beat_eligible(x->x_lasttime, offsets, num_offsets);
			if (candidate)
				offsets[num_offsets++] = candidate;
			candidate  = x->x_this_input + x->x_lasttime;	/* add current and previous offsets */
			candidate  = beat_eligible(candidate, offsets, num_offsets);
			if (candidate)									/* add to list if eligible */
				offsets[num_offsets++] = candidate;
		}

		if(x->x_num_beats > 3)
		{
			candidate  = beat_eligible(x->x_lastlasttime, offsets, num_offsets);
			if (candidate)
				offsets[num_offsets++] = candidate;
			candidate += x->x_lasttime;
			candidate  = beat_eligible(candidate, offsets, num_offsets);
			if (candidate)
				offsets[num_offsets++] = candidate;
		}

		indx = 0;
		for(i = num_offsets; i < 7; i++)
		{
			offsets[i] = 0;
			if (indx >= 10) break;
			candidate  = 0;
			while ((indx < 10) && (!candidate))
				candidate = beat_eligible((x->x_this_input * factors[indx++])/100, offsets, num_offsets);
			if (candidate)
				offsets[num_offsets++] = candidate;
		}
		
		for(i = 0; i < num_offsets; i++)
		{
			band = offsets[i] * x->x_band_percent / 100;
			if ((low_limit = offsets[i] - band) < 0)		/* add points in a critical band */
				low_limit  = 0;								/* around calculated offset */
			if ((high_limit = offsets[i] + band) > BEAT_LONG)
				high_limit = BEAT_LONG;
			center_offset = offsets[i];						/* midpoint of increment */
			points = 0;
			for (j = low_limit; j < high_limit; j++)
			{
				if ((points = x->x_beats[j].points) > 0)
				{		/* if there is already activation */
					deviation = j - center_offset;			/* record deviation from midpoint */
					x->x_beats[j].points = 0;
					if (deviation < 0) {					/* if there is activation below midpoint */
						t = &(x->x_beats[j+1]);				/* take theory one above prior */
					} else
					if (deviation > 0) {					/* if there is activation above midpoint */
						t = &(x->x_beats[j-1]);				/* take theory one below prior */
					} else
						t = &(x->x_beats[j]);				/* landed right on it */
					t->points = points + (num_offsets-i);
					break;
				}
			}
			if (!points)
				x->x_beats[center_offset].points = num_offsets - i;
		}

			/* boost hits, and suppress theories with missed beats */
		period = 0;
		points = 0;
		for (i = BEAT_SHORT; i < BEAT_LONG; i++)
		{
			t = &(x->x_beats[i]);
			width = 5 > (t->expect / 7) ? 5 : (t->expect / 7);
			t->expect -= x->x_this_input;
			t->onbeat  = 0;
			if(double_abs(t->expect) <= width)	/* lies within range */
			{
				t->expect = i;
				t->onbeat = 1;
	 			if (t->points > 0)
					t->points += 4;				/* add 4 points */
			}
			else if(t->expect < 0)
			{
				t->points -= 8;
				t->expect  = i;
			}
			if (t->points < 0)   t->points =   0; else
			if (t->points > 200) t->points = 200;
			if (t->points > points)
			{
				points = t->points;
				period = i;
			}
		}



		x->x_beat_period    = (double)period;
		t					= &(x->x_beats[period]);
		x->x_beatexpect		= now + (double)t->expect;
		x->x_on_beat	    = t->onbeat;
		
time:
		x->x_lastlasttime = x->x_lasttime;
		x->x_lasttime = x->x_this_input; //now;
		x->x_last_input = now;

		if(x->x_on_beat)outlet_bang(x->x_outbeat);
		outvalue = (t_float)beat_evaluate(x);
		outlet_float(x->x_outms, outvalue);
		if(x->x_beat_period)outlet_float(x->x_outbpm, (t_float)(60000.0 / outvalue));
	}
	return;
}

static void beat_ft1(t_beat *x, t_floatarg f)
{
	x->x_velo = (t_int)f;
}

	/* toggle printing on/off */
static void beat_print(t_beat *x)
{
	if(x->x_print)x->x_print = 0;
	else x->x_print = 1;
}

static void beat_reset(t_beat *x)
{
	int i;

	for(i = 0; i < BEAT_LONG; i++)
	{
		x->x_beats[i].points = 0;
		x->x_beats[i].expect = i;
		x->x_beats[i].onbeat = 0;
	}
	x->x_lastlasttime = 0;
	x->x_lasttime = 0;
	x->x_num_beats = 0;
	x->x_beat_period = 0;
	x->x_on_beat = 0;
}

static t_class *beat_class;

static void beat_free(t_beat *x)
{
	/* nothing to do */
}

static void *beat_new(t_floatarg f)
{
    t_beat *x = (t_beat *)pd_new(beat_class);
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	x->x_outbpm = outlet_new(&x->x_ob, gensym("float"));
	x->x_outms = outlet_new(&x->x_ob, gensym("float"));
	x->x_outbeat = outlet_new(&x->x_ob, gensym("bang"));

	beat_reset(x);
	x->x_band_percent = 4;	/* allow 4% 'jitter' by default */
	if(f)x->x_band_percent = (t_int)f;

	post("beat: band percentage set to %d", x->x_band_percent);

    return (void *)x;
}

#ifndef MAXLIB
void beat_setup(void)
{
    beat_class = class_new(gensym("beat"), (t_newmethod)beat_new,
    	(t_method)beat_free, sizeof(t_beat), 0, A_DEFFLOAT, 0);
#else
void maxlib_beat_setup(void)
{
    beat_class = class_new(gensym("maxlib_beat"), (t_newmethod)beat_new,
    	(t_method)beat_free, sizeof(t_beat), 0, A_DEFFLOAT, 0);
#endif
    class_addfloat(beat_class, beat_float);
	class_addmethod(beat_class, (t_method)beat_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(beat_class, (t_method)beat_reset, gensym("reset"), 0);
	class_addmethod(beat_class, (t_method)beat_print, gensym("print"), 0);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)beat_new, gensym("beat"), A_DEFFLOAT, 0);
    class_sethelpsymbol(beat_class, gensym("maxlib/beat-help.pd"));
#endif
}

