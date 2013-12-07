/* -------------------------  rewrap  ------------------------------------------ */
/*                                                                              */
/* rewraps input to lie within an output range.                                  */
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

static char *version = "rewrap v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct rewrap
{
  t_object x_ob;
  t_float  x_min;                  /* low border of input range */
  t_float  x_max;                  /* high border of input range */
  t_outlet *x_outlet1;             /* path-through outlet */
  t_outlet *x_outlet2;             /* rewrap outlet */
} t_rewrap;

static void rewrap_float(t_rewrap *x, t_floatarg f)
{
  t_float min = x->x_min;
  t_float max = x->x_max;
  t_float range = 2.0f * (max - min);
  t_int i;

  if(range == 0.0f)
    {
      f = min;
      outlet_float(x->x_outlet2, 0);
    }
  else if(f < min)
    {
      float diff = min - f;
      float n = ceil(diff / range);

      f += n * range;

      if(f >= max)
	{
	  f = 2 * max - f;
	  n -= 0.5;
	}

      outlet_float(x->x_outlet2, (t_int)(-2.0f * n));
    }
  else if (f >= max)
    {
      float diff = f - max;
      float n = floor(diff / range) + 1.0f;

      f -= n * range;

      if(f < min)
	{
	  f = 2 * min - f;
	  n -= 0.5;
	}

      outlet_float(x->x_outlet2, (t_int)(2.0f * n));
    }
  else
    outlet_float(x->x_outlet2, 0.0f);
    

  outlet_float(x->x_outlet1, f);
}

static void rewrap_a(t_rewrap *x, t_floatarg a)
{
	t_float max = x->x_max;

	if(a <= max)
		x->x_min = a;
	else
	{
		x->x_min = max;
		x->x_max = a;
	}
}

static void rewrap_b(t_rewrap *x, t_floatarg b)
{
	t_float min = x->x_min;

	if(b >= min)
		x->x_max = b;
	else
	{
		x->x_max = min;
		x->x_min = b;
	}
}

static t_class *rewrap_class;

static void *rewrap_new(t_floatarg fmin, t_floatarg fmax)
{
    t_rewrap *x = (t_rewrap *)pd_new(rewrap_class);

    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("a"));
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("b"));

	x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));
	x->x_outlet2 = outlet_new(&x->x_ob, gensym("float"));

	x->x_min = fmin;
	rewrap_b(x, fmax);

    return (void *)x;
}

#ifndef MAXLIB
void rewrap_setup(void)
{
    rewrap_class = class_new(gensym("rewrap"), (t_newmethod)rewrap_new,
    	0, sizeof(t_rewrap), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(rewrap_class, rewrap_float);
	class_addmethod(rewrap_class, (t_method)rewrap_a, gensym("a"), A_FLOAT, 0);
	class_addmethod(rewrap_class, (t_method)rewrap_b, gensym("b"), A_FLOAT, 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_rewrap_setup(void)
{
    rewrap_class = class_new(gensym("maxlib_rewrap"), (t_newmethod)rewrap_new,
    	0, sizeof(t_rewrap), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)rewrap_new, gensym("rewrap"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(rewrap_class, rewrap_float);
	class_addmethod(rewrap_class, (t_method)rewrap_a, gensym("a"), A_FLOAT, 0);
	class_addmethod(rewrap_class, (t_method)rewrap_b, gensym("b"), A_FLOAT, 0);
    class_sethelpsymbol(rewrap_class, gensym("maxlib/rewrap-help.pd"));
}
#endif
