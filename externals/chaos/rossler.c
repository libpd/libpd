/* rossler Attractor PD External */
/* Copyright Ben Bogart, 2002 */
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

#define M_h_lo -1000
#define M_h_hi 1000
#define M_a_lo -1000
#define M_a_hi 1000
#define M_b_lo -1000
#define M_b_hi 1000
#define M_c_lo -1000
#define M_c_hi 1000

#define M_h 0
#define M_a 1
#define M_b 2
#define M_c 3

#define M_x 0
#define M_y 1
#define M_z 2

#define M_param_count 4
#define M_var_count 3
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "rossler v0.0, by Ben Bogart, 2002";

t_class *rossler_class;

typedef struct rossler_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double h, h_lo, h_hi, a, a_lo, a_hi, b, b_lo, b_hi, c, c_lo, c_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} rossler_struct;

static void calc(rossler_struct *rossler, double *vars) {
	double x_0, y_0, z_0;
	x_0 =vars[M_x]+rossler -> h*(-vars[M_y]-vars[M_z]);
	y_0 =vars[M_y]+rossler -> h*(vars[M_x]+(rossler -> a*vars[M_y]));
	z_0 =vars[M_z]+rossler -> h*(rossler -> b+(vars[M_x]*vars[M_z])-(rossler -> c*vars[M_z]));
	vars[M_x] = x_0;
	vars[M_y] = y_0;
	vars[M_z] = z_0;
} // end calc

static void calculate(rossler_struct *rossler) {
	calc(rossler, rossler -> vars);
	outlet_float(rossler -> outlets[M_z - 1], rossler -> vars[M_z]);
	outlet_float(rossler -> outlets[M_y - 1], rossler -> vars[M_y]);
	outlet_float(rossler -> x_obj.ob_outlet, rossler -> vars[M_x]);
} // end calculate

static void reset(rossler_struct *rossler, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		rossler -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		rossler -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
		rossler -> vars[M_z] = (double) atom_getfloatarg(M_z, argc, argv);
	} else {
		rossler -> vars[M_x] = rossler -> vars_init[M_x];
		rossler -> vars[M_y] = rossler -> vars_init[M_y];
		rossler -> vars[M_z] = rossler -> vars_init[M_z];
	} // end if
} // end reset

