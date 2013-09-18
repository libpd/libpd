/* strange1 Attractor PD External */
/* Copyright Michael McGonagle, from ???pbourke???, 2003 */
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
#include "chaos.h"

#define M_a0_lo -2
#define M_a0_hi 2
#define M_a1_lo -2
#define M_a1_hi 2
#define M_a2_lo -2
#define M_a2_hi 2
#define M_a3_lo -2
#define M_a3_hi 2
#define M_a4_lo -2
#define M_a4_hi 2
#define M_a5_lo -2
#define M_a5_hi 2
#define M_b0_lo -2
#define M_b0_hi 2
#define M_b1_lo -2
#define M_b1_hi 2
#define M_b2_lo -2
#define M_b2_hi 2
#define M_b3_lo -2
#define M_b3_hi 2
#define M_b4_lo -2
#define M_b4_hi 2
#define M_b5_lo -2
#define M_b5_hi 2

#define M_a0 0
#define M_a1 1
#define M_a2 2
#define M_a3 3
#define M_a4 4
#define M_a5 5
#define M_b0 6
#define M_b1 7
#define M_b2 8
#define M_b3 9
#define M_b4 10
#define M_b5 11

#define M_x 0
#define M_y 1

#define M_param_count 12
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "strange1 v0.0, by Michael McGonagle, from ???pbourke???, 2003";

t_class *strange1_class;

typedef struct strange1_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a0, a0_lo, a0_hi, a1, a1_lo, a1_hi, a2, a2_lo, a2_hi, a3, a3_lo, a3_hi, a4, a4_lo, a4_hi, a5, a5_lo, a5_hi, b0, b0_lo, b0_hi, b1, b1_lo, b1_hi, b2, b2_lo, b2_hi, b3, b3_lo, b3_hi, b4, b4_lo, b4_hi, b5, b5_lo, b5_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} strange1_struct;

static void calc(strange1_struct *strange1, double *vars) {
	double x_0, y_0;
	x_0 =strange1 -> a0+strange1 -> a1*vars[M_x]+strange1 -> a2*vars[M_x]*vars[M_x]+strange1 -> a3*vars[M_x]*vars[M_y]+strange1 -> a4*vars[M_y]+strange1 -> a5*vars[M_y]*vars[M_y];
	y_0 =strange1 -> b0+strange1 -> b1*vars[M_x]+strange1 -> b2*vars[M_x]*vars[M_x]+strange1 -> b3*vars[M_x]*vars[M_y]+strange1 -> b4*vars[M_y]+strange1 -> b5*vars[M_y]*vars[M_y];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(strange1_struct *strange1) {
	calc(strange1, strange1 -> vars);
	outlet_float(strange1 -> outlets[M_y - 1], strange1 -> vars[M_y]);
	outlet_float(strange1 -> x_obj.ob_outlet, strange1 -> vars[M_x]);
} // end calculate

static void reset(strange1_struct *strange1, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		strange1 -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		strange1 -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		strange1 -> vars[M_x] = strange1 -> vars_init[M_x];
		strange1 -> vars[M_y] = strange1 -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(strange1_struct *strange1) {
	static char buff[13];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((strange1 -> a0 - M_a0_lo) * (1.0 / (M_a0_hi - M_a0_lo))) * 26)];
	buff[1] = c[(int) (((strange1 -> a1 - M_a1_lo) * (1.0 / (M_a1_hi - M_a1_lo))) * 26)];
	buff[2] = c[(int) (((strange1 -> a2 - M_a2_lo) * (1.0 / (M_a2_hi - M_a2_lo))) * 26)];
	buff[3] = c[(int) (((strange1 -> a3 - M_a3_lo) * (1.0 / (M_a3_hi - M_a3_lo))) * 26)];
	buff[4] = c[(int) (((strange1 -> a4 - M_a4_lo) * (1.0 / (M_a4_hi - M_a4_lo))) * 26)];
	buff[5] = c[(int) (((strange1 -> a5 - M_a5_lo) * (1.0 / (M_a5_hi - M_a5_lo))) * 26)];
	buff[6] = c[(int) (((strange1 -> b0 - M_b0_lo) * (1.0 / (M_b0_hi - M_b0_lo))) * 26)];
	buff[7] = c[(int) (((strange1 -> b1 - M_b1_lo) * (1.0 / (M_b1_hi - M_b1_lo))) * 26)];
	buff[8] = c[(int) (((strange1 -> b2 - M_b2_lo) * (1.0 / (M_b2_hi - M_b2_lo))) * 26)];
	buff[9] = c[(int) (((strange1 -> b3 - M_b3_lo) * (1.0 / (M_b3_hi - M_b3_lo))) * 26)];
	buff[10] = c[(int) (((strange1 -> b4 - M_b4_lo) * (1.0 / (M_b4_hi - M_b4_lo))) * 26)];
	buff[11] = c[(int) (((strange1 -> b5 - M_b5_lo) * (1.0 / (M_b5_hi - M_b5_lo))) * 26)];
	buff[12] = '\0';
	return buff;
}

