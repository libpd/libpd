/* lorenz Attractor PD External */
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

static char *version = "lorenz v0.0, by Ben Bogart, 2002";

t_class *lorenz_class;

typedef struct lorenz_struct {
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
} lorenz_struct;

static void calc(lorenz_struct *lorenz, double *vars) {
	double x_0, y_0, z_0;
	x_0 =vars[M_x]+lorenz -> h*lorenz -> a*(vars[M_y]-vars[M_x]);
	y_0 =vars[M_y]+lorenz -> h*(vars[M_x]*(lorenz -> b-vars[M_z])-vars[M_y]);
	z_0 =vars[M_z]+lorenz -> h*(vars[M_x]*vars[M_y]-lorenz -> c*vars[M_z]);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
	vars[M_z] = z_0;
} // end calc

static void calculate(lorenz_struct *lorenz) {
	calc(lorenz, lorenz -> vars);
	outlet_float(lorenz -> outlets[M_z - 1], lorenz -> vars[M_z]);
	outlet_float(lorenz -> outlets[M_y - 1], lorenz -> vars[M_y]);
	outlet_float(lorenz -> x_obj.ob_outlet, lorenz -> vars[M_x]);
} // end calculate

static void reset(lorenz_struct *lorenz, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		lorenz -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		lorenz -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
		lorenz -> vars[M_z] = (double) atom_getfloatarg(M_z, argc, argv);
	} else {
		lorenz -> vars[M_x] = lorenz -> vars_init[M_x];
		lorenz -> vars[M_y] = lorenz -> vars_init[M_y];
		lorenz -> vars[M_z] = lorenz -> vars_init[M_z];
	} // end if
} // end reset

