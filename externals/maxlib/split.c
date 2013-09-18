/* -------------------------  split  ------------------------------------------ */
/*                                                                              */
/* splits input to lie within an output range.                                  */
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

static char *version = "split v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct split
{
  t_object x_ob;
  t_float  x_min;                  /* low border of input range */
  t_float  x_max;                  /* high border of input range */
  t_int    x_revert;               /* range is inverted */
  t_outlet *x_outlet1;             /* path-through outlet */
  t_outlet *x_outlet2;             /* split outlet */
} t_split;

static void split_float(t_split *x, t_floatarg f)
{
	if(x->x_max >= x->x_min)
	{
		if(f <= x->x_max && f >= x->x_min)
			outlet_float(x->x_outlet1, f);
		else
			outlet_float(x->x_outlet2, f);
	}
	else
	{
		if(f >= x->x_max && f <= x->x_min)
			outlet_float(x->x_outlet1, f);
		else
			outlet_float(x->x_outlet2, f);
	}
}

static t_class *split_class;

static void *split_new(t_floatarg fmin, t_floatarg fmax)
{
    t_split *x = (t_split *)pd_new(split_class);

	floatinlet_new(&x->x_ob, &x->x_min);
	floatinlet_new(&x->x_ob, &x->x_max);

	x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));
	x->x_outlet2 = outlet_new(&x->x_ob, gensym("float"));

	x->x_min = fmin;
	x->x_max = fmax;

    return (void *)x;
}

#ifndef MAXLIB
void split_setup(void)
{
    split_class = class_new(gensym("split"), (t_newmethod)split_new,
    	0, sizeof(t_split), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(split_class, split_float);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_split_setup(void)
{
    split_class = class_new(gensym("maxlib_split"), (t_newmethod)split_new,
    	0, sizeof(t_split), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)split_new, gensym("split"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(split_class, split_float);
    class_sethelpsymbol(split_class, gensym("maxlib/split-help.pd"));
}
#endif
