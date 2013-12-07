/* ---------------------------- rand_beta ------------------------------------- */
/*                                                                              */
/* rand_beta generates a beta distributed random variable.                      */
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

static char *version = "beta v0.1, generates a beta distributed random variable\n"
                       "           written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_beta ------------------------------ */

static t_class *rand_beta_class;

typedef struct _rand_beta
{
    t_object x_obj;
	t_float  x_a;
	t_float  x_b;
} t_rand_beta;

static void *rand_beta_new(t_floatarg a, t_floatarg b)
{
    t_rand_beta *x = (t_rand_beta *)pd_new(rand_beta_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_a);
    floatinlet_new(&x->x_obj, &x->x_b);
    outlet_new(&x->x_obj, &s_float);
	x->x_a = a;
	x->x_b = b;
    return (x);
}

static void rand_beta_bang(t_rand_beta *x)
{
	t_float u1, u2, y01, y2, sum, a, b, ainv, binv;
	a = (x->x_a <= 0 ? 0.0001 : x->x_a);
	b = (x->x_b <= 0 ? 0.0001 : x->x_b);
	ainv = 1/a;
	binv = 1/b;
	do
	{
		do
		{
			u1 = fran();
		}
		while(u1 == 0);
		do
		{
			u2 = fran();
		}
		while(u2 == 0);
		y01 = pow(u1, ainv);
		y2 = pow(u2, binv);
		sum = y01 + y2;
	}
	while(sum > 1);
    outlet_float(x->x_obj.ob_outlet, y01/sum);
}

#ifndef MAXLIB
void beta_setup(void)
{
    rand_beta_class = class_new(gensym("beta"), (t_newmethod)rand_beta_new, 0,
    	sizeof(t_rand_beta), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(rand_beta_class, rand_beta_bang);
	class_sethelpsymbol(rand_beta_class, gensym("beta-help.pd"));
    logpost(NULL, 4, version);
}
#else
void maxlib_beta_setup(void)
{
    rand_beta_class = class_new(gensym("maxlib_beta"), (t_newmethod)rand_beta_new, 0,
    	sizeof(t_rand_beta), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(rand_beta_class, rand_beta_bang);
	class_addcreator((t_newmethod)rand_beta_new, gensym("beta"), A_DEFFLOAT, A_DEFFLOAT, 0);
	class_sethelpsymbol(rand_beta_class, gensym("maxlib/beta-help.pd"));
}
#endif