static char *classify(lorenz_struct *lorenz) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((lorenz -> h - M_h_lo) * (1.0 / (M_h_hi - M_h_lo))) * 26)];
	buff[1] = c[(int) (((lorenz -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[2] = c[(int) (((lorenz -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[3] = c[(int) (((lorenz -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(lorenz_struct *lorenz) {
	SETFLOAT(&lorenz -> search_out[0], lorenz -> lyap_exp);
	SETSYMBOL(&lorenz -> search_out[1], gensym(classify(lorenz)));
	SETFLOAT(&lorenz -> search_out[2], lorenz -> failure_ratio);
	SETFLOAT(&lorenz -> vars_out[M_x], lorenz -> vars[M_x]);
	SETFLOAT(&lorenz -> vars_out[M_y], lorenz -> vars[M_y]);
	SETFLOAT(&lorenz -> vars_out[M_z], lorenz -> vars[M_z]);
	SETFLOAT(&lorenz -> params_out[M_h], lorenz -> h);
	SETFLOAT(&lorenz -> params_out[M_a], lorenz -> a);
	SETFLOAT(&lorenz -> params_out[M_b], lorenz -> b);
	SETFLOAT(&lorenz -> params_out[M_c], lorenz -> c);
	outlet_list(lorenz -> params_outlet, gensym("list"), M_param_count, lorenz -> params_out);
	outlet_list(lorenz -> vars_outlet, gensym("list"), M_var_count, lorenz -> vars_out);
}

static void show(lorenz_struct *lorenz) {
	make_results(lorenz);
	outlet_anything(lorenz -> search_outlet, gensym("show"), M_search_count, lorenz -> search_out);
}

static void param(lorenz_struct *lorenz, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for lorenz fractal. Expecting 4 arguments.");
		return;
	}
	lorenz -> h = (double) atom_getfloatarg(0, argc, argv);
	lorenz -> a = (double) atom_getfloatarg(1, argc, argv);
	lorenz -> b = (double) atom_getfloatarg(2, argc, argv);
	lorenz -> c = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(lorenz_struct *lorenz, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(lorenz_struct *lorenz, t_floatarg l, t_floatarg h, t_floatarg lim) {
	lorenz -> lyap_lo = l;
	lorenz -> lyap_hi = h;
	lorenz -> lyap_limit = (double) ((int) lim);
}

static void elyap(lorenz_struct *lorenz) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) lorenz, (t_gotfn) calc, M_var_count, lorenz -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(lorenz_struct *lorenz) {
	if (lorenz -> h_lo < M_h_lo) { lorenz -> h_lo = M_h_lo; }
	if (lorenz -> h_lo > M_h_hi) { lorenz -> h_lo = M_h_hi; }
	if (lorenz -> h_hi < M_h_lo) { lorenz -> h_hi = M_h_lo; }
	if (lorenz -> h_hi > M_h_hi) { lorenz -> h_hi = M_h_hi; }
	if (lorenz -> a_lo < M_a_lo) { lorenz -> a_lo = M_a_lo; }
	if (lorenz -> a_lo > M_a_hi) { lorenz -> a_lo = M_a_hi; }
	if (lorenz -> a_hi < M_a_lo) { lorenz -> a_hi = M_a_lo; }
	if (lorenz -> a_hi > M_a_hi) { lorenz -> a_hi = M_a_hi; }
	if (lorenz -> b_lo < M_b_lo) { lorenz -> b_lo = M_b_lo; }
	if (lorenz -> b_lo > M_b_hi) { lorenz -> b_lo = M_b_hi; }
	if (lorenz -> b_hi < M_b_lo) { lorenz -> b_hi = M_b_lo; }
	if (lorenz -> b_hi > M_b_hi) { lorenz -> b_hi = M_b_hi; }
	if (lorenz -> c_lo < M_c_lo) { lorenz -> c_lo = M_c_lo; }
	if (lorenz -> c_lo > M_c_hi) { lorenz -> c_lo = M_c_hi; }
	if (lorenz -> c_hi < M_c_lo) { lorenz -> c_hi = M_c_lo; }
	if (lorenz -> c_hi > M_c_hi) { lorenz -> c_hi = M_c_hi; }
}

static void constrain(lorenz_struct *lorenz, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		lorenz -> h_lo = M_h_lo;
		lorenz -> h_hi = M_h_hi;
		lorenz -> a_lo = M_a_lo;
		lorenz -> a_hi = M_a_hi;
		lorenz -> b_lo = M_b_lo;
		lorenz -> b_hi = M_b_hi;
		lorenz -> c_lo = M_c_lo;
		lorenz -> c_hi = M_c_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double h_spread = ((M_h_hi - M_h_lo) * percent) / 2;
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		lorenz -> h_lo = lorenz -> h - h_spread;
		lorenz -> h_hi = lorenz -> h + h_spread;
		lorenz -> a_lo = lorenz -> a - a_spread;
		lorenz -> a_hi = lorenz -> a + a_spread;
		lorenz -> b_lo = lorenz -> b - b_spread;
		lorenz -> b_hi = lorenz -> b + b_spread;
		lorenz -> c_lo = lorenz -> c - c_spread;
		lorenz -> c_hi = lorenz -> c + c_spread;
		limiter(lorenz);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for lorenz constraints, requires 8 values, got %d", argc);
		return;
	}
	lorenz -> h_lo = atom_getfloat(arg++);
	lorenz -> h_hi = atom_getfloat(arg++);
	lorenz -> a_lo = atom_getfloat(arg++);
	lorenz -> a_hi = atom_getfloat(arg++);
	lorenz -> b_lo = atom_getfloat(arg++);
	lorenz -> b_hi = atom_getfloat(arg++);
	lorenz -> c_lo = atom_getfloat(arg++);
	lorenz -> c_hi = atom_getfloat(arg++);
	limiter(lorenz);
}

static void search(lorenz_struct *lorenz, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = lorenz -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_h = lorenz -> h;
	double temp_a = lorenz -> a;
	double temp_b = lorenz -> b;
	double temp_c = lorenz -> c;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], lorenz -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		lorenz -> h = (drand48() * (lorenz -> h_hi - lorenz -> h_lo)) + lorenz -> h_lo;
		lorenz -> a = (drand48() * (lorenz -> a_hi - lorenz -> a_lo)) + lorenz -> a_lo;
		lorenz -> b = (drand48() * (lorenz -> b_hi - lorenz -> b_lo)) + lorenz -> b_lo;
		lorenz -> c = (drand48() * (lorenz -> c_hi - lorenz -> c_lo)) + lorenz -> c_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(lorenz, NULL, argc, vars);
		do { calc(lorenz, lorenz -> vars); } while(jump--);
		lorenz -> lyap_exp = lyapunov((void *) lorenz, (t_gotfn) calc, M_var_count, (double *) lorenz -> vars);
		if (isnan(lorenz -> lyap_exp)) { not_found = 1; }
		if (lorenz -> lyap_exp < lorenz -> lyap_lo || lorenz -> lyap_exp > lorenz -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(lorenz, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) lorenz -> lyap_limit);
		post("Try using wider constraints.");
		lorenz -> h = temp_h;
		lorenz -> a = temp_a;
		lorenz -> b = temp_b;
		lorenz -> c = temp_c;
		outlet_anything(lorenz -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		lorenz -> failure_ratio = (lorenz -> lyap_limit - not_expired) / lorenz -> lyap_limit;
		make_results(lorenz);
		outlet_anything(lorenz -> search_outlet, gensym("search"), M_search_count, lorenz -> search_out);
	}
}

void *lorenz_new(t_symbol *s, int argc, t_atom *argv) {
	lorenz_struct *lorenz = (lorenz_struct *) pd_new(lorenz_class);
	if (lorenz != NULL) {
		outlet_new(&lorenz -> x_obj, &s_float);
		lorenz -> outlets[0] = outlet_new(&lorenz -> x_obj, &s_float);
		lorenz -> outlets[1] = outlet_new(&lorenz -> x_obj, &s_float);
		lorenz -> search_outlet = outlet_new(&lorenz -> x_obj, &s_list);
		lorenz -> vars_outlet = outlet_new(&lorenz -> x_obj, &s_list);
		lorenz -> params_outlet = outlet_new(&lorenz -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			lorenz -> vars_init[M_x] = lorenz -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			lorenz -> vars_init[M_y] = lorenz -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			lorenz -> vars_init[M_z] = lorenz -> vars[M_z] = (double) atom_getfloatarg(2, argc, argv);
			lorenz -> h = (double) atom_getfloatarg(3, argc, argv);
			lorenz -> a = (double) atom_getfloatarg(4, argc, argv);
			lorenz -> b = (double) atom_getfloatarg(5, argc, argv);
			lorenz -> c = (double) atom_getfloatarg(6, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for lorenz fractal. Expecting 7 arguments.");
			}
			lorenz -> vars_init[M_x] = 0.1;
			lorenz -> vars_init[M_y] = 0;
			lorenz -> vars_init[M_z] = 0;
			lorenz -> h = 0.01;
			lorenz -> a = 10;
			lorenz -> b = 28;
			lorenz -> c = 2.66667;
		}
		constrain(lorenz, NULL, 0, NULL);
		lyap(lorenz, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)lorenz;
}

void lorenz_setup(void) {
	lorenz_class = class_new(gensym("lorenz"), (t_newmethod) lorenz_new, 0, sizeof(lorenz_struct), 0, A_GIMME, 0);
	class_addbang(lorenz_class, (t_method) calculate);
	class_addmethod(lorenz_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(lorenz_class, (t_method) show, gensym("show"), 0);
	class_addmethod(lorenz_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(lorenz_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(lorenz_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(lorenz_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(lorenz_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(lorenz_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

