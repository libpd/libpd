/* ---------------------------- fifo ------------------------------------------ */
/*                                                                              */
/* Fifo buffer of floats, empties itselfe on every bang (in order of coming in) */
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
/* Fifi-code based St. Rainstick fifi.c for Max,                                */
/* copyright St. Rainstick, Amsterdam 1995                                      */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"

static char *version = "fifo v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct fifo
{
	t_object d_ob;
	t_float *getal;
	t_int count, end, size;
	t_outlet *out;

}t_fifo;

static t_class *fifo_class;

static void fifo_int(t_fifo *x, t_floatarg n)
{
	x->getal[x->count] = n;
	x->count = (x->count + 1) % x->size;
}

static void fifo_bang(t_fifo *x)
{
	if (x->end != x->count){
		outlet_float(x->out,x->getal[x->end]);
		x->end = (x->end + 1) % x->size;
	}
}

static void fifo_free(t_fifo *x)
{
	freebytes(x->getal, x->size * sizeof(t_float));
}

static void *fifo_new(t_floatarg n)
{

	t_fifo *x = (t_fifo *)pd_new(fifo_class);
	if (n<10) n = 10;
	x->size = (t_int)n;
	x->end = 0;
	x->count = 0;
	x->getal = (t_float *)getbytes(x->size * sizeof(t_float));
	x->out = outlet_new(&x->d_ob, gensym("float"));
		
	return (x);
}

#ifndef MAXLIB
void fifo_setup(void)
{
    fifo_class = class_new(gensym("fifo"), (t_newmethod)fifo_new,
    	(t_method)fifo_free, sizeof(t_fifo), 0, A_DEFFLOAT, 0);
    class_addfloat(fifo_class, fifo_int);
	class_addbang(fifo_class, fifo_bang);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_fifo_setup(void)
{
    fifo_class = class_new(gensym("maxlib_fifo"), (t_newmethod)fifo_new,
    	(t_method)fifo_free, sizeof(t_fifo), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)fifo_new, gensym("fifo"), A_DEFFLOAT, 0);
    class_addfloat(fifo_class, fifo_int);
	class_addbang(fifo_class, fifo_bang);
    class_sethelpsymbol(fifo_class, gensym("maxlib/fifo-help.pd"));
}
#endif
