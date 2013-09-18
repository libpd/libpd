/* quadruptwo Attractor PD External */
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

static char *version = "quadruptwo v0.0, by Michael McGonagle, from ??????, 2003";

t_class *quadruptwo_class;

typedef struct quadruptwo_struct {
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
} quadruptwo_struct;

static void calc(quadruptwo_struct *quadruptwo, double *vars) {
	double x_0, y_0;
	x_0 =vars[M_y]-((vars[M_x]<0)?-1:1)*sin(log(abs(quadruptwo -> b*vars[M_x]-quadruptwo -> c)))*atan(pow(abs(quadruptwo -> c*vars[M_x]-quadruptwo -> b),2));
	y_0 =quadruptwo -> a-vars[M_x];
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(quadruptwo_struct *quadruptwo) {
	calc(quadruptwo, quadruptwo -> vars);
	outlet_float(quadruptwo -> outlets[M_y - 1], quadruptwo -> vars[M_y]);
	outlet_float(quadruptwo -> x_obj.ob_outlet, quadruptwo -> vars[M_x]);
} // end calculate

static void reset(quadruptwo_struct *quadruptwo, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		quadruptwo -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		quadruptwo -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		quadruptwo -> vars[M_x] = quadruptwo -> vars_init[M_x];
		quadruptwo -> vars[M_y] = quadruptwo -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(quadruptwo_struct *quadruptwo) {
	static char buff[4];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((quadruptwo -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((quadruptwo -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((quadruptwo -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = '\0';
	return buff;
}

static void make_results(quadruptwo_struct *quadruptwo) {
	SETFLOAT(&quadruptwo -> search_out[0], quadruptwo -> lyap_exp);
	SETSYMBOL(&quadruptwo -> search_out[1], gensym(classify(quadruptwo)));
	SETFLOAT(&quadruptwo -> search_out[2], quadruptwo -> failure_ratio);
	SETFLOAT(&quadruptwo -> vars_out[M_x], quadruptwo -> vars[M_x]);
	SETFLOAT(&quadruptwo -> vars_out[M_y], quadruptwo -> vars[M_y]);
	SETFLOAT(&quadruptwo -> params_out[M_a], quadruptwo -> a);
	SETFLOAT(&quadruptwo -> params_out[M_b], quadruptwo -> b);
	SETFLOAT(&quadruptwo -> params_out[M_c], quadruptwo -> c);
	outlet_list(quadruptwo -> params_outlet, gensym("list"), M_param_count, quadruptwo -> params_out);
	outlet_list(quadruptwo -> vars_outlet, gensym("list"), M_var_count, quadruptwo -> vars_out);
}

static void show(quadruptwo_struct *quadruptwo) {
	make_results(quadruptwo);
	outlet_anything(quadruptwo -> search_outlet, gensym("show"), M_search_count, quadruptwo -> search_out);
}

static void param(quadruptwo_struct *quadruptwo, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 3) {
		post("Incorrect number of arguments for quadruptwo fractal. Expecting 3 arguments.");
		return;
	}
	quadruptwo -> a = (double) atom_getfloatarg(0, argc, argv);
	quadruptwo -> b = (double) atom_getfloatarg(1, argc, argv);
	quadruptwo -> c = (double) atom_getfloatarg(2, argc, argv);
}

static void seed(quadruptwo_struct *quadruptwo, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(quadruptwo_struct *quadruptwo, t_floatarg l, t_floatarg h, t_floatarg lim) {
	quadruptwo -> lyap_lo = l;
	quadruptwo -> lyap_hi = h;
	quadruptwo -> lyap_limit = (double) ((int) lim);
}

static void elyap(quadruptwo_struct *quadruptwo) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) quadruptwo, (t_gotfn) calc, M_var_count, quadruptwo -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(quadruptwo_struct *quadruptwo) {
	if (quadruptwo -> a_lo < M_a_lo) { quadruptwo -> a_lo = M_a_lo; }
	if (quadruptwo -> a_lo > M_a_hi) { quadruptwo -> a_lo = M_a_hi; }
	if (quadruptwo -> a_hi < M_a_lo) { quadruptwo -> a_hi = M_a_lo; }
	if (quadruptwo -> a_hi > M_a_hi) { quadruptwo -> a_hi = M_a_hi; }
	if (quadruptwo -> b_lo < M_b_lo) { quadruptwo -> b_lo = M_b_lo; }
	if (quadruptwo -> b_lo > M_b_hi) { quadruptwo -> b_lo = M_b_hi; }
	if (quadruptwo -> b_hi < M_b_lo) { quadruptwo -> b_hi = M_b_lo; }
	if (quadruptwo -> b_hi > M_b_hi) { quadruptwo -> b_hi = M_b_hi; }
	if (quadruptwo -> c_lo < M_c_lo) { quadruptwo -> c_lo = M_c_lo; }
	if (quadruptwo -> c_lo > M_c_hi) { quadruptwo -> c_lo = M_c_hi; }
	if (quadruptwo -> c_hi < M_c_lo) { quadruptwo -> c_hi = M_c_lo; }
	if (quadruptwo -> c_hi > M_c_hi) { quadruptwo -> c_hi = M_c_hi; }
}

static void constrain(quadruptwo_struct *quadruptwo, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		quadruptwo -> a_lo = M_a_lo;
		quadruptwo -> a_hi = M_a_hi;
		quadruptwo -> b_lo = M_b_lo;
		quadruptwo -> b_hi = M_b_hi;
		quadruptwo -> c_lo = M_c_lo;
		quadruptwo -> c_hi = M_c_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		quadruptwo -> a_lo = quadruptwo -> a - a_spread;
		quadruptwo -> a_hi = quadruptwo -> a + a_spread;
		quadruptwo -> b_lo = quadruptwo -> b - b_spread;
		quadruptwo -> b_hi = quadruptwo -> b + b_spread;
		quadruptwo -> c_lo = quadruptwo -> c - c_spread;
		quadruptwo -> c_hi = quadruptwo -> c + c_spread;
		limiter(quadruptwo);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for quadruptwo constraints, requires 6 values, got %d", argc);
		return;
	}
	quadruptwo -> a_lo = atom_getfloat(arg++);
	quadruptwo -> a_hi = atom_getfloat(arg++);
	quadruptwo -> b_lo = atom_getfloat(arg++);
	quadruptwo -> b_hi = atom_getfloat(arg++);
	quadruptwo -> c_lo = atom_getfloat(arg++);
	quadruptwo -> c_hi = atom_getfloat(arg++);
	limiter(quadruptwo);
}

static void search(quadruptwo_struct *quadruptwo, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = quadruptwo -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = quadruptwo -> a;
	double temp_b = quadruptwo -> b;
	double temp_c = quadruptwo -> c;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], quadruptwo -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		quadruptwo -> a = (drand48() * (quadruptwo -> a_hi - quadruptwo -> a_lo)) + quadruptwo -> a_lo;
		quadruptwo -> b = (drand48() * (quadruptwo -> b_hi - quadruptwo -> b_lo)) + quadruptwo -> b_lo;
		quadruptwo -> c = (drand48() * (quadruptwo -> c_hi - quadruptwo -> c_lo)) + quadruptwo -> c_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(quadruptwo, NULL, argc, vars);
		do { calc(quadruptwo, quadruptwo -> vars); } while(jump--);
		quadruptwo -> lyap_exp = lyapunov((void *) quadruptwo, (t_gotfn) calc, M_var_count, (double *) quadruptwo -> vars);
		if (isnan(quadruptwo -> lyap_exp)) { not_found = 1; }
		if (quadruptwo -> lyap_exp < quadruptwo -> lyap_lo || quadruptwo -> lyap_exp > quadruptwo -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(quadruptwo, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) quadruptwo -> lyap_limit);
		post("Try using wider constraints.");
		quadruptwo -> a = temp_a;
		quadruptwo -> b = temp_b;
		quadruptwo -> c = temp_c;
		outlet_anything(quadruptwo -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		quadruptwo -> failure_ratio = (quadruptwo -> lyap_limit - not_expired) / quadruptwo -> lyap_limit;
		make_results(quadruptwo);
		outlet_anything(quadruptwo -> search_outlet, gensym("search"), M_search_count, quadruptwo -> search_out);
	}
}

void *quadruptwo_new(t_symbol *s, int argc, t_atom *argv) {
	quadruptwo_struct *quadruptwo = (quadruptwo_struct *) pd_new(quadruptwo_class);
	if (quadruptwo != NULL) {
		outlet_new(&quadruptwo -> x_obj, &s_float);
		quadruptwo -> outlets[0] = outlet_new(&quadruptwo -> x_obj, &s_float);
		quadruptwo -> search_outlet = outlet_new(&quadruptwo -> x_obj, &s_list);
		quadruptwo -> vars_outlet = outlet_new(&quadruptwo -> x_obj, &s_list);
		quadruptwo -> params_outlet = outlet_new(&quadruptwo -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			quadruptwo -> vars_init[M_x] = quadruptwo -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			quadruptwo -> vars_init[M_y] = quadruptwo -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			quadruptwo -> a = (double) atom_getfloatarg(2, argc, argv);
			quadruptwo -> b = (double) atom_getfloatarg(3, argc, argv);
			quadruptwo -> c = (double) atom_getfloatarg(4, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for quadruptwo fractal. Expecting 5 arguments.");
			}
			quadruptwo -> vars_init[M_x] = 0;
			quadruptwo -> vars_init[M_y] = 0;
			quadruptwo -> a = 34;
			quadruptwo -> b = 1;
			quadruptwo -> c = 5;
		}
		constrain(quadruptwo, NULL, 0, NULL);
		lyap(quadruptwo, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)quadruptwo;
}

void quadruptwo_setup(void) {
	quadruptwo_class = class_new(gensym("quadruptwo"), (t_newmethod) quadruptwo_new, 0, sizeof(quadruptwo_struct), 0, A_GIMME, 0);
	class_addbang(quadruptwo_class, (t_method) calculate);
	class_addmethod(quadruptwo_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(quadruptwo_class, (t_method) show, gensym("show"), 0);
	class_addmethod(quadruptwo_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(quadruptwo_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(quadruptwo_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(quadruptwo_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(quadruptwo_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(quadruptwo_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

