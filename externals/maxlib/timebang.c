/* -------------------------  timebang  --------------------------------------- */
/*                                                                              */
/* Send out bangs at given times (time of day!).                                */
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
#include <stdio.h>
#include <math.h>
#include <time.h>

#define MAX_TIMES 256			/* maximum number of times to process */

static char *version = "timebang v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct timebang
{
  t_object x_ob;
  t_clock  *x_clock;
  t_outlet *x_outlet[MAX_TIMES];   /* the bang outlets */
  t_int    x_sec[MAX_TIMES];       /* seconds (0 - 59) */
  t_int    x_min[MAX_TIMES];       /* minutes (0 - 59) */
  t_int    x_hour[MAX_TIMES];      /* hours (0 - 11) */
  t_int    x_mday[MAX_TIMES];      /* day of month (1 - 31) */
  t_int    x_mon[MAX_TIMES];       /* month (0 - 11) */
  t_int    x_year[MAX_TIMES];      /* year (current year minus 1900) */
  t_int    x_wday[MAX_TIMES];      /* day of week (0 - 6, Sunday = 0, -1 = all days) */
  t_int    x_over[MAX_TIMES];      /* indicate that time is over */
  t_int    x_notimes;              /* number of times to bang */
} t_timebang;


static void timebang_tick(t_timebang *x)
{
	time_t now = time(NULL);
	struct tm *newtime;
	int i;

	newtime = localtime(&now);  /* convert to local time. */
	for(i = 0; i < x->x_notimes; i++)
	{
		if(!x->x_over[i])
		{
			if(newtime->tm_hour == x->x_hour[i] && 
			   newtime->tm_min == x->x_min[i] &&
			   newtime->tm_sec >= x->x_sec[i])
			{
				x->x_over[i] = 1;				/* mark as 'time is over' */
				outlet_bang(x->x_outlet[i]);	/* send corresponding bang */
			}
		}
		else if(newtime->tm_hour != x->x_hour[i])
			x->x_over[i] = 0;					/* reactivate time one hour later */
	}

	clock_delay(x->x_clock, 1000);	/* come back in one second */
}

static void timebang_set(t_timebang *x, t_symbol *s, int ac, t_atom *av)
{
	int i, j;

	if(ac == x->x_notimes * 3)
	{
		for(i = 0, j = 0; i < ac; i += 3, j++)
		{
			if (av[i].a_type == A_FLOAT) x->x_hour[j] = av[i].a_w.w_float;
			else { post ("timebang: first argument must be (int) hours"); return; }
			if (av[i+1].a_type == A_FLOAT) x->x_min[j] = av[i+1].a_w.w_float;
			else { post ("timebang: second argument must be (int) minutes"); return; }
			if (av[i+2].a_type == A_FLOAT) x->x_sec[j] = av[i+2].a_w.w_float;
			else { post ("timebang: third argument must be (int) seconds"); return; }
			x->x_over[i] = 0;
		}
		post("timebang: read in %d times of day:", x->x_notimes);
		for(i = 0; i < x->x_notimes; i++)
		{
			post("          %02d:%02d:%02d", x->x_hour[i], x->x_min[i], x->x_sec[i]);
		}
	}
	else post("timebang: wrong number of parameter");
}

static void timebang_bang(t_timebang *x)
{
	time_t now = time(NULL);
	struct tm *newtime = localtime(&now);  /* convert to local time. */
	post("timebang: local time is %02d:%02d:%02d", newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
}

static t_class *timebang_class;

static void *timebang_new(t_symbol *s, int ac, t_atom *av)
{
	int i;
    t_timebang *x = (t_timebang *)pd_new(timebang_class);

	x->x_clock = clock_new(x, (t_method)timebang_tick);

	if(ac > MAX_TIMES * 3)
	{
		post("timebang: too many creation arguments");
		ac = MAX_TIMES * 3;
	}

	x->x_notimes = 0;
	for(i = 0; i < ac; i += 3)
	{
		if (av[i].a_type == A_FLOAT) x->x_hour[x->x_notimes] = av[i].a_w.w_float;
		else { post ("timebang: first argument must be (int) hours"); return 0; }
		if (av[i+1].a_type == A_FLOAT) x->x_min[x->x_notimes] = av[i+1].a_w.w_float;
		else { post ("timebang: second argument must be (int) minutes"); return 0; }
		if (av[i+2].a_type == A_FLOAT) x->x_sec[x->x_notimes] = av[i+2].a_w.w_float;
		else { post ("timebang: third argument must be (int) seconds"); return 0; }
		x->x_over[x->x_notimes] = 0;
		x->x_notimes++;
	}
	post("timebang: read in %d times of day:", x->x_notimes);
	for(i = 0; i < x->x_notimes; i++)
	{
		x->x_outlet[i] = outlet_new(&x->x_ob, gensym("bang"));	/* create specific bang outlet for time */
		post("          %02d:%02d:%02d", x->x_hour[i], x->x_min[i], x->x_sec[i]);
	}

	clock_set(x->x_clock, 0);

    return (void *)x;
}

static void timebang_free(t_timebang *x)
{
	clock_free(x->x_clock);
}

#ifndef MAXLIB
void timebang_setup(void)
{
    timebang_class = class_new(gensym("timebang"), (t_newmethod)timebang_new,
    	(t_method)timebang_free, sizeof(t_timebang), 0, A_GIMME, 0);
#else
void maxlib_timebang_setup(void)
{
    timebang_class = class_new(gensym("maxlib_timebang"), (t_newmethod)timebang_new,
    	(t_method)timebang_free, sizeof(t_timebang), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)timebang_new, gensym("timebang"), A_GIMME, 0);
#endif
	class_addmethod(timebang_class, (t_method)timebang_set, gensym("set"), A_GIMME, 0);
	class_addbang(timebang_class, (t_method)timebang_bang);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(timebang_class, gensym("maxlib/timebang-help.pd"));
#endif
}

