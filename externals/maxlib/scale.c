/* -------------------------  scale  ------------------------------------------ */
/*                                                                              */
/* Scales input to lie within an output range.                                  */
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

static char *version = "scale v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct scale
{
  t_object x_ob;
  t_float  x_f;                    /* current input value */
  t_float  x_il;                   /* low border of input range */
  t_float  x_ih;                   /* high border of input range */
  t_float  x_ol;                   /* low border of output range */
  t_float  x_oh;                   /* high border of output range */
  t_float  x_logcoeff;             /* log-coefficient */
  t_outlet *x_outlet1;             /* result */
} t_scale;

static void scale_float(t_scale *x, t_floatarg f)
{
	t_float ir = x->x_ih - x->x_il;
	t_float or = x->x_oh - x->x_ol;
	double oq;
	double result;
	double k;
	if(ir == 0)
	{
		post("scale: input range must not be 0");
		return;
	}
	/* we accept an output range of 0 in case someone really wants this */
	if(!x->x_logcoeff)	/* linear */
	{
		k = (or / ir);
		result = ((f - x->x_il) * k) + x->x_ol;
	}
	else	/* logarythmical scale */
	{
		oq = x->x_oh / x->x_ol;
	//	k = (log((double)oq)/log(x->x_logcoeff))/((double)ir);
		k = log((double)oq)/((double)ir);

		if(x->x_ol)
		{
		//	result = (double)x->x_ol*exp(k*(double)(f - x->x_il)*log(x->x_logcoeff));
			result = (double)x->x_ol*exp(k*(double)(f - x->x_il));
		}
		else
		{
				/* in case the low output is 0 we have to cheat... */
				/* okay, here's the chating: we calculate for a lower out limit  
				   of 1 and remove this shift after the calculation */
			result = ((double)(x->x_ol+1)*exp(k*(double)(f - x->x_il)))-1.0;
		}
	}

	x->x_f = f;         /* save current input value */

	outlet_float(x->x_outlet1, result);
}

static void scale_bang(t_scale *x)
{
	scale_float(x, x->x_f);	/* recalculate result */
}

static t_class *scale_class;

static void *scale_new(t_floatarg fil, t_floatarg fih, t_floatarg fol, t_floatarg foh, t_floatarg flc)
{
    t_scale *x = (t_scale *)pd_new(scale_class);

	floatinlet_new(&x->x_ob, &x->x_il);
	floatinlet_new(&x->x_ob, &x->x_ih);
	floatinlet_new(&x->x_ob, &x->x_ol);
	floatinlet_new(&x->x_ob, &x->x_oh);
	floatinlet_new(&x->x_ob, &x->x_logcoeff);

	x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));

		/* default values taken from Max's scale */
	x->x_il = fil;
	x->x_ih = fih;
	if(!x->x_ih)x->x_ih = 127.0;
	x->x_ol = fol;
	x->x_oh = foh;
	if(!x->x_oh)x->x_oh = 1.0;
	x->x_logcoeff = flc;
	x->x_f = 0;

    return (void *)x;
}

#ifndef MAXLIB
void scale_setup(void)
{
    scale_class = class_new(gensym("scale"), (t_newmethod)scale_new,
    	0, sizeof(t_scale), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(scale_class, scale_float);
    class_addbang(scale_class, scale_bang);
    
    logpost(NULL, 4, version);
#else
void maxlib_scale_setup(void)
{
    scale_class = class_new(gensym("maxlib_scale"), (t_newmethod)scale_new,
    	0, sizeof(t_scale), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)scale_new, gensym("scale"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(scale_class, scale_float);
    class_addbang(scale_class, scale_bang);
    class_sethelpsymbol(scale_class, gensym("maxlib/scale-help.pd"));
#endif
}

