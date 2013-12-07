/* ---------------------------- rand_weibull ---------------------------------- */
/*                                                                              */
/* rand_weibull generates a weibull distributed random variable.                */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Based on code found in Dodge/Jerse "Computer Music"                          */
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
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define fran()         (t_float)rand()/(t_float)RAND_MAX
#ifndef M_PI
#define M_PI           3.1415927
#endif

static char *version = "weibull v0.1, generates a weibull distributed random variable\n"
                       "              written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_weibull ------------------------------ */

static t_class *rand_weibull_class;

typedef struct _rand_weibull
{
    t_object x_obj;
	t_float  x_s;
	t_float  x_t;
} t_rand_weibull;

static void *rand_weibull_new(t_floatarg s, t_floatarg t)
{
    t_rand_weibull *x = (t_rand_weibull *)pd_new(rand_weibull_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_s);
    floatinlet_new(&x->x_obj, &x->x_t);
    outlet_new(&x->x_obj, &s_float);
	x->x_s = s;
	x->x_t = t;
    return (x);
}

static void rand_weibull_bang(t_rand_weibull *x)
{
	t_float u, a, t, tinv;
	t = (x->x_t <= 0 ? 0.0001 : x->x_t);
	tinv = 1/t;
	do
	{
		u = fran();
	}
	while(u == 0 || u == 1);
	a = 1/(1 - u);
    outlet_float(x->x_obj.ob_outlet, x->x_s*pow(log(a), tinv));
}

#ifndef MAXLIB
void weibull_setup(void)
{
    rand_weibull_class = class_new(gensym("weibull"), (t_newmethod)rand_weibull_new, 0,
    	sizeof(t_rand_weibull), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
#else
void maxlib_weibull_setup(void)
{
    rand_weibull_class = class_new(gensym("maxlib_weibull"), (t_newmethod)rand_weibull_new, 0,
    	sizeof(t_rand_weibull), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)rand_weibull_new, gensym("weibull"), A_DEFFLOAT, A_DEFFLOAT, 0);
#endif
	class_addbang(rand_weibull_class, rand_weibull_bang);
#ifndef MAXLIB
	class_sethelpsymbol(rand_weibull_class, gensym("weibull-help.pd"));
    logpost(NULL, 4, version);
#else
	class_sethelpsymbol(rand_weibull_class, gensym("maxlib/weibull-help.pd"));
#endif
}
