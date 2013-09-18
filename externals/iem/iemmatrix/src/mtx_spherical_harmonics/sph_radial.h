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

#ifndef __SPH_RADIAL_H__
#define __SPH_RADIAL_H__

void sphBessel (double x, double *y, int n);

void sphNeumann (double x, double *y, int n);

void sphBesselDiff (double x, double *y, int n);

void sphNeumannDiff (double x, double *y, int n);

#endif  // __SPH_RADIAL_H__
