/* -------------------------  limit  ------------------------------------------ */
/*                                                                              */
/* limits input to lie within an output range.                                  */
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

static char *version = "limit v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct limit
{
  t_object x_ob;
  t_float  x_ol;                   /* low border of output range */
  t_float  x_oh;                   /* high border of output range */
  t_float  x_ratio;                /* 'compression ratio' */
  t_outlet *x_outlet1;             /* result */
  t_float  x_f;
} t_limit;

static void limit_float(t_limit *x, t_floatarg f)
{
	if(x->x_oh < x->x_ol)	/* swap values */
	{
		int i = x->x_oh;
		x->x_oh = x->x_ol;
		x->x_ol = i;
	}
	if(x->x_ratio == 0)	/* 'clip' mode */
	{
		if(f > x->x_oh)f = x->x_oh;
		else if(f < x->x_ol)f = x->x_ol;
	}
	else	/* 'compress' mode */
	{
		int diff;
		if(f > x->x_oh)
		{
			diff = f - x->x_oh;
			f = x->x_oh + (diff / x->x_ratio);
		}
		else if(f < x->x_ol)
		{
			diff = x->x_ol - f;
			f = x->x_ol - (diff / x->x_ratio);
		}
	}
	outlet_float(x->x_outlet1, f);
	x->x_f = f;
}

static void limit_bang(t_limit *x)
{
	limit_float(x, x->x_f);	/* recalculate result */
}

static t_class *limit_class;

static void *limit_new(t_floatarg fol, t_floatarg foh, t_floatarg fr)
{
    t_limit *x = (t_limit *)pd_new(limit_class);

	floatinlet_new(&x->x_ob, &x->x_ol);
	floatinlet_new(&x->x_ob, &x->x_oh);
	floatinlet_new(&x->x_ob, &x->x_ratio);

	x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));

		/* default values taken from Max's limit */
	x->x_ol = fol;
	x->x_oh = foh;
	if(x->x_oh < x->x_ol)	/* swap values */
	{
		int i = x->x_oh;
		x->x_oh = x->x_ol;
		x->x_ol = i;
	}
	x->x_ratio = fr;
	x->x_f = 0;

    return (void *)x;
}

#ifndef MAXLIB
void limit_setup(void)
{
    limit_class = class_new(gensym("limit"), (t_newmethod)limit_new,
    	0, sizeof(t_limit), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
#else
void maxlib_limit_setup(void)
{
    limit_class = class_new(gensym("maxlib_limit"), (t_newmethod)limit_new,
    	0, sizeof(t_limit), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)limit_new, gensym("limit"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
#endif
    class_addfloat(limit_class, limit_float);
    class_addbang(limit_class, limit_bang);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(limit_class, gensym("maxlib/limit-help.pd"));
#endif
}

