/* latoomutgamma Attractor PD External */
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

static char *version = "latoomutgamma v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *latoomutgamma_class;

typedef struct latoomutgamma_struct {
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
} latoomutgamma_struct;

static void calc(latoomutgamma_struct *latoomutgamma, double *vars) {
	double x_0, y_0;
	x_0 =abs(sin(vars[M_y]*latoomutgamma -> b))+pow(sin(vars[M_x]*latoomutgamma -> b),2);
	y_0 =abs(sin(vars[M_x]*latoomutgamma -> a))+pow(sin(vars[M_y]*latoomutgamma -> b),2);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(latoomutgamma_struct *latoomutgamma) {
	calc(latoomutgamma, latoomutgamma -> vars);
	outlet_float(latoomutgamma -> outlets[M_y - 1], latoomutgamma -> vars[M_y]);
	outlet_float(latoomutgamma -> x_obj.ob_outlet, latoomutgamma -> vars[M_x]);
} // end calculate

static void reset(latoomutgamma_struct *latoomutgamma, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		latoomutgamma -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		latoomutgamma -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		latoomutgamma -> vars[M_x] = latoomutgamma -> vars_init[M_x];
		latoomutgamma -> vars[M_y] = latoomutgamma -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(latoomutgamma_struct *latoomutgamma) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((latoomutgamma -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((latoomutgamma -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((latoomutgamma -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((latoomutgamma -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(latoomutgamma_struct *latoomutgamma) {
	SETFLOAT(&latoomutgamma -> search_out[0], latoomutgamma -> lyap_exp);
	SETSYMBOL(&latoomutgamma -> search_out[1], gensym(classify(latoomutgamma)));
	SETFLOAT(&latoomutgamma -> search_out[2], latoomutgamma -> failure_ratio);
	SETFLOAT(&latoomutgamma -> vars_out[M_x], latoomutgamma -> vars[M_x]);
	SETFLOAT(&latoomutgamma -> vars_out[M_y], latoomutgamma -> vars[M_y]);
	SETFLOAT(&latoomutgamma -> params_out[M_a], latoomutgamma -> a);
	SETFLOAT(&latoomutgamma -> params_out[M_b], latoomutgamma -> b);
	SETFLOAT(&latoomutgamma -> params_out[M_c], latoomutgamma -> c);
	SETFLOAT(&latoomutgamma -> params_out[M_d], latoomutgamma -> d);
	outlet_list(latoomutgamma -> params_outlet, gensym("list"), M_param_count, latoomutgamma -> params_out);
	outlet_list(latoomutgamma -> vars_outlet, gensym("list"), M_var_count, latoomutgamma -> vars_out);
}

static void show(latoomutgamma_struct *latoomutgamma) {
	make_results(latoomutgamma);
	outlet_anything(latoomutgamma -> search_outlet, gensym("show"), M_search_count, latoomutgamma -> search_out);
}

static void param(latoomutgamma_struct *latoomutgamma, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for latoomutgamma fractal. Expecting 4 arguments.");
		return;
	}
	latoomutgamma -> a = (double) atom_getfloatarg(0, argc, argv);
	latoomutgamma -> b = (double) atom_getfloatarg(1, argc, argv);
	latoomutgamma -> c = (double) atom_getfloatarg(2, argc, argv);
	latoomutgamma -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(latoomutgamma_struct *latoomutgamma, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(latoomutgamma_struct *latoomutgamma, t_floatarg l, t_floatarg h, t_floatarg lim) {
	latoomutgamma -> lyap_lo = l;
	latoomutgamma -> lyap_hi = h;
	latoomutgamma -> lyap_limit = (double) ((int) lim);
}

static void elyap(latoomutgamma_struct *latoomutgamma) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) latoomutgamma, (t_gotfn) calc, M_var_count, latoomutgamma -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(latoomutgamma_struct *latoomutgamma) {
	if (latoomutgamma -> a_lo < M_a_lo) { latoomutgamma -> a_lo = M_a_lo; }
	if (latoomutgamma -> a_lo > M_a_hi) { latoomutgamma -> a_lo = M_a_hi; }
	if (latoomutgamma -> a_hi < M_a_lo) { latoomutgamma -> a_hi = M_a_lo; }
	if (latoomutgamma -> a_hi > M_a_hi) { latoomutgamma -> a_hi = M_a_hi; }
	if (latoomutgamma -> b_lo < M_b_lo) { latoomutgamma -> b_lo = M_b_lo; }
	if (latoomutgamma -> b_lo > M_b_hi) { latoomutgamma -> b_lo = M_b_hi; }
	if (latoomutgamma -> b_hi < M_b_lo) { latoomutgamma -> b_hi = M_b_lo; }
	if (latoomutgamma -> b_hi > M_b_hi) { latoomutgamma -> b_hi = M_b_hi; }
	if (latoomutgamma -> c_lo < M_c_lo) { latoomutgamma -> c_lo = M_c_lo; }
	if (latoomutgamma -> c_lo > M_c_hi) { latoomutgamma -> c_lo = M_c_hi; }
	if (latoomutgamma -> c_hi < M_c_lo) { latoomutgamma -> c_hi = M_c_lo; }
	if (latoomutgamma -> c_hi > M_c_hi) { latoomutgamma -> c_hi = M_c_hi; }
	if (latoomutgamma -> d_lo < M_d_lo) { latoomutgamma -> d_lo = M_d_lo; }
	if (latoomutgamma -> d_lo > M_d_hi) { latoomutgamma -> d_lo = M_d_hi; }
	if (latoomutgamma -> d_hi < M_d_lo) { latoomutgamma -> d_hi = M_d_lo; }
	if (latoomutgamma -> d_hi > M_d_hi) { latoomutgamma -> d_hi = M_d_hi; }
}

static void constrain(latoomutgamma_struct *latoomutgamma, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		latoomutgamma -> a_lo = M_a_lo;
		latoomutgamma -> a_hi = M_a_hi;
		latoomutgamma -> b_lo = M_b_lo;
		latoomutgamma -> b_hi = M_b_hi;
		latoomutgamma -> c_lo = M_c_lo;
		latoomutgamma -> c_hi = M_c_hi;
		latoomutgamma -> d_lo = M_d_lo;
		latoomutgamma -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		latoomutgamma -> a_lo = latoomutgamma -> a - a_spread;
		latoomutgamma -> a_hi = latoomutgamma -> a + a_spread;
		latoomutgamma -> b_lo = latoomutgamma -> b - b_spread;
		latoomutgamma -> b_hi = latoomutgamma -> b + b_spread;
		latoomutgamma -> c_lo = latoomutgamma -> c - c_spread;
		latoomutgamma -> c_hi = latoomutgamma -> c + c_spread;
		latoomutgamma -> d_lo = latoomutgamma -> d - d_spread;
		latoomutgamma -> d_hi = latoomutgamma -> d + d_spread;
		limiter(latoomutgamma);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for latoomutgamma constraints, requires 8 values, got %d", argc);
		return;
	}
	latoomutgamma -> a_lo = atom_getfloat(arg++);
	latoomutgamma -> a_hi = atom_getfloat(arg++);
	latoomutgamma -> b_lo = atom_getfloat(arg++);
	latoomutgamma -> b_hi = atom_getfloat(arg++);
	latoomutgamma -> c_lo = atom_getfloat(arg++);
	latoomutgamma -> c_hi = atom_getfloat(arg++);
	latoomutgamma -> d_lo = atom_getfloat(arg++);
	latoomutgamma -> d_hi = atom_getfloat(arg++);
	limiter(latoomutgamma);
}

static void search(latoomutgamma_struct *latoomutgamma, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = latoomutgamma -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = latoomutgamma -> a;
	double temp_b = latoomutgamma -> b;
	double temp_c = latoomutgamma -> c;
	double temp_d = latoomutgamma -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], latoomutgamma -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		latoomutgamma -> a = (drand48() * (latoomutgamma -> a_hi - latoomutgamma -> a_lo)) + latoomutgamma -> a_lo;
		latoomutgamma -> b = (drand48() * (latoomutgamma -> b_hi - latoomutgamma -> b_lo)) + latoomutgamma -> b_lo;
		latoomutgamma -> c = (drand48() * (latoomutgamma -> c_hi - latoomutgamma -> c_lo)) + latoomutgamma -> c_lo;
		latoomutgamma -> d = (drand48() * (latoomutgamma -> d_hi - latoomutgamma -> d_lo)) + latoomutgamma -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(latoomutgamma, NULL, argc, vars);
		do { calc(latoomutgamma, latoomutgamma -> vars); } while(jump--);
		latoomutgamma -> lyap_exp = lyapunov((void *) latoomutgamma, (t_gotfn) calc, M_var_count, (double *) latoomutgamma -> vars);
		if (isnan(latoomutgamma -> lyap_exp)) { not_found = 1; }
		if (latoomutgamma -> lyap_exp < latoomutgamma -> lyap_lo || latoomutgamma -> lyap_exp > latoomutgamma -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(latoomutgamma, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) latoomutgamma -> lyap_limit);
		post("Try using wider constraints.");
		latoomutgamma -> a = temp_a;
		latoomutgamma -> b = temp_b;
		latoomutgamma -> c = temp_c;
		latoomutgamma -> d = temp_d;
		outlet_anything(latoomutgamma -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		latoomutgamma -> failure_ratio = (latoomutgamma -> lyap_limit - not_expired) / latoomutgamma -> lyap_limit;
		make_results(latoomutgamma);
		outlet_anything(latoomutgamma -> search_outlet, gensym("search"), M_search_count, latoomutgamma -> search_out);
	}
}

void *latoomutgamma_new(t_symbol *s, int argc, t_atom *argv) {
	latoomutgamma_struct *latoomutgamma = (latoomutgamma_struct *) pd_new(latoomutgamma_class);
	if (latoomutgamma != NULL) {
		outlet_new(&latoomutgamma -> x_obj, &s_float);
		latoomutgamma -> outlets[0] = outlet_new(&latoomutgamma -> x_obj, &s_float);
		latoomutgamma -> search_outlet = outlet_new(&latoomutgamma -> x_obj, &s_list);
		latoomutgamma -> vars_outlet = outlet_new(&latoomutgamma -> x_obj, &s_list);
		latoomutgamma -> params_outlet = outlet_new(&latoomutgamma -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			latoomutgamma -> vars_init[M_x] = latoomutgamma -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			latoomutgamma -> vars_init[M_y] = latoomutgamma -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			latoomutgamma -> a = (double) atom_getfloatarg(2, argc, argv);
			latoomutgamma -> b = (double) atom_getfloatarg(3, argc, argv);
			latoomutgamma -> c = (double) atom_getfloatarg(4, argc, argv);
			latoomutgamma -> d = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for latoomutgamma fractal. Expecting 6 arguments.");
			}
			latoomutgamma -> vars_init[M_x] = 0.1;
			latoomutgamma -> vars_init[M_y] = 0.1;
			latoomutgamma -> a = 1;
			latoomutgamma -> b = 1;
			latoomutgamma -> c = 1;
			latoomutgamma -> d = 1;
		}
		constrain(latoomutgamma, NULL, 0, NULL);
		lyap(latoomutgamma, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)latoomutgamma;
}

void latoomutgamma_setup(void) {
	latoomutgamma_class = class_new(gensym("latoomutgamma"), (t_newmethod) latoomutgamma_new, 0, sizeof(latoomutgamma_struct), 0, A_GIMME, 0);
	class_addbang(latoomutgamma_class, (t_method) calculate);
	class_addmethod(latoomutgamma_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(latoomutgamma_class, (t_method) show, gensym("show"), 0);
	class_addmethod(latoomutgamma_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(latoomutgamma_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(latoomutgamma_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(latoomutgamma_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(latoomutgamma_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(latoomutgamma_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

