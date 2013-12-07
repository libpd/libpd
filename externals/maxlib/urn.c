/* -------------------------------  urn  -------------------------------------- */
/*                                                                              */
/* urn - urn selection model (random numbers without repetition).               */
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

static char *version = "urn v0.1, urn selection model\n"
                       "          written by Olaf Matthes <olaf.matthes@gmx.de>";

/* -------------------------- urn ------------------------------ */

static t_class *urn_class;

typedef struct _urn
{
    t_object x_obj;
	t_outlet *x_numberout;
	t_outlet *x_notify;

	t_float  x_f;           /* number of numbers in urn */
	t_int    x_numbers;     /* numbers left in urn */
	t_int    *x_selected;
    unsigned int x_state;
} t_urn;

static int makeseed(void)
{
    static unsigned int random_nextseed = 1489853723;
    random_nextseed = random_nextseed * 435898247 + 938284287;
    return (random_nextseed & 0x7fffffff);
}

static void *urn_new(t_floatarg f)
{
    t_urn *x = (t_urn *)pd_new(urn_class);
    srand( (unsigned)time( NULL ) );
	x->x_numbers = x->x_f = f;
	if(x->x_f < 0)x->x_f = 0;
	x->x_selected = getbytes(((t_int)x->x_f+1)*sizeof(t_int));

    x->x_state = makeseed();

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("f"));

    x->x_numberout = outlet_new(&x->x_obj, &s_float);
    x->x_notify = outlet_new(&x->x_obj, &s_bang);
    return (x);
}

	/* set new size of urn */
static void urn_f(t_urn *x, t_floatarg f)
{
	int i;
	if(f < 0)
		f = 0;
	freebytes(x->x_selected, ((t_int)x->x_f+1)*sizeof(t_int));
	x->x_numbers = x->x_f = f;
	x->x_selected = getbytes(((t_int)x->x_f+1)*sizeof(t_int));
	for(i = 0; i <= x->x_f; i++)
		x->x_selected[i] = 0;
}

	/* clear (refill) urn */
static void urn_clear(t_urn *x)
{
	int i;
	x->x_numbers = x->x_f;
	for(i = 0; i <= x->x_f; i++)
		x->x_selected[i] = 0;
}

static void urn_seed(t_urn *x, float f, float glob)
{
    x->x_state = f;
}

	/* choose from urn */
static void urn_bang(t_urn *x)
{
    int n = x->x_f, nval;
    int range = (n < 1 ? 1 : n);
    unsigned int randval = x->x_state;
	if(x->x_numbers == 0)
		goto notify;
	do
	{
		x->x_state = randval = randval * 472940017 + 832416023;
		nval = ((double)range) * ((double)randval)
    		* (1./4294967296.);
		if (nval >= range) nval = range-1;
	}
	while(x->x_selected[nval]);

	x->x_selected[nval] = 1;

    outlet_float(x->x_numberout, nval);

	if(--x->x_numbers == 0)	/* urn is now empty */
		goto notify;
	return;

notify:
	outlet_bang(x->x_notify);
}

#ifndef MAXLIB
void urn_setup(void)
{
    urn_class = class_new(gensym("urn"), (t_newmethod)urn_new, 0,
    	sizeof(t_urn), 0, A_DEFFLOAT, 0);
#else
void maxlib_urn_setup(void)
{
    urn_class = class_new(gensym("maxlib_urn"), (t_newmethod)urn_new, 0,
    	sizeof(t_urn), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)urn_new, gensym("urn"), A_DEFFLOAT, 0);
#endif
    class_addbang(urn_class, urn_bang);
	class_addmethod(urn_class, (t_method)urn_f, gensym("f"), A_FLOAT, 0);
	class_addmethod(urn_class, (t_method)urn_clear, gensym("clear"), 0);
    class_addmethod(urn_class, (t_method)urn_seed, gensym("seed"), A_FLOAT, 0);
#ifndef MAXLIB
	
    logpost(NULL, 4, version);
#else
	class_sethelpsymbol(urn_class, gensym("maxlib/urn-help.pd"));
#endif
}
