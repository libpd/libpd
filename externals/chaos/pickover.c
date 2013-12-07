/* pickover Attractor PD External */
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
#define M_z 2

#define M_param_count 4
#define M_var_count 3
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "pickover v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *pickover_class;

typedef struct pickover_struct {
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
} pickover_struct;

static void calc(pickover_struct *pickover, double *vars) {
	double x_0, y_0, z_0;
	x_0 =sin(pickover -> a*vars[M_y])-vars[M_z]*cos(pickover -> b*vars[M_x]);
	y_0 =vars[M_z]*sin(pickover -> c*vars[M_x])-cos(pickover -> d*vars[M_y]);
	z_0 =sin(vars[M_x]);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
	vars[M_z] = z_0;
} // end calc

static void calculate(pickover_struct *pickover) {
	calc(pickover, pickover -> vars);
	outlet_float(pickover -> outlets[M_z - 1], pickover -> vars[M_z]);
	outlet_float(pickover -> outlets[M_y - 1], pickover -> vars[M_y]);
	outlet_float(pickover -> x_obj.ob_outlet, pickover -> vars[M_x]);
} // end calculate

static void reset(pickover_struct *pickover, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		pickover -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		pickover -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
		pickover -> vars[M_z] = (double) atom_getfloatarg(M_z, argc, argv);
	} else {
		pickover -> vars[M_x] = pickover -> vars_init[M_x];
		pickover -> vars[M_y] = pickover -> vars_init[M_y];
		pickover -> vars[M_z] = pickover -> vars_init[M_z];
	} // end if
} // end reset

