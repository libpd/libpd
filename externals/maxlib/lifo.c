/* ---------------------------- lifo ------------------------------------------ */
/*                                                                              */
/* lifo buffer of floats, empties itselfe on every bang (in order of coming in) */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
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
/* Fifi-code based St. Rainstick fifo.c for Max,                                */
/* copyright St. Rainstick, Amsterdam 1995                                      */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"

static char *version = "lifo v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct lifo
{
	t_object d_ob;
	t_float *getal;
	t_int size, teller;
	t_outlet *out;

}t_lifo;

static t_class *lifo_class;

static void lifo_int(t_lifo *x, t_floatarg n)
{
    if(x->teller < x->size )
    {
      	x->getal[x->teller] = n;
        x->teller++;
    }
    else
        post("no more lifo memory");

}

static void lifo_bang(t_lifo *x)
{
	if (x->teller > 0)
	{
		outlet_float(x->out,x->getal[x->teller-1]);
		x->teller--;
	}
}

static void lifo_clear(t_lifo *x)
{
	x->teller = 0;
}

static void lifo_free(t_lifo *x)
{
	freebytes(x->getal, x->size * sizeof(t_float));
}

static void *lifo_new(t_floatarg n)
{

	t_lifo *x = (t_lifo *)pd_new(lifo_class);
	if (n<10) n = 10;
	x->size = (t_int)n;
	x->teller = 0;
	x->getal = (t_float *)getbytes(x->size * sizeof(t_float));
	x->out = outlet_new(&x->d_ob, gensym("float"));

	return (x);
}

#ifndef MAXLIB
void lifo_setup(void)
{
    lifo_class = class_new(gensym("lifo"), (t_newmethod)lifo_new,
    	(t_method)lifo_free, sizeof(t_lifo), 0, A_DEFFLOAT, 0);
    class_addfloat(lifo_class, lifo_int);
	class_addbang(lifo_class, lifo_bang);
	class_addmethod(lifo_class, (t_method)lifo_clear, gensym("clear"), 0);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_lifo_setup(void)
{
    lifo_class = class_new(gensym("maxlib_lifo"), (t_newmethod)lifo_new,
    	(t_method)lifo_free, sizeof(t_lifo), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)lifo_new, gensym("lifo"), A_DEFFLOAT, 0);
    class_addfloat(lifo_class, lifo_int);
	class_addbang(lifo_class, lifo_bang);
	class_addmethod(lifo_class, (t_method)lifo_clear, gensym("clear"), 0);
    class_sethelpsymbol(lifo_class, gensym("maxlib/lifo-help.pd"));
}
#endif