static void make_results(strange1_struct *strange1) {
	SETFLOAT(&strange1 -> search_out[0], strange1 -> lyap_exp);
	SETSYMBOL(&strange1 -> search_out[1], gensym(classify(strange1)));
	SETFLOAT(&strange1 -> search_out[2], strange1 -> failure_ratio);
	SETFLOAT(&strange1 -> vars_out[M_x], strange1 -> vars[M_x]);
	SETFLOAT(&strange1 -> vars_out[M_y], strange1 -> vars[M_y]);
	SETFLOAT(&strange1 -> params_out[M_a0], strange1 -> a0);
	SETFLOAT(&strange1 -> params_out[M_a1], strange1 -> a1);
	SETFLOAT(&strange1 -> params_out[M_a2], strange1 -> a2);
	SETFLOAT(&strange1 -> params_out[M_a3], strange1 -> a3);
	SETFLOAT(&strange1 -> params_out[M_a4], strange1 -> a4);
	SETFLOAT(&strange1 -> params_out[M_a5], strange1 -> a5);
	SETFLOAT(&strange1 -> params_out[M_b0], strange1 -> b0);
	SETFLOAT(&strange1 -> params_out[M_b1], strange1 -> b1);
	SETFLOAT(&strange1 -> params_out[M_b2], strange1 -> b2);
	SETFLOAT(&strange1 -> params_out[M_b3], strange1 -> b3);
	SETFLOAT(&strange1 -> params_out[M_b4], strange1 -> b4);
	SETFLOAT(&strange1 -> params_out[M_b5], strange1 -> b5);
	outlet_list(strange1 -> params_outlet, gensym("list"), M_param_count, strange1 -> params_out);
	outlet_list(strange1 -> vars_outlet, gensym("list"), M_var_count, strange1 -> vars_out);
}

static void show(strange1_struct *strange1) {
	make_results(strange1);
	outlet_anything(strange1 -> search_outlet, gensym("show"), M_search_count, strange1 -> search_out);
}

static void param(strange1_struct *strange1, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 12) {
		post("Incorrect number of arguments for strange1 fractal. Expecting 12 arguments.");
		return;
	}
	strange1 -> a0 = (double) atom_getfloatarg(0, argc, argv);
	strange1 -> a1 = (double) atom_getfloatarg(1, argc, argv);
	strange1 -> a2 = (double) atom_getfloatarg(2, argc, argv);
	strange1 -> a3 = (double) atom_getfloatarg(3, argc, argv);
	strange1 -> a4 = (double) atom_getfloatarg(4, argc, argv);
	strange1 -> a5 = (double) atom_getfloatarg(5, argc, argv);
	strange1 -> b0 = (double) atom_getfloatarg(6, argc, argv);
	strange1 -> b1 = (double) atom_getfloatarg(7, argc, argv);
	strange1 -> b2 = (double) atom_getfloatarg(8, argc, argv);
	strange1 -> b3 = (double) atom_getfloatarg(9, argc, argv);
	strange1 -> b4 = (double) atom_getfloatarg(10, argc, argv);
	strange1 -> b5 = (double) atom_getfloatarg(11, argc, argv);
}

