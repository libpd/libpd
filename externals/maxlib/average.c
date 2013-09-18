/* -------------------------- average ----------------------------------------- */
/*                                                                              */
/* Calculates the average value of the last N elements.                         */
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

#define MAX_ARG  128                /* maximum number of items to average */

static char *version = "average v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct average
{
  t_object x_ob;
  t_clock  *x_clock;
  t_inlet  *x_inindex;
  t_outlet *x_outfloat;             /* output the average */
  t_outlet *x_outtendency;          /* outputs the tendency of the average */
  t_int    x_limit;                 /* indicates if input is 'blocked' (1) */
  t_int    x_index;                 /* the number of elements to average */
  t_float  x_input[MAX_ARG];        /* stores the input values we need for averaging */
  t_int    x_inpointer;             /* actual position in above array */
  t_float  x_average;               /* what do you guess ? */
  t_float  x_lastaverage;
  t_int    x_mode;                  /* how to average: linear or geometric */

} t_average;

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

static void average_float(t_average *x, t_floatarg f)
{
	int i, j = 0;
	t_float tendency;
	t_float geo = 1.0;

	x->x_average = 0;
		/* put value into array */
	x->x_input[x->x_inpointer] = f;
		/* calulate average */
	for(i = 0; i < x->x_index; i++)
	{
		if(x->x_mode == 0)	/* linear */
		{
			x->x_average += x->x_input[i] * (1.0 / (float)x->x_index);
		}
		else if(x->x_mode == 1)	/* geometric */
		{
			if(x->x_input[i] == 0)x->x_input[i] = 0.001;	/* need to cheat a bit... */
			geo *= x->x_input[i];
			if(i == x->x_index - 1)
				x->x_average = pow(geo, (1.0/(float)x->x_index));
		}
		else if(x->x_mode == 2)	/* weighted */
		{
			x->x_average += x->x_input[(j + x->x_inpointer + x->x_index) % x->x_index] * (float)(x->x_index - (i + 1));
			j--;	/* go back in array */
				/* normalise output */
			if(i == x->x_index - 1)
				x->x_average = x->x_average / (float)normalise(x->x_index - 1);
		} else post("average: internal error!");
	}
	if(++x->x_inpointer > x->x_index)
	{
		x->x_inpointer = 0;
		if(x->x_lastaverage < x->x_average)
		{
			tendency = 1;	/* getting more */
		}
		else if(x->x_lastaverage > x->x_average)
		{
			tendency = -1;	/* getting less */
		}
		else tendency = 0;	/* nothing has changed */
		outlet_float(x->x_outtendency, tendency);
		x->x_lastaverage = x->x_average;
	}
	outlet_float(x->x_outfloat, x->x_average);
}

static void average_index(t_average *x, t_floatarg f)
{
	x->x_index = (t_int)f;
	if(x->x_index > MAX_ARG)x->x_index = MAX_ARG;
}

static void average_reset(t_average *x)
{
	int i;
		/* zeroe out the array */
	for(i = 0; i < MAX_ARG; i++)x->x_input[i] = 0.0;
	x->x_inpointer = 0;
	x->x_average = 0;
	x->x_lastaverage = 0;
    post("average: reset");
}

static void average_linear(t_average *x)
{
	x->x_mode = 0;
    post("average: linear");
}

static void average_geometric(t_average *x)
{
	x->x_mode = 1;
    post("average: geometric");
}

static void average_weight(t_average *x)
{
	x->x_mode = 2;
    post("average: weighted");
}

static void average_free(t_average *x)
{
	/* nothing to do */
}

static t_class *average_class;

static void *average_new(t_floatarg f)
{
	int i;

    t_average *x = (t_average *)pd_new(average_class);
	x->x_inindex = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("index"));
	x->x_outfloat = outlet_new(&x->x_ob, gensym("float"));
	x->x_outtendency = outlet_new(&x->x_ob, gensym("float"));

		/* zeroe out the array */
	for(i = 0; i < MAX_ARG; i++)x->x_input[i] = 0.0;
	x->x_index = (t_int)f;
	if(x->x_index > MAX_ARG)
	{
		x->x_index = MAX_ARG;
		post("average: set number of items to %d", x->x_index);
	}
	x->x_inpointer = 0;
	x->x_average = 0;
	x->x_mode = 0;
    return (void *)x;
}

#ifndef MAXLIB
void average_setup(void)
{
    average_class = class_new(gensym("average"), (t_newmethod)average_new,
    	(t_method)average_free, sizeof(t_average), 0, A_DEFFLOAT, 0);
#else
void maxlib_average_setup(void)
{
    average_class = class_new(gensym("maxlib_average"), (t_newmethod)average_new,
    	(t_method)average_free, sizeof(t_average), 0, A_DEFFLOAT, 0);
#endif
    class_addmethod(average_class, (t_method)average_reset, gensym("reset"), 0);
    class_addmethod(average_class, (t_method)average_linear, gensym("linear"), 0);
    class_addmethod(average_class, (t_method)average_geometric, gensym("geometric"), 0);
    class_addmethod(average_class, (t_method)average_weight, gensym("weight"), 0);
    class_addfloat(average_class, average_float);
	class_addmethod(average_class, (t_method)average_index, gensym("index"), A_FLOAT, 0);
#ifndef MAXLIB
    logpost(NULL, 4, version);
    
#else
	class_addcreator((t_newmethod)average_new, gensym("average"), A_DEFFLOAT, 0);
    class_sethelpsymbol(average_class, gensym("maxlib/average-help.pd"));
#endif
}

