/* latoomutbeta Attractor PD External */
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

static char *version = "latoomutbeta v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *latoomutbeta_class;

typedef struct latoomutbeta_struct {
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
} latoomutbeta_struct;

static void calc(latoomutbeta_struct *latoomutbeta, double *vars) {
	double x_0, y_0;
	x_0 =sin(vars[M_y]*latoomutbeta -> b)+pow(sin(vars[M_x]*latoomutbeta -> b),2);
	y_0 =sin(vars[M_x]*latoomutbeta -> a)+pow(sin(vars[M_y]*latoomutbeta -> a),2);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(latoomutbeta_struct *latoomutbeta) {
	calc(latoomutbeta, latoomutbeta -> vars);
	outlet_float(latoomutbeta -> outlets[M_y - 1], latoomutbeta -> vars[M_y]);
	outlet_float(latoomutbeta -> x_obj.ob_outlet, latoomutbeta -> vars[M_x]);
} // end calculate

static void reset(latoomutbeta_struct *latoomutbeta, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		latoomutbeta -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		latoomutbeta -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		latoomutbeta -> vars[M_x] = latoomutbeta -> vars_init[M_x];
		latoomutbeta -> vars[M_y] = latoomutbeta -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(latoomutbeta_struct *latoomutbeta) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((latoomutbeta -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((latoomutbeta -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((latoomutbeta -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((latoomutbeta -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(latoomutbeta_struct *latoomutbeta) {
	SETFLOAT(&latoomutbeta -> search_out[0], latoomutbeta -> lyap_exp);
	SETSYMBOL(&latoomutbeta -> search_out[1], gensym(classify(latoomutbeta)));
	SETFLOAT(&latoomutbeta -> search_out[2], latoomutbeta -> failure_ratio);
	SETFLOAT(&latoomutbeta -> vars_out[M_x], latoomutbeta -> vars[M_x]);
	SETFLOAT(&latoomutbeta -> vars_out[M_y], latoomutbeta -> vars[M_y]);
	SETFLOAT(&latoomutbeta -> params_out[M_a], latoomutbeta -> a);
	SETFLOAT(&latoomutbeta -> params_out[M_b], latoomutbeta -> b);
	SETFLOAT(&latoomutbeta -> params_out[M_c], latoomutbeta -> c);
	SETFLOAT(&latoomutbeta -> params_out[M_d], latoomutbeta -> d);
	outlet_list(latoomutbeta -> params_outlet, gensym("list"), M_param_count, latoomutbeta -> params_out);
	outlet_list(latoomutbeta -> vars_outlet, gensym("list"), M_var_count, latoomutbeta -> vars_out);
}

static void show(latoomutbeta_struct *latoomutbeta) {
	make_results(latoomutbeta);
	outlet_anything(latoomutbeta -> search_outlet, gensym("show"), M_search_count, latoomutbeta -> search_out);
}

static void param(latoomutbeta_struct *latoomutbeta, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for latoomutbeta fractal. Expecting 4 arguments.");
		return;
	}
	latoomutbeta -> a = (double) atom_getfloatarg(0, argc, argv);
	latoomutbeta -> b = (double) atom_getfloatarg(1, argc, argv);
	latoomutbeta -> c = (double) atom_getfloatarg(2, argc, argv);
	latoomutbeta -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(latoomutbeta_struct *latoomutbeta, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(latoomutbeta_struct *latoomutbeta, t_floatarg l, t_floatarg h, t_floatarg lim) {
	latoomutbeta -> lyap_lo = l;
	latoomutbeta -> lyap_hi = h;
	latoomutbeta -> lyap_limit = (double) ((int) lim);
}

static void elyap(latoomutbeta_struct *latoomutbeta) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) latoomutbeta, (t_gotfn) calc, M_var_count, latoomutbeta -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(latoomutbeta_struct *latoomutbeta) {
	if (latoomutbeta -> a_lo < M_a_lo) { latoomutbeta -> a_lo = M_a_lo; }
	if (latoomutbeta -> a_lo > M_a_hi) { latoomutbeta -> a_lo = M_a_hi; }
	if (latoomutbeta -> a_hi < M_a_lo) { latoomutbeta -> a_hi = M_a_lo; }
	if (latoomutbeta -> a_hi > M_a_hi) { latoomutbeta -> a_hi = M_a_hi; }
	if (latoomutbeta -> b_lo < M_b_lo) { latoomutbeta -> b_lo = M_b_lo; }
	if (latoomutbeta -> b_lo > M_b_hi) { latoomutbeta -> b_lo = M_b_hi; }
	if (latoomutbeta -> b_hi < M_b_lo) { latoomutbeta -> b_hi = M_b_lo; }
	if (latoomutbeta -> b_hi > M_b_hi) { latoomutbeta -> b_hi = M_b_hi; }
	if (latoomutbeta -> c_lo < M_c_lo) { latoomutbeta -> c_lo = M_c_lo; }
	if (latoomutbeta -> c_lo > M_c_hi) { latoomutbeta -> c_lo = M_c_hi; }
	if (latoomutbeta -> c_hi < M_c_lo) { latoomutbeta -> c_hi = M_c_lo; }
	if (latoomutbeta -> c_hi > M_c_hi) { latoomutbeta -> c_hi = M_c_hi; }
	if (latoomutbeta -> d_lo < M_d_lo) { latoomutbeta -> d_lo = M_d_lo; }
	if (latoomutbeta -> d_lo > M_d_hi) { latoomutbeta -> d_lo = M_d_hi; }
	if (latoomutbeta -> d_hi < M_d_lo) { latoomutbeta -> d_hi = M_d_lo; }
	if (latoomutbeta -> d_hi > M_d_hi) { latoomutbeta -> d_hi = M_d_hi; }
}

static void constrain(latoomutbeta_struct *latoomutbeta, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		latoomutbeta -> a_lo = M_a_lo;
		latoomutbeta -> a_hi = M_a_hi;
		latoomutbeta -> b_lo = M_b_lo;
		latoomutbeta -> b_hi = M_b_hi;
		latoomutbeta -> c_lo = M_c_lo;
		latoomutbeta -> c_hi = M_c_hi;
		latoomutbeta -> d_lo = M_d_lo;
		latoomutbeta -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		latoomutbeta -> a_lo = latoomutbeta -> a - a_spread;
		latoomutbeta -> a_hi = latoomutbeta -> a + a_spread;
		latoomutbeta -> b_lo = latoomutbeta -> b - b_spread;
		latoomutbeta -> b_hi = latoomutbeta -> b + b_spread;
		latoomutbeta -> c_lo = latoomutbeta -> c - c_spread;
		latoomutbeta -> c_hi = latoomutbeta -> c + c_spread;
		latoomutbeta -> d_lo = latoomutbeta -> d - d_spread;
		latoomutbeta -> d_hi = latoomutbeta -> d + d_spread;
		limiter(latoomutbeta);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for latoomutbeta constraints, requires 8 values, got %d", argc);
		return;
	}
	latoomutbeta -> a_lo = atom_getfloat(arg++);
	latoomutbeta -> a_hi = atom_getfloat(arg++);
	latoomutbeta -> b_lo = atom_getfloat(arg++);
	latoomutbeta -> b_hi = atom_getfloat(arg++);
	latoomutbeta -> c_lo = atom_getfloat(arg++);
	latoomutbeta -> c_hi = atom_getfloat(arg++);
	latoomutbeta -> d_lo = atom_getfloat(arg++);
	latoomutbeta -> d_hi = atom_getfloat(arg++);
	limiter(latoomutbeta);
}

static void search(latoomutbeta_struct *latoomutbeta, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = latoomutbeta -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = latoomutbeta -> a;
	double temp_b = latoomutbeta -> b;
	double temp_c = latoomutbeta -> c;
	double temp_d = latoomutbeta -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], latoomutbeta -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		latoomutbeta -> a = (drand48() * (latoomutbeta -> a_hi - latoomutbeta -> a_lo)) + latoomutbeta -> a_lo;
		latoomutbeta -> b = (drand48() * (latoomutbeta -> b_hi - latoomutbeta -> b_lo)) + latoomutbeta -> b_lo;
		latoomutbeta -> c = (drand48() * (latoomutbeta -> c_hi - latoomutbeta -> c_lo)) + latoomutbeta -> c_lo;
		latoomutbeta -> d = (drand48() * (latoomutbeta -> d_hi - latoomutbeta -> d_lo)) + latoomutbeta -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(latoomutbeta, NULL, argc, vars);
		do { calc(latoomutbeta, latoomutbeta -> vars); } while(jump--);
		latoomutbeta -> lyap_exp = lyapunov((void *) latoomutbeta, (t_gotfn) calc, M_var_count, (double *) latoomutbeta -> vars);
		if (isnan(latoomutbeta -> lyap_exp)) { not_found = 1; }
		if (latoomutbeta -> lyap_exp < latoomutbeta -> lyap_lo || latoomutbeta -> lyap_exp > latoomutbeta -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(latoomutbeta, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) latoomutbeta -> lyap_limit);
		post("Try using wider constraints.");
		latoomutbeta -> a = temp_a;
		latoomutbeta -> b = temp_b;
		latoomutbeta -> c = temp_c;
		latoomutbeta -> d = temp_d;
		outlet_anything(latoomutbeta -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		latoomutbeta -> failure_ratio = (latoomutbeta -> lyap_limit - not_expired) / latoomutbeta -> lyap_limit;
		make_results(latoomutbeta);
		outlet_anything(latoomutbeta -> search_outlet, gensym("search"), M_search_count, latoomutbeta -> search_out);
	}
}

void *latoomutbeta_new(t_symbol *s, int argc, t_atom *argv) {
	latoomutbeta_struct *latoomutbeta = (latoomutbeta_struct *) pd_new(latoomutbeta_class);
	if (latoomutbeta != NULL) {
		outlet_new(&latoomutbeta -> x_obj, &s_float);
		latoomutbeta -> outlets[0] = outlet_new(&latoomutbeta -> x_obj, &s_float);
		latoomutbeta -> search_outlet = outlet_new(&latoomutbeta -> x_obj, &s_list);
		latoomutbeta -> vars_outlet = outlet_new(&latoomutbeta -> x_obj, &s_list);
		latoomutbeta -> params_outlet = outlet_new(&latoomutbeta -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			latoomutbeta -> vars_init[M_x] = latoomutbeta -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			latoomutbeta -> vars_init[M_y] = latoomutbeta -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			latoomutbeta -> a = (double) atom_getfloatarg(2, argc, argv);
			latoomutbeta -> b = (double) atom_getfloatarg(3, argc, argv);
			latoomutbeta -> c = (double) atom_getfloatarg(4, argc, argv);
			latoomutbeta -> d = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for latoomutbeta fractal. Expecting 6 arguments.");
			}
			latoomutbeta -> vars_init[M_x] = 0.1;
			latoomutbeta -> vars_init[M_y] = 0.1;
			latoomutbeta -> a = 1;
			latoomutbeta -> b = 1;
			latoomutbeta -> c = 1;
			latoomutbeta -> d = 1;
		}
		constrain(latoomutbeta, NULL, 0, NULL);
		lyap(latoomutbeta, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)latoomutbeta;
}

void latoomutbeta_setup(void) {
	latoomutbeta_class = class_new(gensym("latoomutbeta"), (t_newmethod) latoomutbeta_new, 0, sizeof(latoomutbeta_struct), 0, A_GIMME, 0);
	class_addbang(latoomutbeta_class, (t_method) calculate);
	class_addmethod(latoomutbeta_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(latoomutbeta_class, (t_method) show, gensym("show"), 0);
	class_addmethod(latoomutbeta_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(latoomutbeta_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(latoomutbeta_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(latoomutbeta_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(latoomutbeta_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(latoomutbeta_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

