/* base Attractor PD External */
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

static char *version = "base v0.0, by Michael McGonagle, 2003";

t_class *base_class;

typedef struct base_struct {
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
} base_struct;

static void calc(base_struct *base, double *vars) {
	double x_0;
	x_0 =vars[M_x]+(base -> a*sin(pow(vars[M_x],base -> b)));
	vars[M_x] = x_0;
} // end calc

static void calculate(base_struct *base) {
	calc(base, base -> vars);
	outlet_float(base -> x_obj.ob_outlet, base -> vars[M_x]);
} // end calculate

static void reset(base_struct *base, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		base -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
	} else {
		base -> vars[M_x] = base -> vars_init[M_x];
	} // end if
} // end reset

static char *classify(base_struct *base) {
	static char buff[3];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((base -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((base -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = '\0';
	return buff;
}

static void make_results(base_struct *base) {
	SETFLOAT(&base -> search_out[0], base -> lyap_exp);
	SETSYMBOL(&base -> search_out[1], gensym(classify(base)));
	SETFLOAT(&base -> search_out[2], base -> failure_ratio);
	SETFLOAT(&base -> vars_out[M_x], base -> vars[M_x]);
	SETFLOAT(&base -> params_out[M_a], base -> a);
	SETFLOAT(&base -> params_out[M_b], base -> b);
	outlet_list(base -> params_outlet, gensym("list"), M_param_count, base -> params_out);
	outlet_list(base -> vars_outlet, gensym("list"), M_var_count, base -> vars_out);
}

static void show(base_struct *base) {
	make_results(base);
	outlet_anything(base -> search_outlet, gensym("show"), M_search_count, base -> search_out);
}

static void param(base_struct *base, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 2) {
		post("Incorrect number of arguments for base fractal. Expecting 2 arguments.");
		return;
	}
	base -> a = (double) atom_getfloatarg(0, argc, argv);
	base -> b = (double) atom_getfloatarg(1, argc, argv);
}

static void seed(base_struct *base, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(base_struct *base, t_floatarg l, t_floatarg h, t_floatarg lim) {
	base -> lyap_lo = l;
	base -> lyap_hi = h;
	base -> lyap_limit = (double) ((int) lim);
}

static void elyap(base_struct *base) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) base, (t_gotfn) calc, M_var_count, base -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(base_struct *base) {
	if (base -> a_lo < M_a_lo) { base -> a_lo = M_a_lo; }
	if (base -> a_lo > M_a_hi) { base -> a_lo = M_a_hi; }
	if (base -> a_hi < M_a_lo) { base -> a_hi = M_a_lo; }
	if (base -> a_hi > M_a_hi) { base -> a_hi = M_a_hi; }
	if (base -> b_lo < M_b_lo) { base -> b_lo = M_b_lo; }
	if (base -> b_lo > M_b_hi) { base -> b_lo = M_b_hi; }
	if (base -> b_hi < M_b_lo) { base -> b_hi = M_b_lo; }
	if (base -> b_hi > M_b_hi) { base -> b_hi = M_b_hi; }
}

static void constrain(base_struct *base, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		base -> a_lo = M_a_lo;
		base -> a_hi = M_a_hi;
		base -> b_lo = M_b_lo;
		base -> b_hi = M_b_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		base -> a_lo = base -> a - a_spread;
		base -> a_hi = base -> a + a_spread;
		base -> b_lo = base -> b - b_spread;
		base -> b_hi = base -> b + b_spread;
		limiter(base);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for base constraints, requires 4 values, got %d", argc);
		return;
	}
	base -> a_lo = atom_getfloat(arg++);
	base -> a_hi = atom_getfloat(arg++);
	base -> b_lo = atom_getfloat(arg++);
	base -> b_hi = atom_getfloat(arg++);
	limiter(base);
}

static void search(base_struct *base, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = base -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = base -> a;
	double temp_b = base -> b;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], base -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		base -> a = (drand48() * (base -> a_hi - base -> a_lo)) + base -> a_lo;
		base -> b = (drand48() * (base -> b_hi - base -> b_lo)) + base -> b_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(base, NULL, argc, vars);
		do { calc(base, base -> vars); } while(jump--);
		base -> lyap_exp = lyapunov((void *) base, (t_gotfn) calc, M_var_count, (double *) base -> vars);
		if (isnan(base -> lyap_exp)) { not_found = 1; }
		if (base -> lyap_exp < base -> lyap_lo || base -> lyap_exp > base -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(base, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) base -> lyap_limit);
		post("Try using wider constraints.");
		base -> a = temp_a;
		base -> b = temp_b;
		outlet_anything(base -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		base -> failure_ratio = (base -> lyap_limit - not_expired) / base -> lyap_limit;
		make_results(base);
		outlet_anything(base -> search_outlet, gensym("search"), M_search_count, base -> search_out);
	}
}

void *base_new(t_symbol *s, int argc, t_atom *argv) {
	base_struct *base = (base_struct *) pd_new(base_class);
	if (base != NULL) {
		outlet_new(&base -> x_obj, &s_float);
		base -> search_outlet = outlet_new(&base -> x_obj, &s_list);
		base -> vars_outlet = outlet_new(&base -> x_obj, &s_list);
		base -> params_outlet = outlet_new(&base -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			base -> vars_init[M_x] = base -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			base -> a = (double) atom_getfloatarg(1, argc, argv);
			base -> b = (double) atom_getfloatarg(2, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for base fractal. Expecting 3 arguments.");
			}
			base -> vars_init[M_x] = 0.1;
			base -> a = 1;
			base -> b = 1;
		}
		constrain(base, NULL, 0, NULL);
		lyap(base, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)base;
}

void base_setup(void) {
	base_class = class_new(gensym("base"), (t_newmethod) base_new, 0, sizeof(base_struct), 0, A_GIMME, 0);
	class_addbang(base_class, (t_method) calculate);
	class_addmethod(base_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(base_class, (t_method) show, gensym("show"), 0);
	class_addmethod(base_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(base_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(base_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(base_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(base_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(base_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

