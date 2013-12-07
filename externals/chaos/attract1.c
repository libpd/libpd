/* attract1 Attractor PD External */
/* Copyright Michael McGonagle, from 'attract.java' by Julian Sprott, 2003 */
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

#define M_a0_lo -3
#define M_a0_hi 3
#define M_a1_lo -3
#define M_a1_hi 3
#define M_a2_lo -3
#define M_a2_hi 3
#define M_a3_lo -3
#define M_a3_hi 3
#define M_a4_lo -3
#define M_a4_hi 3
#define M_a5_lo -3
#define M_a5_hi 3

#define M_a0 0
#define M_a1 1
#define M_a2 2
#define M_a3 3
#define M_a4 4
#define M_a5 5

#define M_x 0
#define M_y 1

#define M_param_count 6
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "attract1 v0.0, by Michael McGonagle, from 'attract.java' by Julian Sprott, 2003";

t_class *attract1_class;

typedef struct attract1_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a0, a0_lo, a0_hi, a1, a1_lo, a1_hi, a2, a2_lo, a2_hi, a3, a3_lo, a3_hi, a4, a4_lo, a4_hi, a5, a5_lo, a5_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} attract1_struct;

static void calc(attract1_struct *attract1, double *vars) {
	double x_0, y_0;
	x_0 =attract1 -> a0+vars[M_x]*(attract1 -> a1+attract1 -> a2*vars[M_x]+attract1 -> a3*vars[M_y])+vars[M_y]*(attract1 -> a4+attract1 -> a5*vars[M_y]);
	y_0 =vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(attract1_struct *attract1) {
	calc(attract1, attract1 -> vars);
	outlet_float(attract1 -> outlets[M_y - 1], attract1 -> vars[M_y]);
	outlet_float(attract1 -> x_obj.ob_outlet, attract1 -> vars[M_x]);
} // end calculate

static void reset(attract1_struct *attract1, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		attract1 -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		attract1 -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		attract1 -> vars[M_x] = attract1 -> vars_init[M_x];
		attract1 -> vars[M_y] = attract1 -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(attract1_struct *attract1) {
	static char buff[7];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((attract1 -> a0 - M_a0_lo) * (1.0 / (M_a0_hi - M_a0_lo))) * 26)];
	buff[1] = c[(int) (((attract1 -> a1 - M_a1_lo) * (1.0 / (M_a1_hi - M_a1_lo))) * 26)];
	buff[2] = c[(int) (((attract1 -> a2 - M_a2_lo) * (1.0 / (M_a2_hi - M_a2_lo))) * 26)];
	buff[3] = c[(int) (((attract1 -> a3 - M_a3_lo) * (1.0 / (M_a3_hi - M_a3_lo))) * 26)];
	buff[4] = c[(int) (((attract1 -> a4 - M_a4_lo) * (1.0 / (M_a4_hi - M_a4_lo))) * 26)];
	buff[5] = c[(int) (((attract1 -> a5 - M_a5_lo) * (1.0 / (M_a5_hi - M_a5_lo))) * 26)];
	buff[6] = '\0';
	return buff;
}

static void make_results(attract1_struct *attract1) {
	SETFLOAT(&attract1 -> search_out[0], attract1 -> lyap_exp);
	SETSYMBOL(&attract1 -> search_out[1], gensym(classify(attract1)));
	SETFLOAT(&attract1 -> search_out[2], attract1 -> failure_ratio);
	SETFLOAT(&attract1 -> vars_out[M_x], attract1 -> vars[M_x]);
	SETFLOAT(&attract1 -> vars_out[M_y], attract1 -> vars[M_y]);
	SETFLOAT(&attract1 -> params_out[M_a0], attract1 -> a0);
	SETFLOAT(&attract1 -> params_out[M_a1], attract1 -> a1);
	SETFLOAT(&attract1 -> params_out[M_a2], attract1 -> a2);
	SETFLOAT(&attract1 -> params_out[M_a3], attract1 -> a3);
	SETFLOAT(&attract1 -> params_out[M_a4], attract1 -> a4);
	SETFLOAT(&attract1 -> params_out[M_a5], attract1 -> a5);
	outlet_list(attract1 -> params_outlet, gensym("list"), M_param_count, attract1 -> params_out);
	outlet_list(attract1 -> vars_outlet, gensym("list"), M_var_count, attract1 -> vars_out);
}

