/* tent Attractor PD External */
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

#define M_r_lo -1000
#define M_r_hi 1000

#define M_r 0

#define M_x 0

#define M_param_count 1
#define M_var_count 1
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "tent v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *tent_class;

typedef struct tent_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double r, r_lo, r_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
} tent_struct;

static void calc(tent_struct *tent, double *vars) {
	double x_0;
	x_0 =(vars[M_x]<=0.5)?2*tent -> r*vars[M_x]:2*tent -> r*(1.0-vars[M_x]);
	vars[M_x] = x_0;
} // end calc

static void calculate(tent_struct *tent) {
	calc(tent, tent -> vars);
	outlet_float(tent -> x_obj.ob_outlet, tent -> vars[M_x]);
} // end calculate

static void reset(tent_struct *tent, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		tent -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
	} else {
		tent -> vars[M_x] = tent -> vars_init[M_x];
	} // end if
} // end reset

static char *classify(tent_struct *tent) {
	static char buff[2];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((tent -> r - M_r_lo) * (1.0 / (M_r_hi - M_r_lo))) * 26)];
	buff[1] = '\0';
	return buff;
}

static void make_results(tent_struct *tent) {
	SETFLOAT(&tent -> search_out[0], tent -> lyap_exp);
	SETSYMBOL(&tent -> search_out[1], gensym(classify(tent)));
	SETFLOAT(&tent -> search_out[2], tent -> failure_ratio);
	SETFLOAT(&tent -> vars_out[M_x], tent -> vars[M_x]);
	SETFLOAT(&tent -> params_out[M_r], tent -> r);
	outlet_list(tent -> params_outlet, gensym("list"), M_param_count, tent -> params_out);
	outlet_list(tent -> vars_outlet, gensym("list"), M_var_count, tent -> vars_out);
}

static void show(tent_struct *tent) {
	make_results(tent);
	outlet_anything(tent -> search_outlet, gensym("show"), M_search_count, tent -> search_out);
}

static void param(tent_struct *tent, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 1) {
		post("Incorrect number of arguments for tent fractal. Expecting 1 arguments.");
		return;
	}
	tent -> r = (double) atom_getfloatarg(0, argc, argv);
}

static void seed(tent_struct *tent, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(tent_struct *tent, t_floatarg l, t_floatarg h, t_floatarg lim) {
	tent -> lyap_lo = l;
	tent -> lyap_hi = h;
	tent -> lyap_limit = (double) ((int) lim);
}

static void elyap(tent_struct *tent) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) tent, (t_gotfn) calc, M_var_count, tent -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(tent_struct *tent) {
	if (tent -> r_lo < M_r_lo) { tent -> r_lo = M_r_lo; }
	if (tent -> r_lo > M_r_hi) { tent -> r_lo = M_r_hi; }
	if (tent -> r_hi < M_r_lo) { tent -> r_hi = M_r_lo; }
	if (tent -> r_hi > M_r_hi) { tent -> r_hi = M_r_hi; }
}

static void constrain(tent_struct *tent, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		tent -> r_lo = M_r_lo;
		tent -> r_hi = M_r_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double r_spread = ((M_r_hi - M_r_lo) * percent) / 2;
		tent -> r_lo = tent -> r - r_spread;
		tent -> r_hi = tent -> r + r_spread;
		limiter(tent);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for tent constraints, requires 2 values, got %d", argc);
		return;
	}
	tent -> r_lo = atom_getfloat(arg++);
	tent -> r_hi = atom_getfloat(arg++);
	limiter(tent);
}

static void search(tent_struct *tent, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = tent -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_r = tent -> r;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], tent -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		tent -> r = (drand48() * (tent -> r_hi - tent -> r_lo)) + tent -> r_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(tent, NULL, argc, vars);
		do { calc(tent, tent -> vars); } while(jump--);
		tent -> lyap_exp = lyapunov((void *) tent, (t_gotfn) calc, M_var_count, (double *) tent -> vars);
		if (isnan(tent -> lyap_exp)) { not_found = 1; }
		if (tent -> lyap_exp < tent -> lyap_lo || tent -> lyap_exp > tent -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(tent, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) tent -> lyap_limit);
		post("Try using wider constraints.");
		tent -> r = temp_r;
		outlet_anything(tent -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		tent -> failure_ratio = (tent -> lyap_limit - not_expired) / tent -> lyap_limit;
		make_results(tent);
		outlet_anything(tent -> search_outlet, gensym("search"), M_search_count, tent -> search_out);
	}
}

void *tent_new(t_symbol *s, int argc, t_atom *argv) {
	tent_struct *tent = (tent_struct *) pd_new(tent_class);
	if (tent != NULL) {
		outlet_new(&tent -> x_obj, &s_float);
		tent -> search_outlet = outlet_new(&tent -> x_obj, &s_list);
		tent -> vars_outlet = outlet_new(&tent -> x_obj, &s_list);
		tent -> params_outlet = outlet_new(&tent -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			tent -> vars_init[M_x] = tent -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			tent -> r = (double) atom_getfloatarg(1, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for tent fractal. Expecting 2 arguments.");
			}
			tent -> vars_init[M_x] = 0.1;
			tent -> r = 1;
		}
		constrain(tent, NULL, 0, NULL);
		lyap(tent, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)tent;
}

void tent_setup(void) {
	tent_class = class_new(gensym("tent"), (t_newmethod) tent_new, 0, sizeof(tent_struct), 0, A_GIMME, 0);
	class_addbang(tent_class, (t_method) calculate);
	class_addmethod(tent_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(tent_class, (t_method) show, gensym("show"), 0);
	class_addmethod(tent_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(tent_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(tent_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(tent_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(tent_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(tent_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

