/* ------------------------- listfunnel   ------------------------------------- */
/*                                                                              */
/* Convert list into two-element lists with source index.                       */
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
#include <stdio.h>
#include <stdlib.h>

static char *version = "listfunnel v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct listfunnel
{
  t_object x_ob;
  t_outlet *x_outlet;               /* result */
} t_listfunnel;

static void listfunnel_list(t_listfunnel *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	t_atom list[2];

	for(i = 0; i < argc; i++)
	{
		SETFLOAT(list, i);
		list[1] = argv[i]; // SETFLOAT(list+1, atom_getfloatarg(i, argc, argv));
		outlet_list(x->x_outlet, NULL, 2, list);
	}
}

static void listfunnel_float(t_listfunnel *x, t_floatarg f)
{
	t_atom list[2];

	SETFLOAT(list, 0);
	SETFLOAT(list+1, f);
	outlet_list(x->x_outlet, NULL, 2, list);
}

static t_class *listfunnel_class;

static void *listfunnel_new(void)
{
	int i;

    t_listfunnel *x = (t_listfunnel *)pd_new(listfunnel_class);
	x->x_outlet = outlet_new(&x->x_ob, gensym("float"));

    return (void *)x;
}

#ifndef MAXLIB
void listfunnel_setup(void)
{
    listfunnel_class = class_new(gensym("listfunnel"), (t_newmethod)listfunnel_new,
    	0, sizeof(t_listfunnel), 0, 0, 0);
    class_addfloat(listfunnel_class, listfunnel_float);
    class_addlist(listfunnel_class, listfunnel_list);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_listfunnel_setup(void)
{
    listfunnel_class = class_new(gensym("maxlib_listfunnel"), (t_newmethod)listfunnel_new,
    	0, sizeof(t_listfunnel), 0, 0, 0);
	class_addcreator((t_newmethod)listfunnel_new, gensym("listfunnel"), 0);
    class_addfloat(listfunnel_class, listfunnel_float);
    class_addlist(listfunnel_class, listfunnel_list);
    class_sethelpsymbol(listfunnel_class, gensym("maxlib/listfunnel-help.pd"));
}
#endif