static void show(attract1_struct *attract1) {
	make_results(attract1);
	outlet_anything(attract1 -> search_outlet, gensym("show"), M_search_count, attract1 -> search_out);
}

static void param(attract1_struct *attract1, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 6) {
		post("Incorrect number of arguments for attract1 fractal. Expecting 6 arguments.");
		return;
	}
	attract1 -> a0 = (double) atom_getfloatarg(0, argc, argv);
	attract1 -> a1 = (double) atom_getfloatarg(1, argc, argv);
	attract1 -> a2 = (double) atom_getfloatarg(2, argc, argv);
	attract1 -> a3 = (double) atom_getfloatarg(3, argc, argv);
	attract1 -> a4 = (double) atom_getfloatarg(4, argc, argv);
	attract1 -> a5 = (double) atom_getfloatarg(5, argc, argv);
}

static void seed(attract1_struct *attract1, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(attract1_struct *attract1, t_floatarg l, t_floatarg h, t_floatarg lim) {
	attract1 -> lyap_lo = l;
	attract1 -> lyap_hi = h;
	attract1 -> lyap_limit = (double) ((int) lim);
}

static void elyap(attract1_struct *attract1) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) attract1, (t_gotfn) calc, M_var_count, attract1 -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(attract1_struct *attract1) {
	if (attract1 -> a0_lo < M_a0_lo) { attract1 -> a0_lo = M_a0_lo; }
	if (attract1 -> a0_lo > M_a0_hi) { attract1 -> a0_lo = M_a0_hi; }
	if (attract1 -> a0_hi < M_a0_lo) { attract1 -> a0_hi = M_a0_lo; }
	if (attract1 -> a0_hi > M_a0_hi) { attract1 -> a0_hi = M_a0_hi; }
	if (attract1 -> a1_lo < M_a1_lo) { attract1 -> a1_lo = M_a1_lo; }
	if (attract1 -> a1_lo > M_a1_hi) { attract1 -> a1_lo = M_a1_hi; }
	if (attract1 -> a1_hi < M_a1_lo) { attract1 -> a1_hi = M_a1_lo; }
	if (attract1 -> a1_hi > M_a1_hi) { attract1 -> a1_hi = M_a1_hi; }
	if (attract1 -> a2_lo < M_a2_lo) { attract1 -> a2_lo = M_a2_lo; }
	if (attract1 -> a2_lo > M_a2_hi) { attract1 -> a2_lo = M_a2_hi; }
	if (attract1 -> a2_hi < M_a2_lo) { attract1 -> a2_hi = M_a2_lo; }
	if (attract1 -> a2_hi > M_a2_hi) { attract1 -> a2_hi = M_a2_hi; }
	if (attract1 -> a3_lo < M_a3_lo) { attract1 -> a3_lo = M_a3_lo; }
	if (attract1 -> a3_lo > M_a3_hi) { attract1 -> a3_lo = M_a3_hi; }
	if (attract1 -> a3_hi < M_a3_lo) { attract1 -> a3_hi = M_a3_lo; }
	if (attract1 -> a3_hi > M_a3_hi) { attract1 -> a3_hi = M_a3_hi; }
	if (attract1 -> a4_lo < M_a4_lo) { attract1 -> a4_lo = M_a4_lo; }
	if (attract1 -> a4_lo > M_a4_hi) { attract1 -> a4_lo = M_a4_hi; }
	if (attract1 -> a4_hi < M_a4_lo) { attract1 -> a4_hi = M_a4_lo; }
	if (attract1 -> a4_hi > M_a4_hi) { attract1 -> a4_hi = M_a4_hi; }
	if (attract1 -> a5_lo < M_a5_lo) { attract1 -> a5_lo = M_a5_lo; }
	if (attract1 -> a5_lo > M_a5_hi) { attract1 -> a5_lo = M_a5_hi; }
	if (attract1 -> a5_hi < M_a5_lo) { attract1 -> a5_hi = M_a5_lo; }
	if (attract1 -> a5_hi > M_a5_hi) { attract1 -> a5_hi = M_a5_hi; }
}

