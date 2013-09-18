/*
 * Recursive computation of (arbitrary degree) spherical harmonics, 
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

#ifndef _sh_h__
#define _sh_h__

#include <stdlib.h>
#include "legendre_a.h"
#include "chebyshev12.h"
#include "sharmonics_normalization.h"

typedef struct _SHWorkSpace_
{
   size_t nmax;
   size_t l;
   
   double *y;

   SHNorml *wn;
   Cheby12WorkSpace *wc;
   LegendreWorkSpace *wl;

} SHWorkSpace;

SHWorkSpace *sharmonics_alloc(size_t nmax, size_t l);

void sharmonics_free(SHWorkSpace *sh);

void sharmonics(double *phi, double *theta, SHWorkSpace *ws);

#endif

