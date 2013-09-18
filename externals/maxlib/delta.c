/* ------------------------- delta   ------------------------------------------ */
/*                                                                              */
/* Claculate 1st or 2nd order difference.                                       */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Inspired by code written by Trond Lossius.                                   */
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

#define MAXSIZE 32

static char *version = "delta v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct delta
{
  t_object x_ob;
  t_outlet *x_out;                  /* result */
  t_int    x_order;                 /* 1st or second order */
  t_int    x_clearflag;
  t_float  x_delta;                 /* the result */

  t_float  x_prev;                  /* previous value */
  t_float  x_prev2;                 /* value previous to previous value */
} t_delta;

static void delta_clear(t_delta *x)
{
	if(x->x_order == 2)
		x->x_clearflag = 2;
	else
		x->x_clearflag = 1;
	x->x_delta = 0;
}

static void delta_bang(t_delta *x)
{
	outlet_float(x->x_out, x->x_delta);
}

static void delta_float(t_delta *x, t_floatarg f)
{
	if(x->x_order != 2)	/* first order */
	{
		if(x->x_clearflag)
		{
			x->x_prev = f;
			x->x_delta = 0;
			x->x_clearflag = 0;
		}
		else
		{
			x->x_delta = f - x->x_prev;
			x->x_prev = f;
		}
	}
	else
	{
		switch(x->x_clearflag)
		{
			case 0:
				x->x_delta = f - 2*x->x_prev + x->x_prev2;
				x->x_prev2 = x->x_prev;
				x->x_prev = f;
				break;
			case 1:
				x->x_prev = f;
				x->x_clearflag--;
				break;
			case 2:
				x->x_prev2 = f;
				x->x_clearflag--;
				break;
		}
	}
	delta_bang(x);
}

static t_class *delta_class;

static void *delta_new(t_floatarg f)
{
	int i;

    t_delta *x = (t_delta *)pd_new(delta_class);
	x->x_out = outlet_new(&x->x_ob, gensym("float"));

	x->x_order = (int)f;
	if(x->x_order == 2)
		x->x_clearflag = 2;
	else
		x->x_clearflag = 1;
	x->x_delta = 0;

    return (void *)x;
}

#ifndef MAXLIB
void delta_setup(void)
{
    delta_class = class_new(gensym("delta"), (t_newmethod)delta_new,
    	0, sizeof(t_delta), 0, A_DEFFLOAT, 0);
#else
void maxlib_delta_setup(void)
{
    delta_class = class_new(gensym("maxlib_delta"), (t_newmethod)delta_new,
    	0, sizeof(t_delta), 0, A_DEFFLOAT, 0);
#endif
    class_addfloat(delta_class, delta_float);
	class_addbang(delta_class, (t_method)delta_bang);
	class_addmethod(delta_class, (t_method)delta_clear, gensym("clear"), 0);
    class_sethelpsymbol(delta_class, gensym("maxlib/delta-help.pd"));
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)delta_new, gensym("delta"), A_DEFFLOAT, 0);
    class_sethelpsymbol(delta_class, gensym("maxlib/delta-help.pd"));
#endif
}