static void seed(strange1_struct *strange1, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(strange1_struct *strange1, t_floatarg l, t_floatarg h, t_floatarg lim) {
	strange1 -> lyap_lo = l;
	strange1 -> lyap_hi = h;
	strange1 -> lyap_limit = (double) ((int) lim);
}

static void elyap(strange1_struct *strange1) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) strange1, (t_gotfn) calc, M_var_count, strange1 -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(strange1_struct *strange1) {
	if (strange1 -> a0_lo < M_a0_lo) { strange1 -> a0_lo = M_a0_lo; }
	if (strange1 -> a0_lo > M_a0_hi) { strange1 -> a0_lo = M_a0_hi; }
	if (strange1 -> a0_hi < M_a0_lo) { strange1 -> a0_hi = M_a0_lo; }
	if (strange1 -> a0_hi > M_a0_hi) { strange1 -> a0_hi = M_a0_hi; }
	if (strange1 -> a1_lo < M_a1_lo) { strange1 -> a1_lo = M_a1_lo; }
	if (strange1 -> a1_lo > M_a1_hi) { strange1 -> a1_lo = M_a1_hi; }
	if (strange1 -> a1_hi < M_a1_lo) { strange1 -> a1_hi = M_a1_lo; }
	if (strange1 -> a1_hi > M_a1_hi) { strange1 -> a1_hi = M_a1_hi; }
	if (strange1 -> a2_lo < M_a2_lo) { strange1 -> a2_lo = M_a2_lo; }
	if (strange1 -> a2_lo > M_a2_hi) { strange1 -> a2_lo = M_a2_hi; }
	if (strange1 -> a2_hi < M_a2_lo) { strange1 -> a2_hi = M_a2_lo; }
	if (strange1 -> a2_hi > M_a2_hi) { strange1 -> a2_hi = M_a2_hi; }
	if (strange1 -> a3_lo < M_a3_lo) { strange1 -> a3_lo = M_a3_lo; }
	if (strange1 -> a3_lo > M_a3_hi) { strange1 -> a3_lo = M_a3_hi; }
	if (strange1 -> a3_hi < M_a3_lo) { strange1 -> a3_hi = M_a3_lo; }
	if (strange1 -> a3_hi > M_a3_hi) { strange1 -> a3_hi = M_a3_hi; }
	if (strange1 -> a4_lo < M_a4_lo) { strange1 -> a4_lo = M_a4_lo; }
	if (strange1 -> a4_lo > M_a4_hi) { strange1 -> a4_lo = M_a4_hi; }
	if (strange1 -> a4_hi < M_a4_lo) { strange1 -> a4_hi = M_a4_lo; }
	if (strange1 -> a4_hi > M_a4_hi) { strange1 -> a4_hi = M_a4_hi; }
	if (strange1 -> a5_lo < M_a5_lo) { strange1 -> a5_lo = M_a5_lo; }
	if (strange1 -> a5_lo > M_a5_hi) { strange1 -> a5_lo = M_a5_hi; }
	if (strange1 -> a5_hi < M_a5_lo) { strange1 -> a5_hi = M_a5_lo; }
	if (strange1 -> a5_hi > M_a5_hi) { strange1 -> a5_hi = M_a5_hi; }
	if (strange1 -> b0_lo < M_b0_lo) { strange1 -> b0_lo = M_b0_lo; }
	if (strange1 -> b0_lo > M_b0_hi) { strange1 -> b0_lo = M_b0_hi; }
	if (strange1 -> b0_hi < M_b0_lo) { strange1 -> b0_hi = M_b0_lo; }
	if (strange1 -> b0_hi > M_b0_hi) { strange1 -> b0_hi = M_b0_hi; }
	if (strange1 -> b1_lo < M_b1_lo) { strange1 -> b1_lo = M_b1_lo; }
	if (strange1 -> b1_lo > M_b1_hi) { strange1 -> b1_lo = M_b1_hi; }
	if (strange1 -> b1_hi < M_b1_lo) { strange1 -> b1_hi = M_b1_lo; }
	if (strange1 -> b1_hi > M_b1_hi) { strange1 -> b1_hi = M_b1_hi; }
	if (strange1 -> b2_lo < M_b2_lo) { strange1 -> b2_lo = M_b2_lo; }
	if (strange1 -> b2_lo > M_b2_hi) { strange1 -> b2_lo = M_b2_hi; }
	if (strange1 -> b2_hi < M_b2_lo) { strange1 -> b2_hi = M_b2_lo; }
	if (strange1 -> b2_hi > M_b2_hi) { strange1 -> b2_hi = M_b2_hi; }
	if (strange1 -> b3_lo < M_b3_lo) { strange1 -> b3_lo = M_b3_lo; }
	if (strange1 -> b3_lo > M_b3_hi) { strange1 -> b3_lo = M_b3_hi; }
	if (strange1 -> b3_hi < M_b3_lo) { strange1 -> b3_hi = M_b3_lo; }
	if (strange1 -> b3_hi > M_b3_hi) { strange1 -> b3_hi = M_b3_hi; }
	if (strange1 -> b4_lo < M_b4_lo) { strange1 -> b4_lo = M_b4_lo; }
	if (strange1 -> b4_lo > M_b4_hi) { strange1 -> b4_lo = M_b4_hi; }
	if (strange1 -> b4_hi < M_b4_lo) { strange1 -> b4_hi = M_b4_lo; }
	if (strange1 -> b4_hi > M_b4_hi) { strange1 -> b4_hi = M_b4_hi; }
	if (strange1 -> b5_lo < M_b5_lo) { strange1 -> b5_lo = M_b5_lo; }
	if (strange1 -> b5_lo > M_b5_hi) { strange1 -> b5_lo = M_b5_hi; }
	if (strange1 -> b5_hi < M_b5_lo) { strange1 -> b5_hi = M_b5_lo; }
	if (strange1 -> b5_hi > M_b5_hi) { strange1 -> b5_hi = M_b5_hi; }
}

