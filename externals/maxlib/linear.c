/* ---------------------------- rand_linear ----------------------------------- */
/*                                                                              */
/* rand_linear generates a linearly distributed random variable.                */
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

static char *version = "linear v0.1, generates linearly distributed random variable\n"
                       "             written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_linear ------------------------------ */

static t_class *rand_linear_class;

typedef struct _rand_linear
{
    t_object x_obj;
} t_rand_linear;

static void *rand_linear_new(t_floatarg f)
{
    t_rand_linear *x = (t_rand_linear *)pd_new(rand_linear_class);
    srand( (unsigned)time( NULL ) );
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void rand_linear_bang(t_rand_linear *x)
{
	t_float u1, u2;
    u1 = fran();
    u2 = fran();
	if(u2 < u1)
		u1 = u2;
    outlet_float(x->x_obj.ob_outlet, u1);
}

#ifndef MAXLIB
void linear_setup(void)
{
    rand_linear_class = class_new(gensym("linear"), (t_newmethod)rand_linear_new, 0,
    	sizeof(t_rand_linear), 0, A_DEFFLOAT, 0);
    class_addbang(rand_linear_class, rand_linear_bang);
	class_sethelpsymbol(rand_linear_class, gensym("linear-help.pd"));
    logpost(NULL, 4, version);
}		
#else
void maxlib_linear_setup(void)
{
    rand_linear_class = class_new(gensym("maxlib_linear"), (t_newmethod)rand_linear_new, 0,
    	sizeof(t_rand_linear), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)rand_linear_new, gensym("linear"), A_DEFFLOAT, 0);
    class_addbang(rand_linear_class, rand_linear_bang);
	class_sethelpsymbol(rand_linear_class, gensym("maxlib/linear-help.pd"));
}		
#endif
