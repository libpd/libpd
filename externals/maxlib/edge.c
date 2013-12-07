/* ---------------------------  edge  ----------------------------------------- */
/*                                                                              */
/* Detect rising or falling edge of float input.                                */
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

static char *version = "edge v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct edge
{
  t_object x_ob;
  t_outlet *x_out1;                 /* bang on rising edge */
  t_outlet *x_out2;                 /* bang on falling edge */
  t_float  x_lastval;               /* last input value */
} t_edge;

static void edge_float(t_edge *x, t_floatarg f)
{
	if((x->x_lastval <= 0) && (f >= 1))	/* rising edge */
		outlet_bang(x->x_out1);
	else if((x->x_lastval >= 1) && (f <= 0))	/* falling edge */
		outlet_bang(x->x_out2);

	x->x_lastval = f;	/* save last value */
}

static t_class *edge_class;

static void *edge_new(t_floatarg f)
{
	int i;

    t_edge *x = (t_edge *)pd_new(edge_class);
	x->x_out1 = outlet_new(&x->x_ob, gensym("bang"));
	x->x_out2 = outlet_new(&x->x_ob, gensym("bang"));

	x->x_lastval = f;

    return (void *)x;
}

#ifndef MAXLIB
void edge_setup(void)
{
    edge_class = class_new(gensym("edge"), (t_newmethod)edge_new,
    	0, sizeof(t_edge), 0, A_DEFFLOAT, 0);
    class_addfloat(edge_class, edge_float);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_edge_setup(void)
{
    edge_class = class_new(gensym("maxlib_edge"), (t_newmethod)edge_new,
    	0, sizeof(t_edge), 0, A_DEFFLOAT, 0);
    class_addfloat(edge_class, edge_float);
	class_addcreator((t_newmethod)edge_new, gensym("edge"), A_DEFFLOAT, 0);
    class_sethelpsymbol(edge_class, gensym("maxlib/edge-help.pd"));
}
#endif

