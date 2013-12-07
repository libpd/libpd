/* -------------------------  wrap  ------------------------------------------ */
/*                                                                              */
/* wraps input to lie within an output range.                                  */
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

static char *version = "wrap v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct wrap
{
  t_object x_ob;
  t_float  x_min;                  /* low border of input range */
  t_float  x_max;                  /* high border of input range */
  t_outlet *x_outlet1;             /* path-through outlet */
  t_outlet *x_outlet2;             /* wrap outlet */
} t_wrap;

static void wrap_float(t_wrap *x, t_floatarg f)
{
  t_float min = x->x_min;
  t_float max = x->x_max;
  t_float range = max - min;
  t_int i;

  if(range == 0.0f)
    {
      f = min;
      outlet_float(x->x_outlet2, 0);
    }
  else if(f < min)
    {
      t_float diff = min - f;
      t_float n = ceil(diff / range);

      f += n * range;

      outlet_float(x->x_outlet2, -(t_int)n);
    }
  else if (f >= max)
    {
      t_float diff = f - max;
      t_float n = floor(diff / range) + 1.0f;

      f -= n * range;

      outlet_float(x->x_outlet2, (t_int)n);
    }      
  else
    outlet_float(x->x_outlet2, 0.0f);

  outlet_float(x->x_outlet1, f);
}

static void wrap_a(t_wrap *x, t_floatarg a)
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

static void wrap_b(t_wrap *x, t_floatarg b)
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

static t_class *wrap_class;

static void *wrap_new(t_floatarg fmin, t_floatarg fmax)
{
    t_wrap *x = (t_wrap *)pd_new(wrap_class);

    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("a"));
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("b"));

	x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));
	x->x_outlet2 = outlet_new(&x->x_ob, gensym("float"));

	x->x_min = fmin;
	wrap_b(x, fmax);

    return (void *)x;
}

#ifndef MAXLIB
void wrap_setup(void)
{
    wrap_class = class_new(gensym("wrap"), (t_newmethod)wrap_new,
    	0, sizeof(t_wrap), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
#else
void maxlib_wrap_setup(void)
{
    wrap_class = class_new(gensym("maxlib_wrap"), (t_newmethod)wrap_new,
    	0, sizeof(t_wrap), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)wrap_new, gensym("wrap"), A_DEFFLOAT, A_DEFFLOAT, 0);
#endif
    class_addfloat(wrap_class, wrap_float);
	class_addmethod(wrap_class, (t_method)wrap_a, gensym("a"), A_FLOAT, 0);
	class_addmethod(wrap_class, (t_method)wrap_b, gensym("b"), A_FLOAT, 0);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(wrap_class, gensym("maxlib/wrap-help.pd"));
#endif
}