static char *classify(pickover_struct *pickover) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((pickover -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((pickover -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((pickover -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((pickover -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(pickover_struct *pickover) {
	SETFLOAT(&pickover -> search_out[0], pickover -> lyap_exp);
	SETSYMBOL(&pickover -> search_out[1], gensym(classify(pickover)));
	SETFLOAT(&pickover -> search_out[2], pickover -> failure_ratio);
	SETFLOAT(&pickover -> vars_out[M_x], pickover -> vars[M_x]);
	SETFLOAT(&pickover -> vars_out[M_y], pickover -> vars[M_y]);
	SETFLOAT(&pickover -> vars_out[M_z], pickover -> vars[M_z]);
	SETFLOAT(&pickover -> params_out[M_a], pickover -> a);
	SETFLOAT(&pickover -> params_out[M_b], pickover -> b);
	SETFLOAT(&pickover -> params_out[M_c], pickover -> c);
	SETFLOAT(&pickover -> params_out[M_d], pickover -> d);
	outlet_list(pickover -> params_outlet, gensym("list"), M_param_count, pickover -> params_out);
	outlet_list(pickover -> vars_outlet, gensym("list"), M_var_count, pickover -> vars_out);
}

static void show(pickover_struct *pickover) {
	make_results(pickover);
	outlet_anything(pickover -> search_outlet, gensym("show"), M_search_count, pickover -> search_out);
}

static void param(pickover_struct *pickover, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for pickover fractal. Expecting 4 arguments.");
		return;
	}
	pickover -> a = (double) atom_getfloatarg(0, argc, argv);
	pickover -> b = (double) atom_getfloatarg(1, argc, argv);
	pickover -> c = (double) atom_getfloatarg(2, argc, argv);
	pickover -> d = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(pickover_struct *pickover, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(pickover_struct *pickover, t_floatarg l, t_floatarg h, t_floatarg lim) {
	pickover -> lyap_lo = l;
	pickover -> lyap_hi = h;
	pickover -> lyap_limit = (double) ((int) lim);
}

static void elyap(pickover_struct *pickover) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) pickover, (t_gotfn) calc, M_var_count, pickover -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(pickover_struct *pickover) {
	if (pickover -> a_lo < M_a_lo) { pickover -> a_lo = M_a_lo; }
	if (pickover -> a_lo > M_a_hi) { pickover -> a_lo = M_a_hi; }
	if (pickover -> a_hi < M_a_lo) { pickover -> a_hi = M_a_lo; }
	if (pickover -> a_hi > M_a_hi) { pickover -> a_hi = M_a_hi; }
	if (pickover -> b_lo < M_b_lo) { pickover -> b_lo = M_b_lo; }
	if (pickover -> b_lo > M_b_hi) { pickover -> b_lo = M_b_hi; }
	if (pickover -> b_hi < M_b_lo) { pickover -> b_hi = M_b_lo; }
	if (pickover -> b_hi > M_b_hi) { pickover -> b_hi = M_b_hi; }
	if (pickover -> c_lo < M_c_lo) { pickover -> c_lo = M_c_lo; }
	if (pickover -> c_lo > M_c_hi) { pickover -> c_lo = M_c_hi; }
	if (pickover -> c_hi < M_c_lo) { pickover -> c_hi = M_c_lo; }
	if (pickover -> c_hi > M_c_hi) { pickover -> c_hi = M_c_hi; }
	if (pickover -> d_lo < M_d_lo) { pickover -> d_lo = M_d_lo; }
	if (pickover -> d_lo > M_d_hi) { pickover -> d_lo = M_d_hi; }
	if (pickover -> d_hi < M_d_lo) { pickover -> d_hi = M_d_lo; }
	if (pickover -> d_hi > M_d_hi) { pickover -> d_hi = M_d_hi; }
}

static void constrain(pickover_struct *pickover, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		pickover -> a_lo = M_a_lo;
		pickover -> a_hi = M_a_hi;
		pickover -> b_lo = M_b_lo;
		pickover -> b_hi = M_b_hi;
		pickover -> c_lo = M_c_lo;
		pickover -> c_hi = M_c_hi;
		pickover -> d_lo = M_d_lo;
		pickover -> d_hi = M_d_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		pickover -> a_lo = pickover -> a - a_spread;
		pickover -> a_hi = pickover -> a + a_spread;
		pickover -> b_lo = pickover -> b - b_spread;
		pickover -> b_hi = pickover -> b + b_spread;
		pickover -> c_lo = pickover -> c - c_spread;
		pickover -> c_hi = pickover -> c + c_spread;
		pickover -> d_lo = pickover -> d - d_spread;
		pickover -> d_hi = pickover -> d + d_spread;
		limiter(pickover);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for pickover constraints, requires 8 values, got %d", argc);
		return;
	}
	pickover -> a_lo = atom_getfloat(arg++);
	pickover -> a_hi = atom_getfloat(arg++);
	pickover -> b_lo = atom_getfloat(arg++);
	pickover -> b_hi = atom_getfloat(arg++);
	pickover -> c_lo = atom_getfloat(arg++);
	pickover -> c_hi = atom_getfloat(arg++);
	pickover -> d_lo = atom_getfloat(arg++);
	pickover -> d_hi = atom_getfloat(arg++);
	limiter(pickover);
}

static void search(pickover_struct *pickover, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = pickover -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = pickover -> a;
	double temp_b = pickover -> b;
	double temp_c = pickover -> c;
	double temp_d = pickover -> d;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], pickover -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		pickover -> a = (drand48() * (pickover -> a_hi - pickover -> a_lo)) + pickover -> a_lo;
		pickover -> b = (drand48() * (pickover -> b_hi - pickover -> b_lo)) + pickover -> b_lo;
		pickover -> c = (drand48() * (pickover -> c_hi - pickover -> c_lo)) + pickover -> c_lo;
		pickover -> d = (drand48() * (pickover -> d_hi - pickover -> d_lo)) + pickover -> d_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(pickover, NULL, argc, vars);
		do { calc(pickover, pickover -> vars); } while(jump--);
		pickover -> lyap_exp = lyapunov((void *) pickover, (t_gotfn) calc, M_var_count, (double *) pickover -> vars);
		if (isnan(pickover -> lyap_exp)) { not_found = 1; }
		if (pickover -> lyap_exp < pickover -> lyap_lo || pickover -> lyap_exp > pickover -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(pickover, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) pickover -> lyap_limit);
		post("Try using wider constraints.");
		pickover -> a = temp_a;
		pickover -> b = temp_b;
		pickover -> c = temp_c;
		pickover -> d = temp_d;
		outlet_anything(pickover -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		pickover -> failure_ratio = (pickover -> lyap_limit - not_expired) / pickover -> lyap_limit;
		make_results(pickover);
		outlet_anything(pickover -> search_outlet, gensym("search"), M_search_count, pickover -> search_out);
	}
}

void *pickover_new(t_symbol *s, int argc, t_atom *argv) {
	pickover_struct *pickover = (pickover_struct *) pd_new(pickover_class);
	if (pickover != NULL) {
		outlet_new(&pickover -> x_obj, &s_float);
		pickover -> outlets[0] = outlet_new(&pickover -> x_obj, &s_float);
		pickover -> outlets[1] = outlet_new(&pickover -> x_obj, &s_float);
		pickover -> search_outlet = outlet_new(&pickover -> x_obj, &s_list);
		pickover -> vars_outlet = outlet_new(&pickover -> x_obj, &s_list);
		pickover -> params_outlet = outlet_new(&pickover -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			pickover -> vars_init[M_x] = pickover -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			pickover -> vars_init[M_y] = pickover -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			pickover -> vars_init[M_z] = pickover -> vars[M_z] = (double) atom_getfloatarg(2, argc, argv);
			pickover -> a = (double) atom_getfloatarg(3, argc, argv);
			pickover -> b = (double) atom_getfloatarg(4, argc, argv);
			pickover -> c = (double) atom_getfloatarg(5, argc, argv);
			pickover -> d = (double) atom_getfloatarg(6, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for pickover fractal. Expecting 7 arguments.");
			}
			pickover -> vars_init[M_x] = 0.01;
			pickover -> vars_init[M_y] = 0;
			pickover -> vars_init[M_z] = 0;
			pickover -> a = 2.24;
			pickover -> b = 0.43;
			pickover -> c = -0.65;
			pickover -> d = -2.43;
		}
		constrain(pickover, NULL, 0, NULL);
		lyap(pickover, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)pickover;
}

void pickover_setup(void) {
	pickover_class = class_new(gensym("pickover"), (t_newmethod) pickover_new, 0, sizeof(pickover_struct), 0, A_GIMME, 0);
	class_addbang(pickover_class, (t_method) calculate);
	class_addmethod(pickover_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(pickover_class, (t_method) show, gensym("show"), 0);
	class_addmethod(pickover_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(pickover_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(pickover_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pickover_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(pickover_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(pickover_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

