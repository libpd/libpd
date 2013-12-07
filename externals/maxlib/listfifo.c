/* ---------------------------- listfifo -------------------------------------- */
/*                                                                              */
/* Fifo buffer of lists, empties itselfe on every bang (in order of coming in)  */
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
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"

#define MAX_ELEM  256           /* maximum number of list elements to pass on */

static char *version = "listfifo v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct listfifo
{
	t_object d_ob;
	t_atom *getal;				/* stores the list values */
	t_int  *getsize;			/* stores the number of elements in list */
	t_int count, end, size;
	t_outlet *out;

}t_listfifo;

static t_class *listfifo_class;

static void listfifo_list(t_listfifo *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	if(argc > MAX_ELEM)
	{
		post("listfifo: to many arguments in list, ignored");
		return;
	}
	for(i = 0; i < argc; i++)
		x->getal[(x->count * MAX_ELEM) + i] = argv[i];
	x->getsize[x->count] = argc;
	x->count = (x->count + 1) % x->size;
	// post("got %d elements", argc);
}

static void listfifo_bang(t_listfifo *x)
{
	// post("count = %d, end = %d", x->count, x->end);
	if (x->end != x->count){
		outlet_list(x->out, NULL, x->getsize[x->end], x->getal+(x->end * MAX_ELEM));
		x->end = (x->end + 1) % x->size;
	}
}

static void listfifo_free(t_listfifo *x)
{
	freebytes(x->getsize, x->size * sizeof(t_int));
	freebytes(x->getal, x->size * sizeof(t_float) * MAX_ELEM);
}

static void *listfifo_new(t_floatarg n)
{
	int i;

	t_listfifo *x = (t_listfifo *)pd_new(listfifo_class);
	if (n<10) n = 10;
	x->size = (t_int)n;
	x->end = 0;
	x->count = 0;
	x->getsize = (t_int *)getbytes(x->size * sizeof(t_int));
	x->getal = (t_atom *)getbytes(x->size * sizeof(t_atom) * MAX_ELEM);
	x->out = outlet_new(&x->d_ob, gensym("list"));
		
	return (x);
}

#ifndef MAXLIB
void listfifo_setup(void)
{
    listfifo_class = class_new(gensym("listfifo"), (t_newmethod)listfifo_new,
    	(t_method)listfifo_free, sizeof(t_listfifo), 0, A_DEFFLOAT, 0);
	class_addbang(listfifo_class, listfifo_bang);
    class_addlist(listfifo_class, listfifo_list);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_listfifo_setup(void)
{
    listfifo_class = class_new(gensym("maxlib_listfifo"), (t_newmethod)listfifo_new,
    	(t_method)listfifo_free, sizeof(t_listfifo), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)listfifo_new, gensym("listfifo"), A_DEFFLOAT, 0);
	class_addbang(listfifo_class, listfifo_bang);
    class_addlist(listfifo_class, listfifo_list);
    class_sethelpsymbol(listfifo_class, gensym("maxlib/listfifo-help.pd"));
}
#endif