static void constrain(strange1_struct *strange1, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		strange1 -> a0_lo = M_a0_lo;
		strange1 -> a0_hi = M_a0_hi;
		strange1 -> a1_lo = M_a1_lo;
		strange1 -> a1_hi = M_a1_hi;
		strange1 -> a2_lo = M_a2_lo;
		strange1 -> a2_hi = M_a2_hi;
		strange1 -> a3_lo = M_a3_lo;
		strange1 -> a3_hi = M_a3_hi;
		strange1 -> a4_lo = M_a4_lo;
		strange1 -> a4_hi = M_a4_hi;
		strange1 -> a5_lo = M_a5_lo;
		strange1 -> a5_hi = M_a5_hi;
		strange1 -> b0_lo = M_b0_lo;
		strange1 -> b0_hi = M_b0_hi;
		strange1 -> b1_lo = M_b1_lo;
		strange1 -> b1_hi = M_b1_hi;
		strange1 -> b2_lo = M_b2_lo;
		strange1 -> b2_hi = M_b2_hi;
		strange1 -> b3_lo = M_b3_lo;
		strange1 -> b3_hi = M_b3_hi;
		strange1 -> b4_lo = M_b4_lo;
		strange1 -> b4_hi = M_b4_hi;
		strange1 -> b5_lo = M_b5_lo;
		strange1 -> b5_hi = M_b5_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a0_spread = ((M_a0_hi - M_a0_lo) * percent) / 2;
		double a1_spread = ((M_a1_hi - M_a1_lo) * percent) / 2;
		double a2_spread = ((M_a2_hi - M_a2_lo) * percent) / 2;
		double a3_spread = ((M_a3_hi - M_a3_lo) * percent) / 2;
		double a4_spread = ((M_a4_hi - M_a4_lo) * percent) / 2;
		double a5_spread = ((M_a5_hi - M_a5_lo) * percent) / 2;
		double b0_spread = ((M_b0_hi - M_b0_lo) * percent) / 2;
		double b1_spread = ((M_b1_hi - M_b1_lo) * percent) / 2;
		double b2_spread = ((M_b2_hi - M_b2_lo) * percent) / 2;
		double b3_spread = ((M_b3_hi - M_b3_lo) * percent) / 2;
		double b4_spread = ((M_b4_hi - M_b4_lo) * percent) / 2;
		double b5_spread = ((M_b5_hi - M_b5_lo) * percent) / 2;
		strange1 -> a0_lo = strange1 -> a0 - a0_spread;
		strange1 -> a0_hi = strange1 -> a0 + a0_spread;
		strange1 -> a1_lo = strange1 -> a1 - a1_spread;
		strange1 -> a1_hi = strange1 -> a1 + a1_spread;
		strange1 -> a2_lo = strange1 -> a2 - a2_spread;
		strange1 -> a2_hi = strange1 -> a2 + a2_spread;
		strange1 -> a3_lo = strange1 -> a3 - a3_spread;
		strange1 -> a3_hi = strange1 -> a3 + a3_spread;
		strange1 -> a4_lo = strange1 -> a4 - a4_spread;
		strange1 -> a4_hi = strange1 -> a4 + a4_spread;
		strange1 -> a5_lo = strange1 -> a5 - a5_spread;
		strange1 -> a5_hi = strange1 -> a5 + a5_spread;
		strange1 -> b0_lo = strange1 -> b0 - b0_spread;
		strange1 -> b0_hi = strange1 -> b0 + b0_spread;
		strange1 -> b1_lo = strange1 -> b1 - b1_spread;
		strange1 -> b1_hi = strange1 -> b1 + b1_spread;
		strange1 -> b2_lo = strange1 -> b2 - b2_spread;
		strange1 -> b2_hi = strange1 -> b2 + b2_spread;
		strange1 -> b3_lo = strange1 -> b3 - b3_spread;
		strange1 -> b3_hi = strange1 -> b3 + b3_spread;
		strange1 -> b4_lo = strange1 -> b4 - b4_spread;
		strange1 -> b4_hi = strange1 -> b4 + b4_spread;
		strange1 -> b5_lo = strange1 -> b5 - b5_spread;
		strange1 -> b5_hi = strange1 -> b5 + b5_spread;
		limiter(strange1);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for strange1 constraints, requires 24 values, got %d", argc);
		return;
	}
	strange1 -> a0_lo = atom_getfloat(arg++);
	strange1 -> a0_hi = atom_getfloat(arg++);
	strange1 -> a1_lo = atom_getfloat(arg++);
	strange1 -> a1_hi = atom_getfloat(arg++);
	strange1 -> a2_lo = atom_getfloat(arg++);
	strange1 -> a2_hi = atom_getfloat(arg++);
	strange1 -> a3_lo = atom_getfloat(arg++);
	strange1 -> a3_hi = atom_getfloat(arg++);
	strange1 -> a4_lo = atom_getfloat(arg++);
	strange1 -> a4_hi = atom_getfloat(arg++);
	strange1 -> a5_lo = atom_getfloat(arg++);
	strange1 -> a5_hi = atom_getfloat(arg++);
	strange1 -> b0_lo = atom_getfloat(arg++);
	strange1 -> b0_hi = atom_getfloat(arg++);
	strange1 -> b1_lo = atom_getfloat(arg++);
	strange1 -> b1_hi = atom_getfloat(arg++);
	strange1 -> b2_lo = atom_getfloat(arg++);
	strange1 -> b2_hi = atom_getfloat(arg++);
	strange1 -> b3_lo = atom_getfloat(arg++);
	strange1 -> b3_hi = atom_getfloat(arg++);
	strange1 -> b4_lo = atom_getfloat(arg++);
	strange1 -> b4_hi = atom_getfloat(arg++);
	strange1 -> b5_lo = atom_getfloat(arg++);
	strange1 -> b5_hi = atom_getfloat(arg++);
	limiter(strange1);
}