static void constrain(attract1_struct *attract1, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		attract1 -> a0_lo = M_a0_lo;
		attract1 -> a0_hi = M_a0_hi;
		attract1 -> a1_lo = M_a1_lo;
		attract1 -> a1_hi = M_a1_hi;
		attract1 -> a2_lo = M_a2_lo;
		attract1 -> a2_hi = M_a2_hi;
		attract1 -> a3_lo = M_a3_lo;
		attract1 -> a3_hi = M_a3_hi;
		attract1 -> a4_lo = M_a4_lo;
		attract1 -> a4_hi = M_a4_hi;
		attract1 -> a5_lo = M_a5_lo;
		attract1 -> a5_hi = M_a5_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a0_spread = ((M_a0_hi - M_a0_lo) * percent) / 2;
		double a1_spread = ((M_a1_hi - M_a1_lo) * percent) / 2;
		double a2_spread = ((M_a2_hi - M_a2_lo) * percent) / 2;
		double a3_spread = ((M_a3_hi - M_a3_lo) * percent) / 2;
		double a4_spread = ((M_a4_hi - M_a4_lo) * percent) / 2;
		double a5_spread = ((M_a5_hi - M_a5_lo) * percent) / 2;
		attract1 -> a0_lo = attract1 -> a0 - a0_spread;
		attract1 -> a0_hi = attract1 -> a0 + a0_spread;
		attract1 -> a1_lo = attract1 -> a1 - a1_spread;
		attract1 -> a1_hi = attract1 -> a1 + a1_spread;
		attract1 -> a2_lo = attract1 -> a2 - a2_spread;
		attract1 -> a2_hi = attract1 -> a2 + a2_spread;
		attract1 -> a3_lo = attract1 -> a3 - a3_spread;
		attract1 -> a3_hi = attract1 -> a3 + a3_spread;
		attract1 -> a4_lo = attract1 -> a4 - a4_spread;
		attract1 -> a4_hi = attract1 -> a4 + a4_spread;
		attract1 -> a5_lo = attract1 -> a5 - a5_spread;
		attract1 -> a5_hi = attract1 -> a5 + a5_spread;
		limiter(attract1);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for attract1 constraints, requires 12 values, got %d", argc);
		return;
	}
	attract1 -> a0_lo = atom_getfloat(arg++);
	attract1 -> a0_hi = atom_getfloat(arg++);
	attract1 -> a1_lo = atom_getfloat(arg++);
	attract1 -> a1_hi = atom_getfloat(arg++);
	attract1 -> a2_lo = atom_getfloat(arg++);
	attract1 -> a2_hi = atom_getfloat(arg++);
	attract1 -> a3_lo = atom_getfloat(arg++);
	attract1 -> a3_hi = atom_getfloat(arg++);
	attract1 -> a4_lo = atom_getfloat(arg++);
	attract1 -> a4_hi = atom_getfloat(arg++);
	attract1 -> a5_lo = atom_getfloat(arg++);
	attract1 -> a5_hi = atom_getfloat(arg++);
	limiter(attract1);
}

