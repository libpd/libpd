
/* command line test for legendre_a.c 
 * Franz Zotter, 2009, see ../LICENSE.txt */

#include <stdio.h>

#include "legendre_a.h"

int main (int argc, char *argv[]) {
   int nmax, l, lc, n, m;
   LegendreWorkSpace *wl=0;
   double *ptr,*theta;

   if (argc <3) {
      printf("legendre_a requires nmax as input argument followed by theta values\n");
      return 0;
   }

   nmax=atoi(argv[1]);
   l=argc-2;
   if ((theta=(double*)calloc(l,sizeof(double)))==0) {
      printf("legendre_a could not allocate memory for %d theta-values\n",l);
      return 0;
   }
   if ((wl=legendre_a_alloc(nmax,l))==0) {
      printf("legendre_a could not allocate memory for n=%d\n and l=%d\n",nmax,l);
      free(theta);
      return 0;
   }
   for (n=0;n<l;n++) {
      theta[n]=atof(argv[n+2]);
   }
   legendre_a(theta,wl);

   ptr=wl->p;
   for (lc=0;lc<l;lc++) {
      printf("pt %d:\n",lc);
      for (n=0;n<=nmax;n++) {
         for (m=0;m<=n;m++) {
            printf("P[%2d][%2d](%7.4f)=%7.4f\n",n,m,theta[l],*ptr++);
         }
      }
   }
   legendre_a_free(wl);
   free(theta);
   return 1;
}


