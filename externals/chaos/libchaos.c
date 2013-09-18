#include <stdio.h>
#include <math.h>
#include "chaos.h"

#define LY_ITERATIONS 50000

//#define LY_ABERATION 1e-6
#define LY_ABERATION 10e-15

/**
 * this expects the fractal to already be stable (ie iterate it a few hundred/thousand times).
 
 Steps as described by Julian Sprott
 1. Start with any initial condition in the basin of attraction
 2. Iterate until the orbit is on the attractor
 3. Select (almost any) nearby point (separated by d0)
 4. Advancce both orbits one iteration and calculate the new separation d1
 5. Evaluate log(d1/d0) in any convienient base
 6. Readjust one orbit so its separation is d0 in the same direction as d1
 7. Repeat steps 4-6 many times and calculate the average of step 5
 */
double lyapunov_eval(void *fractal, t_gotfn calc, int var_count, double *vars, double *test) {
	int i, j;
	double exponent = 0.0, sum = 0.0, d2, df, rs;
	double diff[MAX_VARS];
	
	for(i = 0; i < LY_ITERATIONS; i++) {
		// 4. iterate each set
		calc(fractal, vars);
		calc(fractal, test);
		// 5. Evaluate
		d2 = 0.0;
		for(j = 0; j < var_count; j++) {
			diff[j] = test[j] - vars[j];
			//fprintf(stderr, "vars[%d] = %0.30f\n test[%d] = %0.30f\n  diff[%d] = %0.30f\n",
			//	j, vars[j], j, test[j], j, diff[j]);
			d2 += diff[j] * diff[j];
		}
		df = 1000000000000.0 * d2;
		rs = 1.0 / sqrt(df);
		sum += log(df);
		exponent = 0.721347 * sum / i;
		//fprintf(stderr, "%d\n  d2 = %0.30f\n  df = %0.30f\n  rs = %0.30f\n  sum = %0.30f\nexponent = %0.30f\n\n",
		//	i, d2, df, rs, sum, exponent);
		// 6. adjust the orbit under test
		for(j = 0; j < var_count; j++) {
			test[j] = vars[j] + (rs * (test[j] - vars[j]));
		}
	}
	return exponent;
}

double lyapunov(void *fractal, t_gotfn calc, int var_count, double *vars) {
	int i;
	double test[MAX_VARS];
	
	// 1. + 2. 'vars' is expected to be ready to start computing
	// 3. create a test set of variables
	test[0] = vars[0] + LY_ABERATION;
	for(i = 1; i < var_count; i++) { test[i] = vars[i]; }
	
	return lyapunov_eval(fractal, calc, var_count, vars, test);
}

double *lyapunov_full(void *fractal, t_gotfn calc, int var_count, double *vars, double *results) {
	int i, j;
	double initial[MAX_VARS];
	double test[MAX_VARS];
	
	// save the starting values
	for(i = 0; i < var_count; i++) {
		initial[i] = vars[i];
	}
	for(i = 0; i < var_count; i++) {
		// aberate each variable in turn
		for(j = 0; j < var_count; j++) {
			if (j == i) {
				test[j] = vars[j] + LY_ABERATION;
			} else {
				test[j] = vars[j];
			}
		}
		results[i] = lyapunov_eval(fractal, calc, var_count, vars, test);
		// set the vars back the initial values
		for(j = 0; j < var_count; j++) {
			vars[j] = initial[j];
		}
	}
	return results;
}
