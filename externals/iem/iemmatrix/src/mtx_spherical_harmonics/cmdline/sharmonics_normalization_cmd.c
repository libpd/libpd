
/* command line test for sharmonics_normalization.c 
 * Franz Zotter, 2009, see ../LICENSE.txt */

#include <stdio.h>

#include "sharmonics_normalization.h"

int main (int argc, char *argv[]) {
   int nmax, n, m;
   SHNorml *ws=0;
   double *ptr;

   if (argc!=2) {
      printf("sharmonics_normalization requires nmax as input argument\n");
      return 0;
   }
   
   nmax=atoi(argv[1]);

   if ((ws=sharmonics_normalization_new(nmax))==0) {
      printf("sharmonics_normalization could not allocate memory for n=%d",nmax);
      return 0;
   }

   ptr=ws->n;
   for (n=0;n<=nmax;n++) {
      for (m=0;m<=n;m++) {
         printf("N[%2d][%2d]=%7.4f\n",n,m,*ptr++);
      }
   }
   
   sharmonics_normalization_free(ws);
   return 1;
}


