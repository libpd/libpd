/* ---------------------------- rand_expo ------------------------------------- */
/*                                                                              */
/* rand_expo generates a exponentially distributed random variable.             */
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

static char *version = "expo v0.1, generates exponentially distributed random variable\n"
                       "           written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_expo ------------------------------ */

static t_class *rand_expo_class;

typedef struct _rand_expo
{
    t_object x_obj;
	t_float  x_lambda;
} t_rand_expo;

static void *rand_expo_new(t_floatarg f)
{
    t_rand_expo *x = (t_rand_expo *)pd_new(rand_expo_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_lambda);
    outlet_new(&x->x_obj, &s_float);
	x->x_lambda = f;
    return (x);
}

static void rand_expo_bang(t_rand_expo *x)
{
	t_float u, l;
	l = (x->x_lambda <= 0 ? 0.0001 : x->x_lambda);
	do
	{
		u = fran();
	}
	while(u == 0);
    outlet_float(x->x_obj.ob_outlet, -log(u)/l);
}

#ifndef MAXLIB
void expo_setup(void)
{
    rand_expo_class = class_new(gensym("expo"), (t_newmethod)rand_expo_new, 0,
    	sizeof(t_rand_expo), 0, A_DEFFLOAT, 0);
    class_addbang(rand_expo_class, rand_expo_bang);
	class_sethelpsymbol(rand_expo_class, gensym("expo-help.pd"));
    logpost(NULL, 4, version);
}
#else
void maxlib_expo_setup(void)
{
    rand_expo_class = class_new(gensym("maxlib_expo"), (t_newmethod)rand_expo_new, 0,
    	sizeof(t_rand_expo), 0, A_DEFFLOAT, 0);
    class_addbang(rand_expo_class, rand_expo_bang);
	class_addcreator((t_newmethod)rand_expo_new, gensym("expo"), A_DEFFLOAT, 0);
	class_sethelpsymbol(rand_expo_class, gensym("maxlib/expo-help.pd"));
}
#endif
