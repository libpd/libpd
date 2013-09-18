/*

 These filter coefficients computations are taken from
 http://www.harmony-central.com/Computer/Programming/Audio-EQ-Cookbook.txt  

 written by Robert Bristow-Johnson

*/


#ifndef __GGEE_FILTERS_H__
#define __GGEE_FILTERS_H__



#ifndef M_PI
#define M_PI 3.141593f
#endif


#include <math.h>
#define LN2 0.69314718
#define e_A(g) (pow(10,(g/40.)))
#define e_omega(f,r) (2.0*M_PI*f/r)
#define e_alpha(bw,omega) (sin(omega)*sinh(LN2/2. * bw * omega/sin(omega)))
#define e_beta(a,S) (sqrt((a*a + 1)/(S) - (a-1)*(a-1)))




typedef struct _rbjfilter
{
     t_object x_obj;
     t_float  x_rate;
     t_float  x_freq;
     t_float  x_gain;
     t_float  x_bw;
} t_rbjfilter;


static int check_stability(t_float fb1,
			    t_float fb2, 
			    t_float ff1,
			    t_float ff2,
			    t_float ff3)
{
    float discriminant = fb1 * fb1 + 4 * fb2;

    if (discriminant < 0) /* imaginary roots -- resonant filter */
    {
    	    /* they're conjugates so we just check that the product
    	    is less than one */
    	if (fb2 >= -1.0f) goto stable;
    }
    else    /* real roots */
    {
    	    /* check that the parabola 1 - fb1 x - fb2 x^2 has a
    	    	vertex between -1 and 1, and that it's nonnegative
    	    	at both ends, which implies both roots are in [1-,1]. */
    	if (fb1 <= 2.0f && fb1 >= -2.0f &&
    	    1.0f - fb1 -fb2 >= 0 && 1.0f + fb1 - fb2 >= 0)
    	    	goto stable;
    }
    return 0;
stable:
    return 1;
}






#endif
