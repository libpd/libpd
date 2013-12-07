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

#include "mtx_spherical_harmonics/legendre_a.h"

static void legendre_first_recurrence (double *sintheta, LegendreWorkSpace *wl) {
   int n,l,l0;
   int nmo0=0;
   int n0=1;
   const int incr=(wl->nmax+1)*(wl->nmax+2)/2;

   // computes the legendre functions P_n^m(costheta) for m=n
   // from P_0^0
   for (n=1; n<=wl->nmax; n++) {
      for (l=0,l0=0; l<wl->l; l++,l0+=incr) {
         wl->p[l0+n0+n] = -(2*n-1) * wl->p[l0+nmo0+n-1] * sintheta[l];
      }
      nmo0=n0;
      n0+=n+1;
   }
}

static void legendre_second_recurrence (double *costheta, LegendreWorkSpace *wl) {
   int m,n,l,l0;
   int nmt0=-1;
   int nmo0=0;
   int n0=1;
   const int incr=(wl->nmax+1)*(wl->nmax+2)/2;

   // computes the Legendre functions P_n^m(costheta) from
   // P_n^m with m=n
   for (n=1; n<=wl->nmax; n++) {
      for (m=0; m<n; m++) {
	 if (m<=n-2) {
	    for (l=0,l0=0; l<wl->l; l++,l0+=incr) {
	       wl->p[l0+n0+m] = (
		     (2*n-1) * costheta[l] * wl->p[l0+nmo0+m]
		     - (n+m-1) * wl->p[l0+nmt0+m]
		     ) / (n-m);
	    }
	 }
	 else {
	    for (l=0,l0=0; l<wl->l; l++,l0+=incr) {
	       wl->p[l0+n0+m] = (
		     (2*n-1) * costheta[l] * wl->p[l0+nmo0+m]
		     ) / (n-m);
	    }
	 }
      }
      nmt0=nmo0;
      nmo0=n0;
      n0+=n+1;
   }
}

LegendreWorkSpace *legendre_a_alloc(const size_t nmax, const size_t l) {
   LegendreWorkSpace *wl;
   // memory allocation
   if ((wl=(LegendreWorkSpace*)calloc(1,sizeof(LegendreWorkSpace)))!=0) {
      wl->l=l;
      wl->nmax=nmax;
      if ((wl->p=(double*)calloc(l*(nmax+1)*(nmax+2)/2,sizeof(double)))==0) {
         free(wl);
         wl = 0;
      }
   }
   return wl;
}

void legendre_a_free(LegendreWorkSpace *wl) {
   if (wl!=0) {
      free(wl->p);
      free(wl);
   }
}

void legendre_a(double *theta, LegendreWorkSpace *wl) {
   int l,l0;
   const int incr=(wl->nmax+1)*(wl->nmax+2)/2;
   double *costheta;
   double *sintheta;
   
   // memory allocation
   if ((wl!=0)&&(theta!=0)) {
      if ((costheta=(double*)calloc(wl->l,sizeof(double)))==0) {
         return;
      }
      if ((sintheta=(double*)calloc(wl->l,sizeof(double)))==0) {
         free(costheta);
         return;
      }
      // constants and initialization
      for (l=0, l0=0; l<wl->l; l++, l0+=incr) {
         costheta[l]=cos(theta[l]);
         sintheta[l]=sin(theta[l]);
         // initial value P_0^0=1
         wl->p[l0]=1;
      }
      // recurrences evaluating all the Legendre functions
      legendre_first_recurrence(sintheta,wl);
      legendre_second_recurrence(costheta,wl);
      free(sintheta);
      free(costheta);
   }
}


