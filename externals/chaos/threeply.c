/* threeply Attractor PD External */
/* Copyright Michael McGonagle, from ??????, 2003 */
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

#define M_a 0
#define M_b 1
#define M_c 2

#define M_x 0
#define M_y 1

#define M_param_count 3
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "threeply v0.0, by Michael McGonagle, from ??????, 2003";

t_class *threeply_class;

typedef struct threeply_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi, b, b_lo, b_hi, c, c_lo, c_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} threeply_struct;

static void calc(threeply_struct *threeply, double *vars) {
	double x_0, y_0;
	x_0 =vars[M_y]-((vars[M_x]<0)?-1:1)*abs(sin(vars[M_x])*cos(threeply -> b)+threeply -> c-vars[M_x]*sin(threeply -> a+threeply -> b+threeply -> c));
	y_0 =threeply -> a-vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(threeply_struct *threeply) {
	calc(threeply, threeply -> vars);
	outlet_float(threeply -> outlets[M_y - 1], threeply -> vars[M_y]);
	outlet_float(threeply -> x_obj.ob_outlet, threeply -> vars[M_x]);
} // end calculate

static void reset(threeply_struct *threeply, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		threeply -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		threeply -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		threeply -> vars[M_x] = threeply -> vars_init[M_x];
		threeply -> vars[M_y] = threeply -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(threeply_struct *threeply) {
	static char buff[4];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((threeply -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((threeply -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((threeply -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = '\0';
	return buff;
}

static void make_results(threeply_struct *threeply) {
	SETFLOAT(&threeply -> search_out[0], threeply -> lyap_exp);
	SETSYMBOL(&threeply -> search_out[1], gensym(classify(threeply)));
	SETFLOAT(&threeply -> search_out[2], threeply -> failure_ratio);
	SETFLOAT(&threeply -> vars_out[M_x], threeply -> vars[M_x]);
	SETFLOAT(&threeply -> vars_out[M_y], threeply -> vars[M_y]);
	SETFLOAT(&threeply -> params_out[M_a], threeply -> a);
	SETFLOAT(&threeply -> params_out[M_b], threeply -> b);
	SETFLOAT(&threeply -> params_out[M_c], threeply -> c);
	outlet_list(threeply -> params_outlet, gensym("list"), M_param_count, threeply -> params_out);
	outlet_list(threeply -> vars_outlet, gensym("list"), M_var_count, threeply -> vars_out);
}

static void show(threeply_struct *threeply) {
	make_results(threeply);
	outlet_anything(threeply -> search_outlet, gensym("show"), M_search_count, threeply -> search_out);
}

static void param(threeply_struct *threeply, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 3) {
		post("Incorrect number of arguments for threeply fractal. Expecting 3 arguments.");
		return;
	}
	threeply -> a = (double) atom_getfloatarg(0, argc, argv);
	threeply -> b = (double) atom_getfloatarg(1, argc, argv);
	threeply -> c = (double) atom_getfloatarg(2, argc, argv);
}

static void seed(threeply_struct *threeply, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(threeply_struct *threeply, t_floatarg l, t_floatarg h, t_floatarg lim) {
	threeply -> lyap_lo = l;
	threeply -> lyap_hi = h;
	threeply -> lyap_limit = (double) ((int) lim);
}

static void elyap(threeply_struct *threeply) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) threeply, (t_gotfn) calc, M_var_count, threeply -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(threeply_struct *threeply) {
	if (threeply -> a_lo < M_a_lo) { threeply -> a_lo = M_a_lo; }
	if (threeply -> a_lo > M_a_hi) { threeply -> a_lo = M_a_hi; }
	if (threeply -> a_hi < M_a_lo) { threeply -> a_hi = M_a_lo; }
	if (threeply -> a_hi > M_a_hi) { threeply -> a_hi = M_a_hi; }
	if (threeply -> b_lo < M_b_lo) { threeply -> b_lo = M_b_lo; }
	if (threeply -> b_lo > M_b_hi) { threeply -> b_lo = M_b_hi; }
	if (threeply -> b_hi < M_b_lo) { threeply -> b_hi = M_b_lo; }
	if (threeply -> b_hi > M_b_hi) { threeply -> b_hi = M_b_hi; }
	if (threeply -> c_lo < M_c_lo) { threeply -> c_lo = M_c_lo; }
	if (threeply -> c_lo > M_c_hi) { threeply -> c_lo = M_c_hi; }
	if (threeply -> c_hi < M_c_lo) { threeply -> c_hi = M_c_lo; }
	if (threeply -> c_hi > M_c_hi) { threeply -> c_hi = M_c_hi; }
}

static void constrain(threeply_struct *threeply, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		threeply -> a_lo = M_a_lo;
		threeply -> a_hi = M_a_hi;
		threeply -> b_lo = M_b_lo;
		threeply -> b_hi = M_b_hi;
		threeply -> c_lo = M_c_lo;
		threeply -> c_hi = M_c_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		threeply -> a_lo = threeply -> a - a_spread;
		threeply -> a_hi = threeply -> a + a_spread;
		threeply -> b_lo = threeply -> b - b_spread;
		threeply -> b_hi = threeply -> b + b_spread;
		threeply -> c_lo = threeply -> c - c_spread;
		threeply -> c_hi = threeply -> c + c_spread;
		limiter(threeply);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for threeply constraints, requires 6 values, got %d", argc);
		return;
	}
	threeply -> a_lo = atom_getfloat(arg++);
	threeply -> a_hi = atom_getfloat(arg++);
	threeply -> b_lo = atom_getfloat(arg++);
	threeply -> b_hi = atom_getfloat(arg++);
	threeply -> c_lo = atom_getfloat(arg++);
	threeply -> c_hi = atom_getfloat(arg++);
	limiter(threeply);
}

static void search(threeply_struct *threeply, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = threeply -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = threeply -> a;
	double temp_b = threeply -> b;
	double temp_c = threeply -> c;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], threeply -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		threeply -> a = (drand48() * (threeply -> a_hi - threeply -> a_lo)) + threeply -> a_lo;
		threeply -> b = (drand48() * (threeply -> b_hi - threeply -> b_lo)) + threeply -> b_lo;
		threeply -> c = (drand48() * (threeply -> c_hi - threeply -> c_lo)) + threeply -> c_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(threeply, NULL, argc, vars);
		do { calc(threeply, threeply -> vars); } while(jump--);
		threeply -> lyap_exp = lyapunov((void *) threeply, (t_gotfn) calc, M_var_count, (double *) threeply -> vars);
		if (isnan(threeply -> lyap_exp)) { not_found = 1; }
		if (threeply -> lyap_exp < threeply -> lyap_lo || threeply -> lyap_exp > threeply -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(threeply, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) threeply -> lyap_limit);
		post("Try using wider constraints.");
		threeply -> a = temp_a;
		threeply -> b = temp_b;
		threeply -> c = temp_c;
		outlet_anything(threeply -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		threeply -> failure_ratio = (threeply -> lyap_limit - not_expired) / threeply -> lyap_limit;
		make_results(threeply);
		outlet_anything(threeply -> search_outlet, gensym("search"), M_search_count, threeply -> search_out);
	}
}

void *threeply_new(t_symbol *s, int argc, t_atom *argv) {
	threeply_struct *threeply = (threeply_struct *) pd_new(threeply_class);
	if (threeply != NULL) {
		outlet_new(&threeply -> x_obj, &s_float);
		threeply -> outlets[0] = outlet_new(&threeply -> x_obj, &s_float);
		threeply -> search_outlet = outlet_new(&threeply -> x_obj, &s_list);
		threeply -> vars_outlet = outlet_new(&threeply -> x_obj, &s_list);
		threeply -> params_outlet = outlet_new(&threeply -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			threeply -> vars_init[M_x] = threeply -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			threeply -> vars_init[M_y] = threeply -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			threeply -> a = (double) atom_getfloatarg(2, argc, argv);
			threeply -> b = (double) atom_getfloatarg(3, argc, argv);
			threeply -> c = (double) atom_getfloatarg(4, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for threeply fractal. Expecting 5 arguments.");
			}
			threeply -> vars_init[M_x] = 0;
			threeply -> vars_init[M_y] = 0;
			threeply -> a = -55;
			threeply -> b = -1;
			threeply -> c = -42;
		}
		constrain(threeply, NULL, 0, NULL);
		lyap(threeply, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)threeply;
}

void threeply_setup(void) {
	threeply_class = class_new(gensym("threeply"), (t_newmethod) threeply_new, 0, sizeof(threeply_struct), 0, A_GIMME, 0);
	class_addbang(threeply_class, (t_method) calculate);
	class_addmethod(threeply_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(threeply_class, (t_method) show, gensym("show"), 0);
	class_addmethod(threeply_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(threeply_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(threeply_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(threeply_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(threeply_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(threeply_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

