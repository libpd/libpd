/* logistic Attractor PD External */
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

#define M_a_lo 0
#define M_a_hi 4

#define M_a 0

#define M_x 0

#define M_param_count 1
#define M_var_count 1
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "logistic v0.0, by Michael McGonagle, from ??????, 2003";

t_class *logistic_class;

typedef struct logistic_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
} logistic_struct;

static void calc(logistic_struct *logistic, double *vars) {
	double x_0;
	x_0 =logistic -> a*vars[M_x]*(1.0-vars[M_x]);
	vars[M_x] = x_0;
} // end calc

static void calculate(logistic_struct *logistic) {
	calc(logistic, logistic -> vars);
	outlet_float(logistic -> x_obj.ob_outlet, logistic -> vars[M_x]);
} // end calculate

static void reset(logistic_struct *logistic, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		logistic -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
	} else {
		logistic -> vars[M_x] = logistic -> vars_init[M_x];
	} // end if
} // end reset

static char *classify(logistic_struct *logistic) {
	static char buff[2];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((logistic -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = '\0';
	return buff;
}

static void make_results(logistic_struct *logistic) {
	SETFLOAT(&logistic -> search_out[0], logistic -> lyap_exp);
	SETSYMBOL(&logistic -> search_out[1], gensym(classify(logistic)));
	SETFLOAT(&logistic -> search_out[2], logistic -> failure_ratio);
	SETFLOAT(&logistic -> vars_out[M_x], logistic -> vars[M_x]);
	SETFLOAT(&logistic -> params_out[M_a], logistic -> a);
	outlet_list(logistic -> params_outlet, gensym("list"), M_param_count, logistic -> params_out);
	outlet_list(logistic -> vars_outlet, gensym("list"), M_var_count, logistic -> vars_out);
}

static void show(logistic_struct *logistic) {
	make_results(logistic);
	outlet_anything(logistic -> search_outlet, gensym("show"), M_search_count, logistic -> search_out);
}

static void param(logistic_struct *logistic, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 1) {
		post("Incorrect number of arguments for logistic fractal. Expecting 1 arguments.");
		return;
	}
	logistic -> a = (double) atom_getfloatarg(0, argc, argv);
}

static void seed(logistic_struct *logistic, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(logistic_struct *logistic, t_floatarg l, t_floatarg h, t_floatarg lim) {
	logistic -> lyap_lo = l;
	logistic -> lyap_hi = h;
	logistic -> lyap_limit = (double) ((int) lim);
}

static void elyap(logistic_struct *logistic) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) logistic, (t_gotfn) calc, M_var_count, logistic -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(logistic_struct *logistic) {
	if (logistic -> a_lo < M_a_lo) { logistic -> a_lo = M_a_lo; }
	if (logistic -> a_lo > M_a_hi) { logistic -> a_lo = M_a_hi; }
	if (logistic -> a_hi < M_a_lo) { logistic -> a_hi = M_a_lo; }
	if (logistic -> a_hi > M_a_hi) { logistic -> a_hi = M_a_hi; }
}

static void constrain(logistic_struct *logistic, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		logistic -> a_lo = M_a_lo;
		logistic -> a_hi = M_a_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		logistic -> a_lo = logistic -> a - a_spread;
		logistic -> a_hi = logistic -> a + a_spread;
		limiter(logistic);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for logistic constraints, requires 2 values, got %d", argc);
		return;
	}
	logistic -> a_lo = atom_getfloat(arg++);
	logistic -> a_hi = atom_getfloat(arg++);
	limiter(logistic);
}

static void search(logistic_struct *logistic, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = logistic -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = logistic -> a;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], logistic -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		logistic -> a = (drand48() * (logistic -> a_hi - logistic -> a_lo)) + logistic -> a_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(logistic, NULL, argc, vars);
		do { calc(logistic, logistic -> vars); } while(jump--);
		logistic -> lyap_exp = lyapunov((void *) logistic, (t_gotfn) calc, M_var_count, (double *) logistic -> vars);
		if (isnan(logistic -> lyap_exp)) { not_found = 1; }
		if (logistic -> lyap_exp < logistic -> lyap_lo || logistic -> lyap_exp > logistic -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(logistic, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) logistic -> lyap_limit);
		post("Try using wider constraints.");
		logistic -> a = temp_a;
		outlet_anything(logistic -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		logistic -> failure_ratio = (logistic -> lyap_limit - not_expired) / logistic -> lyap_limit;
		make_results(logistic);
		outlet_anything(logistic -> search_outlet, gensym("search"), M_search_count, logistic -> search_out);
	}
}

void *logistic_new(t_symbol *s, int argc, t_atom *argv) {
	logistic_struct *logistic = (logistic_struct *) pd_new(logistic_class);
	if (logistic != NULL) {
		outlet_new(&logistic -> x_obj, &s_float);
		logistic -> search_outlet = outlet_new(&logistic -> x_obj, &s_list);
		logistic -> vars_outlet = outlet_new(&logistic -> x_obj, &s_list);
		logistic -> params_outlet = outlet_new(&logistic -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			logistic -> vars_init[M_x] = logistic -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			logistic -> a = (double) atom_getfloatarg(1, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for logistic fractal. Expecting 2 arguments.");
			}
			logistic -> vars_init[M_x] = 0.1;
			logistic -> a = 4;
		}
		constrain(logistic, NULL, 0, NULL);
		lyap(logistic, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)logistic;
}

void logistic_setup(void) {
	logistic_class = class_new(gensym("logistic"), (t_newmethod) logistic_new, 0, sizeof(logistic_struct), 0, A_GIMME, 0);
	class_addbang(logistic_class, (t_method) calculate);
	class_addmethod(logistic_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(logistic_class, (t_method) show, gensym("show"), 0);
	class_addmethod(logistic_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(logistic_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(logistic_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(logistic_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(logistic_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(logistic_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

