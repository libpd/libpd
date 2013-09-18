/* tinkerbell Attractor PD External */
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

static char *version = "tinkerbell v0.0, by Michael McGonagle, from ??????, 2003";

t_class *tinkerbell_class;

typedef struct tinkerbell_struct {
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
} tinkerbell_struct;

static void calc(tinkerbell_struct *tinkerbell, double *vars) {
	double x_0, y_0;
	x_0 =vars[M_x]*vars[M_x]-vars[M_y]*vars[M_y]+tinkerbell -> a*vars[M_x]+tinkerbell -> b*vars[M_y];
	y_0 =2*vars[M_x]*vars[M_y]-tinkerbell -> c*vars[M_x]+tinkerbell -> d*vars[M_y];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(tinkerbell_struct *tinkerbell) {
	calc(tinkerbell, tinkerbell -> vars);
	outlet_float(tinkerbell -> outlets[M_y - 1], tinkerbell -> vars[M_y]);
	outlet_float(tinkerbell -> x_obj.ob_outlet, tinkerbell -> vars[M_x]);
} // end calculate

static void reset(tinkerbell_struct *tinkerbell, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		tinkerbell -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		tinkerbell -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		tinkerbell -> vars[M_x] = tinkerbell -> vars_init[M_x];
		tinkerbell -> vars[M_y] = tinkerbell -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(tinkerbell_struct *tinkerbell) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((tinkerbell -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((tinkerbell -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((tinkerbell -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((tinkerbell -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(tinkerbell_struct *tinkerbell) {
	SETFLOAT(&tinkerbell -> search_out[0], tinkerbell -> lyap_exp);
	SETSYMBOL(&tinkerbell -> search_out[1], gensym(classify(tinkerbell)));
	SETFLOAT(&tinkerbell -> search_out[2], tinkerbell -> failure_ratio);
	SETFLOAT(&tinkerbell -> vars_out[M_x], tinkerbell -> vars[M_x]);
	SETFLOAT(&tinkerbell -> vars_out[M_y], tinkerbell -> vars[M_y]);
	SETFLOAT(&tinkerbell -> params_out[M_a], tinkerbell -> a);
	SETFLOAT(&tinkerbell -> params_out[M_b], tinkerbell -> b);
	SETFLOAT(&tinkerbell -> params_out[M_c], tinkerbell -> c);
	SETFLOAT(&tinkerbell -> params_out[M_d], tinkerbell -> d);
	outlet_list(tinkerbell -> params_outlet, gensym("list"), M_param_count, tinkerbell -> params_out);
	outlet_list(tinkerbell -> vars_outlet, gensym("list"), M_var_count, tinkerbell -> vars_out);
}

static void show(tinkerbell_struct *tinkerbell) {
	make_results(tinkerbell);
	outlet_anything(tinkerbell -> search_outlet, gensym("show"), M_search_count, tinkerbell -> search_out);
}

static void param(tinkerbell_struct *tinkerbell, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for tinkerbell fractal. Expecting 4 arguments.");
		return;
	}
	tinkerbell -> a = (double) atom_getfloatarg(0, argc, argv);
	tinkerbell -> b = (double) atom_getfloatarg(1, argc, argv);
	tinkerbell -> c = (double) atom_getfloatarg(2, argc, argv);
	tinkerbell -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(tinkerbell_struct *tinkerbell, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(tinkerbell_struct *tinkerbell, t_floatarg l, t_floatarg h, t_floatarg lim) {
	tinkerbell -> lyap_lo = l;
	tinkerbell -> lyap_hi = h;
	tinkerbell -> lyap_limit = (double) ((int) lim);
}

static void elyap(tinkerbell_struct *tinkerbell) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) tinkerbell, (t_gotfn) calc, M_var_count, tinkerbell -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(tinkerbell_struct *tinkerbell) {
	if (tinkerbell -> a_lo < M_a_lo) { tinkerbell -> a_lo = M_a_lo; }
	if (tinkerbell -> a_lo > M_a_hi) { tinkerbell -> a_lo = M_a_hi; }
	if (tinkerbell -> a_hi < M_a_lo) { tinkerbell -> a_hi = M_a_lo; }
	if (tinkerbell -> a_hi > M_a_hi) { tinkerbell -> a_hi = M_a_hi; }
	if (tinkerbell -> b_lo < M_b_lo) { tinkerbell -> b_lo = M_b_lo; }
	if (tinkerbell -> b_lo > M_b_hi) { tinkerbell -> b_lo = M_b_hi; }
	if (tinkerbell -> b_hi < M_b_lo) { tinkerbell -> b_hi = M_b_lo; }
	if (tinkerbell -> b_hi > M_b_hi) { tinkerbell -> b_hi = M_b_hi; }
	if (tinkerbell -> c_lo < M_c_lo) { tinkerbell -> c_lo = M_c_lo; }
	if (tinkerbell -> c_lo > M_c_hi) { tinkerbell -> c_lo = M_c_hi; }
	if (tinkerbell -> c_hi < M_c_lo) { tinkerbell -> c_hi = M_c_lo; }
	if (tinkerbell -> c_hi > M_c_hi) { tinkerbell -> c_hi = M_c_hi; }
	if (tinkerbell -> d_lo < M_d_lo) { tinkerbell -> d_lo = M_d_lo; }
	if (tinkerbell -> d_lo > M_d_hi) { tinkerbell -> d_lo = M_d_hi; }
	if (tinkerbell -> d_hi < M_d_lo) { tinkerbell -> d_hi = M_d_lo; }
	if (tinkerbell -> d_hi > M_d_hi) { tinkerbell -> d_hi = M_d_hi; }
}

static void constrain(tinkerbell_struct *tinkerbell, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		tinkerbell -> a_lo = M_a_lo;
		tinkerbell -> a_hi = M_a_hi;
		tinkerbell -> b_lo = M_b_lo;
		tinkerbell -> b_hi = M_b_hi;
		tinkerbell -> c_lo = M_c_lo;
		tinkerbell -> c_hi = M_c_hi;
		tinkerbell -> d_lo = M_d_lo;
		tinkerbell -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		tinkerbell -> a_lo = tinkerbell -> a - a_spread;
		tinkerbell -> a_hi = tinkerbell -> a + a_spread;
		tinkerbell -> b_lo = tinkerbell -> b - b_spread;
		tinkerbell -> b_hi = tinkerbell -> b + b_spread;
		tinkerbell -> c_lo = tinkerbell -> c - c_spread;
		tinkerbell -> c_hi = tinkerbell -> c + c_spread;
		tinkerbell -> d_lo = tinkerbell -> d - d_spread;
		tinkerbell -> d_hi = tinkerbell -> d + d_spread;
		limiter(tinkerbell);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for tinkerbell constraints, requires 8 values, got %d", argc);
		return;
	}
	tinkerbell -> a_lo = atom_getfloat(arg++);
	tinkerbell -> a_hi = atom_getfloat(arg++);
	tinkerbell -> b_lo = atom_getfloat(arg++);
	tinkerbell -> b_hi = atom_getfloat(arg++);
	tinkerbell -> c_lo = atom_getfloat(arg++);
	tinkerbell -> c_hi = atom_getfloat(arg++);
	tinkerbell -> d_lo = atom_getfloat(arg++);
	tinkerbell -> d_hi = atom_getfloat(arg++);
	limiter(tinkerbell);
}

static void search(tinkerbell_struct *tinkerbell, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = tinkerbell -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = tinkerbell -> a;
	double temp_b = tinkerbell -> b;
	double temp_c = tinkerbell -> c;
	double temp_d = tinkerbell -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], tinkerbell -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		tinkerbell -> a = (drand48() * (tinkerbell -> a_hi - tinkerbell -> a_lo)) + tinkerbell -> a_lo;
		tinkerbell -> b = (drand48() * (tinkerbell -> b_hi - tinkerbell -> b_lo)) + tinkerbell -> b_lo;
		tinkerbell -> c = (drand48() * (tinkerbell -> c_hi - tinkerbell -> c_lo)) + tinkerbell -> c_lo;
		tinkerbell -> d = (drand48() * (tinkerbell -> d_hi - tinkerbell -> d_lo)) + tinkerbell -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(tinkerbell, NULL, argc, vars);
		do { calc(tinkerbell, tinkerbell -> vars); } while(jump--);
		tinkerbell -> lyap_exp = lyapunov((void *) tinkerbell, (t_gotfn) calc, M_var_count, (double *) tinkerbell -> vars);
		if (isnan(tinkerbell -> lyap_exp)) { not_found = 1; }
		if (tinkerbell -> lyap_exp < tinkerbell -> lyap_lo || tinkerbell -> lyap_exp > tinkerbell -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(tinkerbell, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) tinkerbell -> lyap_limit);
		post("Try using wider constraints.");
		tinkerbell -> a = temp_a;
		tinkerbell -> b = temp_b;
		tinkerbell -> c = temp_c;
		tinkerbell -> d = temp_d;
		outlet_anything(tinkerbell -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		tinkerbell -> failure_ratio = (tinkerbell -> lyap_limit - not_expired) / tinkerbell -> lyap_limit;
		make_results(tinkerbell);
		outlet_anything(tinkerbell -> search_outlet, gensym("search"), M_search_count, tinkerbell -> search_out);
	}
}

void *tinkerbell_new(t_symbol *s, int argc, t_atom *argv) {
	tinkerbell_struct *tinkerbell = (tinkerbell_struct *) pd_new(tinkerbell_class);
	if (tinkerbell != NULL) {
		outlet_new(&tinkerbell -> x_obj, &s_float);
		tinkerbell -> outlets[0] = outlet_new(&tinkerbell -> x_obj, &s_float);
		tinkerbell -> search_outlet = outlet_new(&tinkerbell -> x_obj, &s_list);
		tinkerbell -> vars_outlet = outlet_new(&tinkerbell -> x_obj, &s_list);
		tinkerbell -> params_outlet = outlet_new(&tinkerbell -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			tinkerbell -> vars_init[M_x] = tinkerbell -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			tinkerbell -> vars_init[M_y] = tinkerbell -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			tinkerbell -> a = (double) atom_getfloatarg(2, argc, argv);
			tinkerbell -> b = (double) atom_getfloatarg(3, argc, argv);
			tinkerbell -> c = (double) atom_getfloatarg(4, argc, argv);
			tinkerbell -> d = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for tinkerbell fractal. Expecting 6 arguments.");
			}
			tinkerbell -> vars_init[M_x] = 0.1;
			tinkerbell -> vars_init[M_y] = 0.1;
			tinkerbell -> a = 0.9;
			tinkerbell -> b = 0.6;
			tinkerbell -> c = 2;
			tinkerbell -> d = 0.5;
		}
		constrain(tinkerbell, NULL, 0, NULL);
		lyap(tinkerbell, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)tinkerbell;
}

void tinkerbell_setup(void) {
	tinkerbell_class = class_new(gensym("tinkerbell"), (t_newmethod) tinkerbell_new, 0, sizeof(tinkerbell_struct), 0, A_GIMME, 0);
	class_addbang(tinkerbell_class, (t_method) calculate);
	class_addmethod(tinkerbell_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(tinkerbell_class, (t_method) show, gensym("show"), 0);
	class_addmethod(tinkerbell_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(tinkerbell_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(tinkerbell_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(tinkerbell_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(tinkerbell_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(tinkerbell_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

