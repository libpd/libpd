/* ---------------------------- rand_cauchy ----------------------------------- */
/*                                                                              */
/* rand_cauchy generates a Cauchy distributed random variable.                  */
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

static char *version = "cauchy v0.1, generates a Cauchy distributed random variable\n"
                       "             with a spread governed by to parameter 'aplha',\n"
                       "             written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_cauchy ------------------------------ */

static t_class *rand_cauchy_class;

typedef struct _rand_cauchy
{
    t_object x_obj;
	t_float  x_alpha;
} t_rand_cauchy;

static void *rand_cauchy_new(t_floatarg f)
{
    t_rand_cauchy *x = (t_rand_cauchy *)pd_new(rand_cauchy_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_alpha);
    outlet_new(&x->x_obj, &s_float);
	x->x_alpha = f;
    return (x);
}

static void rand_cauchy_bang(t_rand_cauchy *x)
{
	t_float u;
	do
	{
		u = fran();
	}
	while(u == 0.5);
	u *= M_PI;
    outlet_float(x->x_obj.ob_outlet, x->x_alpha*tan(u));
}

#ifndef MAXLIB
void cauchy_setup(void)
{
    rand_cauchy_class = class_new(gensym("cauchy"), (t_newmethod)rand_cauchy_new, 0,
    	sizeof(t_rand_cauchy), 0, A_DEFFLOAT, 0);
    class_addbang(rand_cauchy_class, rand_cauchy_bang);
	class_sethelpsymbol(rand_cauchy_class, gensym("cauchy-help.pd"));
    logpost(NULL, 4, version);
#else
void maxlib_cauchy_setup(void)
{
    rand_cauchy_class = class_new(gensym("maxlib_cauchy"), (t_newmethod)rand_cauchy_new, 0,
    	sizeof(t_rand_cauchy), 0, A_DEFFLOAT, 0);
    class_addbang(rand_cauchy_class, rand_cauchy_bang);
	class_addcreator((t_newmethod)rand_cauchy_new, gensym("cauchy"), A_DEFFLOAT, 0);
	class_sethelpsymbol(rand_cauchy_class, gensym("maxlib/cauchy-help.pd"));
#endif
}