static void search(strange1_struct *strange1, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = strange1 -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a0 = strange1 -> a0;
	double temp_a1 = strange1 -> a1;
	double temp_a2 = strange1 -> a2;
	double temp_a3 = strange1 -> a3;
	double temp_a4 = strange1 -> a4;
	double temp_a5 = strange1 -> a5;
	double temp_b0 = strange1 -> b0;
	double temp_b1 = strange1 -> b1;
	double temp_b2 = strange1 -> b2;
	double temp_b3 = strange1 -> b3;
	double temp_b4 = strange1 -> b4;
	double temp_b5 = strange1 -> b5;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], strange1 -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		strange1 -> a0 = (drand48() * (strange1 -> a0_hi - strange1 -> a0_lo)) + strange1 -> a0_lo;
		strange1 -> a1 = (drand48() * (strange1 -> a1_hi - strange1 -> a1_lo)) + strange1 -> a1_lo;
		strange1 -> a2 = (drand48() * (strange1 -> a2_hi - strange1 -> a2_lo)) + strange1 -> a2_lo;
		strange1 -> a3 = (drand48() * (strange1 -> a3_hi - strange1 -> a3_lo)) + strange1 -> a3_lo;
		strange1 -> a4 = (drand48() * (strange1 -> a4_hi - strange1 -> a4_lo)) + strange1 -> a4_lo;
		strange1 -> a5 = (drand48() * (strange1 -> a5_hi - strange1 -> a5_lo)) + strange1 -> a5_lo;
		strange1 -> b0 = (drand48() * (strange1 -> b0_hi - strange1 -> b0_lo)) + strange1 -> b0_lo;
		strange1 -> b1 = (drand48() * (strange1 -> b1_hi - strange1 -> b1_lo)) + strange1 -> b1_lo;
		strange1 -> b2 = (drand48() * (strange1 -> b2_hi - strange1 -> b2_lo)) + strange1 -> b2_lo;
		strange1 -> b3 = (drand48() * (strange1 -> b3_hi - strange1 -> b3_lo)) + strange1 -> b3_lo;
		strange1 -> b4 = (drand48() * (strange1 -> b4_hi - strange1 -> b4_lo)) + strange1 -> b4_lo;
		strange1 -> b5 = (drand48() * (strange1 -> b5_hi - strange1 -> b5_lo)) + strange1 -> b5_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(strange1, NULL, argc, vars);
		do { calc(strange1, strange1 -> vars); } while(jump--);
		strange1 -> lyap_exp = lyapunov((void *) strange1, (t_gotfn) calc, M_var_count, (double *) strange1 -> vars);
		if (isnan(strange1 -> lyap_exp)) { not_found = 1; }
		if (strange1 -> lyap_exp < strange1 -> lyap_lo || strange1 -> lyap_exp > strange1 -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(strange1, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) strange1 -> lyap_limit);
		post("Try using wider constraints.");
		strange1 -> a0 = temp_a0;
		strange1 -> a1 = temp_a1;
		strange1 -> a2 = temp_a2;
		strange1 -> a3 = temp_a3;
		strange1 -> a4 = temp_a4;
		strange1 -> a5 = temp_a5;
		strange1 -> b0 = temp_b0;
		strange1 -> b1 = temp_b1;
		strange1 -> b2 = temp_b2;
		strange1 -> b3 = temp_b3;
		strange1 -> b4 = temp_b4;
		strange1 -> b5 = temp_b5;
		outlet_anything(strange1 -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		strange1 -> failure_ratio = (strange1 -> lyap_limit - not_expired) / strange1 -> lyap_limit;
		make_results(strange1);
		outlet_anything(strange1 -> search_outlet, gensym("search"), M_search_count, strange1 -> search_out);
	}
}

