/* standardmap Attractor PD External */
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

#define M_k_lo -1000
#define M_k_hi 1000

#define M_k 0

#define M_p 0
#define M_q 1

#define M_param_count 1
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "standardmap v0.0, by Michael McGonagle, from ??????, 2003";

t_class *standardmap_class;

typedef struct standardmap_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double k, k_lo, k_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} standardmap_struct;

static void calc(standardmap_struct *standardmap, double *vars) {
	double p_0, q_0;
	p_0 =vars[M_p]+standardmap -> k*sin(vars[M_q]+vars[M_p]);
	q_0 =vars[M_q]+vars[M_p];
	vars[M_p] = p_0;
	vars[M_q] = q_0;
} // end calc

static void calculate(standardmap_struct *standardmap) {
	calc(standardmap, standardmap -> vars);
	outlet_float(standardmap -> outlets[M_q - 1], standardmap -> vars[M_q]);
	outlet_float(standardmap -> x_obj.ob_outlet, standardmap -> vars[M_p]);
} // end calculate

static void reset(standardmap_struct *standardmap, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		standardmap -> vars[M_p] = (double) atom_getfloatarg(M_p, argc, argv);
		standardmap -> vars[M_q] = (double) atom_getfloatarg(M_q, argc, argv);
	} else {
		standardmap -> vars[M_p] = standardmap -> vars_init[M_p];
		standardmap -> vars[M_q] = standardmap -> vars_init[M_q];
	} // end if
} // end reset

static char *classify(standardmap_struct *standardmap) {
	static char buff[2];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((standardmap -> k - M_k_lo) * (1.0 / (M_k_hi - M_k_lo))) * 26)];
	buff[1] = '\0';
	return buff;
}

static void make_results(standardmap_struct *standardmap) {
	SETFLOAT(&standardmap -> search_out[0], standardmap -> lyap_exp);
	SETSYMBOL(&standardmap -> search_out[1], gensym(classify(standardmap)));
	SETFLOAT(&standardmap -> search_out[2], standardmap -> failure_ratio);
	SETFLOAT(&standardmap -> vars_out[M_p], standardmap -> vars[M_p]);
	SETFLOAT(&standardmap -> vars_out[M_q], standardmap -> vars[M_q]);
	SETFLOAT(&standardmap -> params_out[M_k], standardmap -> k);
	outlet_list(standardmap -> params_outlet, gensym("list"), M_param_count, standardmap -> params_out);
	outlet_list(standardmap -> vars_outlet, gensym("list"), M_var_count, standardmap -> vars_out);
}

static void show(standardmap_struct *standardmap) {
	make_results(standardmap);
	outlet_anything(standardmap -> search_outlet, gensym("show"), M_search_count, standardmap -> search_out);
}

static void param(standardmap_struct *standardmap, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 1) {
		post("Incorrect number of arguments for standardmap fractal. Expecting 1 arguments.");
		return;
	}
	standardmap -> k = (double) atom_getfloatarg(0, argc, argv);
}

static void seed(standardmap_struct *standardmap, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(standardmap_struct *standardmap, t_floatarg l, t_floatarg h, t_floatarg lim) {
	standardmap -> lyap_lo = l;
	standardmap -> lyap_hi = h;
	standardmap -> lyap_limit = (double) ((int) lim);
}

static void elyap(standardmap_struct *standardmap) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) standardmap, (t_gotfn) calc, M_var_count, standardmap -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(standardmap_struct *standardmap) {
	if (standardmap -> k_lo < M_k_lo) { standardmap -> k_lo = M_k_lo; }
	if (standardmap -> k_lo > M_k_hi) { standardmap -> k_lo = M_k_hi; }
	if (standardmap -> k_hi < M_k_lo) { standardmap -> k_hi = M_k_lo; }
	if (standardmap -> k_hi > M_k_hi) { standardmap -> k_hi = M_k_hi; }
}

static void constrain(standardmap_struct *standardmap, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		standardmap -> k_lo = M_k_lo;
		standardmap -> k_hi = M_k_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double k_spread = ((M_k_hi - M_k_lo) * percent) / 2;
		standardmap -> k_lo = standardmap -> k - k_spread;
		standardmap -> k_hi = standardmap -> k + k_spread;
		limiter(standardmap);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for standardmap constraints, requires 2 values, got %d", argc);
		return;
	}
	standardmap -> k_lo = atom_getfloat(arg++);
	standardmap -> k_hi = atom_getfloat(arg++);
	limiter(standardmap);
}

static void search(standardmap_struct *standardmap, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = standardmap -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_k = standardmap -> k;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], standardmap -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		standardmap -> k = (drand48() * (standardmap -> k_hi - standardmap -> k_lo)) + standardmap -> k_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(standardmap, NULL, argc, vars);
		do { calc(standardmap, standardmap -> vars); } while(jump--);
		standardmap -> lyap_exp = lyapunov((void *) standardmap, (t_gotfn) calc, M_var_count, (double *) standardmap -> vars);
		if (isnan(standardmap -> lyap_exp)) { not_found = 1; }
		if (standardmap -> lyap_exp < standardmap -> lyap_lo || standardmap -> lyap_exp > standardmap -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(standardmap, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) standardmap -> lyap_limit);
		post("Try using wider constraints.");
		standardmap -> k = temp_k;
		outlet_anything(standardmap -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		standardmap -> failure_ratio = (standardmap -> lyap_limit - not_expired) / standardmap -> lyap_limit;
		make_results(standardmap);
		outlet_anything(standardmap -> search_outlet, gensym("search"), M_search_count, standardmap -> search_out);
	}
}

void *standardmap_new(t_symbol *s, int argc, t_atom *argv) {
	standardmap_struct *standardmap = (standardmap_struct *) pd_new(standardmap_class);
	if (standardmap != NULL) {
		outlet_new(&standardmap -> x_obj, &s_float);
		standardmap -> outlets[0] = outlet_new(&standardmap -> x_obj, &s_float);
		standardmap -> search_outlet = outlet_new(&standardmap -> x_obj, &s_list);
		standardmap -> vars_outlet = outlet_new(&standardmap -> x_obj, &s_list);
		standardmap -> params_outlet = outlet_new(&standardmap -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			standardmap -> vars_init[M_p] = standardmap -> vars[M_p] = (double) atom_getfloatarg(0, argc, argv);
			standardmap -> vars_init[M_q] = standardmap -> vars[M_q] = (double) atom_getfloatarg(1, argc, argv);
			standardmap -> k = (double) atom_getfloatarg(2, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for standardmap fractal. Expecting 3 arguments.");
			}
			standardmap -> vars_init[M_p] = 0.1;
			standardmap -> vars_init[M_q] = 0.1;
			standardmap -> k = 1;
		}
		constrain(standardmap, NULL, 0, NULL);
		lyap(standardmap, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)standardmap;
}

void standardmap_setup(void) {
	standardmap_class = class_new(gensym("standardmap"), (t_newmethod) standardmap_new, 0, sizeof(standardmap_struct), 0, A_GIMME, 0);
	class_addbang(standardmap_class, (t_method) calculate);
	class_addmethod(standardmap_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(standardmap_class, (t_method) show, gensym("show"), 0);
	class_addmethod(standardmap_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(standardmap_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(standardmap_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(standardmap_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(standardmap_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(standardmap_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

