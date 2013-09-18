/* hopalong Attractor PD External */
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

static char *version = "hopalong v0.0, by Michael McGonagle, from ??????, 2003";

t_class *hopalong_class;

typedef struct hopalong_struct {
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
} hopalong_struct;

static void calc(hopalong_struct *hopalong, double *vars) {
	double x_0, y_0;
	x_0 =vars[M_y]-((vars[M_x]<0)?-1:1)*sqrt(abs(hopalong -> b*vars[M_x]-hopalong -> c));
	y_0 =hopalong -> a-vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(hopalong_struct *hopalong) {
	calc(hopalong, hopalong -> vars);
	outlet_float(hopalong -> outlets[M_y - 1], hopalong -> vars[M_y]);
	outlet_float(hopalong -> x_obj.ob_outlet, hopalong -> vars[M_x]);
} // end calculate

static void reset(hopalong_struct *hopalong, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		hopalong -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		hopalong -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		hopalong -> vars[M_x] = hopalong -> vars_init[M_x];
		hopalong -> vars[M_y] = hopalong -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(hopalong_struct *hopalong) {
	static char buff[4];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((hopalong -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((hopalong -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((hopalong -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = '\0';
	return buff;
}

static void make_results(hopalong_struct *hopalong) {
	SETFLOAT(&hopalong -> search_out[0], hopalong -> lyap_exp);
	SETSYMBOL(&hopalong -> search_out[1], gensym(classify(hopalong)));
	SETFLOAT(&hopalong -> search_out[2], hopalong -> failure_ratio);
	SETFLOAT(&hopalong -> vars_out[M_x], hopalong -> vars[M_x]);
	SETFLOAT(&hopalong -> vars_out[M_y], hopalong -> vars[M_y]);
	SETFLOAT(&hopalong -> params_out[M_a], hopalong -> a);
	SETFLOAT(&hopalong -> params_out[M_b], hopalong -> b);
	SETFLOAT(&hopalong -> params_out[M_c], hopalong -> c);
	outlet_list(hopalong -> params_outlet, gensym("list"), M_param_count, hopalong -> params_out);
	outlet_list(hopalong -> vars_outlet, gensym("list"), M_var_count, hopalong -> vars_out);
}

static void show(hopalong_struct *hopalong) {
	make_results(hopalong);
	outlet_anything(hopalong -> search_outlet, gensym("show"), M_search_count, hopalong -> search_out);
}

static void param(hopalong_struct *hopalong, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 3) {
		post("Incorrect number of arguments for hopalong fractal. Expecting 3 arguments.");
		return;
	}
	hopalong -> a = (double) atom_getfloatarg(0, argc, argv);
	hopalong -> b = (double) atom_getfloatarg(1, argc, argv);
	hopalong -> c = (double) atom_getfloatarg(2, argc, argv);
}

static void seed(hopalong_struct *hopalong, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(hopalong_struct *hopalong, t_floatarg l, t_floatarg h, t_floatarg lim) {
	hopalong -> lyap_lo = l;
	hopalong -> lyap_hi = h;
	hopalong -> lyap_limit = (double) ((int) lim);
}

static void elyap(hopalong_struct *hopalong) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) hopalong, (t_gotfn) calc, M_var_count, hopalong -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(hopalong_struct *hopalong) {
	if (hopalong -> a_lo < M_a_lo) { hopalong -> a_lo = M_a_lo; }
	if (hopalong -> a_lo > M_a_hi) { hopalong -> a_lo = M_a_hi; }
	if (hopalong -> a_hi < M_a_lo) { hopalong -> a_hi = M_a_lo; }
	if (hopalong -> a_hi > M_a_hi) { hopalong -> a_hi = M_a_hi; }
	if (hopalong -> b_lo < M_b_lo) { hopalong -> b_lo = M_b_lo; }
	if (hopalong -> b_lo > M_b_hi) { hopalong -> b_lo = M_b_hi; }
	if (hopalong -> b_hi < M_b_lo) { hopalong -> b_hi = M_b_lo; }
	if (hopalong -> b_hi > M_b_hi) { hopalong -> b_hi = M_b_hi; }
	if (hopalong -> c_lo < M_c_lo) { hopalong -> c_lo = M_c_lo; }
	if (hopalong -> c_lo > M_c_hi) { hopalong -> c_lo = M_c_hi; }
	if (hopalong -> c_hi < M_c_lo) { hopalong -> c_hi = M_c_lo; }
	if (hopalong -> c_hi > M_c_hi) { hopalong -> c_hi = M_c_hi; }
}

static void constrain(hopalong_struct *hopalong, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		hopalong -> a_lo = M_a_lo;
		hopalong -> a_hi = M_a_hi;
		hopalong -> b_lo = M_b_lo;
		hopalong -> b_hi = M_b_hi;
		hopalong -> c_lo = M_c_lo;
		hopalong -> c_hi = M_c_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		hopalong -> a_lo = hopalong -> a - a_spread;
		hopalong -> a_hi = hopalong -> a + a_spread;
		hopalong -> b_lo = hopalong -> b - b_spread;
		hopalong -> b_hi = hopalong -> b + b_spread;
		hopalong -> c_lo = hopalong -> c - c_spread;
		hopalong -> c_hi = hopalong -> c + c_spread;
		limiter(hopalong);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for hopalong constraints, requires 6 values, got %d", argc);
		return;
	}
	hopalong -> a_lo = atom_getfloat(arg++);
	hopalong -> a_hi = atom_getfloat(arg++);
	hopalong -> b_lo = atom_getfloat(arg++);
	hopalong -> b_hi = atom_getfloat(arg++);
	hopalong -> c_lo = atom_getfloat(arg++);
	hopalong -> c_hi = atom_getfloat(arg++);
	limiter(hopalong);
}

static void search(hopalong_struct *hopalong, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = hopalong -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = hopalong -> a;
	double temp_b = hopalong -> b;
	double temp_c = hopalong -> c;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], hopalong -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		hopalong -> a = (drand48() * (hopalong -> a_hi - hopalong -> a_lo)) + hopalong -> a_lo;
		hopalong -> b = (drand48() * (hopalong -> b_hi - hopalong -> b_lo)) + hopalong -> b_lo;
		hopalong -> c = (drand48() * (hopalong -> c_hi - hopalong -> c_lo)) + hopalong -> c_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(hopalong, NULL, argc, vars);
		do { calc(hopalong, hopalong -> vars); } while(jump--);
		hopalong -> lyap_exp = lyapunov((void *) hopalong, (t_gotfn) calc, M_var_count, (double *) hopalong -> vars);
		if (isnan(hopalong -> lyap_exp)) { not_found = 1; }
		if (hopalong -> lyap_exp < hopalong -> lyap_lo || hopalong -> lyap_exp > hopalong -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(hopalong, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) hopalong -> lyap_limit);
		post("Try using wider constraints.");
		hopalong -> a = temp_a;
		hopalong -> b = temp_b;
		hopalong -> c = temp_c;
		outlet_anything(hopalong -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		hopalong -> failure_ratio = (hopalong -> lyap_limit - not_expired) / hopalong -> lyap_limit;
		make_results(hopalong);
		outlet_anything(hopalong -> search_outlet, gensym("search"), M_search_count, hopalong -> search_out);
	}
}

void *hopalong_new(t_symbol *s, int argc, t_atom *argv) {
	hopalong_struct *hopalong = (hopalong_struct *) pd_new(hopalong_class);
	if (hopalong != NULL) {
		outlet_new(&hopalong -> x_obj, &s_float);
		hopalong -> outlets[0] = outlet_new(&hopalong -> x_obj, &s_float);
		hopalong -> search_outlet = outlet_new(&hopalong -> x_obj, &s_list);
		hopalong -> vars_outlet = outlet_new(&hopalong -> x_obj, &s_list);
		hopalong -> params_outlet = outlet_new(&hopalong -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			hopalong -> vars_init[M_x] = hopalong -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			hopalong -> vars_init[M_y] = hopalong -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			hopalong -> a = (double) atom_getfloatarg(2, argc, argv);
			hopalong -> b = (double) atom_getfloatarg(3, argc, argv);
			hopalong -> c = (double) atom_getfloatarg(4, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for hopalong fractal. Expecting 5 arguments.");
			}
			hopalong -> vars_init[M_x] = 0.1;
			hopalong -> vars_init[M_y] = 0.1;
			hopalong -> a = 1;
			hopalong -> b = 1;
			hopalong -> c = 0;
		}
		constrain(hopalong, NULL, 0, NULL);
		lyap(hopalong, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)hopalong;
}

void hopalong_setup(void) {
	hopalong_class = class_new(gensym("hopalong"), (t_newmethod) hopalong_new, 0, sizeof(hopalong_struct), 0, A_GIMME, 0);
	class_addbang(hopalong_class, (t_method) calculate);
	class_addmethod(hopalong_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(hopalong_class, (t_method) show, gensym("show"), 0);
	class_addmethod(hopalong_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(hopalong_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(hopalong_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(hopalong_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(hopalong_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(hopalong_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

