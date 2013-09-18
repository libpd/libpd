/* popcorn Attractor PD External */
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

#define M_h_lo -1000
#define M_h_hi 1000

#define M_h 0

#define M_x 0
#define M_y 1

#define M_param_count 1
#define M_var_count 2
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "popcorn v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *popcorn_class;

typedef struct popcorn_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double h, h_lo, h_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} popcorn_struct;

static void calc(popcorn_struct *popcorn, double *vars) {
	double x_0, y_0;
	x_0 =vars[M_x]-popcorn -> h*sin(vars[M_y]+tan(3*vars[M_y]));
	y_0 =vars[M_y]-popcorn -> h*sin(vars[M_x]+tan(3*vars[M_x]));
	vars[M_x] = x_0;
	vars[M_y] = y_0;
} // end calc

static void calculate(popcorn_struct *popcorn) {
	calc(popcorn, popcorn -> vars);
	outlet_float(popcorn -> outlets[M_y - 1], popcorn -> vars[M_y]);
	outlet_float(popcorn -> x_obj.ob_outlet, popcorn -> vars[M_x]);
} // end calculate

static void reset(popcorn_struct *popcorn, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		popcorn -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		popcorn -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
	} else {
		popcorn -> vars[M_x] = popcorn -> vars_init[M_x];
		popcorn -> vars[M_y] = popcorn -> vars_init[M_y];
	} // end if
} // end reset

static char *classify(popcorn_struct *popcorn) {
	static char buff[2];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((popcorn -> h - M_h_lo) * (1.0 / (M_h_hi - M_h_lo))) * 26)];
	buff[1] = '\0';
	return buff;
}

static void make_results(popcorn_struct *popcorn) {
	SETFLOAT(&popcorn -> search_out[0], popcorn -> lyap_exp);
	SETSYMBOL(&popcorn -> search_out[1], gensym(classify(popcorn)));
	SETFLOAT(&popcorn -> search_out[2], popcorn -> failure_ratio);
	SETFLOAT(&popcorn -> vars_out[M_x], popcorn -> vars[M_x]);
	SETFLOAT(&popcorn -> vars_out[M_y], popcorn -> vars[M_y]);
	SETFLOAT(&popcorn -> params_out[M_h], popcorn -> h);
	outlet_list(popcorn -> params_outlet, gensym("list"), M_param_count, popcorn -> params_out);
	outlet_list(popcorn -> vars_outlet, gensym("list"), M_var_count, popcorn -> vars_out);
}

static void show(popcorn_struct *popcorn) {
	make_results(popcorn);
	outlet_anything(popcorn -> search_outlet, gensym("show"), M_search_count, popcorn -> search_out);
}

static void param(popcorn_struct *popcorn, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 1) {
		post("Incorrect number of arguments for popcorn fractal. Expecting 1 arguments.");
		return;
	}
	popcorn -> h = (double) atom_getfloatarg(0, argc, argv);
}

static void seed(popcorn_struct *popcorn, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(popcorn_struct *popcorn, t_floatarg l, t_floatarg h, t_floatarg lim) {
	popcorn -> lyap_lo = l;
	popcorn -> lyap_hi = h;
	popcorn -> lyap_limit = (double) ((int) lim);
}

static void elyap(popcorn_struct *popcorn) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) popcorn, (t_gotfn) calc, M_var_count, popcorn -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(popcorn_struct *popcorn) {
	if (popcorn -> h_lo < M_h_lo) { popcorn -> h_lo = M_h_lo; }
	if (popcorn -> h_lo > M_h_hi) { popcorn -> h_lo = M_h_hi; }
	if (popcorn -> h_hi < M_h_lo) { popcorn -> h_hi = M_h_lo; }
	if (popcorn -> h_hi > M_h_hi) { popcorn -> h_hi = M_h_hi; }
}

static void constrain(popcorn_struct *popcorn, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		popcorn -> h_lo = M_h_lo;
		popcorn -> h_hi = M_h_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double h_spread = ((M_h_hi - M_h_lo) * percent) / 2;
		popcorn -> h_lo = popcorn -> h - h_spread;
		popcorn -> h_hi = popcorn -> h + h_spread;
		limiter(popcorn);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for popcorn constraints, requires 2 values, got %d", argc);
		return;
	}
	popcorn -> h_lo = atom_getfloat(arg++);
	popcorn -> h_hi = atom_getfloat(arg++);
	limiter(popcorn);
}

static void search(popcorn_struct *popcorn, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = popcorn -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_h = popcorn -> h;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], popcorn -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		popcorn -> h = (drand48() * (popcorn -> h_hi - popcorn -> h_lo)) + popcorn -> h_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(popcorn, NULL, argc, vars);
		do { calc(popcorn, popcorn -> vars); } while(jump--);
		popcorn -> lyap_exp = lyapunov((void *) popcorn, (t_gotfn) calc, M_var_count, (double *) popcorn -> vars);
		if (isnan(popcorn -> lyap_exp)) { not_found = 1; }
		if (popcorn -> lyap_exp < popcorn -> lyap_lo || popcorn -> lyap_exp > popcorn -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(popcorn, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) popcorn -> lyap_limit);
		post("Try using wider constraints.");
		popcorn -> h = temp_h;
		outlet_anything(popcorn -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		popcorn -> failure_ratio = (popcorn -> lyap_limit - not_expired) / popcorn -> lyap_limit;
		make_results(popcorn);
		outlet_anything(popcorn -> search_outlet, gensym("search"), M_search_count, popcorn -> search_out);
	}
}

void *popcorn_new(t_symbol *s, int argc, t_atom *argv) {
	popcorn_struct *popcorn = (popcorn_struct *) pd_new(popcorn_class);
	if (popcorn != NULL) {
		outlet_new(&popcorn -> x_obj, &s_float);
		popcorn -> outlets[0] = outlet_new(&popcorn -> x_obj, &s_float);
		popcorn -> search_outlet = outlet_new(&popcorn -> x_obj, &s_list);
		popcorn -> vars_outlet = outlet_new(&popcorn -> x_obj, &s_list);
		popcorn -> params_outlet = outlet_new(&popcorn -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			popcorn -> vars_init[M_x] = popcorn -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			popcorn -> vars_init[M_y] = popcorn -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			popcorn -> h = (double) atom_getfloatarg(2, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for popcorn fractal. Expecting 3 arguments.");
			}
			popcorn -> vars_init[M_x] = 0;
			popcorn -> vars_init[M_y] = 0;
			popcorn -> h = 0.05;
		}
		constrain(popcorn, NULL, 0, NULL);
		lyap(popcorn, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)popcorn;
}

void popcorn_setup(void) {
	popcorn_class = class_new(gensym("popcorn"), (t_newmethod) popcorn_new, 0, sizeof(popcorn_struct), 0, A_GIMME, 0);
	class_addbang(popcorn_class, (t_method) calculate);
	class_addmethod(popcorn_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(popcorn_class, (t_method) show, gensym("show"), 0);
	class_addmethod(popcorn_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(popcorn_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(popcorn_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(popcorn_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(popcorn_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(popcorn_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

