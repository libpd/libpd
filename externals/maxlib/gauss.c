/* ---------------------------- rand_gauss ----------------------------------- */
/*                                                                              */
/* rand_gauss generates a gauss distributed random variable.                  */
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

static char *version = "gauss v0.1, generates a Gaussian distributed random variable\n"
                       "            with mean 'mu' and standard deviation 'sigma',\n"
                       "            written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_gauss ------------------------------ */

static t_class *rand_gauss_class;

typedef struct _rand_gauss
{
    t_object x_obj;
	t_float  x_sigma;
	t_float  x_mu;
} t_rand_gauss;

static void *rand_gauss_new(t_floatarg fs, t_floatarg fm)
{
    t_rand_gauss *x = (t_rand_gauss *)pd_new(rand_gauss_class);
    srand( (unsigned)time( NULL ) );
    floatinlet_new(&x->x_obj, &x->x_sigma);
    floatinlet_new(&x->x_obj, &x->x_mu);
    outlet_new(&x->x_obj, &s_float);
	x->x_sigma = fs;
    return (x);
}

static void rand_gauss_bang(t_rand_gauss *x)
{
	t_float u, halfN = 6.0, sum = 0, scale;
	t_int k, N = 12;
	scale = 1/sqrt(N/12);
	for(k = 1; k <= N; k++)
		sum += fran();
    outlet_float(x->x_obj.ob_outlet, x->x_sigma*scale*(sum-halfN)+x->x_mu);
}

#ifndef MAXLIB
void gauss_setup(void)
{
    rand_gauss_class = class_new(gensym("gauss"), (t_newmethod)rand_gauss_new, 0,
    	sizeof(t_rand_gauss), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(rand_gauss_class, rand_gauss_bang);
	class_sethelpsymbol(rand_gauss_class, gensym("gauss-help.pd"));
    logpost(NULL, 4, version);
}
#else
void maxlib_gauss_setup(void)
{
    rand_gauss_class = class_new(gensym("maxlib_gauss"), (t_newmethod)rand_gauss_new, 0,
    	sizeof(t_rand_gauss), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)rand_gauss_new, gensym("gauss"), A_DEFFLOAT, 0);
    class_addbang(rand_gauss_class, rand_gauss_bang);
	class_sethelpsymbol(rand_gauss_class, gensym("maxlib/gauss-help.pd"));
}
#endif

