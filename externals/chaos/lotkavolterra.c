/* lotkavolterra Attractor PD External */
/* Copyright Michael McGonagle, 2003 */
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
#define M_e_lo -1000
#define M_e_hi 1000

#define M_a 0
#define M_b 1
#define M_c 2
#define M_e 3

#define M_r 0
#define M_f 1

#define M_param_count 4
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "lotkavolterra v0.0, by Michael McGonagle, 2003";

t_class *lotkavolterra_class;

typedef struct lotkavolterra_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi, b, b_lo, b_hi, c, c_lo, c_hi, e, e_lo, e_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} lotkavolterra_struct;

static void calc(lotkavolterra_struct *lotkavolterra, double *vars) {
	double r_0, f_0;
	r_0 =vars[M_r]+lotkavolterra -> a*vars[M_r]-lotkavolterra -> b*vars[M_r]*vars[M_f];
	f_0 =vars[M_f]+lotkavolterra -> e*lotkavolterra -> b*vars[M_r]*vars[M_f]-lotkavolterra -> c*vars[M_f];
	vars[M_r] = r_0;
	vars[M_f] = f_0;
} // end calc

static void calculate(lotkavolterra_struct *lotkavolterra) {
	calc(lotkavolterra, lotkavolterra -> vars);
	outlet_float(lotkavolterra -> outlets[M_f - 1], lotkavolterra -> vars[M_f]);
	outlet_float(lotkavolterra -> x_obj.ob_outlet, lotkavolterra -> vars[M_r]);
} // end calculate

static void reset(lotkavolterra_struct *lotkavolterra, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		lotkavolterra -> vars[M_r] = (double) atom_getfloatarg(M_r, argc, argv);
		lotkavolterra -> vars[M_f] = (double) atom_getfloatarg(M_f, argc, argv);
	} else {
		lotkavolterra -> vars[M_r] = lotkavolterra -> vars_init[M_r];
		lotkavolterra -> vars[M_f] = lotkavolterra -> vars_init[M_f];
	} // end if
} // end reset

