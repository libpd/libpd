/* martin Attractor PD External */
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
#define M_a_hi 1000

#define M_a 0

#define M_x 0
#define M_y 1

#define M_param_count 1
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "martin v0.0, by Michael McGonagle, from ??????, 2003";

t_class *martin_class;

typedef struct martin_struct {
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
	
	t_outlet *outlets[M_var_count - 1];
} martin_struct;

static void calc(martin_struct *martin, double *vars) {
	double x_0, y_0;
	x_0 =vars[M_y]-sin(vars[M_x]);
	y_0 =martin -> a-vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(martin_struct *martin) {
	calc(martin, martin -> vars);
	outlet_float(martin -> outlets[M_y - 1], martin -> vars[M_y]);
	outlet_float(martin -> x_obj.ob_outlet, martin -> vars[M_x]);
} // end calculate

static void reset(martin_struct *martin, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		martin -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		martin -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		martin -> vars[M_x] = martin -> vars_init[M_x];
		martin -> vars[M_y] = martin -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(martin_struct *martin) {
	static char buff[2];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((martin -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = '\0';
	return buff;
}

static void make_results(martin_struct *martin) {
	SETFLOAT(&martin -> search_out[0], martin -> lyap_exp);
	SETSYMBOL(&martin -> search_out[1], gensym(classify(martin)));
	SETFLOAT(&martin -> search_out[2], martin -> failure_ratio);
	SETFLOAT(&martin -> vars_out[M_x], martin -> vars[M_x]);
	SETFLOAT(&martin -> vars_out[M_y], martin -> vars[M_y]);
	SETFLOAT(&martin -> params_out[M_a], martin -> a);
	outlet_list(martin -> params_outlet, gensym("list"), M_param_count, martin -> params_out);
	outlet_list(martin -> vars_outlet, gensym("list"), M_var_count, martin -> vars_out);
}

static void show(martin_struct *martin) {
	make_results(martin);
	outlet_anything(martin -> search_outlet, gensym("show"), M_search_count, martin -> search_out);
}

static void param(martin_struct *martin, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 1) {
		post("Incorrect number of arguments for martin fractal. Expecting 1 arguments.");
		return;
	}
	martin -> a = (double) atom_getfloatarg(0, argc, argv);
}

static void seed(martin_struct *martin, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(martin_struct *martin, t_floatarg l, t_floatarg h, t_floatarg lim) {
	martin -> lyap_lo = l;
	martin -> lyap_hi = h;
	martin -> lyap_limit = (double) ((int) lim);
}

static void elyap(martin_struct *martin) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) martin, (t_gotfn) calc, M_var_count, martin -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(martin_struct *martin) {
	if (martin -> a_lo < M_a_lo) { martin -> a_lo = M_a_lo; }
	if (martin -> a_lo > M_a_hi) { martin -> a_lo = M_a_hi; }
	if (martin -> a_hi < M_a_lo) { martin -> a_hi = M_a_lo; }
	if (martin -> a_hi > M_a_hi) { martin -> a_hi = M_a_hi; }
}

static void constrain(martin_struct *martin, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		martin -> a_lo = M_a_lo;
		martin -> a_hi = M_a_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		martin -> a_lo = martin -> a - a_spread;
		martin -> a_hi = martin -> a + a_spread;
		limiter(martin);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for martin constraints, requires 2 values, got %d", argc);
		return;
	}
	martin -> a_lo = atom_getfloat(arg++);
	martin -> a_hi = atom_getfloat(arg++);
	limiter(martin);
}

static void search(martin_struct *martin, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = martin -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = martin -> a;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], martin -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		martin -> a = (drand48() * (martin -> a_hi - martin -> a_lo)) + martin -> a_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(martin, NULL, argc, vars);
		do { calc(martin, martin -> vars); } while(jump--);
		martin -> lyap_exp = lyapunov((void *) martin, (t_gotfn) calc, M_var_count, (double *) martin -> vars);
		if (isnan(martin -> lyap_exp)) { not_found = 1; }
		if (martin -> lyap_exp < martin -> lyap_lo || martin -> lyap_exp > martin -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(martin, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) martin -> lyap_limit);
		post("Try using wider constraints.");
		martin -> a = temp_a;
		outlet_anything(martin -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		martin -> failure_ratio = (martin -> lyap_limit - not_expired) / martin -> lyap_limit;
		make_results(martin);
		outlet_anything(martin -> search_outlet, gensym("search"), M_search_count, martin -> search_out);
	}
}

void *martin_new(t_symbol *s, int argc, t_atom *argv) {
	martin_struct *martin = (martin_struct *) pd_new(martin_class);
	if (martin != NULL) {
		outlet_new(&martin -> x_obj, &s_float);
		martin -> outlets[0] = outlet_new(&martin -> x_obj, &s_float);
		martin -> search_outlet = outlet_new(&martin -> x_obj, &s_list);
		martin -> vars_outlet = outlet_new(&martin -> x_obj, &s_list);
		martin -> params_outlet = outlet_new(&martin -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			martin -> vars_init[M_x] = martin -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			martin -> vars_init[M_y] = martin -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			martin -> a = (double) atom_getfloatarg(2, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for martin fractal. Expecting 3 arguments.");
			}
			martin -> vars_init[M_x] = 0.01;
			martin -> vars_init[M_y] = 0;
			martin -> a = 3.14;
		}
		constrain(martin, NULL, 0, NULL);
		lyap(martin, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)martin;
}

void martin_setup(void) {
	martin_class = class_new(gensym("martin"), (t_newmethod) martin_new, 0, sizeof(martin_struct), 0, A_GIMME, 0);
	class_addbang(martin_class, (t_method) calculate);
	class_addmethod(martin_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(martin_class, (t_method) show, gensym("show"), 0);
	class_addmethod(martin_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(martin_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(martin_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(martin_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(martin_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(martin_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

