/* gingerbreadman Attractor PD External */
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

static char *version = "gingerbreadman v0.0, by Michael McGonagle, from ??????, 2003";

t_class *gingerbreadman_class;

typedef struct gingerbreadman_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	
	t_outlet *outlets[M_var_count - 1];
} gingerbreadman_struct;

static void calc(gingerbreadman_struct *gingerbreadman, double *vars) {
	double x_0, y_0;
	x_0 =1-vars[M_y]+abs(vars[M_x]);
	y_0 =vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(gingerbreadman_struct *gingerbreadman) {
	calc(gingerbreadman, gingerbreadman -> vars);
	outlet_float(gingerbreadman -> outlets[M_y - 1], gingerbreadman -> vars[M_y]);
	outlet_float(gingerbreadman -> x_obj.ob_outlet, gingerbreadman -> vars[M_x]);
} // end calculate

static void reset(gingerbreadman_struct *gingerbreadman, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		gingerbreadman -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		gingerbreadman -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		gingerbreadman -> vars[M_x] = gingerbreadman -> vars_init[M_x];
		gingerbreadman -> vars[M_y] = gingerbreadman -> vars_init[M_y];
	} // end if
} // end reset

void *gingerbreadman_new(t_symbol *s, int argc, t_atom *argv) {
	gingerbreadman_struct *gingerbreadman = (gingerbreadman_struct *) pd_new(gingerbreadman_class);
	if (gingerbreadman != NULL) {
		outlet_new(&gingerbreadman -> x_obj, &s_float);
		gingerbreadman -> outlets[0] = outlet_new(&gingerbreadman -> x_obj, &s_float);
		if (argc == M_param_count + M_var_count) {
			gingerbreadman -> vars_init[M_x] = gingerbreadman -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			gingerbreadman -> vars_init[M_y] = gingerbreadman -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for gingerbreadman fractal. Expecting 2 arguments.");
			}
			gingerbreadman -> vars_init[M_x] = -0.1;
			gingerbreadman -> vars_init[M_y] = 0;
		}
	}
	return (void *)gingerbreadman;
}

void gingerbreadman_setup(void) {
	gingerbreadman_class = class_new(gensym("gingerbreadman"), (t_newmethod) gingerbreadman_new, 0, sizeof(gingerbreadman_struct), 0, A_GIMME, 0);
	class_addbang(gingerbreadman_class, (t_method) calculate);
	class_addmethod(gingerbreadman_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	
	
}

