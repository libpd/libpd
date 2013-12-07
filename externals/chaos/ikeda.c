/* ikeda Attractor PD External */
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

#define M_a_lo -350
#define M_a_hi 350
#define M_b_lo -2
#define M_b_hi 2
#define M_c_lo -350
#define M_c_hi 350
#define M_rho_lo -250
#define M_rho_hi 250

#define M_a 0
#define M_b 1
#define M_c 2
#define M_rho 3

#define M_x 0
#define M_y 1

#define M_param_count 4
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "ikeda v0.0, by Ben Bogart, 2002";

t_class *ikeda_class;

typedef struct ikeda_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi, b, b_lo, b_hi, c, c_lo, c_hi, rho, rho_lo, rho_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} ikeda_struct;

static void calc(ikeda_struct *ikeda, double *vars) {
	double t, s, d, x_0, y_0;
	t=ikeda -> a-ikeda -> c/(1.0+vars[M_x]*vars[M_x]+vars[M_y]*vars[M_y]);
	s=sin(t);
	d=cos(t);
	x_0 =ikeda -> rho+ikeda -> b*(vars[M_x]*d-vars[M_y]*s);
	y_0 =ikeda -> b*(vars[M_x]*s+vars[M_y]*d);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(ikeda_struct *ikeda) {
	calc(ikeda, ikeda -> vars);
	outlet_float(ikeda -> outlets[M_y - 1], ikeda -> vars[M_y]);
	outlet_float(ikeda -> x_obj.ob_outlet, ikeda -> vars[M_x]);
} // end calculate

static void reset(ikeda_struct *ikeda, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		ikeda -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		ikeda -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		ikeda -> vars[M_x] = ikeda -> vars_init[M_x];
		ikeda -> vars[M_y] = ikeda -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(ikeda_struct *ikeda) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((ikeda -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((ikeda -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((ikeda -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((ikeda -> rho - M_rho_lo) * (1.0 / (M_rho_hi - M_rho_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(ikeda_struct *ikeda) {
	SETFLOAT(&ikeda -> search_out[0], ikeda -> lyap_exp);
	SETSYMBOL(&ikeda -> search_out[1], gensym(classify(ikeda)));
	SETFLOAT(&ikeda -> search_out[2], ikeda -> failure_ratio);
	SETFLOAT(&ikeda -> vars_out[M_x], ikeda -> vars[M_x]);
	SETFLOAT(&ikeda -> vars_out[M_y], ikeda -> vars[M_y]);
	SETFLOAT(&ikeda -> params_out[M_a], ikeda -> a);
	SETFLOAT(&ikeda -> params_out[M_b], ikeda -> b);
	SETFLOAT(&ikeda -> params_out[M_c], ikeda -> c);
	SETFLOAT(&ikeda -> params_out[M_rho], ikeda -> rho);
	outlet_list(ikeda -> params_outlet, gensym("list"), M_param_count, ikeda -> params_out);
	outlet_list(ikeda -> vars_outlet, gensym("list"), M_var_count, ikeda -> vars_out);
}

static void show(ikeda_struct *ikeda) {
	make_results(ikeda);
	outlet_anything(ikeda -> search_outlet, gensym("show"), M_search_count, ikeda -> search_out);
}

static void param(ikeda_struct *ikeda, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for ikeda fractal. Expecting 4 arguments.");
		return;
	}
	ikeda -> a = (double) atom_getfloatarg(0, argc, argv);
	ikeda -> b = (double) atom_getfloatarg(1, argc, argv);
	ikeda -> c = (double) atom_getfloatarg(2, argc, argv);
	ikeda -> rho = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(ikeda_struct *ikeda, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(ikeda_struct *ikeda, t_floatarg l, t_floatarg h, t_floatarg lim) {
	ikeda -> lyap_lo = l;
	ikeda -> lyap_hi = h;
	ikeda -> lyap_limit = (double) ((int) lim);
}

static void elyap(ikeda_struct *ikeda) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) ikeda, (t_gotfn) calc, M_var_count, ikeda -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(ikeda_struct *ikeda) {
	if (ikeda -> a_lo < M_a_lo) { ikeda -> a_lo = M_a_lo; }
	if (ikeda -> a_lo > M_a_hi) { ikeda -> a_lo = M_a_hi; }
	if (ikeda -> a_hi < M_a_lo) { ikeda -> a_hi = M_a_lo; }
	if (ikeda -> a_hi > M_a_hi) { ikeda -> a_hi = M_a_hi; }
	if (ikeda -> b_lo < M_b_lo) { ikeda -> b_lo = M_b_lo; }
	if (ikeda -> b_lo > M_b_hi) { ikeda -> b_lo = M_b_hi; }
	if (ikeda -> b_hi < M_b_lo) { ikeda -> b_hi = M_b_lo; }
	if (ikeda -> b_hi > M_b_hi) { ikeda -> b_hi = M_b_hi; }
	if (ikeda -> c_lo < M_c_lo) { ikeda -> c_lo = M_c_lo; }
	if (ikeda -> c_lo > M_c_hi) { ikeda -> c_lo = M_c_hi; }
	if (ikeda -> c_hi < M_c_lo) { ikeda -> c_hi = M_c_lo; }
	if (ikeda -> c_hi > M_c_hi) { ikeda -> c_hi = M_c_hi; }
	if (ikeda -> rho_lo < M_rho_lo) { ikeda -> rho_lo = M_rho_lo; }
	if (ikeda -> rho_lo > M_rho_hi) { ikeda -> rho_lo = M_rho_hi; }
	if (ikeda -> rho_hi < M_rho_lo) { ikeda -> rho_hi = M_rho_lo; }
	if (ikeda -> rho_hi > M_rho_hi) { ikeda -> rho_hi = M_rho_hi; }
}

static void constrain(ikeda_struct *ikeda, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		ikeda -> a_lo = M_a_lo;
		ikeda -> a_hi = M_a_hi;
		ikeda -> b_lo = M_b_lo;
		ikeda -> b_hi = M_b_hi;
		ikeda -> c_lo = M_c_lo;
		ikeda -> c_hi = M_c_hi;
		ikeda -> rho_lo = M_rho_lo;
		ikeda -> rho_hi = M_rho_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double rho_spread = ((M_rho_hi - M_rho_lo) * percent) / 2;
		ikeda -> a_lo = ikeda -> a - a_spread;
		ikeda -> a_hi = ikeda -> a + a_spread;
		ikeda -> b_lo = ikeda -> b - b_spread;
		ikeda -> b_hi = ikeda -> b + b_spread;
		ikeda -> c_lo = ikeda -> c - c_spread;
		ikeda -> c_hi = ikeda -> c + c_spread;
		ikeda -> rho_lo = ikeda -> rho - rho_spread;
		ikeda -> rho_hi = ikeda -> rho + rho_spread;
		limiter(ikeda);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for ikeda constraints, requires 8 values, got %d", argc);
		return;
	}
	ikeda -> a_lo = atom_getfloat(arg++);
	ikeda -> a_hi = atom_getfloat(arg++);
	ikeda -> b_lo = atom_getfloat(arg++);
	ikeda -> b_hi = atom_getfloat(arg++);
	ikeda -> c_lo = atom_getfloat(arg++);
	ikeda -> c_hi = atom_getfloat(arg++);
	ikeda -> rho_lo = atom_getfloat(arg++);
	ikeda -> rho_hi = atom_getfloat(arg++);
	limiter(ikeda);
}

static void search(ikeda_struct *ikeda, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = ikeda -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = ikeda -> a;
	double temp_b = ikeda -> b;
	double temp_c = ikeda -> c;
	double temp_rho = ikeda -> rho;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], ikeda -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		ikeda -> a = (drand48() * (ikeda -> a_hi - ikeda -> a_lo)) + ikeda -> a_lo;
		ikeda -> b = (drand48() * (ikeda -> b_hi - ikeda -> b_lo)) + ikeda -> b_lo;
		ikeda -> c = (drand48() * (ikeda -> c_hi - ikeda -> c_lo)) + ikeda -> c_lo;
		ikeda -> rho = (drand48() * (ikeda -> rho_hi - ikeda -> rho_lo)) + ikeda -> rho_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(ikeda, NULL, argc, vars);
		do { calc(ikeda, ikeda -> vars); } while(jump--);
		ikeda -> lyap_exp = lyapunov((void *) ikeda, (t_gotfn) calc, M_var_count, (double *) ikeda -> vars);
		if (isnan(ikeda -> lyap_exp)) { not_found = 1; }
		if (ikeda -> lyap_exp < ikeda -> lyap_lo || ikeda -> lyap_exp > ikeda -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(ikeda, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) ikeda -> lyap_limit);
		post("Try using wider constraints.");
		ikeda -> a = temp_a;
		ikeda -> b = temp_b;
		ikeda -> c = temp_c;
		ikeda -> rho = temp_rho;
		outlet_anything(ikeda -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		ikeda -> failure_ratio = (ikeda -> lyap_limit - not_expired) / ikeda -> lyap_limit;
		make_results(ikeda);
		outlet_anything(ikeda -> search_outlet, gensym("search"), M_search_count, ikeda -> search_out);
	}
}

void *ikeda_new(t_symbol *s, int argc, t_atom *argv) {
	ikeda_struct *ikeda = (ikeda_struct *) pd_new(ikeda_class);
	if (ikeda != NULL) {
		outlet_new(&ikeda -> x_obj, &s_float);
		ikeda -> outlets[0] = outlet_new(&ikeda -> x_obj, &s_float);
		ikeda -> search_outlet = outlet_new(&ikeda -> x_obj, &s_list);
		ikeda -> vars_outlet = outlet_new(&ikeda -> x_obj, &s_list);
		ikeda -> params_outlet = outlet_new(&ikeda -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			ikeda -> vars_init[M_x] = ikeda -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			ikeda -> vars_init[M_y] = ikeda -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			ikeda -> a = (double) atom_getfloatarg(2, argc, argv);
			ikeda -> b = (double) atom_getfloatarg(3, argc, argv);
			ikeda -> c = (double) atom_getfloatarg(4, argc, argv);
			ikeda -> rho = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for ikeda fractal. Expecting 6 arguments.");
			}
			ikeda -> vars_init[M_x] = 0.1;
			ikeda -> vars_init[M_y] = 0.1;
			ikeda -> a = 0.4;
			ikeda -> b = 0.9;
			ikeda -> c = 6;
			ikeda -> rho = 1;
		}
		constrain(ikeda, NULL, 0, NULL);
		lyap(ikeda, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)ikeda;
}

void ikeda_setup(void) {
	ikeda_class = class_new(gensym("ikeda"), (t_newmethod) ikeda_new, 0, sizeof(ikeda_struct), 0, A_GIMME, 0);
	class_addbang(ikeda_class, (t_method) calculate);
	class_addmethod(ikeda_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(ikeda_class, (t_method) show, gensym("show"), 0);
	class_addmethod(ikeda_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(ikeda_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(ikeda_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(ikeda_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(ikeda_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(ikeda_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