static void search(attract1_struct *attract1, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = attract1 -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a0 = attract1 -> a0;
	double temp_a1 = attract1 -> a1;
	double temp_a2 = attract1 -> a2;
	double temp_a3 = attract1 -> a3;
	double temp_a4 = attract1 -> a4;
	double temp_a5 = attract1 -> a5;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], attract1 -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		attract1 -> a0 = (drand48() * (attract1 -> a0_hi - attract1 -> a0_lo)) + attract1 -> a0_lo;
		attract1 -> a1 = (drand48() * (attract1 -> a1_hi - attract1 -> a1_lo)) + attract1 -> a1_lo;
		attract1 -> a2 = (drand48() * (attract1 -> a2_hi - attract1 -> a2_lo)) + attract1 -> a2_lo;
		attract1 -> a3 = (drand48() * (attract1 -> a3_hi - attract1 -> a3_lo)) + attract1 -> a3_lo;
		attract1 -> a4 = (drand48() * (attract1 -> a4_hi - attract1 -> a4_lo)) + attract1 -> a4_lo;
		attract1 -> a5 = (drand48() * (attract1 -> a5_hi - attract1 -> a5_lo)) + attract1 -> a5_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(attract1, NULL, argc, vars);
		do { calc(attract1, attract1 -> vars); } while(jump--);
		attract1 -> lyap_exp = lyapunov((void *) attract1, (t_gotfn) calc, M_var_count, (double *) attract1 -> vars);
		if (isnan(attract1 -> lyap_exp)) { not_found = 1; }
		if (attract1 -> lyap_exp < attract1 -> lyap_lo || attract1 -> lyap_exp > attract1 -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(attract1, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) attract1 -> lyap_limit);
		post("Try using wider constraints.");
		attract1 -> a0 = temp_a0;
		attract1 -> a1 = temp_a1;
		attract1 -> a2 = temp_a2;
		attract1 -> a3 = temp_a3;
		attract1 -> a4 = temp_a4;
		attract1 -> a5 = temp_a5;
		outlet_anything(attract1 -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		attract1 -> failure_ratio = (attract1 -> lyap_limit - not_expired) / attract1 -> lyap_limit;
		make_results(attract1);
		outlet_anything(attract1 -> search_outlet, gensym("search"), M_search_count, attract1 -> search_out);
	}
}

void *attract1_new(t_symbol *s, int argc, t_atom *argv) {
	attract1_struct *attract1 = (attract1_struct *) pd_new(attract1_class);
	if (attract1 != NULL) {
		outlet_new(&attract1 -> x_obj, &s_float);
		attract1 -> outlets[0] = outlet_new(&attract1 -> x_obj, &s_float);
		attract1 -> search_outlet = outlet_new(&attract1 -> x_obj, &s_list);
		attract1 -> vars_outlet = outlet_new(&attract1 -> x_obj, &s_list);
		attract1 -> params_outlet = outlet_new(&attract1 -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			attract1 -> vars_init[M_x] = attract1 -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			attract1 -> vars_init[M_y] = attract1 -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			attract1 -> a0 = (double) atom_getfloatarg(2, argc, argv);
			attract1 -> a1 = (double) atom_getfloatarg(3, argc, argv);
			attract1 -> a2 = (double) atom_getfloatarg(4, argc, argv);
			attract1 -> a3 = (double) atom_getfloatarg(5, argc, argv);
			attract1 -> a4 = (double) atom_getfloatarg(6, argc, argv);
			attract1 -> a5 = (double) atom_getfloatarg(7, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for attract1 fractal. Expecting 8 arguments.");
			}
			attract1 -> vars_init[M_x] = 0;
			attract1 -> vars_init[M_y] = 0;
			attract1 -> a0 = 1;
			attract1 -> a1 = 1;
			attract1 -> a2 = 1;
			attract1 -> a3 = 1;
			attract1 -> a4 = 1;
			attract1 -> a5 = 1;
		}
		constrain(attract1, NULL, 0, NULL);
		lyap(attract1, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)attract1;
}

void attract1_setup(void) {
	attract1_class = class_new(gensym("attract1"), (t_newmethod) attract1_new, 0, sizeof(attract1_struct), 0, A_GIMME, 0);
	class_addbang(attract1_class, (t_method) calculate);
	class_addmethod(attract1_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(attract1_class, (t_method) show, gensym("show"), 0);
	class_addmethod(attract1_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(attract1_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(attract1_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(attract1_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(attract1_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(attract1_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

