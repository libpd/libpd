/* ---------------------------- rand_bilex ------------------------------------ */
/*                                                                              */
/* rand_bilex generates a bilinear exponentially distributed random variable.   */
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

static char *version = "bilex v0.1, generates bilinear exponentially distributed random\n"
                       "            variable, written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_bilex ------------------------------ */

static t_class *rand_bilex_class;

typedef struct _rand_bilex
{
    t_object x_obj;
	t_float  x_lambda;
} t_rand_bilex;

static void *rand_bilex_new(t_floatarg f)
{
    t_rand_bilex *x = (t_rand_bilex *)pd_new(rand_bilex_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_lambda);
    outlet_new(&x->x_obj, &s_float);
	x->x_lambda = f;
    return (x);
}

static void rand_bilex_bang(t_rand_bilex *x)
{
	t_float u, s = 1, l;
	l = (x->x_lambda <= 0 ? 0.0001 : x->x_lambda);
	do
	{
		u = 2*fran();
	}
	while(u == 0 || u == 2);
	if(u > 1)
	{
		u = 2-u;
		s=-1;
	}
    outlet_float(x->x_obj.ob_outlet, s*log(u)/l);
}

#ifndef MAXLIB
void bilex_setup(void)
{
    rand_bilex_class = class_new(gensym("bilex"), (t_newmethod)rand_bilex_new, 0,
    	sizeof(t_rand_bilex), 0, A_DEFFLOAT, 0);
    class_addbang(rand_bilex_class, rand_bilex_bang);
	class_sethelpsymbol(rand_bilex_class, gensym("bilex-help.pd"));
    logpost(NULL, 4, version);
}
#else
void maxlib_bilex_setup(void)
{
    rand_bilex_class = class_new(gensym("maxlib_bilex"), (t_newmethod)rand_bilex_new, 0,
    	sizeof(t_rand_bilex), 0, A_DEFFLOAT, 0);
    class_addbang(rand_bilex_class, rand_bilex_bang);
	class_addcreator((t_newmethod)rand_bilex_new, gensym("bilex"), A_DEFFLOAT, 0);
	class_sethelpsymbol(rand_bilex_class, gensym("maxlib/bilex-help.pd"));
}
#endif
