/* unity Attractor PD External */
/* Copyright Michael McGonagle, from ??????, 2003 */
/* This program is distributed under the params of the GNU Public License */

///////////////////////////////////////////////////////////////////////////////////
/* This file is part of Chaos PD Externals.                                      */
/*                                                                               */
/* Chaos PD Externals are free software; you can redistribute them and/or modify */
/* them under the terms of the GNU General Public License as published by        */
/* the Free Software Foundation; either version 2 of the License, or             */
/* (at your option) any later version.                                           */
/*                                                                               */
/* Chaos PD Externals are distributed in the hope that they will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 */
/* GNU General Public License for more details.                                  */
/*                                                                               */
/* You should have received a copy of the GNU General Public License             */
/* along with the Chaos PD Externals; if not, write to the Free Software         */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     */
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "m_pd.h"



#define M_x 0
#define M_y 1

#define M_param_count 0
#define M_var_count 2
#define M_search_count 0
#define M_failure_limit 1000

static char *version = "unity v0.0, by Michael McGonagle, from ??????, 2003";

t_class *unity_class;

typedef struct unity_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	
	t_outlet *outlets[M_var_count - 1];
} unity_struct;

static void calc(unity_struct *unity, double *vars) {
	double k, x_0, y_0;
	k=(vars[M_x]*vars[M_x])+(vars[M_y]*vars[M_y]);
	x_0 =(2-k)*vars[M_y];
	y_0 =(2-k)*vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(unity_struct *unity) {
	calc(unity, unity -> vars);
	outlet_float(unity -> outlets[M_y - 1], unity -> vars[M_y]);
	outlet_float(unity -> x_obj.ob_outlet, unity -> vars[M_x]);
} // end calculate

static void reset(unity_struct *unity, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		unity -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		unity -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		unity -> vars[M_x] = unity -> vars_init[M_x];
		unity -> vars[M_y] = unity -> vars_init[M_y];
	} // end if
} // end reset

void *unity_new(t_symbol *s, int argc, t_atom *argv) {
	unity_struct *unity = (unity_struct *) pd_new(unity_class);
	if (unity != NULL) {
		outlet_new(&unity -> x_obj, &s_float);
		unity -> outlets[0] = outlet_new(&unity -> x_obj, &s_float);
		if (argc == M_param_count + M_var_count) {
			unity -> vars_init[M_x] = unity -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			unity -> vars_init[M_y] = unity -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for unity fractal. Expecting 2 arguments.");
			}
			unity -> vars_init[M_x] = 0;
			unity -> vars_init[M_y] = 0;
		}
	}
	return (void *)unity;
}

void unity_setup(void) {
	unity_class = class_new(gensym("unity"), (t_newmethod) unity_new, 0, sizeof(unity_struct), 0, A_GIMME, 0);
	class_addbang(unity_class, (t_method) calculate);
	class_addmethod(unity_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	
	
}

