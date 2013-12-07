/* 
 
 Evaluates all associated legendre functions 
 at the angles theta up to the order nmax
 using the three-term recurrence of the Legendre functions.
 P has dimensions length(theta) x (nmax+1)(nmax+2)

 Implementation by Franz Zotter, Institute of Electronic Music and Acoustics
 (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria
 http://iem.at/Members/zotter, 2008.

 This code is published under the Gnu General Public License, see
 "LICENSE.txt"

*/


#ifndef _legendre_a_h__
#define _legendre_a_h__
#include <math.h>
#include <stdlib.h>

typedef struct _LegendreWorkSpace_
{
   size_t nmax;
   size_t l;
   double *p;
} LegendreWorkSpace;

LegendreWorkSpace *legendre_a_alloc(const size_t nmax, const size_t l);

void legendre_a_free(LegendreWorkSpace *wl);

void legendre_a(double *theta, LegendreWorkSpace *wl);

#endif
