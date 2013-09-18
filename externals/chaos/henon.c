/* henon Attractor PD External */
/* Copyright Ben Bogart, 2003 */
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

#define M_a_lo -1
#define M_a_hi 2
#define M_b_lo -1
#define M_b_hi 2

#define M_a 0
#define M_b 1

#define M_x 0
#define M_y 1

#define M_param_count 2
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "henon v0.0, by Ben Bogart, 2003";

t_class *henon_class;

typedef struct henon_struct {
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
	
	t_outlet *outlets[M_var_count - 1];
} henon_struct;

static void calc(henon_struct *henon, double *vars) {
	double x_0, y_0;
	x_0 =(vars[M_y]+1)-(henon -> a*pow(vars[M_x],2));
	y_0 =henon -> b*vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(henon_struct *henon) {
	calc(henon, henon -> vars);
	outlet_float(henon -> outlets[M_y - 1], henon -> vars[M_y]);
	outlet_float(henon -> x_obj.ob_outlet, henon -> vars[M_x]);
} // end calculate

static void reset(henon_struct *henon, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		henon -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		henon -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		henon -> vars[M_x] = henon -> vars_init[M_x];
		henon -> vars[M_y] = henon -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(henon_struct *henon) {
	static char buff[3];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((henon -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((henon -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = '\0';
	return buff;
}

static void make_results(henon_struct *henon) {
	SETFLOAT(&henon -> search_out[0], henon -> lyap_exp);
	SETSYMBOL(&henon -> search_out[1], gensym(classify(henon)));
	SETFLOAT(&henon -> search_out[2], henon -> failure_ratio);
	SETFLOAT(&henon -> vars_out[M_x], henon -> vars[M_x]);
	SETFLOAT(&henon -> vars_out[M_y], henon -> vars[M_y]);
	SETFLOAT(&henon -> params_out[M_a], henon -> a);
	SETFLOAT(&henon -> params_out[M_b], henon -> b);
	outlet_list(henon -> params_outlet, gensym("list"), M_param_count, henon -> params_out);
	outlet_list(henon -> vars_outlet, gensym("list"), M_var_count, henon -> vars_out);
}

static void show(henon_struct *henon) {
	make_results(henon);
	outlet_anything(henon -> search_outlet, gensym("show"), M_search_count, henon -> search_out);
}

static void param(henon_struct *henon, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 2) {
		post("Incorrect number of arguments for henon fractal. Expecting 2 arguments.");
		return;
	}
	henon -> a = (double) atom_getfloatarg(0, argc, argv);
	henon -> b = (double) atom_getfloatarg(1, argc, argv);
}

static void seed(henon_struct *henon, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(henon_struct *henon, t_floatarg l, t_floatarg h, t_floatarg lim) {
	henon -> lyap_lo = l;
	henon -> lyap_hi = h;
	henon -> lyap_limit = (double) ((int) lim);
}

static void elyap(henon_struct *henon) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) henon, (t_gotfn) calc, M_var_count, henon -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(henon_struct *henon) {
	if (henon -> a_lo < M_a_lo) { henon -> a_lo = M_a_lo; }
	if (henon -> a_lo > M_a_hi) { henon -> a_lo = M_a_hi; }
	if (henon -> a_hi < M_a_lo) { henon -> a_hi = M_a_lo; }
	if (henon -> a_hi > M_a_hi) { henon -> a_hi = M_a_hi; }
	if (henon -> b_lo < M_b_lo) { henon -> b_lo = M_b_lo; }
	if (henon -> b_lo > M_b_hi) { henon -> b_lo = M_b_hi; }
	if (henon -> b_hi < M_b_lo) { henon -> b_hi = M_b_lo; }
	if (henon -> b_hi > M_b_hi) { henon -> b_hi = M_b_hi; }
}

static void constrain(henon_struct *henon, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		henon -> a_lo = M_a_lo;
		henon -> a_hi = M_a_hi;
		henon -> b_lo = M_b_lo;
		henon -> b_hi = M_b_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		henon -> a_lo = henon -> a - a_spread;
		henon -> a_hi = henon -> a + a_spread;
		henon -> b_lo = henon -> b - b_spread;
		henon -> b_hi = henon -> b + b_spread;
		limiter(henon);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for henon constraints, requires 4 values, got %d", argc);
		return;
	}
	henon -> a_lo = atom_getfloat(arg++);
	henon -> a_hi = atom_getfloat(arg++);
	henon -> b_lo = atom_getfloat(arg++);
	henon -> b_hi = atom_getfloat(arg++);
	limiter(henon);
}

static void search(henon_struct *henon, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = henon -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = henon -> a;
	double temp_b = henon -> b;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], henon -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		henon -> a = (drand48() * (henon -> a_hi - henon -> a_lo)) + henon -> a_lo;
		henon -> b = (drand48() * (henon -> b_hi - henon -> b_lo)) + henon -> b_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(henon, NULL, argc, vars);
		do { calc(henon, henon -> vars); } while(jump--);
		henon -> lyap_exp = lyapunov((void *) henon, (t_gotfn) calc, M_var_count, (double *) henon -> vars);
		if (isnan(henon -> lyap_exp)) { not_found = 1; }
		if (henon -> lyap_exp < henon -> lyap_lo || henon -> lyap_exp > henon -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(henon, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) henon -> lyap_limit);
		post("Try using wider constraints.");
		henon -> a = temp_a;
		henon -> b = temp_b;
		outlet_anything(henon -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		henon -> failure_ratio = (henon -> lyap_limit - not_expired) / henon -> lyap_limit;
		make_results(henon);
		outlet_anything(henon -> search_outlet, gensym("search"), M_search_count, henon -> search_out);
	}
}

void *henon_new(t_symbol *s, int argc, t_atom *argv) {
	henon_struct *henon = (henon_struct *) pd_new(henon_class);
	if (henon != NULL) {
		outlet_new(&henon -> x_obj, &s_float);
		henon -> outlets[0] = outlet_new(&henon -> x_obj, &s_float);
		henon -> search_outlet = outlet_new(&henon -> x_obj, &s_list);
		henon -> vars_outlet = outlet_new(&henon -> x_obj, &s_list);
		henon -> params_outlet = outlet_new(&henon -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			henon -> vars_init[M_x] = henon -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			henon -> vars_init[M_y] = henon -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			henon -> a = (double) atom_getfloatarg(2, argc, argv);
			henon -> b = (double) atom_getfloatarg(3, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for henon fractal. Expecting 4 arguments.");
			}
			henon -> vars_init[M_x] = 1;
			henon -> vars_init[M_y] = 1;
			henon -> a = 1.4;
			henon -> b = 0.3;
		}
		constrain(henon, NULL, 0, NULL);
		lyap(henon, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)henon;
}

void henon_setup(void) {
	henon_class = class_new(gensym("henon"), (t_newmethod) henon_new, 0, sizeof(henon_struct), 0, A_GIMME, 0);
	class_addbang(henon_class, (t_method) calculate);
	class_addmethod(henon_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(henon_class, (t_method) show, gensym("show"), 0);
	class_addmethod(henon_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(henon_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(henon_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(henon_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(henon_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(henon_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