static char *classify(lotkavolterra_struct *lotkavolterra) {
	static char buff[5];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((lotkavolterra -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((lotkavolterra -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((lotkavolterra -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((lotkavolterra -> e - M_e_lo) * (1.0 / (M_e_hi - M_e_lo))) * 26)];
	buff[4] = '\0';
	return buff;
}

static void make_results(lotkavolterra_struct *lotkavolterra) {
	SETFLOAT(&lotkavolterra -> search_out[0], lotkavolterra -> lyap_exp);
	SETSYMBOL(&lotkavolterra -> search_out[1], gensym(classify(lotkavolterra)));
	SETFLOAT(&lotkavolterra -> search_out[2], lotkavolterra -> failure_ratio);
	SETFLOAT(&lotkavolterra -> vars_out[M_r], lotkavolterra -> vars[M_r]);
	SETFLOAT(&lotkavolterra -> vars_out[M_f], lotkavolterra -> vars[M_f]);
	SETFLOAT(&lotkavolterra -> params_out[M_a], lotkavolterra -> a);
	SETFLOAT(&lotkavolterra -> params_out[M_b], lotkavolterra -> b);
	SETFLOAT(&lotkavolterra -> params_out[M_c], lotkavolterra -> c);
	SETFLOAT(&lotkavolterra -> params_out[M_e], lotkavolterra -> e);
	outlet_list(lotkavolterra -> params_outlet, gensym("list"), M_param_count, lotkavolterra -> params_out);
	outlet_list(lotkavolterra -> vars_outlet, gensym("list"), M_var_count, lotkavolterra -> vars_out);
}

static void show(lotkavolterra_struct *lotkavolterra) {
	make_results(lotkavolterra);
	outlet_anything(lotkavolterra -> search_outlet, gensym("show"), M_search_count, lotkavolterra -> search_out);
}

static void param(lotkavolterra_struct *lotkavolterra, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 4) {
		post("Incorrect number of arguments for lotkavolterra fractal. Expecting 4 arguments.");
		return;
	}
	lotkavolterra -> a = (double) atom_getfloatarg(0, argc, argv);
	lotkavolterra -> b = (double) atom_getfloatarg(1, argc, argv);
	lotkavolterra -> c = (double) atom_getfloatarg(2, argc, argv);
	lotkavolterra -> e = (double) atom_getfloatarg(3, argc, argv);
}

static void seed(lotkavolterra_struct *lotkavolterra, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(lotkavolterra_struct *lotkavolterra, t_floatarg l, t_floatarg h, t_floatarg lim) {
	lotkavolterra -> lyap_lo = l;
	lotkavolterra -> lyap_hi = h;
	lotkavolterra -> lyap_limit = (double) ((int) lim);
}

static void elyap(lotkavolterra_struct *lotkavolterra) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) lotkavolterra, (t_gotfn) calc, M_var_count, lotkavolterra -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(lotkavolterra_struct *lotkavolterra) {
	if (lotkavolterra -> a_lo < M_a_lo) { lotkavolterra -> a_lo = M_a_lo; }
	if (lotkavolterra -> a_lo > M_a_hi) { lotkavolterra -> a_lo = M_a_hi; }
	if (lotkavolterra -> a_hi < M_a_lo) { lotkavolterra -> a_hi = M_a_lo; }
	if (lotkavolterra -> a_hi > M_a_hi) { lotkavolterra -> a_hi = M_a_hi; }
	if (lotkavolterra -> b_lo < M_b_lo) { lotkavolterra -> b_lo = M_b_lo; }
	if (lotkavolterra -> b_lo > M_b_hi) { lotkavolterra -> b_lo = M_b_hi; }
	if (lotkavolterra -> b_hi < M_b_lo) { lotkavolterra -> b_hi = M_b_lo; }
	if (lotkavolterra -> b_hi > M_b_hi) { lotkavolterra -> b_hi = M_b_hi; }
	if (lotkavolterra -> c_lo < M_c_lo) { lotkavolterra -> c_lo = M_c_lo; }
	if (lotkavolterra -> c_lo > M_c_hi) { lotkavolterra -> c_lo = M_c_hi; }
	if (lotkavolterra -> c_hi < M_c_lo) { lotkavolterra -> c_hi = M_c_lo; }
	if (lotkavolterra -> c_hi > M_c_hi) { lotkavolterra -> c_hi = M_c_hi; }
	if (lotkavolterra -> e_lo < M_e_lo) { lotkavolterra -> e_lo = M_e_lo; }
	if (lotkavolterra -> e_lo > M_e_hi) { lotkavolterra -> e_lo = M_e_hi; }
	if (lotkavolterra -> e_hi < M_e_lo) { lotkavolterra -> e_hi = M_e_lo; }
	if (lotkavolterra -> e_hi > M_e_hi) { lotkavolterra -> e_hi = M_e_hi; }
}

static void constrain(lotkavolterra_struct *lotkavolterra, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		lotkavolterra -> a_lo = M_a_lo;
		lotkavolterra -> a_hi = M_a_hi;
		lotkavolterra -> b_lo = M_b_lo;
		lotkavolterra -> b_hi = M_b_hi;
		lotkavolterra -> c_lo = M_c_lo;
		lotkavolterra -> c_hi = M_c_hi;
		lotkavolterra -> e_lo = M_e_lo;
		lotkavolterra -> e_hi = M_e_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double e_spread = ((M_e_hi - M_e_lo) * percent) / 2;
		lotkavolterra -> a_lo = lotkavolterra -> a - a_spread;
		lotkavolterra -> a_hi = lotkavolterra -> a + a_spread;
		lotkavolterra -> b_lo = lotkavolterra -> b - b_spread;
		lotkavolterra -> b_hi = lotkavolterra -> b + b_spread;
		lotkavolterra -> c_lo = lotkavolterra -> c - c_spread;
		lotkavolterra -> c_hi = lotkavolterra -> c + c_spread;
		lotkavolterra -> e_lo = lotkavolterra -> e - e_spread;
		lotkavolterra -> e_hi = lotkavolterra -> e + e_spread;
		limiter(lotkavolterra);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for lotkavolterra constraints, requires 8 values, got %d", argc);
		return;
	}
	lotkavolterra -> a_lo = atom_getfloat(arg++);
	lotkavolterra -> a_hi = atom_getfloat(arg++);
	lotkavolterra -> b_lo = atom_getfloat(arg++);
	lotkavolterra -> b_hi = atom_getfloat(arg++);
	lotkavolterra -> c_lo = atom_getfloat(arg++);
	lotkavolterra -> c_hi = atom_getfloat(arg++);
	lotkavolterra -> e_lo = atom_getfloat(arg++);
	lotkavolterra -> e_hi = atom_getfloat(arg++);
	limiter(lotkavolterra);
}

static void search(lotkavolterra_struct *lotkavolterra, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = lotkavolterra -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = lotkavolterra -> a;
	double temp_b = lotkavolterra -> b;
	double temp_c = lotkavolterra -> c;
	double temp_e = lotkavolterra -> e;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], lotkavolterra -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		lotkavolterra -> a = (drand48() * (lotkavolterra -> a_hi - lotkavolterra -> a_lo)) + lotkavolterra -> a_lo;
		lotkavolterra -> b = (drand48() * (lotkavolterra -> b_hi - lotkavolterra -> b_lo)) + lotkavolterra -> b_lo;
		lotkavolterra -> c = (drand48() * (lotkavolterra -> c_hi - lotkavolterra -> c_lo)) + lotkavolterra -> c_lo;
		lotkavolterra -> e = (drand48() * (lotkavolterra -> e_hi - lotkavolterra -> e_lo)) + lotkavolterra -> e_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(lotkavolterra, NULL, argc, vars);
		do { calc(lotkavolterra, lotkavolterra -> vars); } while(jump--);
		lotkavolterra -> lyap_exp = lyapunov((void *) lotkavolterra, (t_gotfn) calc, M_var_count, (double *) lotkavolterra -> vars);
		if (isnan(lotkavolterra -> lyap_exp)) { not_found = 1; }
		if (lotkavolterra -> lyap_exp < lotkavolterra -> lyap_lo || lotkavolterra -> lyap_exp > lotkavolterra -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(lotkavolterra, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) lotkavolterra -> lyap_limit);
		post("Try using wider constraints.");
		lotkavolterra -> a = temp_a;
		lotkavolterra -> b = temp_b;
		lotkavolterra -> c = temp_c;
		lotkavolterra -> e = temp_e;
		outlet_anything(lotkavolterra -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		lotkavolterra -> failure_ratio = (lotkavolterra -> lyap_limit - not_expired) / lotkavolterra -> lyap_limit;
		make_results(lotkavolterra);
		outlet_anything(lotkavolterra -> search_outlet, gensym("search"), M_search_count, lotkavolterra -> search_out);
	}
}

void *lotkavolterra_new(t_symbol *s, int argc, t_atom *argv) {
	lotkavolterra_struct *lotkavolterra = (lotkavolterra_struct *) pd_new(lotkavolterra_class);
	if (lotkavolterra != NULL) {
		outlet_new(&lotkavolterra -> x_obj, &s_float);
		lotkavolterra -> outlets[0] = outlet_new(&lotkavolterra -> x_obj, &s_float);
		lotkavolterra -> search_outlet = outlet_new(&lotkavolterra -> x_obj, &s_list);
		lotkavolterra -> vars_outlet = outlet_new(&lotkavolterra -> x_obj, &s_list);
		lotkavolterra -> params_outlet = outlet_new(&lotkavolterra -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			lotkavolterra -> vars_init[M_r] = lotkavolterra -> vars[M_r] = (double) atom_getfloatarg(0, argc, argv);
			lotkavolterra -> vars_init[M_f] = lotkavolterra -> vars[M_f] = (double) atom_getfloatarg(1, argc, argv);
			lotkavolterra -> a = (double) atom_getfloatarg(2, argc, argv);
			lotkavolterra -> b = (double) atom_getfloatarg(3, argc, argv);
			lotkavolterra -> c = (double) atom_getfloatarg(4, argc, argv);
			lotkavolterra -> e = (double) atom_getfloatarg(5, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for lotkavolterra fractal. Expecting 6 arguments.");
			}
			lotkavolterra -> vars_init[M_r] = 0.1;
			lotkavolterra -> vars_init[M_f] = 0.1;
			lotkavolterra -> a = 0.04;
			lotkavolterra -> b = 0.0005;
			lotkavolterra -> c = 0.2;
			lotkavolterra -> e = 0.1;
		}
		constrain(lotkavolterra, NULL, 0, NULL);
		lyap(lotkavolterra, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)lotkavolterra;
}

void lotkavolterra_setup(void) {
	lotkavolterra_class = class_new(gensym("lotkavolterra"), (t_newmethod) lotkavolterra_new, 0, sizeof(lotkavolterra_struct), 0, A_GIMME, 0);
	class_addbang(lotkavolterra_class, (t_method) calculate);
	class_addmethod(lotkavolterra_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(lotkavolterra_class, (t_method) show, gensym("show"), 0);
	class_addmethod(lotkavolterra_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(lotkavolterra_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(lotkavolterra_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(lotkavolterra_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(lotkavolterra_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(lotkavolterra_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