void *strange1_new(t_symbol *s, int argc, t_atom *argv) {
	strange1_struct *strange1 = (strange1_struct *) pd_new(strange1_class);
	if (strange1 != NULL) {
		outlet_new(&strange1 -> x_obj, &s_float);
		strange1 -> outlets[0] = outlet_new(&strange1 -> x_obj, &s_float);
		strange1 -> search_outlet = outlet_new(&strange1 -> x_obj, &s_list);
		strange1 -> vars_outlet = outlet_new(&strange1 -> x_obj, &s_list);
		strange1 -> params_outlet = outlet_new(&strange1 -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			strange1 -> vars_init[M_x] = strange1 -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			strange1 -> vars_init[M_y] = strange1 -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			strange1 -> a0 = (double) atom_getfloatarg(2, argc, argv);
			strange1 -> a1 = (double) atom_getfloatarg(3, argc, argv);
			strange1 -> a2 = (double) atom_getfloatarg(4, argc, argv);
			strange1 -> a3 = (double) atom_getfloatarg(5, argc, argv);
			strange1 -> a4 = (double) atom_getfloatarg(6, argc, argv);
			strange1 -> a5 = (double) atom_getfloatarg(7, argc, argv);
			strange1 -> b0 = (double) atom_getfloatarg(8, argc, argv);
			strange1 -> b1 = (double) atom_getfloatarg(9, argc, argv);
			strange1 -> b2 = (double) atom_getfloatarg(10, argc, argv);
			strange1 -> b3 = (double) atom_getfloatarg(11, argc, argv);
			strange1 -> b4 = (double) atom_getfloatarg(12, argc, argv);
			strange1 -> b5 = (double) atom_getfloatarg(13, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for strange1 fractal. Expecting 14 arguments.");
			}
			strange1 -> vars_init[M_x] = 0;
			strange1 -> vars_init[M_y] = 0;
			strange1 -> a0 = 1;
			strange1 -> a1 = 1;
			strange1 -> a2 = 1;
			strange1 -> a3 = 1;
			strange1 -> a4 = 1;
			strange1 -> a5 = 1;
			strange1 -> b0 = 1;
			strange1 -> b1 = 1;
			strange1 -> b2 = 1;
			strange1 -> b3 = 1;
			strange1 -> b4 = 1;
			strange1 -> b5 = 1;
		}
		constrain(strange1, NULL, 0, NULL);
		lyap(strange1, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)strange1;
}

void strange1_setup(void) {
	strange1_class = class_new(gensym("strange1"), (t_newmethod) strange1_new, 0, sizeof(strange1_struct), 0, A_GIMME, 0);
	class_addbang(strange1_class, (t_method) calculate);
	class_addmethod(strange1_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(strange1_class, (t_method) show, gensym("show"), 0);
	class_addmethod(strange1_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(strange1_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(strange1_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(strange1_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(strange1_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(strange1_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

