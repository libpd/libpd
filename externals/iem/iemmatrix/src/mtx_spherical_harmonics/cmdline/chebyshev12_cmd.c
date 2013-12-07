
/* command line test for chebyshev12.c 
 * Franz Zotter, 2009, see ../LICENSE.txt */

#include <stdio.h>

#include "chebyshev12.h"

int main (int argc, char *argv[]) {
   int nmax, l, lc, n, m;
   Cheby12WorkSpace *wc=0;
   double *ptr,*phi;

   if (argc <3) {
      printf("chebyshev12 requires nmax as input argument followed by phi values\n");
      return 0;
   }

   nmax=atoi(argv[1]);
   l=argc-2;
   if ((phi=(double*)calloc(l,sizeof(double)))==0) {
      printf("chebyshev12 could not allocate memory for %d phi-values\n",l);
      return 0;
   }
   if ((wc=chebyshev12_alloc(nmax,l))==0) {
      printf("chebyshev12 could not allocate memory for n=%d\n and l=%d\n",nmax,l);
      return 0;
   }
   for (n=0;n<l;n++) {
      phi[n]=atof(argv[n+2]);
   }
   chebyshev12(phi,wc);

   ptr=wc->t;
   for (lc=0;lc<l;lc++) {
      printf("pt %d:\n",lc);
      for (m=-nmax;m<=nmax;m++) {
         printf("T[%2d](%7.4f)=%7.4f\n",m,phi[lc],*ptr++);
      }
   }
   chebyshev12_free(wc);
   free(phi);
   return 1;
}


