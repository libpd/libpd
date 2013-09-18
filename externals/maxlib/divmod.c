/* -------------------------  divmod  ----------------------------------------- */
/*                                                                              */
/* Calculates / and % together.                                                 */
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

static char *version = "divmod v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct divmod
{
  t_object x_ob;
  t_inlet  *x_inleft;               /* leftmost inlet */
  t_inlet  *x_inright;              /* right inlet */
  t_outlet *x_outlet1;              /* result of division */
  t_outlet *x_outlet2;              /* result of modulo */

  t_int    x_leftvalue;
  t_int    x_rightvalue;

} t_divmod;

static void divmod_float(t_divmod *x, t_floatarg f)
{
	x->x_leftvalue = (t_int)f;
	outlet_float(x->x_outlet1, x->x_leftvalue / x->x_rightvalue);
	outlet_float(x->x_outlet2, x->x_leftvalue % x->x_rightvalue);
}

static void divmod_ft1(t_divmod *x, t_floatarg f)
{
	x->x_rightvalue = (t_int)f;
	outlet_float(x->x_outlet1, x->x_leftvalue / x->x_rightvalue);
	outlet_float(x->x_outlet2, x->x_leftvalue % x->x_rightvalue);
}

static void divmod_bang(t_divmod *x)
{
	outlet_float(x->x_outlet1, x->x_leftvalue / x->x_rightvalue);
	outlet_float(x->x_outlet2, x->x_leftvalue % x->x_rightvalue);
}

static t_class *divmod_class;

static void *divmod_new(t_floatarg fl, t_floatarg fr)
{
    t_divmod *x = (t_divmod *)pd_new(divmod_class);
    x->x_inright = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));
	x->x_outlet2 = outlet_new(&x->x_ob, gensym("float"));

	x->x_rightvalue = fr;
	x->x_leftvalue = fl;

    return (void *)x;
}

#ifndef MAXLIB
void divmod_setup(void)
{
    divmod_class = class_new(gensym("divmod"), (t_newmethod)divmod_new,
    	0, sizeof(t_divmod), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
#else
void maxlib_divmod_setup(void)
{
    divmod_class = class_new(gensym("maxlib_divmod"), (t_newmethod)divmod_new,
    	0, sizeof(t_divmod), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
#endif
    class_addfloat(divmod_class, divmod_float);
    class_addmethod(divmod_class, (t_method)divmod_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addbang(divmod_class, (t_method)divmod_bang);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)divmod_new, gensym("divmod"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_sethelpsymbol(divmod_class, gensym("maxlib/divmod-help.pd"));
#endif
}

