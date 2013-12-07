/* mlogistic Attractor PD External */
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

#define M_c_lo 0
#define M_c_hi 4

#define M_c 0

#define M_x 0

#define M_param_count 1
#define M_var_count 1
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "mlogistic v0.0, by Michael McGonagle, from ??????, 2003";

t_class *mlogistic_class;

typedef struct mlogistic_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double c, c_lo, c_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
} mlogistic_struct;

static void calc(mlogistic_struct *mlogistic, double *vars) {
	double x_0;
	x_0 =(vars[M_x]*vars[M_x])+mlogistic -> c;
	vars[M_x] = x_0;
} // end calc

static void calculate(mlogistic_struct *mlogistic) {
	calc(mlogistic, mlogistic -> vars);
	outlet_float(mlogistic -> x_obj.ob_outlet, mlogistic -> vars[M_x]);
} // end calculate

static void reset(mlogistic_struct *mlogistic, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		mlogistic -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
	} else {
		mlogistic -> vars[M_x] = mlogistic -> vars_init[M_x];
	} // end if
} // end reset

static char *classify(mlogistic_struct *mlogistic) {
	static char buff[2];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((mlogistic -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[1] = '\0';
	return buff;
}

static void make_results(mlogistic_struct *mlogistic) {
	SETFLOAT(&mlogistic -> search_out[0], mlogistic -> lyap_exp);
	SETSYMBOL(&mlogistic -> search_out[1], gensym(classify(mlogistic)));
	SETFLOAT(&mlogistic -> search_out[2], mlogistic -> failure_ratio);
	SETFLOAT(&mlogistic -> vars_out[M_x], mlogistic -> vars[M_x]);
	SETFLOAT(&mlogistic -> params_out[M_c], mlogistic -> c);
	outlet_list(mlogistic -> params_outlet, gensym("list"), M_param_count, mlogistic -> params_out);
	outlet_list(mlogistic -> vars_outlet, gensym("list"), M_var_count, mlogistic -> vars_out);
}

static void show(mlogistic_struct *mlogistic) {
	make_results(mlogistic);
	outlet_anything(mlogistic -> search_outlet, gensym("show"), M_search_count, mlogistic -> search_out);
}

static void param(mlogistic_struct *mlogistic, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 1) {
		post("Incorrect number of arguments for mlogistic fractal. Expecting 1 arguments.");
		return;
	}
	mlogistic -> c = (double) atom_getfloatarg(0, argc, argv);
}

static void seed(mlogistic_struct *mlogistic, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(mlogistic_struct *mlogistic, t_floatarg l, t_floatarg h, t_floatarg lim) {
	mlogistic -> lyap_lo = l;
	mlogistic -> lyap_hi = h;
	mlogistic -> lyap_limit = (double) ((int) lim);
}

static void elyap(mlogistic_struct *mlogistic) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) mlogistic, (t_gotfn) calc, M_var_count, mlogistic -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(mlogistic_struct *mlogistic) {
	if (mlogistic -> c_lo < M_c_lo) { mlogistic -> c_lo = M_c_lo; }
	if (mlogistic -> c_lo > M_c_hi) { mlogistic -> c_lo = M_c_hi; }
	if (mlogistic -> c_hi < M_c_lo) { mlogistic -> c_hi = M_c_lo; }
	if (mlogistic -> c_hi > M_c_hi) { mlogistic -> c_hi = M_c_hi; }
}

static void constrain(mlogistic_struct *mlogistic, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		mlogistic -> c_lo = M_c_lo;
		mlogistic -> c_hi = M_c_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		mlogistic -> c_lo = mlogistic -> c - c_spread;
		mlogistic -> c_hi = mlogistic -> c + c_spread;
		limiter(mlogistic);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for mlogistic constraints, requires 2 values, got %d", argc);
		return;
	}
	mlogistic -> c_lo = atom_getfloat(arg++);
	mlogistic -> c_hi = atom_getfloat(arg++);
	limiter(mlogistic);
}

static void search(mlogistic_struct *mlogistic, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = mlogistic -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_c = mlogistic -> c;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], mlogistic -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		mlogistic -> c = (drand48() * (mlogistic -> c_hi - mlogistic -> c_lo)) + mlogistic -> c_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(mlogistic, NULL, argc, vars);
		do { calc(mlogistic, mlogistic -> vars); } while(jump--);
		mlogistic -> lyap_exp = lyapunov((void *) mlogistic, (t_gotfn) calc, M_var_count, (double *) mlogistic -> vars);
		if (isnan(mlogistic -> lyap_exp)) { not_found = 1; }
		if (mlogistic -> lyap_exp < mlogistic -> lyap_lo || mlogistic -> lyap_exp > mlogistic -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(mlogistic, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) mlogistic -> lyap_limit);
		post("Try using wider constraints.");
		mlogistic -> c = temp_c;
		outlet_anything(mlogistic -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		mlogistic -> failure_ratio = (mlogistic -> lyap_limit - not_expired) / mlogistic -> lyap_limit;
		make_results(mlogistic);
		outlet_anything(mlogistic -> search_outlet, gensym("search"), M_search_count, mlogistic -> search_out);
	}
}

void *mlogistic_new(t_symbol *s, int argc, t_atom *argv) {
	mlogistic_struct *mlogistic = (mlogistic_struct *) pd_new(mlogistic_class);
	if (mlogistic != NULL) {
		outlet_new(&mlogistic -> x_obj, &s_float);
		mlogistic -> search_outlet = outlet_new(&mlogistic -> x_obj, &s_list);
		mlogistic -> vars_outlet = outlet_new(&mlogistic -> x_obj, &s_list);
		mlogistic -> params_outlet = outlet_new(&mlogistic -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			mlogistic -> vars_init[M_x] = mlogistic -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			mlogistic -> c = (double) atom_getfloatarg(1, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for mlogistic fractal. Expecting 2 arguments.");
			}
			mlogistic -> vars_init[M_x] = 0.1;
			mlogistic -> c = 4;
		}
		constrain(mlogistic, NULL, 0, NULL);
		lyap(mlogistic, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)mlogistic;
}

void mlogistic_setup(void) {
	mlogistic_class = class_new(gensym("mlogistic"), (t_newmethod) mlogistic_new, 0, sizeof(mlogistic_struct), 0, A_GIMME, 0);
	class_addbang(mlogistic_class, (t_method) calculate);
	class_addmethod(mlogistic_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(mlogistic_class, (t_method) show, gensym("show"), 0);
	class_addmethod(mlogistic_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(mlogistic_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(mlogistic_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(mlogistic_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(mlogistic_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(mlogistic_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

