/*
 a test file for chaos.h
 */

#ifndef PD_VERSION
#include "m_pd.h"
#endif

#define MAX_VARS 20

/*
 * fractal - pointer to the object under test
 * calc - the iteration function
 * var_count - the number of variables for this fractal class
 * vars - pointer to the fractal variables array
 */
double lyapunov(void *fractal, t_gotfn calc, int var_count, double *vars);

/*
 * CAUTION: ON FRACTALS WITH LOTS OF VARIABLES, THIS WILL TAKE A WHILE.
 * This is an experimental function to test the ability of the function to
 * calculate an accurate exponent by aberating each variable in the fractal.
 * returns 'results' on completion.
 */
double *lyapunov_full(void *fractal, t_gotfn calc, int var_count, double *vars, double *results);
