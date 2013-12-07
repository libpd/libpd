/* base3 Attractor PD External */
/* Copyright Michael McGonagle, 2003 */
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

#define M_a_lo 0
#define M_a_hi 3
#define M_b_lo 0.001
#define M_b_hi 2.6667

#define M_a 0
#define M_b 1

#define M_x 0

#define M_param_count 2
#define M_var_count 1
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "base3 v0.0, by Michael McGonagle, 2003";

t_class *base3_class;

typedef struct base3_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi, b, b_lo, b_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
} base3_struct;

static void calc(base3_struct *base3, double *vars) {
	double x_0;
	x_0 =base3 -> a*sin(pow(vars[M_x],base3 -> b));
	vars[M_x] = x_0;
} // end calc

static void calculate(base3_struct *base3) {
	calc(base3, base3 -> vars);
	outlet_float(base3 -> x_obj.ob_outlet, base3 -> vars[M_x]);
} // end calculate

static void reset(base3_struct *base3, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		base3 -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
	} else {
		base3 -> vars[M_x] = base3 -> vars_init[M_x];
	} // end if
} // end reset

static char *classify(base3_struct *base3) {
	static char buff[3];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((base3 -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((base3 -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = '\0';
	return buff;
}

static void make_results(base3_struct *base3) {
	SETFLOAT(&base3 -> search_out[0], base3 -> lyap_exp);
	SETSYMBOL(&base3 -> search_out[1], gensym(classify(base3)));
	SETFLOAT(&base3 -> search_out[2], base3 -> failure_ratio);
	SETFLOAT(&base3 -> vars_out[M_x], base3 -> vars[M_x]);
	SETFLOAT(&base3 -> params_out[M_a], base3 -> a);
	SETFLOAT(&base3 -> params_out[M_b], base3 -> b);
	outlet_list(base3 -> params_outlet, gensym("list"), M_param_count, base3 -> params_out);
	outlet_list(base3 -> vars_outlet, gensym("list"), M_var_count, base3 -> vars_out);
}

static void show(base3_struct *base3) {
	make_results(base3);
	outlet_anything(base3 -> search_outlet, gensym("show"), M_search_count, base3 -> search_out);
}

static void param(base3_struct *base3, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 2) {
		post("Incorrect number of arguments for base3 fractal. Expecting 2 arguments.");
		return;
	}
	base3 -> a = (double) atom_getfloatarg(0, argc, argv);
	base3 -> b = (double) atom_getfloatarg(1, argc, argv);
}

static void seed(base3_struct *base3, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(base3_struct *base3, t_floatarg l, t_floatarg h, t_floatarg lim) {
	base3 -> lyap_lo = l;
	base3 -> lyap_hi = h;
	base3 -> lyap_limit = (double) ((int) lim);
}

static void elyap(base3_struct *base3) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) base3, (t_gotfn) calc, M_var_count, base3 -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(base3_struct *base3) {
	if (base3 -> a_lo < M_a_lo) { base3 -> a_lo = M_a_lo; }
	if (base3 -> a_lo > M_a_hi) { base3 -> a_lo = M_a_hi; }
	if (base3 -> a_hi < M_a_lo) { base3 -> a_hi = M_a_lo; }
	if (base3 -> a_hi > M_a_hi) { base3 -> a_hi = M_a_hi; }
	if (base3 -> b_lo < M_b_lo) { base3 -> b_lo = M_b_lo; }
	if (base3 -> b_lo > M_b_hi) { base3 -> b_lo = M_b_hi; }
	if (base3 -> b_hi < M_b_lo) { base3 -> b_hi = M_b_lo; }
	if (base3 -> b_hi > M_b_hi) { base3 -> b_hi = M_b_hi; }
}

static void constrain(base3_struct *base3, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		base3 -> a_lo = M_a_lo;
		base3 -> a_hi = M_a_hi;
		base3 -> b_lo = M_b_lo;
		base3 -> b_hi = M_b_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		base3 -> a_lo = base3 -> a - a_spread;
		base3 -> a_hi = base3 -> a + a_spread;
		base3 -> b_lo = base3 -> b - b_spread;
		base3 -> b_hi = base3 -> b + b_spread;
		limiter(base3);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for base3 constraints, requires 4 values, got %d", argc);
		return;
	}
	base3 -> a_lo = atom_getfloat(arg++);
	base3 -> a_hi = atom_getfloat(arg++);
	base3 -> b_lo = atom_getfloat(arg++);
	base3 -> b_hi = atom_getfloat(arg++);
	limiter(base3);
}

static void search(base3_struct *base3, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = base3 -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = base3 -> a;
	double temp_b = base3 -> b;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], base3 -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		base3 -> a = (drand48() * (base3 -> a_hi - base3 -> a_lo)) + base3 -> a_lo;
		base3 -> b = (drand48() * (base3 -> b_hi - base3 -> b_lo)) + base3 -> b_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(base3, NULL, argc, vars);
		do { calc(base3, base3 -> vars); } while(jump--);
		base3 -> lyap_exp = lyapunov((void *) base3, (t_gotfn) calc, M_var_count, (double *) base3 -> vars);
		if (isnan(base3 -> lyap_exp)) { not_found = 1; }
		if (base3 -> lyap_exp < base3 -> lyap_lo || base3 -> lyap_exp > base3 -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(base3, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) base3 -> lyap_limit);
		post("Try using wider constraints.");
		base3 -> a = temp_a;
		base3 -> b = temp_b;
		outlet_anything(base3 -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		base3 -> failure_ratio = (base3 -> lyap_limit - not_expired) / base3 -> lyap_limit;
		make_results(base3);
		outlet_anything(base3 -> search_outlet, gensym("search"), M_search_count, base3 -> search_out);
	}
}

void *base3_new(t_symbol *s, int argc, t_atom *argv) {
	base3_struct *base3 = (base3_struct *) pd_new(base3_class);
	if (base3 != NULL) {
		outlet_new(&base3 -> x_obj, &s_float);
		base3 -> search_outlet = outlet_new(&base3 -> x_obj, &s_list);
		base3 -> vars_outlet = outlet_new(&base3 -> x_obj, &s_list);
		base3 -> params_outlet = outlet_new(&base3 -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			base3 -> vars_init[M_x] = base3 -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			base3 -> a = (double) atom_getfloatarg(1, argc, argv);
			base3 -> b = (double) atom_getfloatarg(2, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for base3 fractal. Expecting 3 arguments.");
			}
			base3 -> vars_init[M_x] = 0.1;
			base3 -> a = 1;
			base3 -> b = 1;
		}
		constrain(base3, NULL, 0, NULL);
		lyap(base3, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)base3;
}

void base3_setup(void) {
	base3_class = class_new(gensym("base3"), (t_newmethod) base3_new, 0, sizeof(base3_struct), 0, A_GIMME, 0);
	class_addbang(base3_class, (t_method) calculate);
	class_addmethod(base3_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(base3_class, (t_method) show, gensym("show"), 0);
	class_addmethod(base3_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(base3_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(base3_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(base3_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(base3_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(base3_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

