/* dejong Attractor PD External */
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

#define M_a_lo -1000
#define M_a_hi 1000
#define M_b_lo -1000
#define M_b_hi 1000
#define M_c_lo -1000
#define M_c_hi 1000
#define M_d_lo -1000
#define M_d_hi 1000

#define M_a 0
#define M_b 1
#define M_c 2
#define M_d 3

#define M_x 0
#define M_y 1

#define M_param_count 4
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "dejong v0.0, by Michael McGonagle, from ???pbourke???, 2003";

t_class *dejong_class;

typedef struct dejong_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi, b, b_lo, b_hi, c, c_lo, c_hi, d, d_lo, d_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} dejong_struct;

static void calc(dejong_struct *dejong, double *vars) {
	double x_0, y_0;
	x_0 =sin(dejong -> a*vars[M_y])-cos(dejong -> b*vars[M_x]);
	y_0 =sin(dejong -> c*vars[M_x])-cos(dejong -> d*vars[M_y]);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(dejong_struct *dejong) {
	calc(dejong, dejong -> vars);
	outlet_float(dejong -> outlets[M_y - 1], dejong -> vars[M_y]);
	outlet_float(dejong -> x_obj.ob_outlet, dejong -> vars[M_x]);
} // end calculate

static void reset(dejong_struct *dejong, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		dejong -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		dejong -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		dejong -> vars[M_x] = dejong -> vars_init[M_x];
		dejong -> vars[M_y] = dejong -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(dejong_struct *dejong) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((dejong -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((dejong -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((dejong -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((dejong -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(dejong_struct *dejong) {
	SETFLOAT(&dejong -> search_out[0], dejong -> lyap_exp);
	SETSYMBOL(&dejong -> search_out[1], gensym(classify(dejong)));
	SETFLOAT(&dejong -> search_out[2], dejong -> failure_ratio);
	SETFLOAT(&dejong -> vars_out[M_x], dejong -> vars[M_x]);
	SETFLOAT(&dejong -> vars_out[M_y], dejong -> vars[M_y]);
	SETFLOAT(&dejong -> params_out[M_a], dejong -> a);
	SETFLOAT(&dejong -> params_out[M_b], dejong -> b);
	SETFLOAT(&dejong -> params_out[M_c], dejong -> c);
	SETFLOAT(&dejong -> params_out[M_d], dejong -> d);
	outlet_list(dejong -> params_outlet, gensym("list"), M_param_count, dejong -> params_out);
	outlet_list(dejong -> vars_outlet, gensym("list"), M_var_count, dejong -> vars_out);
}

static void show(dejong_struct *dejong) {
	make_results(dejong);
	outlet_anything(dejong -> search_outlet, gensym("show"), M_search_count, dejong -> search_out);
}

static void param(dejong_struct *dejong, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for dejong fractal. Expecting 4 arguments.");
		return;
	}
	dejong -> a = (double) atom_getfloatarg(0, argc, argv);
	dejong -> b = (double) atom_getfloatarg(1, argc, argv);
	dejong -> c = (double) atom_getfloatarg(2, argc, argv);
	dejong -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(dejong_struct *dejong, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(dejong_struct *dejong, t_floatarg l, t_floatarg h, t_floatarg lim) {
	dejong -> lyap_lo = l;
	dejong -> lyap_hi = h;
	dejong -> lyap_limit = (double) ((int) lim);
}

static void elyap(dejong_struct *dejong) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) dejong, (t_gotfn) calc, M_var_count, dejong -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(dejong_struct *dejong) {
	if (dejong -> a_lo < M_a_lo) { dejong -> a_lo = M_a_lo; }
	if (dejong -> a_lo > M_a_hi) { dejong -> a_lo = M_a_hi; }
	if (dejong -> a_hi < M_a_lo) { dejong -> a_hi = M_a_lo; }
	if (dejong -> a_hi > M_a_hi) { dejong -> a_hi = M_a_hi; }
	if (dejong -> b_lo < M_b_lo) { dejong -> b_lo = M_b_lo; }
	if (dejong -> b_lo > M_b_hi) { dejong -> b_lo = M_b_hi; }
	if (dejong -> b_hi < M_b_lo) { dejong -> b_hi = M_b_lo; }
	if (dejong -> b_hi > M_b_hi) { dejong -> b_hi = M_b_hi; }
	if (dejong -> c_lo < M_c_lo) { dejong -> c_lo = M_c_lo; }
	if (dejong -> c_lo > M_c_hi) { dejong -> c_lo = M_c_hi; }
	if (dejong -> c_hi < M_c_lo) { dejong -> c_hi = M_c_lo; }
	if (dejong -> c_hi > M_c_hi) { dejong -> c_hi = M_c_hi; }
	if (dejong -> d_lo < M_d_lo) { dejong -> d_lo = M_d_lo; }
	if (dejong -> d_lo > M_d_hi) { dejong -> d_lo = M_d_hi; }
	if (dejong -> d_hi < M_d_lo) { dejong -> d_hi = M_d_lo; }
	if (dejong -> d_hi > M_d_hi) { dejong -> d_hi = M_d_hi; }
}

static void constrain(dejong_struct *dejong, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		dejong -> a_lo = M_a_lo;
		dejong -> a_hi = M_a_hi;
		dejong -> b_lo = M_b_lo;
		dejong -> b_hi = M_b_hi;
		dejong -> c_lo = M_c_lo;
		dejong -> c_hi = M_c_hi;
		dejong -> d_lo = M_d_lo;
		dejong -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		dejong -> a_lo = dejong -> a - a_spread;
		dejong -> a_hi = dejong -> a + a_spread;
		dejong -> b_lo = dejong -> b - b_spread;
		dejong -> b_hi = dejong -> b + b_spread;
		dejong -> c_lo = dejong -> c - c_spread;
		dejong -> c_hi = dejong -> c + c_spread;
		dejong -> d_lo = dejong -> d - d_spread;
		dejong -> d_hi = dejong -> d + d_spread;
		limiter(dejong);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for dejong constraints, requires 8 values, got %d", argc);
		return;
	}
	dejong -> a_lo = atom_getfloat(arg++);
	dejong -> a_hi = atom_getfloat(arg++);
	dejong -> b_lo = atom_getfloat(arg++);
	dejong -> b_hi = atom_getfloat(arg++);
	dejong -> c_lo = atom_getfloat(arg++);
	dejong -> c_hi = atom_getfloat(arg++);
	dejong -> d_lo = atom_getfloat(arg++);
	dejong -> d_hi = atom_getfloat(arg++);
	limiter(dejong);
}

static void search(dejong_struct *dejong, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = dejong -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = dejong -> a;
	double temp_b = dejong -> b;
	double temp_c = dejong -> c;
	double temp_d = dejong -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], dejong -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		dejong -> a = (drand48() * (dejong -> a_hi - dejong -> a_lo)) + dejong -> a_lo;
		dejong -> b = (drand48() * (dejong -> b_hi - dejong -> b_lo)) + dejong -> b_lo;
		dejong -> c = (drand48() * (dejong -> c_hi - dejong -> c_lo)) + dejong -> c_lo;
		dejong -> d = (drand48() * (dejong -> d_hi - dejong -> d_lo)) + dejong -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(dejong, NULL, argc, vars);
		do { calc(dejong, dejong -> vars); } while(jump--);
		dejong -> lyap_exp = lyapunov((void *) dejong, (t_gotfn) calc, M_var_count, (double *) dejong -> vars);
		if (isnan(dejong -> lyap_exp)) { not_found = 1; }
		if (dejong -> lyap_exp < dejong -> lyap_lo || dejong -> lyap_exp > dejong -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(dejong, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) dejong -> lyap_limit);
		post("Try using wider constraints.");
		dejong -> a = temp_a;
		dejong -> b = temp_b;
		dejong -> c = temp_c;
		dejong -> d = temp_d;
		outlet_anything(dejong -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		dejong -> failure_ratio = (dejong -> lyap_limit - not_expired) / dejong -> lyap_limit;
		make_results(dejong);
		outlet_anything(dejong -> search_outlet, gensym("search"), M_search_count, dejong -> search_out);
	}
}

void *dejong_new(t_symbol *s, int argc, t_atom *argv) {
	dejong_struct *dejong = (dejong_struct *) pd_new(dejong_class);
	if (dejong != NULL) {
		outlet_new(&dejong -> x_obj, &s_float);
		dejong -> outlets[0] = outlet_new(&dejong -> x_obj, &s_float);
		dejong -> search_outlet = outlet_new(&dejong -> x_obj, &s_list);
		dejong -> vars_outlet = outlet_new(&dejong -> x_obj, &s_list);
		dejong -> params_outlet = outlet_new(&dejong -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			dejong -> vars_init[M_x] = dejong -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			dejong -> vars_init[M_y] = dejong -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			dejong -> a = (double) atom_getfloatarg(2, argc, argv);
			dejong -> b = (double) atom_getfloatarg(3, argc, argv);
			dejong -> c = (double) atom_getfloatarg(4, argc, argv);
			dejong -> d = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for dejong fractal. Expecting 6 arguments.");
			}
			dejong -> vars_init[M_x] = 0.01;
			dejong -> vars_init[M_y] = 0;
			dejong -> a = -2.24;
			dejong -> b = -0.65;
			dejong -> c = 0.43;
			dejong -> d = -2.43;
		}
		constrain(dejong, NULL, 0, NULL);
		lyap(dejong, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)dejong;
}

void dejong_setup(void) {
	dejong_class = class_new(gensym("dejong"), (t_newmethod) dejong_new, 0, sizeof(dejong_struct), 0, A_GIMME, 0);
	class_addbang(dejong_class, (t_method) calculate);
	class_addmethod(dejong_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(dejong_class, (t_method) show, gensym("show"), 0);
	class_addmethod(dejong_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(dejong_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(dejong_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(dejong_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(dejong_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(dejong_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

