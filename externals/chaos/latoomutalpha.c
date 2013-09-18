/* latoomutalpha Attractor PD External */
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

static char *version = "latoomutalpha v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *latoomutalpha_class;

typedef struct latoomutalpha_struct {
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
} latoomutalpha_struct;

static void calc(latoomutalpha_struct *latoomutalpha, double *vars) {
	double x_0, y_0;
	x_0 =sin(vars[M_y]*latoomutalpha -> b)+pow(sin(vars[M_x]*latoomutalpha -> b),2)+pow(sin(vars[M_x]*latoomutalpha -> b),3);
	y_0 =sin(vars[M_x]*latoomutalpha -> a)+pow(sin(vars[M_y]*latoomutalpha -> a),2)+pow(sin(vars[M_y]*latoomutalpha -> c),3);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(latoomutalpha_struct *latoomutalpha) {
	calc(latoomutalpha, latoomutalpha -> vars);
	outlet_float(latoomutalpha -> outlets[M_y - 1], latoomutalpha -> vars[M_y]);
	outlet_float(latoomutalpha -> x_obj.ob_outlet, latoomutalpha -> vars[M_x]);
} // end calculate

static void reset(latoomutalpha_struct *latoomutalpha, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		latoomutalpha -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		latoomutalpha -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		latoomutalpha -> vars[M_x] = latoomutalpha -> vars_init[M_x];
		latoomutalpha -> vars[M_y] = latoomutalpha -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(latoomutalpha_struct *latoomutalpha) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((latoomutalpha -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((latoomutalpha -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((latoomutalpha -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((latoomutalpha -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(latoomutalpha_struct *latoomutalpha) {
	SETFLOAT(&latoomutalpha -> search_out[0], latoomutalpha -> lyap_exp);
	SETSYMBOL(&latoomutalpha -> search_out[1], gensym(classify(latoomutalpha)));
	SETFLOAT(&latoomutalpha -> search_out[2], latoomutalpha -> failure_ratio);
	SETFLOAT(&latoomutalpha -> vars_out[M_x], latoomutalpha -> vars[M_x]);
	SETFLOAT(&latoomutalpha -> vars_out[M_y], latoomutalpha -> vars[M_y]);
	SETFLOAT(&latoomutalpha -> params_out[M_a], latoomutalpha -> a);
	SETFLOAT(&latoomutalpha -> params_out[M_b], latoomutalpha -> b);
	SETFLOAT(&latoomutalpha -> params_out[M_c], latoomutalpha -> c);
	SETFLOAT(&latoomutalpha -> params_out[M_d], latoomutalpha -> d);
	outlet_list(latoomutalpha -> params_outlet, gensym("list"), M_param_count, latoomutalpha -> params_out);
	outlet_list(latoomutalpha -> vars_outlet, gensym("list"), M_var_count, latoomutalpha -> vars_out);
}

static void show(latoomutalpha_struct *latoomutalpha) {
	make_results(latoomutalpha);
	outlet_anything(latoomutalpha -> search_outlet, gensym("show"), M_search_count, latoomutalpha -> search_out);
}

static void param(latoomutalpha_struct *latoomutalpha, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for latoomutalpha fractal. Expecting 4 arguments.");
		return;
	}
	latoomutalpha -> a = (double) atom_getfloatarg(0, argc, argv);
	latoomutalpha -> b = (double) atom_getfloatarg(1, argc, argv);
	latoomutalpha -> c = (double) atom_getfloatarg(2, argc, argv);
	latoomutalpha -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(latoomutalpha_struct *latoomutalpha, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(latoomutalpha_struct *latoomutalpha, t_floatarg l, t_floatarg h, t_floatarg lim) {
	latoomutalpha -> lyap_lo = l;
	latoomutalpha -> lyap_hi = h;
	latoomutalpha -> lyap_limit = (double) ((int) lim);
}

static void elyap(latoomutalpha_struct *latoomutalpha) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) latoomutalpha, (t_gotfn) calc, M_var_count, latoomutalpha -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(latoomutalpha_struct *latoomutalpha) {
	if (latoomutalpha -> a_lo < M_a_lo) { latoomutalpha -> a_lo = M_a_lo; }
	if (latoomutalpha -> a_lo > M_a_hi) { latoomutalpha -> a_lo = M_a_hi; }
	if (latoomutalpha -> a_hi < M_a_lo) { latoomutalpha -> a_hi = M_a_lo; }
	if (latoomutalpha -> a_hi > M_a_hi) { latoomutalpha -> a_hi = M_a_hi; }
	if (latoomutalpha -> b_lo < M_b_lo) { latoomutalpha -> b_lo = M_b_lo; }
	if (latoomutalpha -> b_lo > M_b_hi) { latoomutalpha -> b_lo = M_b_hi; }
	if (latoomutalpha -> b_hi < M_b_lo) { latoomutalpha -> b_hi = M_b_lo; }
	if (latoomutalpha -> b_hi > M_b_hi) { latoomutalpha -> b_hi = M_b_hi; }
	if (latoomutalpha -> c_lo < M_c_lo) { latoomutalpha -> c_lo = M_c_lo; }
	if (latoomutalpha -> c_lo > M_c_hi) { latoomutalpha -> c_lo = M_c_hi; }
	if (latoomutalpha -> c_hi < M_c_lo) { latoomutalpha -> c_hi = M_c_lo; }
	if (latoomutalpha -> c_hi > M_c_hi) { latoomutalpha -> c_hi = M_c_hi; }
	if (latoomutalpha -> d_lo < M_d_lo) { latoomutalpha -> d_lo = M_d_lo; }
	if (latoomutalpha -> d_lo > M_d_hi) { latoomutalpha -> d_lo = M_d_hi; }
	if (latoomutalpha -> d_hi < M_d_lo) { latoomutalpha -> d_hi = M_d_lo; }
	if (latoomutalpha -> d_hi > M_d_hi) { latoomutalpha -> d_hi = M_d_hi; }
}

static void constrain(latoomutalpha_struct *latoomutalpha, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		latoomutalpha -> a_lo = M_a_lo;
		latoomutalpha -> a_hi = M_a_hi;
		latoomutalpha -> b_lo = M_b_lo;
		latoomutalpha -> b_hi = M_b_hi;
		latoomutalpha -> c_lo = M_c_lo;
		latoomutalpha -> c_hi = M_c_hi;
		latoomutalpha -> d_lo = M_d_lo;
		latoomutalpha -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		latoomutalpha -> a_lo = latoomutalpha -> a - a_spread;
		latoomutalpha -> a_hi = latoomutalpha -> a + a_spread;
		latoomutalpha -> b_lo = latoomutalpha -> b - b_spread;
		latoomutalpha -> b_hi = latoomutalpha -> b + b_spread;
		latoomutalpha -> c_lo = latoomutalpha -> c - c_spread;
		latoomutalpha -> c_hi = latoomutalpha -> c + c_spread;
		latoomutalpha -> d_lo = latoomutalpha -> d - d_spread;
		latoomutalpha -> d_hi = latoomutalpha -> d + d_spread;
		limiter(latoomutalpha);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for latoomutalpha constraints, requires 8 values, got %d", argc);
		return;
	}
	latoomutalpha -> a_lo = atom_getfloat(arg++);
	latoomutalpha -> a_hi = atom_getfloat(arg++);
	latoomutalpha -> b_lo = atom_getfloat(arg++);
	latoomutalpha -> b_hi = atom_getfloat(arg++);
	latoomutalpha -> c_lo = atom_getfloat(arg++);
	latoomutalpha -> c_hi = atom_getfloat(arg++);
	latoomutalpha -> d_lo = atom_getfloat(arg++);
	latoomutalpha -> d_hi = atom_getfloat(arg++);
	limiter(latoomutalpha);
}

static void search(latoomutalpha_struct *latoomutalpha, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = latoomutalpha -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = latoomutalpha -> a;
	double temp_b = latoomutalpha -> b;
	double temp_c = latoomutalpha -> c;
	double temp_d = latoomutalpha -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], latoomutalpha -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		latoomutalpha -> a = (drand48() * (latoomutalpha -> a_hi - latoomutalpha -> a_lo)) + latoomutalpha -> a_lo;
		latoomutalpha -> b = (drand48() * (latoomutalpha -> b_hi - latoomutalpha -> b_lo)) + latoomutalpha -> b_lo;
		latoomutalpha -> c = (drand48() * (latoomutalpha -> c_hi - latoomutalpha -> c_lo)) + latoomutalpha -> c_lo;
		latoomutalpha -> d = (drand48() * (latoomutalpha -> d_hi - latoomutalpha -> d_lo)) + latoomutalpha -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(latoomutalpha, NULL, argc, vars);
		do { calc(latoomutalpha, latoomutalpha -> vars); } while(jump--);
		latoomutalpha -> lyap_exp = lyapunov((void *) latoomutalpha, (t_gotfn) calc, M_var_count, (double *) latoomutalpha -> vars);
		if (isnan(latoomutalpha -> lyap_exp)) { not_found = 1; }
		if (latoomutalpha -> lyap_exp < latoomutalpha -> lyap_lo || latoomutalpha -> lyap_exp > latoomutalpha -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(latoomutalpha, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) latoomutalpha -> lyap_limit);
		post("Try using wider constraints.");
		latoomutalpha -> a = temp_a;
		latoomutalpha -> b = temp_b;
		latoomutalpha -> c = temp_c;
		latoomutalpha -> d = temp_d;
		outlet_anything(latoomutalpha -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		latoomutalpha -> failure_ratio = (latoomutalpha -> lyap_limit - not_expired) / latoomutalpha -> lyap_limit;
		make_results(latoomutalpha);
		outlet_anything(latoomutalpha -> search_outlet, gensym("search"), M_search_count, latoomutalpha -> search_out);
	}
}

void *latoomutalpha_new(t_symbol *s, int argc, t_atom *argv) {
	latoomutalpha_struct *latoomutalpha = (latoomutalpha_struct *) pd_new(latoomutalpha_class);
	if (latoomutalpha != NULL) {
		outlet_new(&latoomutalpha -> x_obj, &s_float);
		latoomutalpha -> outlets[0] = outlet_new(&latoomutalpha -> x_obj, &s_float);
		latoomutalpha -> search_outlet = outlet_new(&latoomutalpha -> x_obj, &s_list);
		latoomutalpha -> vars_outlet = outlet_new(&latoomutalpha -> x_obj, &s_list);
		latoomutalpha -> params_outlet = outlet_new(&latoomutalpha -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			latoomutalpha -> vars_init[M_x] = latoomutalpha -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			latoomutalpha -> vars_init[M_y] = latoomutalpha -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			latoomutalpha -> a = (double) atom_getfloatarg(2, argc, argv);
			latoomutalpha -> b = (double) atom_getfloatarg(3, argc, argv);
			latoomutalpha -> c = (double) atom_getfloatarg(4, argc, argv);
			latoomutalpha -> d = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for latoomutalpha fractal. Expecting 6 arguments.");
			}
			latoomutalpha -> vars_init[M_x] = 0.1;
			latoomutalpha -> vars_init[M_y] = 0.1;
			latoomutalpha -> a = 1;
			latoomutalpha -> b = 1;
			latoomutalpha -> c = 1;
			latoomutalpha -> d = 1;
		}
		constrain(latoomutalpha, NULL, 0, NULL);
		lyap(latoomutalpha, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)latoomutalpha;
}

void latoomutalpha_setup(void) {
	latoomutalpha_class = class_new(gensym("latoomutalpha"), (t_newmethod) latoomutalpha_new, 0, sizeof(latoomutalpha_struct), 0, A_GIMME, 0);
	class_addbang(latoomutalpha_class, (t_method) calculate);
	class_addmethod(latoomutalpha_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(latoomutalpha_class, (t_method) show, gensym("show"), 0);
	class_addmethod(latoomutalpha_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(latoomutalpha_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(latoomutalpha_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(latoomutalpha_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(latoomutalpha_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(latoomutalpha_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

