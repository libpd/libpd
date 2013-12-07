/* three_d Attractor PD External */
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

#define M_a_lo -1000
#define M_a_hi 1000
#define M_b_lo -1000
#define M_b_hi 1000
#define M_c_lo -1000
#define M_c_hi 1000
#define M_d_lo -1000
#define M_d_hi 1000
#define M_e_lo -1000
#define M_e_hi 1000

#define M_a 0
#define M_b 1
#define M_c 2
#define M_d 3
#define M_e 4

#define M_x 0
#define M_y 1
#define M_z 2

#define M_param_count 5
#define M_var_count 3
#define M_search_count 3
#define M_failure_limit 1000

static char *version = "three_d v0.0, by Michael McGonagle, from Cliff Pickover, 2003";

t_class *three_d_class;

typedef struct three_d_struct {
	t_object x_obj;

	double vars[M_var_count];
	double vars_init[M_var_count];
	t_atom vars_out[M_var_count];
	t_outlet *vars_outlet;
	
	t_atom search_out[M_search_count];
	t_outlet *search_outlet;
	
	double a, a_lo, a_hi, b, b_lo, b_hi, c, c_lo, c_hi, d, d_lo, d_hi, e, e_lo, e_hi;
	t_atom params_out[M_param_count];
	t_outlet *params_outlet;
	double lyap_exp, lyap_lo, lyap_hi, lyap_limit, failure_ratio;
	
	t_outlet *outlets[M_var_count - 1];
} three_d_struct;

static void calc(three_d_struct *three_d, double *vars) {
	double x_0, y_0, z_0;
	x_0 =sin(three_d -> a*vars[M_y])-vars[M_z]*cos(three_d -> b*vars[M_x]);
	y_0 =vars[M_z]*sin(three_d -> c*vars[M_x])-cos(three_d -> d*vars[M_y]);
	z_0 =three_d -> e*sin(vars[M_x]);
	vars[M_x] = x_0;
	vars[M_y] = y_0;
	vars[M_z] = z_0;
} // end calc

static void calculate(three_d_struct *three_d) {
	calc(three_d, three_d -> vars);
	outlet_float(three_d -> outlets[M_z - 1], three_d -> vars[M_z]);
	outlet_float(three_d -> outlets[M_y - 1], three_d -> vars[M_y]);
	outlet_float(three_d -> x_obj.ob_outlet, three_d -> vars[M_x]);
} // end calculate

static void reset(three_d_struct *three_d, t_symbol *s, int argc, t_atom *argv) {
	if (argc == M_var_count) {
		three_d -> vars[M_x] = (double) atom_getfloatarg(M_x, argc, argv);
		three_d -> vars[M_y] = (double) atom_getfloatarg(M_y, argc, argv);
		three_d -> vars[M_z] = (double) atom_getfloatarg(M_z, argc, argv);
	} else {
		three_d -> vars[M_x] = three_d -> vars_init[M_x];
		three_d -> vars[M_y] = three_d -> vars_init[M_y];
		three_d -> vars[M_z] = three_d -> vars_init[M_z];
	} // end if
} // end reset

static char *classify(three_d_struct *three_d) {
	static char buff[6];
	char *c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	buff[0] = c[(int) (((three_d -> a - M_a_lo) * (1.0 / (M_a_hi - M_a_lo))) * 26)];
	buff[1] = c[(int) (((three_d -> b - M_b_lo) * (1.0 / (M_b_hi - M_b_lo))) * 26)];
	buff[2] = c[(int) (((three_d -> c - M_c_lo) * (1.0 / (M_c_hi - M_c_lo))) * 26)];
	buff[3] = c[(int) (((three_d -> d - M_d_lo) * (1.0 / (M_d_hi - M_d_lo))) * 26)];
	buff[4] = c[(int) (((three_d -> e - M_e_lo) * (1.0 / (M_e_hi - M_e_lo))) * 26)];
	buff[5] = '\0';
	return buff;
}

