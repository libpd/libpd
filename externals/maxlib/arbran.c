/* ---------------------------- rand_arbran ----------------------------------- */
/*                                                                              */
/* rand_arbran generates a random variable that conforms to the                 */
/* piecewise probability density in two arrays                                  */
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

static char *version = "arbran v0.1b, generates a random variable that conforms to the\n"
                       "              piecewise probability density in two arrays\n"
                       "              written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- rand_arbran ------------------------------ */

static t_class *rand_arbran_class;

typedef struct _rand_arbran
{
    t_object x_obj;
	t_symbol *x_x;
	t_symbol *x_p;
    t_garray *x_bufx;
    t_garray *x_bufp;
} t_rand_arbran;

static void rand_arbran_pdfscale(t_rand_arbran *x)
{
	t_garray *bx = x->x_bufx, *bp = x->x_bufp;
	t_float a = 0;
	t_int k = 0;
	t_float *tx, *tp;
	int ix, ip;
	if (!garray_getfloatarray(bx, &ix, &tx))
	{
		post("arbran: couldn't read from array!");
		return;
	}
	if (!garray_getfloatarray(bp, &ip, &tp))
	{
		post("arbran: couldn't read from array!");
		return;
	}

	for(k = 1; k < ix; k++)
	{
		a += (tx[k]-tx[k-1])*(tp[k]+tp[k-1])/2.0;
	}
	for(k = 0; k < ix; k++)
	{
		tp[k] = tp[k]/a;
	}
	garray_redraw(x->x_bufp);
}

static void rand_arbran_bang(t_rand_arbran *x)
{
	t_garray *bx = x->x_bufx, *bp = x->x_bufp;
	t_float a, u, a0, slope, b, d, r;
	t_int k = 0;
	t_float *tx, *tp;
	int ix, ip;
	if (!garray_getfloatarray(bx, &ix, &tx))
	{
		post("arbran: couldn't read from array!");
		return;
	}
	if (!garray_getfloatarray(bp, &ip, &tp))
	{
		post("arbran: couldn't read from array!");
		return;
	}

	a = 0;
	a0 = 0;
	u = fran();
	while(u > a)
	{
		a0 = (tx[k+1]-tx[k])*(tp[k+1]+tp[k])/2.0;
		a += a0;
		k++;
	}
	k--;
	slope = (tp[k+1]-tp[k])/(tx[k+1]-tx[k]);
	if(slope == 0)
	{
		r = (u-a+a0)/tp[k]+tx[k];
	}
	else
	{
		b=tp[k]/slope-tx[k];
		d=b*b+tx[k]*tx[k]+2*b*tx[k]+2*(u-a+a0)/slope;
		if(slope > 0)
			r=-b+sqrt(d);
		else
			r=-b-sqrt(d);
	}
    outlet_float(x->x_obj.ob_outlet, r);
}

static void rand_arbran_set(t_rand_arbran *x)
{
	t_garray *b, *b2;
	
	if ((b = (t_garray *)pd_findbyclass(x->x_x, garray_class)))
	{
		post("arbran: array set to \"%s\"", x->x_x->s_name);
		x->x_bufx = b;
	} else {
		post("arbran: no array \"%s\" (error %d)", x->x_x->s_name, b);
		x->x_bufx = 0;
	}
	if ((b2 = (t_garray *)pd_findbyclass(x->x_p, garray_class)))
	{
		post("arbran: array set to \"%s\"", x->x_p->s_name);
		x->x_bufp = b2;
	} else {
		post("arbran: no array \"%s\" (error %d)", x->x_p->s_name, b);
		x->x_bufp = 0;
	}
}

static void rand_arbran_setarrays(t_rand_arbran *x, t_symbol *s1, t_symbol *s2)
{
	x->x_x = s1;
	x->x_p = s2;
	rand_arbran_set(x);
}

static void *rand_arbran_new(t_symbol *s1, t_symbol *s2)
{
    t_rand_arbran *x = (t_rand_arbran *)pd_new(rand_arbran_class);
    srand( (unsigned)time( NULL ) );
    outlet_new(&x->x_obj, &s_float);
	x->x_x = s1;
	x->x_p = s2;
	rand_arbran_set(x);
    return (x);
}

#ifndef MAXLIB
void arbran_setup(void)
{
    rand_arbran_class = class_new(gensym("arbran"), (t_newmethod)rand_arbran_new, 0,
    	sizeof(t_rand_arbran), 0, A_SYMBOL, A_SYMBOL, 0);
#else
void maxlib_arbran_setup(void)
{
    rand_arbran_class = class_new(gensym("maxlib_arbran"), (t_newmethod)rand_arbran_new, 0,
    	sizeof(t_rand_arbran), 0, A_SYMBOL, A_SYMBOL, 0);
#endif
    class_addbang(rand_arbran_class, rand_arbran_bang);
	class_addmethod(rand_arbran_class, (t_method)rand_arbran_pdfscale, gensym("pdfscale"), 0);
	class_addmethod(rand_arbran_class, (t_method)rand_arbran_setarrays, gensym("set"), A_SYMBOL, A_SYMBOL, 0);
#ifndef MAXLIB
	class_sethelpsymbol(rand_arbran_class, gensym("arbran-help.pd"));
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)rand_arbran_new, gensym("arbran"), A_SYMBOL, A_SYMBOL, 0);
	class_sethelpsymbol(rand_arbran_class, gensym("maxlib/arbran-help.pd"));
#endif
}
