/* -------------------------- history ----------------------------------------- */
/*                                                                              */
/* Calculates the average value of the elements within the last N seconds.      */
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
#include <math.h>

#define MAX_ARG   1024              /* maximum number of items to average */
#define MAX_TIME  60000             /* maximum time to look back */

static char *version = "history v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct history
{
  t_object x_ob;
  t_clock  *x_clock;
  t_inlet  *x_inindex;
  t_outlet *x_outfloat;             /* output the history */
  t_outlet *x_outtendency;          /* outputs the tendency of the average */
  t_int    x_limit;                 /* indicates if input is 'blocked' (1) */
  t_int    x_index;                 /* the number of elements to average */
  t_float  x_input[MAX_ARG];        /* stores the input values we need for averaging */
  double   x_intime[MAX_ARG];       /* stores the time of arrival of an element */
  t_int    x_inpointer;             /* actual position in above array */
  t_float  x_average;               /* what do you guess ? */
  t_float  x_lastaverage;
  t_int    x_mode;                  /* how to history: linear or geometric */
  t_int    x_time;

} t_history;

	/* there must be a function for this in math.h but how is the 
	   german 'Fakultät' called in english ????  */
static int normalise(int i)
{
	int ret = i;
	while(i--)
	{
		if(i == 0)break;
		ret += i;
	}
	return (ret);
}

static void history_tick(t_history *x)
{
	t_float tendency = 0.0;
	if(x->x_lastaverage < x->x_average)
	{
		tendency = 1.0;	/* getting more */
	}
	else if(x->x_lastaverage > x->x_average)
	{
		tendency = -1.0;	/* getting less */
	}
	else tendency = 0.0;	/* nothing has changed */
	outlet_float(x->x_outtendency, tendency);
	x->x_lastaverage = x->x_average;
	clock_delay(x->x_clock, x->x_time);
}

static void history_float(t_history *x, t_floatarg f)
{
	int i, j = 0, k = 0, l;
	t_float geo = 1.0;

	x->x_average = 0;
		/* put value into array */
	x->x_input[x->x_inpointer] = f;
	x->x_intime[x->x_inpointer] = clock_getlogicaltime();
		/* look for elements that are too old */
	x->x_index = 0;
	for(i = 0; i < MAX_ARG; i++)	/* check all valid elements */
	{
		if(x->x_intime[i] != 0)
		{
			if(clock_gettimesince(x->x_intime[i]) <= x->x_time)	/* it's in our time window */
			{
				x->x_index++;	/* count valid entries */
			}
			else	/* too old, delete entry */
			{
				x->x_intime[i] = 0;
			}
		}
	}
	if(x->x_index > 1)
	{
			/* calulate history */
		for(i = 0; i < MAX_ARG; i++)	/* check all valid elements */
		{
			if(x->x_intime[i] != 0)	/* it's a valid entry */
			{
				k++;
				l = MAX_ARG;

				if(x->x_mode == 0)	/* linear */
				{
					x->x_average += x->x_input[i] * (1.0 / (float)x->x_index);
				}
				else if(x->x_mode == 1)	/* geometric */
				{
					if(x->x_input[i] == 0)x->x_input[i] = 0.001;	/* need to cheat a bit... */
					geo *= x->x_input[i];
					if(k == x->x_index)
						x->x_average = pow(geo, (1.0/(float)x->x_index));
				}
				else if(x->x_mode == 2)	/* weighted */
				{
						/* normalise output */
					if(k == x->x_index)
					{
						x->x_average += x->x_input[(j + x->x_inpointer + MAX_ARG) % MAX_ARG] * (float)(x->x_index - k);
						x->x_average = x->x_average / (float)normalise(x->x_index - 1);
					}
					else
					{
						x->x_average += x->x_input[(j + x->x_inpointer + MAX_ARG) % MAX_ARG] * (float)(x->x_index - k);
						j--;	/* go back in array */
						while(l--)	/* check if this will result in a valid value */
						{
							if(x->x_intime[(j + x->x_inpointer + MAX_ARG) % MAX_ARG] == 0)
							{
								j--;	/* go back more if necessary */
							}
							else break;	/* finished on first non-zero */
						}
					}
				} else post("history: internal error!");
			}
		}
	}
	else x->x_average = x->x_input[x->x_inpointer];

	if(++x->x_inpointer > MAX_ARG)
	{
		x->x_inpointer = 0;
	}
	outlet_float(x->x_outfloat, x->x_average);
}

