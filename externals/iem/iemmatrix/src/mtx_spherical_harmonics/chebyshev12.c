/* 
 Evaluates all fully normalized circular harmonics 
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

#include "mtx_spherical_harmonics/chebyshev12.h"

Cheby12WorkSpace *chebyshev12_alloc(const size_t nmax, const size_t l) {
   Cheby12WorkSpace *wc;
   // memory allocation
   if ((wc=(Cheby12WorkSpace*)calloc(1,sizeof(Cheby12WorkSpace)))!=0) {
      wc->l=l;
      wc->nmax=nmax;
      if ((wc->t=(double*)calloc(l*(2*nmax+1),sizeof(double)))==0) {
         free(wc);
         return 0;
      }
      return wc;
   }
   return 0;
}

void chebyshev12_free(Cheby12WorkSpace *wc) {
   if (wc!=0) {
      free(wc->t);
      free(wc);
   }
}

void chebyshev12(double *phi, Cheby12WorkSpace *wc) {
   int l,l0,n;
   const int incr=2*wc->nmax+1;
   double *cosphi;
   double *sinphi;
   const double oneoversqrt2pi=1.0/sqrt(2.0*M_PI);
   const double oneoversqrtpi=1.0/sqrt(M_PI);
   // memory allocation
   if ((wc!=0)&&(phi!=0)) {
      if ((cosphi=(double*)calloc(wc->l,sizeof(double)))==0) {
         return;
      }
      if ((sinphi=(double*)calloc(wc->l,sizeof(double)))==0) {
         free(cosphi);
         return;
      }
      // constants and initialization
      for (l=0, l0=wc->nmax; l<wc->l; l++, l0+=incr) {
         cosphi[l]=cos(phi[l]);
         sinphi[l]=sin(phi[l]);
         // initial value T_0=1
         wc->t[l0]=oneoversqrt2pi;
         wc->t[l0+1]=cosphi[l]*oneoversqrtpi;
         wc->t[l0-1]=sinphi[l]*oneoversqrtpi;
      }
      // recurrence for n>1
      for (n=2; n<=wc->nmax; n++) {
         for (l=0, l0=wc->nmax; l<wc->l; l++, l0+=incr) {
            wc->t[l0+n]=cosphi[l]* wc->t[l0+n-1] - sinphi[l]* wc->t[l0-n+1];
            wc->t[l0-n]=sinphi[l]* wc->t[l0+n-1] + cosphi[l]* wc->t[l0-n+1];
         }
      }
      free(cosphi);
      free(sinphi);
   }
}


