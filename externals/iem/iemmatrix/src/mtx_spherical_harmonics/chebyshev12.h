/* 
 Evaluates all circular harmonics 
 at the angles phi up to the order nmax. 
 using the recurrence for the Chebyshev
 polynomials of the first and second kind
 T has the dimensions length(phi) x 2nmax+1

 Implementation by Franz Zotter, Institute of Electronic Music and Acoustics
 (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria
 http://iem.at/Members/zotter, 2008.

 This code is published under the Gnu General Public License, see
 "LICENSE.txt"
*/

#ifndef _chebyshev12_h__
#define _chebyshev12_h__
#include <math.h>
#include <stdlib.h>

typedef struct _Cheby12WorkSpace_ 
{
   size_t nmax;
   size_t l;
   double *t;
} Cheby12WorkSpace;

Cheby12WorkSpace *chebyshev12_alloc(const size_t nmax, const size_t l);

void chebyshev12_free(Cheby12WorkSpace *wc);

void chebyshev12(double *phi, Cheby12WorkSpace *wc);

#endif