static char *classify(rossler_struct *rossler) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((rossler -> h - M_h_lo) * (1.0 / (M_h_hi - M_h_lo))) * 26)];
	buff[1] = c[(int) (((rossler -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[2] = c[(int) (((rossler -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[3] = c[(int) (((rossler -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(rossler_struct *rossler) {
	SETFLOAT(&rossler -> search_out[0], rossler -> lyap_exp);
	SETSYMBOL(&rossler -> search_out[1], gensym(classify(rossler)));
	SETFLOAT(&rossler -> search_out[2], rossler -> failure_ratio);
	SETFLOAT(&rossler -> vars_out[M_x], rossler -> vars[M_x]);
	SETFLOAT(&rossler -> vars_out[M_y], rossler -> vars[M_y]);
	SETFLOAT(&rossler -> vars_out[M_z], rossler -> vars[M_z]);
	SETFLOAT(&rossler -> params_out[M_h], rossler -> h);
	SETFLOAT(&rossler -> params_out[M_a], rossler -> a);
	SETFLOAT(&rossler -> params_out[M_b], rossler -> b);
	SETFLOAT(&rossler -> params_out[M_c], rossler -> c);
	outlet_list(rossler -> params_outlet, gensym("list"), M_param_count, rossler -> params_out);
	outlet_list(rossler -> vars_outlet, gensym("list"), M_var_count, rossler -> vars_out);
}

static void show(rossler_struct *rossler) {
	make_results(rossler);
	outlet_anything(rossler -> search_outlet, gensym("show"), M_search_count, rossler -> search_out);
}

static void param(rossler_struct *rossler, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for rossler fractal. Expecting 4 arguments.");
		return;
	}
	rossler -> h = (double) atom_getfloatarg(0, argc, argv);
	rossler -> a = (double) atom_getfloatarg(1, argc, argv);
	rossler -> b = (double) atom_getfloatarg(2, argc, argv);
	rossler -> c = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(rossler_struct *rossler, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(rossler_struct *rossler, t_floatarg l, t_floatarg h, t_floatarg lim) {
	rossler -> lyap_lo = l;
	rossler -> lyap_hi = h;
	rossler -> lyap_limit = (double) ((int) lim);
}

static void elyap(rossler_struct *rossler) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) rossler, (t_gotfn) calc, M_var_count, rossler -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(rossler_struct *rossler) {
	if (rossler -> h_lo < M_h_lo) { rossler -> h_lo = M_h_lo; }
	if (rossler -> h_lo > M_h_hi) { rossler -> h_lo = M_h_hi; }
	if (rossler -> h_hi < M_h_lo) { rossler -> h_hi = M_h_lo; }
	if (rossler -> h_hi > M_h_hi) { rossler -> h_hi = M_h_hi; }
	if (rossler -> a_lo < M_a_lo) { rossler -> a_lo = M_a_lo; }
	if (rossler -> a_lo > M_a_hi) { rossler -> a_lo = M_a_hi; }
	if (rossler -> a_hi < M_a_lo) { rossler -> a_hi = M_a_lo; }
	if (rossler -> a_hi > M_a_hi) { rossler -> a_hi = M_a_hi; }
	if (rossler -> b_lo < M_b_lo) { rossler -> b_lo = M_b_lo; }
	if (rossler -> b_lo > M_b_hi) { rossler -> b_lo = M_b_hi; }
	if (rossler -> b_hi < M_b_lo) { rossler -> b_hi = M_b_lo; }
	if (rossler -> b_hi > M_b_hi) { rossler -> b_hi = M_b_hi; }
	if (rossler -> c_lo < M_c_lo) { rossler -> c_lo = M_c_lo; }
	if (rossler -> c_lo > M_c_hi) { rossler -> c_lo = M_c_hi; }
	if (rossler -> c_hi < M_c_lo) { rossler -> c_hi = M_c_lo; }
	if (rossler -> c_hi > M_c_hi) { rossler -> c_hi = M_c_hi; }
}

static void constrain(rossler_struct *rossler, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		rossler -> h_lo = M_h_lo;
		rossler -> h_hi = M_h_hi;
		rossler -> a_lo = M_a_lo;
		rossler -> a_hi = M_a_hi;
		rossler -> b_lo = M_b_lo;
		rossler -> b_hi = M_b_hi;
		rossler -> c_lo = M_c_lo;
		rossler -> c_hi = M_c_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double h_spread = ((M_h_hi - M_h_lo) * percent) / 2;
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		rossler -> h_lo = rossler -> h - h_spread;
		rossler -> h_hi = rossler -> h + h_spread;
		rossler -> a_lo = rossler -> a - a_spread;
		rossler -> a_hi = rossler -> a + a_spread;
		rossler -> b_lo = rossler -> b - b_spread;
		rossler -> b_hi = rossler -> b + b_spread;
		rossler -> c_lo = rossler -> c - c_spread;
		rossler -> c_hi = rossler -> c + c_spread;
		limiter(rossler);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for rossler constraints, requires 8 values, got %d", argc);
		return;
	}
	rossler -> h_lo = atom_getfloat(arg++);
	rossler -> h_hi = atom_getfloat(arg++);
	rossler -> a_lo = atom_getfloat(arg++);
	rossler -> a_hi = atom_getfloat(arg++);
	rossler -> b_lo = atom_getfloat(arg++);
	rossler -> b_hi = atom_getfloat(arg++);
	rossler -> c_lo = atom_getfloat(arg++);
	rossler -> c_hi = atom_getfloat(arg++);
	limiter(rossler);
}

static void search(rossler_struct *rossler, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = rossler -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_h = rossler -> h;
	double temp_a = rossler -> a;
	double temp_b = rossler -> b;
	double temp_c = rossler -> c;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], rossler -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		rossler -> h = (drand48() * (rossler -> h_hi - rossler -> h_lo)) + rossler -> h_lo;
		rossler -> a = (drand48() * (rossler -> a_hi - rossler -> a_lo)) + rossler -> a_lo;
		rossler -> b = (drand48() * (rossler -> b_hi - rossler -> b_lo)) + rossler -> b_lo;
		rossler -> c = (drand48() * (rossler -> c_hi - rossler -> c_lo)) + rossler -> c_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(rossler, NULL, argc, vars);
		do { calc(rossler, rossler -> vars); } while(jump--);
		rossler -> lyap_exp = lyapunov((void *) rossler, (t_gotfn) calc, M_var_count, (double *) rossler -> vars);
		if (isnan(rossler -> lyap_exp)) { not_found = 1; }
		if (rossler -> lyap_exp < rossler -> lyap_lo || rossler -> lyap_exp > rossler -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(rossler, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) rossler -> lyap_limit);
		post("Try using wider constraints.");
		rossler -> h = temp_h;
		rossler -> a = temp_a;
		rossler -> b = temp_b;
		rossler -> c = temp_c;
		outlet_anything(rossler -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		rossler -> failure_ratio = (rossler -> lyap_limit - not_expired) / rossler -> lyap_limit;
		make_results(rossler);
		outlet_anything(rossler -> search_outlet, gensym("search"), M_search_count, rossler -> search_out);
	}
}

void *rossler_new(t_symbol *s, int argc, t_atom *argv) {
	rossler_struct *rossler = (rossler_struct *) pd_new(rossler_class);
	if (rossler != NULL) {
		outlet_new(&rossler -> x_obj, &s_float);
		rossler -> outlets[0] = outlet_new(&rossler -> x_obj, &s_float);
		rossler -> outlets[1] = outlet_new(&rossler -> x_obj, &s_float);
		rossler -> search_outlet = outlet_new(&rossler -> x_obj, &s_list);
		rossler -> vars_outlet = outlet_new(&rossler -> x_obj, &s_list);
		rossler -> params_outlet = outlet_new(&rossler -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			rossler -> vars_init[M_x] = rossler -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			rossler -> vars_init[M_y] = rossler -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			rossler -> vars_init[M_z] = rossler -> vars[M_z] = (double) atom_getfloatarg(2, argc, argv);
			rossler -> h = (double) atom_getfloatarg(3, argc, argv);
			rossler -> a = (double) atom_getfloatarg(4, argc, argv);
			rossler -> b = (double) atom_getfloatarg(5, argc, argv);
			rossler -> c = (double) atom_getfloatarg(6, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for rossler fractal. Expecting 7 arguments.");
			}
			rossler -> vars_init[M_x] = 0.1;
			rossler -> vars_init[M_y] = 0;
			rossler -> vars_init[M_z] = 0;
			rossler -> h = 0.01;
			rossler -> a = 0.2;
			rossler -> b = 0.2;
			rossler -> c = 5.7;
		}
		constrain(rossler, NULL, 0, NULL);
		lyap(rossler, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)rossler;
}

void rossler_setup(void) {
	rossler_class = class_new(gensym("rossler"), (t_newmethod) rossler_new, 0, sizeof(rossler_struct), 0, A_GIMME, 0);
	class_addbang(rossler_class, (t_method) calculate);
	class_addmethod(rossler_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(rossler_class, (t_method) show, gensym("show"), 0);
	class_addmethod(rossler_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(rossler_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(rossler_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(rossler_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(rossler_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(rossler_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

