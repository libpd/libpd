/*
 * Recursive computation of (arbitrary degree) spherical harmonics, 
 * according to Gumerov and Duraiswami,
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

#include "mtx_spherical_harmonics/sharmonics.h"

// HELPER ROUTINES

// preliminarily writing normalized Legendre functions into the result
// Y_n^m(theta) = N_n^m * P_n^m(cos(theta))
// ny0 and np0 denote where the position (n,m)=(n,0) is in the arrays
// ly0 and lp0 denote the starting position for one vertex in the arrays
// see below to find out how the data is arranged
static void sharmonics_initlegendrenormlzd(SHWorkSpace *ws) {
   int n,m,ny0,np0;
   int l,ly0,lp0;
   const int pincr=(ws->nmax+1)*(ws->nmax+2)/2;
   const int yincr=(ws->nmax+1)*(ws->nmax+1);

   for (n=0,ny0=0,np0=0; n<=ws->nmax; n++) {
      for (m=0; m<=n; m++) {
	 ly0=0;
	 lp0=0;
	 for (l=0; l<ws->l; l++) {
	    ws->y[ly0+ny0+m] = ws->wn->n[np0+m] * ws->wl->p[lp0+np0+m];
	    ws->y[ly0+ny0-m] = ws->y[ly0+ny0+m];
            ly0+=yincr;
	    lp0+=pincr;
	 }
      }
      ny0+=2*n+2;
      np0+=n+1;
   }
}

// multiplying normalized Chebyshev sin/cos to the preliminary result
// Y_n^m(phi,theta) = Y_n^m(theta) * T_m(phi)
// ny0 and nt0 denote where the position (n,m)=(n,0) or m=0 is in the arrays
// ly0 and lt0 denote the starting position for one vertex in the arrays
// see below to find out how the data is arranged
static void sharmonics_multcheby12(SHWorkSpace *ws) {
   int n,m,ny0;
   const int nt0=ws->nmax;
   int l,ly0,lt0;
   const int tincr=2*ws->nmax+1;
   const int yincr=(ws->nmax+1)*(ws->nmax+1);

   for (n=0,ny0=0; n<=ws->nmax; n++) {
      m=0;
      ly0=0;
      lt0=nt0;
      for (l=0; l<ws->l; l++) {
         ws->y[ly0+ny0+m]*= ws->wc->t[lt0+m];
         ly0+=yincr;
         lt0+=tincr;
      }
      for (m=1; m<=n; m++) {
	 ly0=0;
	 lt0=nt0;
	 for (l=0; l<ws->l; l++) {
	    ws->y[ly0+ny0-m]*= -ws->wc->t[lt0-m];
	    ws->y[ly0+ny0+m]*= ws->wc->t[lt0+m];
            ly0+=yincr;
	    lt0+=tincr;
	 }
      }
      ny0+=2*n+2;
   }
}


/* MAIN PROGRAM. IMPORTANT EXPRESSIONS
 
   p... vector containing Legendre functions evaluated at the vector z=cos(theta)
        structure [P_0^0(z1) P_1^0(z1) P_1^1(z1) P_2^0(z1) .... Pnmax^nmax(z1)
                   P_0^0(z2) P_1^0(z1) P_1^1(z2) P_2^0(z2) .... Pnmax^nmax(z2)
		   ...
                   P_0^0(zL) P_1^0(zL) P_1^1(zL) P_2^0(zL) .... Pnmax^nmax(zL)]
        with length L X (nmax+1)*(nmax+2)/2

   t... vector containing Chebyshev polynomials sin/cos evaluated at the vector phi
        structure [T_-nmax(phi1) ... T_-1(phi1) T_0(phi1) T_1(phi1) ... T_nmax(phi1)
                   T_-nmax(phi2) ... T_-1(phi2) T_0(phi2) T_1(phi2) ... T_nmax(phi2)
		   ...
                   T_-nmax(phiL) ... T_-1(phiL) T_0(phiL) T_1(phiL) ... T_nmax(phiL)]
        with length L X (2*nmax+1); negative indices are sine, positive ones 
	cosine terms

   norml ... vector containing normalization terms
        structure [N_0^0 N_1^0 N_1^1 N_2^0 N_2^1 N_2^2 .... N_nmax^nmax]
	with length (nmax+1)*(nmax+2)/2


   y ... THE RESULT: containing the spherical harmonics, with negative m for sine
        positive m for cosine terms; p=(phi,theta)
        structure [Y_0^0(p1) Y_1^-1(p1) Y_1^0(p1) Y_1^1 ... Y_nmax^nmax(p1)
                   Y_0^0(p2) Y_1^-1(p2) Y_1^0(p2) Y_1^1 ... Y_nmax^nmax(p2)
		   ...
                   Y_0^0(pL) Y_1^-1(pL) Y_1^0(pL) Y_1^1 ... Y_nmax^nmax(pL)]
        with length L X (nmax+1)^2

*/

SHWorkSpace *sharmonics_alloc(size_t nmax, size_t l) {
   SHWorkSpace *ws=0;

   if ((ws=(SHWorkSpace*)calloc(1,sizeof(SHWorkSpace)))!=0) {
      ws->y=(double*)calloc(l*(nmax+1)*(nmax+1),sizeof(double));

      ws->wl=(LegendreWorkSpace*)legendre_a_alloc(nmax,l);
      ws->wc=(Cheby12WorkSpace*)chebyshev12_alloc(nmax,l);
      ws->wn=(SHNorml*)sharmonics_normalization_new(nmax);

      if ((ws->y==0)||(ws->wl==0)||(ws->wc==0)||(ws->wn==0)) {
         sharmonics_free(ws); 
         ws=0;
      }
      else {
         ws->l=l;
         ws->nmax=nmax;
      }
        
   }
   return ws;
}

void sharmonics_free(SHWorkSpace *ws) {
   if (ws!=0) {
      legendre_a_free(ws->wl);
      chebyshev12_free(ws->wc);
      sharmonics_normalization_free(ws->wn);
      free(ws);
   }
}

void sharmonics(double *phi, double *theta, SHWorkSpace *ws) {

   if ((ws!=0)&&(theta!=0)&&(phi!=0)) {
      chebyshev12(phi,ws->wc);
      legendre_a(theta,ws->wl);

      sharmonics_initlegendrenormlzd(ws);
      sharmonics_multcheby12(ws);
   }
}


