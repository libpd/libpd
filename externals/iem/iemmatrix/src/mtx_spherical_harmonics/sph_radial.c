/*
 * Recursive computation of (arbitrary degree) spherical Bessel/Neumann/Hankel functions, 
 * according to Gumerov and Duraiswami,
 * "The Fast Multipole Methods for the Helmholtz Equation in Three Dimensions",
 * Elsevier, 2005.
 *
 * Implementation by Franz Zotter, Institute of Electronic Music and Acoustics
 * (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria
 * http://iem.at/Members/zotter, 2007.
 *
 * This code is published under the Gnu General Public License, see
 * "LICENSE.txt"
 */

#include <math.h>

#include "sph_radial.h"

#define EPS 1e-10

static void radialRecurrence (double x, double *y, int n);

// the two recurrences for higher n:
// by now no numeric stabilization for the bessel function is performed

static void radialRecurrence (double x, double *y, int n) {
   int k;
   for (k=1;k<n;k++) {
      y[k+1] = -y[k-1] + y[k]/x * (2*k+1);
   }
}

static void radialDiffRecurrence (double x, double *y1, double *yd, int n) {
   int k;
   for (k=0;k<n;k++) {
      yd[k] = y1[k]/x * n - y1[k+1];
   }
}

void sphBessel (double x, double *y, int n) { //TODO: small values!
   if (y==0) 
      return;
   if (n>=0) 
      y[0] = (x<EPS)?1.0:sin(x)/x;
   if (n>=1) 
      y[1] = -cos(x)/x + y[0]/x;
   radialRecurrence (x,y,n);
}

void sphNeumann (double x, double *y, int n) {
   if (y==0) 
      return;
   if (n>=0) 
      y[0] = -cos(x)/x;
   if (n>=1) 
      y[1] = ((x<EPS)?1.0:sin(x)/x) - y[0]/x;
   radialRecurrence (x,y,n);
}

void sphBesselDiff (double x, double *y, int n) {
   double *y1;
   if (n<0)
      return;
   if ((y1 = (double*)calloc(n+2,sizeof(double)))==0)
      return;
   sphBessel (x,y1,n+1);
   radialDiffRecurrence (x,y1,y,n);
   free(y1);
}

void sphNeumannDiff (double x, double *y, int n) {
   double *y1;
   if (n<0)
      return;
   if ((y1 = (double*)calloc(n+2,sizeof(double)))==0)
      return;
   sphNeumann (x,y,n+1);
   radialDiffRecurrence (x,y1,y,n);
   free(y1);
}
