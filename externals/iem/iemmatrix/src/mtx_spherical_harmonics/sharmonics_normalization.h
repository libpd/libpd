/*
 * Recursive computation of (arbitrary degree) normalization constants
 * for spherical harmonics, according to Gumerov and Duraiswami,
 * "The Fast Multipole Methods for the Helmholtz Equation in Three Dimensions",
 * Elsevier, 2005.
 *
 * Implementation by Franz Zotter, Institute of Electronic Music and Acoustics
 * (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria
 * http://iem.at/Members/zotter, 2008.
 *
 * This code is published under the Gnu General Public License, see
 * "LICENSE.txt"
 */


#ifndef _sharmonics_normalization_h__
#define _sharmonics_normalization_h__
#include <math.h>
#include <stdlib.h>

typedef struct _SHNorml_
{
   double *n;
   size_t nmax;
} SHNorml;

SHNorml *sharmonics_normalization_new (const size_t nmax);

void sharmonics_normalization_free (SHNorml *wn);

#endif
