/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "clc.h"

/* Problem:  find a function f : p -> q (where p is user's curve control
   parameter, q is log factor) such that the curves will bend in
   a semi-linear way over the p's range of 0..1.  The curve function is
   then g(x, p) = (exp(f(p) * x) - 1) / (exp(f(p)) - 1), where x is
   curve's domain.  If, for example, the points g(0.5, p) are to make
   a semi-linear pattern, then the solution is a function f that minimizes
   the integral of the error function e(p) = sqr(((1-p)/2)-g(.5, p))
   over 0..1.  Until someone does this analytically, we are left with
   a lame formula, which has been tweaked and tested in gnuplot:
   f(p) = h(p) / (1 - h(p)), where h(p) = (((p + 1e-20) * 1.2) ** .41) * .91.
   The file curve.gp, in the sickle's source directory, may come handy,
   in case there is anyone, who fancy tweaking it even further.

   To implement this, start from these equations:
     nhops = npoints - 1
     bb * mm ^ nhops = bb + 1
     (bb ^ 2) * (mm ^ nhops) = ((exp(ff/2) - 1) / (exp(ff) - 1)) ^ 2

   and calculate:
     hh = pow(((p + c1) * c2), c3) * c4
     ff = hh / (1 - hh)
     eff = exp(ff) - 1
     gh = (exp(ff * .5) - 1) / eff
     bb = gh * (gh / (1 - (gh + gh)))
     mm = ((exp(ff * (1/nhops)) - 1) / (eff * bb)) + 1

   The loop is:
     for (vv = bb, i = 0; i <= nhops; vv *= mm, i++)
         result = (vv - bb) * (y1 - y0) + y0
   where y0, y1 are start and destination values

   This formula generates curves with < .000004% deviation from the straight
   line for p = 0 at half-domain, range 1.  There are no nans for -1 <= p <= 1.
*/

#define CLCCURVE_C1   1e-20
#define CLCCURVE_C2   1.2
#define CLCCURVE_C3   0.41
#define CLCCURVE_C4   0.91

void clccurve_coefs(int nhops, double crv, double *bbp, double *mmp)
{
    if (nhops > 0)
    {
	double hh, ff, eff, gh;
	if (crv < 0)
	{
	    if (crv < -1.)
		crv = -1.;
	    hh = pow(((CLCCURVE_C1 - crv) * CLCCURVE_C2), CLCCURVE_C3)
		* CLCCURVE_C4;
	    ff = hh / (1. - hh);
	    eff = exp(ff) - 1.;
	    gh = (exp(ff * .5) - 1.) / eff;
	    *bbp = gh * (gh / (1. - (gh + gh)));
	    *mmp = 1. / (((exp(ff * (1. / (double)nhops)) - 1.) /
			  (eff * *bbp)) + 1.);
	    *bbp += 1.;
	}
	else
	{
	    if (crv > 1.)
		crv = 1.;
	    hh = pow(((crv + CLCCURVE_C1) * CLCCURVE_C2), CLCCURVE_C3)
		* CLCCURVE_C4;
	    ff = hh / (1. - hh);
	    eff = exp(ff) - 1.;
	    gh = (exp(ff * .5) - 1.) / eff;
	    *bbp = gh * (gh / (1. - (gh + gh)));
	    *mmp = ((exp(ff * (1. / (double)nhops)) - 1.) /
		    (eff * *bbp)) + 1.;
	}
    }
    else if (crv < 0)
	*bbp = 2., *mmp = 1.;
    else
	*bbp = *mmp = 1.;
}
