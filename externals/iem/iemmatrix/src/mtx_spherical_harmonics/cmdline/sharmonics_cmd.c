
/* command line test for chebyshev12.c 
 * Franz Zotter, 2009, see ../LICENSE.txt */

#include <stdio.h>

#include "sharmonics.h"

int main (int argc, char *argv[]) {
   int nmax, l, lc, n, m;
   SHWorkSpace *ws=0;
   double *ptr,*phi,*theta;

   if (argc <4) {
      printf("sharmonics requires nmax as input argument followed by phi theta value pairs\n");
      return 0;
   }

   nmax=atoi(argv[1]);
   l=(argc-2)/2;
   if ((phi=(double*)calloc(l,sizeof(double)))==0) {
      printf("sharmonics could not allocate memory for %d phi-values\n",l);
      return 0;
   }
   if ((theta=(double*)calloc(l,sizeof(double)))==0) {
      printf("sharmonics could not allocate memory for %d theta-values\n",l);
      free(phi);
      return 0;
   }
   if ((ws=sharmonics_alloc(nmax,l))==0) {
      printf("sharmonics could not allocate memory for n=%d\n and l=%d\n",nmax,l);
      free(theta);
      free(phi);
      return 0;
   }
   for (n=0;n<l;n++) {
      phi[n]=atof(argv[2*n+2]);
      theta[n]=atof(argv[2*n+3]);
   }
   sharmonics(phi,theta,ws);

   ptr=ws->y;
   for (lc=0;lc<l;lc++) {
      printf("pt %d:\n",lc);
      for (n=0;n<=nmax;n++) {
         for (m=-n;m<=n;m++) {
            printf("Y[%2d][%2d](%7.4f,%7.4f)=%7.4f\n",n,m,phi[lc],theta[lc],*ptr++);
         }
      }
   }
   sharmonics_free(ws);
   free(phi);
   free(theta);
   return 1;
}