static void history_time(t_history *x, t_floatarg f)
{
	x->x_time = (t_int)f;
	if(x->x_time < 1) x->x_time = 1;
	if(x->x_time > MAX_TIME)x->x_time = MAX_TIME;
	clock_unset(x->x_clock);
	clock_delay(x->x_clock, 0);
}

static void history_reset(t_history *x)
{
	int i;
		/* zeroe out the array */
	for(i = 0; i < MAX_ARG; i++)
	{
		x->x_input[i] = 0.0;
		x->x_intime[i] = 0.0;
	}
	x->x_index = 0;
	x->x_inpointer = 0;
	x->x_average = 0;
	x->x_lastaverage = 0;
    post("history: reset");
}

static void history_linear(t_history *x)
{
	x->x_mode = 0;
    post("history: linear");
}

static void history_geometric(t_history *x)
{
	x->x_mode = 1;
    post("history: geometric");
}

static void history_weight(t_history *x)
{
	x->x_mode = 2;
    post("history: weighted");
}

static void history_free(t_history *x)
{
	clock_free(x->x_clock);
}

static t_class *history_class;

static void *history_new(t_floatarg f)
{
	int i;

    t_history *x = (t_history *)pd_new(history_class);
	x->x_inindex = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("time"));
	x->x_outfloat = outlet_new(&x->x_ob, gensym("float"));
	x->x_outtendency = outlet_new(&x->x_ob, gensym("float"));
	x->x_clock = clock_new(x, (t_method)history_tick);
		/* zeroe out the array */
	for(i = 0; i < MAX_ARG; i++)
	{
		x->x_input[i] = 0.0;
		x->x_intime[i] = 0.0;
	}
	x->x_time = (t_int)f;
	if(x->x_time < 1) x->x_time = 1;
	if(x->x_time > MAX_TIME)
	{
		x->x_time = MAX_TIME;
		post("history: set number time to %d", x->x_time);
	}
	x->x_index = 0;
	x->x_inpointer = 0;
	x->x_average = 0;
	x->x_mode = 0;
	clock_delay(x->x_clock, 0);

    return (void *)x;
}

#ifndef MAXLIB
void history_setup(void)
{
    history_class = class_new(gensym("history"), (t_newmethod)history_new,
    	(t_method)history_free, sizeof(t_history), 0, A_DEFFLOAT, 0);
    class_addmethod(history_class, (t_method)history_reset, gensym("reset"), 0);
    class_addmethod(history_class, (t_method)history_linear, gensym("linear"), 0);
    class_addmethod(history_class, (t_method)history_geometric, gensym("geometric"), 0);
    class_addmethod(history_class, (t_method)history_weight, gensym("weight"), 0);
    class_addfloat(history_class, history_float);
	class_addmethod(history_class, (t_method)history_time, gensym("time"), A_FLOAT, 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_history_setup(void)
{
    history_class = class_new(gensym("maxlib_history"), (t_newmethod)history_new,
    	(t_method)history_free, sizeof(t_history), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)history_new, gensym("history"), A_DEFFLOAT, 0);
    class_addmethod(history_class, (t_method)history_reset, gensym("reset"), 0);
    class_addmethod(history_class, (t_method)history_linear, gensym("linear"), 0);
    class_addmethod(history_class, (t_method)history_geometric, gensym("geometric"), 0);
    class_addmethod(history_class, (t_method)history_weight, gensym("weight"), 0);
    class_addfloat(history_class, history_float);
	class_addmethod(history_class, (t_method)history_time, gensym("time"), A_FLOAT, 0);
    class_sethelpsymbol(history_class, gensym("maxlib/history-help.pd"));
}
#endif
