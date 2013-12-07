/* ---------------------------- rand_poisson ---------------------------------- */
/*                                                                              */
/* rand_poisson generates a poisson distributed random variable.                */
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

static char *version = "poisson v0.1, generates a poisson distributed random variable\n"
                       "              written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_poisson ------------------------------ */

static t_class *rand_poisson_class;

typedef struct _rand_poisson
{
    t_object x_obj;
	t_float  x_lambda;
} t_rand_poisson;

static void *rand_poisson_new(t_floatarg f)
{
    t_rand_poisson *x = (t_rand_poisson *)pd_new(rand_poisson_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_lambda);
    outlet_new(&x->x_obj, &s_float);
	x->x_lambda = f;
    return (x);
}

static void rand_poisson_bang(t_rand_poisson *x)
{
	t_float u, v;
	t_int n = 0;
	v = exp(-x->x_lambda);
	u = fran();
	while(u > v)
	{
		u *= fran();
		n++;
	}
    outlet_float(x->x_obj.ob_outlet, n);
}

#ifndef MAXLIB
void poisson_setup(void)
{
    rand_poisson_class = class_new(gensym("poisson"), (t_newmethod)rand_poisson_new, 0,
    	sizeof(t_rand_poisson), 0, A_DEFFLOAT, 0);
    class_addbang(rand_poisson_class, rand_poisson_bang);
	class_sethelpsymbol(rand_poisson_class, gensym("poisson-help.pd"));
    logpost(NULL, 4, version);
}
#else
void maxlib_poisson_setup(void)
{
    rand_poisson_class = class_new(gensym("maxlib_poisson"), (t_newmethod)rand_poisson_new, 0,
    	sizeof(t_rand_poisson), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)rand_poisson_new, gensym("poisson"), A_DEFFLOAT, 0);
    class_addbang(rand_poisson_class, rand_poisson_bang);
	class_sethelpsymbol(rand_poisson_class, gensym("maxlib/poisson-help.pd"));
}
#endif