static void make_results(three_d_struct *three_d) {
	SETFLOAT(&three_d -> search_out[0], three_d -> lyap_exp);
	SETSYMBOL(&three_d -> search_out[1], gensym(classify(three_d)));
	SETFLOAT(&three_d -> search_out[2], three_d -> failure_ratio);
	SETFLOAT(&three_d -> vars_out[M_x], three_d -> vars[M_x]);
	SETFLOAT(&three_d -> vars_out[M_y], three_d -> vars[M_y]);
	SETFLOAT(&three_d -> vars_out[M_z], three_d -> vars[M_z]);
	SETFLOAT(&three_d -> params_out[M_a], three_d -> a);
	SETFLOAT(&three_d -> params_out[M_b], three_d -> b);
	SETFLOAT(&three_d -> params_out[M_c], three_d -> c);
	SETFLOAT(&three_d -> params_out[M_d], three_d -> d);
	SETFLOAT(&three_d -> params_out[M_e], three_d -> e);
	outlet_list(three_d -> params_outlet, gensym("list"), M_param_count, three_d -> params_out);
	outlet_list(three_d -> vars_outlet, gensym("list"), M_var_count, three_d -> vars_out);
}

static void show(three_d_struct *three_d) {
	make_results(three_d);
	outlet_anything(three_d -> search_outlet, gensym("show"), M_search_count, three_d -> search_out);
}

static void param(three_d_struct *three_d, t_symbol *s, int argc, t_atom *argv) {
	if (argc != 5) {
		post("Incorrect number of arguments for three_d fractal. Expecting 5 arguments.");
		return;
	}
	three_d -> a = (double) atom_getfloatarg(0, argc, argv);
	three_d -> b = (double) atom_getfloatarg(1, argc, argv);
	three_d -> c = (double) atom_getfloatarg(2, argc, argv);
	three_d -> d = (double) atom_getfloatarg(3, argc, argv);
	three_d -> e = (double) atom_getfloatarg(4, argc, argv);
}

static void seed(three_d_struct *three_d, t_symbol *s, int argc, t_atom *argv) {
	if (argc > 0) {
		srand48(((unsigned int)time(0))|1);
	} else {
		srand48((unsigned int) atom_getfloatarg(0, argc, argv));
	}
}

static void lyap(three_d_struct *three_d, t_floatarg l, t_floatarg h, t_floatarg lim) {
	three_d -> lyap_lo = l;
	three_d -> lyap_hi = h;
	three_d -> lyap_limit = (double) ((int) lim);
}

static void elyap(three_d_struct *three_d) {
	double results[M_var_count];
	int i;
	if (lyapunov_full((void *) three_d, (t_gotfn) calc, M_var_count, three_d -> vars, results) != NULL) {
		post("elyapunov:");
		for(i = 0; i < M_var_count; i++) { post("%d: %3.80f", i, results[i]); }
	}
}

static void limiter(three_d_struct *three_d) {
	if (three_d -> a_lo < M_a_lo) { three_d -> a_lo = M_a_lo; }
	if (three_d -> a_lo > M_a_hi) { three_d -> a_lo = M_a_hi; }
	if (three_d -> a_hi < M_a_lo) { three_d -> a_hi = M_a_lo; }
	if (three_d -> a_hi > M_a_hi) { three_d -> a_hi = M_a_hi; }
	if (three_d -> b_lo < M_b_lo) { three_d -> b_lo = M_b_lo; }
	if (three_d -> b_lo > M_b_hi) { three_d -> b_lo = M_b_hi; }
	if (three_d -> b_hi < M_b_lo) { three_d -> b_hi = M_b_lo; }
	if (three_d -> b_hi > M_b_hi) { three_d -> b_hi = M_b_hi; }
	if (three_d -> c_lo < M_c_lo) { three_d -> c_lo = M_c_lo; }
	if (three_d -> c_lo > M_c_hi) { three_d -> c_lo = M_c_hi; }
	if (three_d -> c_hi < M_c_lo) { three_d -> c_hi = M_c_lo; }
	if (three_d -> c_hi > M_c_hi) { three_d -> c_hi = M_c_hi; }
	if (three_d -> d_lo < M_d_lo) { three_d -> d_lo = M_d_lo; }
	if (three_d -> d_lo > M_d_hi) { three_d -> d_lo = M_d_hi; }
	if (three_d -> d_hi < M_d_lo) { three_d -> d_hi = M_d_lo; }
	if (three_d -> d_hi > M_d_hi) { three_d -> d_hi = M_d_hi; }
	if (three_d -> e_lo < M_e_lo) { three_d -> e_lo = M_e_lo; }
	if (three_d -> e_lo > M_e_hi) { three_d -> e_lo = M_e_hi; }
	if (three_d -> e_hi < M_e_lo) { three_d -> e_hi = M_e_lo; }
	if (three_d -> e_hi > M_e_hi) { three_d -> e_hi = M_e_hi; }
}

