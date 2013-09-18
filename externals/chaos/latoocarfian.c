/* latoocarfian Attractor PD External */
/* Copyright Michael McGonagle, from Cliff Pickover, 2003 */
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

#define M_a_lo -3
#define M_a_hi 3
#define M_b_lo -3
#define M_b_hi 3
#define M_c_lo 0.5
#define M_c_hi 1.5
#define M_d_lo 0.5
#define M_d_hi 1.5

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

static char *version = "latoocarfian v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *latoocarfian_class;

typedef struct latoocarfian_struct {
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
} latoocarfian_struct;

static void calc(latoocarfian_struct *latoocarfian, double *vars) {
	double x_0, y_0;
	x_0 =sin(vars[M_y]*latoocarfian -> b)+(latoocarfian -> c*sin(vars[M_x]*latoocarfian -> b));
	y_0 =sin(vars[M_x]*latoocarfian -> a)+(latoocarfian -> d*sin(vars[M_y]*latoocarfian -> a));
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(latoocarfian_struct *latoocarfian) {
	calc(latoocarfian, latoocarfian -> vars);
	outlet_float(latoocarfian -> outlets[M_y - 1], latoocarfian -> vars[M_y]);
	outlet_float(latoocarfian -> x_obj.ob_outlet, latoocarfian -> vars[M_x]);
} // end calculate

static void reset(latoocarfian_struct *latoocarfian, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		latoocarfian -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		latoocarfian -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		latoocarfian -> vars[M_x] = latoocarfian -> vars_init[M_x];
		latoocarfian -> vars[M_y] = latoocarfian -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(latoocarfian_struct *latoocarfian) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((latoocarfian -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((latoocarfian -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((latoocarfian -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((latoocarfian -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(latoocarfian_struct *latoocarfian) {
	SETFLOAT(&latoocarfian -> search_out[0], latoocarfian -> lyap_exp);
	SETSYMBOL(&latoocarfian -> search_out[1], gensym(classify(latoocarfian)));
	SETFLOAT(&latoocarfian -> search_out[2], latoocarfian -> failure_ratio);
	SETFLOAT(&latoocarfian -> vars_out[M_x], latoocarfian -> vars[M_x]);
	SETFLOAT(&latoocarfian -> vars_out[M_y], latoocarfian -> vars[M_y]);
	SETFLOAT(&latoocarfian -> params_out[M_a], latoocarfian -> a);
	SETFLOAT(&latoocarfian -> params_out[M_b], latoocarfian -> b);
	SETFLOAT(&latoocarfian -> params_out[M_c], latoocarfian -> c);
	SETFLOAT(&latoocarfian -> params_out[M_d], latoocarfian -> d);
	outlet_list(latoocarfian -> params_outlet, gensym("list"), M_param_count, latoocarfian -> params_out);
	outlet_list(latoocarfian -> vars_outlet, gensym("list"), M_var_count, latoocarfian -> vars_out);
}

static void show(latoocarfian_struct *latoocarfian) {
	make_results(latoocarfian);
	outlet_anything(latoocarfian -> search_outlet, gensym("show"), M_search_count, latoocarfian -> search_out);
}

static void param(latoocarfian_struct *latoocarfian, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for latoocarfian fractal. Expecting 4 arguments.");
		return;
	}
	latoocarfian -> a = (double) atom_getfloatarg(0, argc, argv);
	latoocarfian -> b = (double) atom_getfloatarg(1, argc, argv);
	latoocarfian -> c = (double) atom_getfloatarg(2, argc, argv);
	latoocarfian -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(latoocarfian_struct *latoocarfian, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(latoocarfian_struct *latoocarfian, t_floatarg l, t_floatarg h, t_floatarg lim) {
	latoocarfian -> lyap_lo = l;
	latoocarfian -> lyap_hi = h;
	latoocarfian -> lyap_limit = (double) ((int) lim);
}

static void elyap(latoocarfian_struct *latoocarfian) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) latoocarfian, (t_gotfn) calc, M_var_count, latoocarfian -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(latoocarfian_struct *latoocarfian) {
	if (latoocarfian -> a_lo < M_a_lo) { latoocarfian -> a_lo = M_a_lo; }
	if (latoocarfian -> a_lo > M_a_hi) { latoocarfian -> a_lo = M_a_hi; }
	if (latoocarfian -> a_hi < M_a_lo) { latoocarfian -> a_hi = M_a_lo; }
	if (latoocarfian -> a_hi > M_a_hi) { latoocarfian -> a_hi = M_a_hi; }
	if (latoocarfian -> b_lo < M_b_lo) { latoocarfian -> b_lo = M_b_lo; }
	if (latoocarfian -> b_lo > M_b_hi) { latoocarfian -> b_lo = M_b_hi; }
	if (latoocarfian -> b_hi < M_b_lo) { latoocarfian -> b_hi = M_b_lo; }
	if (latoocarfian -> b_hi > M_b_hi) { latoocarfian -> b_hi = M_b_hi; }
	if (latoocarfian -> c_lo < M_c_lo) { latoocarfian -> c_lo = M_c_lo; }
	if (latoocarfian -> c_lo > M_c_hi) { latoocarfian -> c_lo = M_c_hi; }
	if (latoocarfian -> c_hi < M_c_lo) { latoocarfian -> c_hi = M_c_lo; }
	if (latoocarfian -> c_hi > M_c_hi) { latoocarfian -> c_hi = M_c_hi; }
	if (latoocarfian -> d_lo < M_d_lo) { latoocarfian -> d_lo = M_d_lo; }
	if (latoocarfian -> d_lo > M_d_hi) { latoocarfian -> d_lo = M_d_hi; }
	if (latoocarfian -> d_hi < M_d_lo) { latoocarfian -> d_hi = M_d_lo; }
	if (latoocarfian -> d_hi > M_d_hi) { latoocarfian -> d_hi = M_d_hi; }
}

static void constrain(latoocarfian_struct *latoocarfian, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		latoocarfian -> a_lo = M_a_lo;
		latoocarfian -> a_hi = M_a_hi;
		latoocarfian -> b_lo = M_b_lo;
		latoocarfian -> b_hi = M_b_hi;
		latoocarfian -> c_lo = M_c_lo;
		latoocarfian -> c_hi = M_c_hi;
		latoocarfian -> d_lo = M_d_lo;
		latoocarfian -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		latoocarfian -> a_lo = latoocarfian -> a - a_spread;
		latoocarfian -> a_hi = latoocarfian -> a + a_spread;
		latoocarfian -> b_lo = latoocarfian -> b - b_spread;
		latoocarfian -> b_hi = latoocarfian -> b + b_spread;
		latoocarfian -> c_lo = latoocarfian -> c - c_spread;
		latoocarfian -> c_hi = latoocarfian -> c + c_spread;
		latoocarfian -> d_lo = latoocarfian -> d - d_spread;
		latoocarfian -> d_hi = latoocarfian -> d + d_spread;
		limiter(latoocarfian);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for latoocarfian constraints, requires 8 values, got %d", argc);
		return;
	}
	latoocarfian -> a_lo = atom_getfloat(arg++);
	latoocarfian -> a_hi = atom_getfloat(arg++);
	latoocarfian -> b_lo = atom_getfloat(arg++);
	latoocarfian -> b_hi = atom_getfloat(arg++);
	latoocarfian -> c_lo = atom_getfloat(arg++);
	latoocarfian -> c_hi = atom_getfloat(arg++);
	latoocarfian -> d_lo = atom_getfloat(arg++);
	latoocarfian -> d_hi = atom_getfloat(arg++);
	limiter(latoocarfian);
}

static void search(latoocarfian_struct *latoocarfian, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = latoocarfian -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = latoocarfian -> a;
	double temp_b = latoocarfian -> b;
	double temp_c = latoocarfian -> c;
	double temp_d = latoocarfian -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], latoocarfian -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		latoocarfian -> a = (drand48() * (latoocarfian -> a_hi - latoocarfian -> a_lo)) + latoocarfian -> a_lo;
		latoocarfian -> b = (drand48() * (latoocarfian -> b_hi - latoocarfian -> b_lo)) + latoocarfian -> b_lo;
		latoocarfian -> c = (drand48() * (latoocarfian -> c_hi - latoocarfian -> c_lo)) + latoocarfian -> c_lo;
		latoocarfian -> d = (drand48() * (latoocarfian -> d_hi - latoocarfian -> d_lo)) + latoocarfian -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(latoocarfian, NULL, argc, vars);
		do { calc(latoocarfian, latoocarfian -> vars); } while(jump--);
		latoocarfian -> lyap_exp = lyapunov((void *) latoocarfian, (t_gotfn) calc, M_var_count, (double *) latoocarfian -> vars);
		if (isnan(latoocarfian -> lyap_exp)) { not_found = 1; }
		if (latoocarfian -> lyap_exp < latoocarfian -> lyap_lo || latoocarfian -> lyap_exp > latoocarfian -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(latoocarfian, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) latoocarfian -> lyap_limit);
		post("Try using wider constraints.");
		latoocarfian -> a = temp_a;
		latoocarfian -> b = temp_b;
		latoocarfian -> c = temp_c;
		latoocarfian -> d = temp_d;
		outlet_anything(latoocarfian -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		latoocarfian -> failure_ratio = (latoocarfian -> lyap_limit - not_expired) / latoocarfian -> lyap_limit;
		make_results(latoocarfian);
		outlet_anything(latoocarfian -> search_outlet, gensym("search"), M_search_count, latoocarfian -> search_out);
	}
}

void *latoocarfian_new(t_symbol *s, int argc, t_atom *argv) {
	latoocarfian_struct *latoocarfian = (latoocarfian_struct *) pd_new(latoocarfian_class);
	if (latoocarfian != NULL) {
		outlet_new(&latoocarfian -> x_obj, &s_float);
		latoocarfian -> outlets[0] = outlet_new(&latoocarfian -> x_obj, &s_float);
		latoocarfian -> search_outlet = outlet_new(&latoocarfian -> x_obj, &s_list);
		latoocarfian -> vars_outlet = outlet_new(&latoocarfian -> x_obj, &s_list);
		latoocarfian -> params_outlet = outlet_new(&latoocarfian -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			latoocarfian -> vars_init[M_x] = latoocarfian -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			latoocarfian -> vars_init[M_y] = latoocarfian -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			latoocarfian -> a = (double) atom_getfloatarg(2, argc, argv);
			latoocarfian -> b = (double) atom_getfloatarg(3, argc, argv);
			latoocarfian -> c = (double) atom_getfloatarg(4, argc, argv);
			latoocarfian -> d = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for latoocarfian fractal. Expecting 6 arguments.");
			}
			latoocarfian -> vars_init[M_x] = 0.1;
			latoocarfian -> vars_init[M_y] = 0.1;
			latoocarfian -> a = -0.966918;
			latoocarfian -> b = 2.87988;
			latoocarfian -> c = 0.756145;
			latoocarfian -> d = 0.744728;
		}
		constrain(latoocarfian, NULL, 0, NULL);
		lyap(latoocarfian, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)latoocarfian;
}

void latoocarfian_setup(void) {
	latoocarfian_class = class_new(gensym("latoocarfian"), (t_newmethod) latoocarfian_new, 0, sizeof(latoocarfian_struct), 0, A_GIMME, 0);
	class_addbang(latoocarfian_class, (t_method) calculate);
	class_addmethod(latoocarfian_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(latoocarfian_class, (t_method) show, gensym("show"), 0);
	class_addmethod(latoocarfian_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(latoocarfian_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(latoocarfian_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(latoocarfian_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(latoocarfian_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(latoocarfian_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

