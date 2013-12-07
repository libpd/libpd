/* ---------------------------- rand_triang ----------------------------------- */
/*                                                                              */
/* rand_triang generates a triangularly distributed random variable.            */
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

static char *version = "triang v0.1, generates triangularly distributed random variable\n"
                       "             written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_triang ------------------------------ */

static t_class *rand_triang_class;

typedef struct _rand_triang
{
    t_object x_obj;
} t_rand_triang;

static void *rand_triang_new(t_floatarg f)
{
    t_rand_triang *x = (t_rand_triang *)pd_new(rand_triang_class);
    srand( (unsigned)time( NULL ) );
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void rand_triang_bang(t_rand_triang *x)
{
	t_float u1, u2;
    u1 = fran();
    u2 = fran();
    outlet_float(x->x_obj.ob_outlet, 0.5*(u1+u2));
}

#ifndef MAXLIB
void triang_setup(void)
{
    rand_triang_class = class_new(gensym("triang"), (t_newmethod)rand_triang_new, 0,
    	sizeof(t_rand_triang), 0, A_DEFFLOAT, 0);
#else
void maxlib_triang_setup(void)
{
    rand_triang_class = class_new(gensym("maxlib_triang"), (t_newmethod)rand_triang_new, 0,
    	sizeof(t_rand_triang), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)rand_triang_new, gensym("triang"), A_DEFFLOAT, 0);
#endif
    class_addbang(rand_triang_class, rand_triang_bang);
#ifndef MAXLIB
	class_sethelpsymbol(rand_triang_class, gensym("triang-help.pd"));
    logpost(NULL, 4, version);
#else
	class_sethelpsymbol(rand_triang_class, gensym("maxlib/triang-help.pd"));
#endif
}