static void constrain(three_d_struct *three_d, t_symbol *s, int argc, t_atom *argv) {
	int i;
	t_atom *arg = argv;
	if (argc == 0) {
		// reset to full limits of search ranges
		three_d -> a_lo = M_a_lo;
		three_d -> a_hi = M_a_hi;
		three_d -> b_lo = M_b_lo;
		three_d -> b_hi = M_b_hi;
		three_d -> c_lo = M_c_lo;
		three_d -> c_hi = M_c_hi;
		three_d -> d_lo = M_d_lo;
		three_d -> d_hi = M_d_hi;
		three_d -> e_lo = M_e_lo;
		three_d -> e_hi = M_e_hi;
		return;
	}
	if (argc == 1) {
		// set the ranges based on percentage of full range
		double percent = atom_getfloat(arg);
		double a_spread = ((M_a_hi - M_a_lo) * percent) / 2;
		double b_spread = ((M_b_hi - M_b_lo) * percent) / 2;
		double c_spread = ((M_c_hi - M_c_lo) * percent) / 2;
		double d_spread = ((M_d_hi - M_d_lo) * percent) / 2;
		double e_spread = ((M_e_hi - M_e_lo) * percent) / 2;
		three_d -> a_lo = three_d -> a - a_spread;
		three_d -> a_hi = three_d -> a + a_spread;
		three_d -> b_lo = three_d -> b - b_spread;
		three_d -> b_hi = three_d -> b + b_spread;
		three_d -> c_lo = three_d -> c - c_spread;
		three_d -> c_hi = three_d -> c + c_spread;
		three_d -> d_lo = three_d -> d - d_spread;
		three_d -> d_hi = three_d -> d + d_spread;
		three_d -> e_lo = three_d -> e - e_spread;
		three_d -> e_hi = three_d -> e + e_spread;
		limiter(three_d);
		return;
	}
	if (argc != M_param_count * 2) {
		post("Invalid number of arguments for three_d constraints, requires 10 values, got %d", argc);
		return;
	}
	three_d -> a_lo = atom_getfloat(arg++);
	three_d -> a_hi = atom_getfloat(arg++);
	three_d -> b_lo = atom_getfloat(arg++);
	three_d -> b_hi = atom_getfloat(arg++);
	three_d -> c_lo = atom_getfloat(arg++);
	three_d -> c_hi = atom_getfloat(arg++);
	three_d -> d_lo = atom_getfloat(arg++);
	three_d -> d_hi = atom_getfloat(arg++);
	three_d -> e_lo = atom_getfloat(arg++);
	three_d -> e_hi = atom_getfloat(arg++);
	limiter(three_d);
}

static void search(three_d_struct *three_d, t_symbol *s, int argc, t_atom *argv) {
	int not_found, not_expired = three_d -> lyap_limit;
	int jump, i, iterations;
	t_atom vars[M_var_count];
	double temp_a = three_d -> a;
	double temp_b = three_d -> b;
	double temp_c = three_d -> c;
	double temp_d = three_d -> d;
	double temp_e = three_d -> e;
	if (argc > 0) {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], atom_getfloatarg(i, argc, argv));
		}
	} else {
		for (i = 0; i < M_var_count; i++) {
			SETFLOAT(&vars[i], three_d -> vars_init[i]);
		}
	}
	do {
		jump = 500;
		not_found = 0;
		iterations = 10000;
		bad_params:
		three_d -> a = (drand48() * (three_d -> a_hi - three_d -> a_lo)) + three_d -> a_lo;
		three_d -> b = (drand48() * (three_d -> b_hi - three_d -> b_lo)) + three_d -> b_lo;
		three_d -> c = (drand48() * (three_d -> c_hi - three_d -> c_lo)) + three_d -> c_lo;
		three_d -> d = (drand48() * (three_d -> d_hi - three_d -> d_lo)) + three_d -> d_lo;
		three_d -> e = (drand48() * (three_d -> e_hi - three_d -> e_lo)) + three_d -> e_lo;
		// put any preliminary checks specific to this fractal to eliminate bad_params

		reset(three_d, NULL, argc, vars);
		do { calc(three_d, three_d -> vars); } while(jump--);
		three_d -> lyap_exp = lyapunov((void *) three_d, (t_gotfn) calc, M_var_count, (double *) three_d -> vars);
		if (isnan(three_d -> lyap_exp)) { not_found = 1; }
		if (three_d -> lyap_exp < three_d -> lyap_lo || three_d -> lyap_exp > three_d -> lyap_hi) { not_found = 1; }
		not_expired--;
	} while(not_found && not_expired);
	reset(three_d, NULL, argc, vars);
	if (!not_expired) {
		post("Could not find a fractal after %d attempts.", (int) three_d -> lyap_limit);
		post("Try using wider constraints.");
		three_d -> a = temp_a;
		three_d -> b = temp_b;
		three_d -> c = temp_c;
		three_d -> d = temp_d;
		three_d -> e = temp_e;
		outlet_anything(three_d -> search_outlet, gensym("invalid"), 0, NULL);
	} else {
		three_d -> failure_ratio = (three_d -> lyap_limit - not_expired) / three_d -> lyap_limit;
		make_results(three_d);
		outlet_anything(three_d -> search_outlet, gensym("search"), M_search_count, three_d -> search_out);
	}
}

void *three_d_new(t_symbol *s, int argc, t_atom *argv) {
	three_d_struct *three_d = (three_d_struct *) pd_new(three_d_class);
	if (three_d != NULL) {
		outlet_new(&three_d -> x_obj, &s_float);
		three_d -> outlets[0] = outlet_new(&three_d -> x_obj, &s_float);
		three_d -> outlets[1] = outlet_new(&three_d -> x_obj, &s_float);
		three_d -> search_outlet = outlet_new(&three_d -> x_obj, &s_list);
		three_d -> vars_outlet = outlet_new(&three_d -> x_obj, &s_list);
		three_d -> params_outlet = outlet_new(&three_d -> x_obj, &s_list);
		if (argc == M_param_count + M_var_count) {
			three_d -> vars_init[M_x] = three_d -> vars[M_x] = (double) atom_getfloatarg(0, argc, argv);
			three_d -> vars_init[M_y] = three_d -> vars[M_y] = (double) atom_getfloatarg(1, argc, argv);
			three_d -> vars_init[M_z] = three_d -> vars[M_z] = (double) atom_getfloatarg(2, argc, argv);
			three_d -> a = (double) atom_getfloatarg(3, argc, argv);
			three_d -> b = (double) atom_getfloatarg(4, argc, argv);
			three_d -> c = (double) atom_getfloatarg(5, argc, argv);
			three_d -> d = (double) atom_getfloatarg(6, argc, argv);
			three_d -> e = (double) atom_getfloatarg(7, argc, argv);
		} else {
			if (argc != 0 && argc != M_param_count + M_var_count) {
				post("Incorrect number of arguments for three_d fractal. Expecting 8 arguments.");
			}
			three_d -> vars_init[M_x] = 0;
			three_d -> vars_init[M_y] = 0;
			three_d -> vars_init[M_z] = 0;
			three_d -> a = 2.24;
			three_d -> b = 0.43;
			three_d -> c = -0.65;
			three_d -> d = -2.43;
			three_d -> e = 1;
		}
		constrain(three_d, NULL, 0, NULL);
		lyap(three_d, -1000000.0, 1000000.0, M_failure_limit);
	}
	return (void *)three_d;
}

void three_d_setup(void) {
	three_d_class = class_new(gensym("three_d"), (t_newmethod) three_d_new, 0, sizeof(three_d_struct), 0, A_GIMME, 0);
	class_addbang(three_d_class, (t_method) calculate);
	class_addmethod(three_d_class, (t_method) reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(three_d_class, (t_method) show, gensym("show"), 0);
	class_addmethod(three_d_class, (t_method) param, gensym("param"), A_GIMME, 0);
	class_addmethod(three_d_class, (t_method) seed, gensym("seed"), A_GIMME, 0);
	class_addmethod(three_d_class, (t_method) lyap, gensym("lyapunov"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(three_d_class, (t_method) elyap, gensym("elyapunov"), 0);
	class_addmethod(three_d_class, (t_method) search, gensym("search"), A_GIMME, 0);
	class_addmethod(three_d_class, (t_method) constrain, gensym("constrain"), A_GIMME, 0);
	
	
}

