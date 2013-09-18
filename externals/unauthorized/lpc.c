/*
vox - a musical real-time vocoder. version 1.0
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "tables.h"
#include "lpc.h"
#define THRES 0.06
#define Dmin 10.81e-3

/* Levinson Durbin algorithm for computing LPC coefficients using
autocorrelation fonction */
void lev_durb(double *corr,double *lpc_coef)
{
    double k[11],tab[11];
    double err,acc;
    int i,j;
    double *a=tab;
    double *prev_a=lpc_coef;
    double *exch;


    /*init vectors*/
    for (i=0; i<11; i++)
    {
        prev_a[i]=0;
        a[i]=0;
    };
    err=corr[0];
    for(i=1; i<11; i++)
    {
        prev_a[0]=1;
        acc=0;
        for(j=0; j<i; j++)
        {
            acc=acc+prev_a[j]*corr[i-j];
        };
        a[i]=k[i]=-acc/err;
        for(j=1; j<i; j++)
        {
            a[j]=prev_a[j]+k[i]*prev_a[i-j];
        };
        err=(1-k[i]*k[i])*err;
        exch=prev_a;
        prev_a=a;
        a=exch;
    };
}

void comp_lpc(double *buf_x,double *corr,double *lpc_coef,int n)
{
    double buffer[n*3];
    double acc,max=0;
    int i,j;
    /* computes LPC analysis for one subframe */
    /* hamming windowing*/
    acc=0;
    for(i=0; i<2*n; i++)
    {
        acc+=buf_x[i]*buf_x[i];
    };
    if (acc>THRES)
    {
        for(i=0; i<3*n; i++)
        {
            buffer[i]=buf_x[i-n]*HammingWindowTable[i];
        };
        /* autocorrelation computation*/
        for(i=0; i<11; i++)
        {
            acc=0;
            for(j=i; j<n*3; j++)
            {
                acc=acc+buffer[j]*buffer[j-i];
            };
            /* correction with binomial coeffs */
            corr[i]=acc;//*BinomialWindowTable[i];
        };
        corr[0]=corr[0]*(1.0+1.0/1024.0);
        lev_durb(corr,lpc_coef);
    }
    else
    {
        for(i=0; i<11; i++)
        {
            lpc_coef[i]=0;
        };
    }
}



/* LPC to LSP coefficients conversion */

/* evaluate  function C(x) (whose roots are  LSP coeffs)*/

double evalc(double cw,double *fonc)
{
    double b[7];
    double x,res;
    int k;

    x=cw;
    b[5]=1;
    b[6]=0;
    for(k=4; k>0; k--)
    {
        b[k]=2*x*b[k+1]-b[k+2]+fonc[5-k];
    };
    res=x*b[1]-b[2]+fonc[5]/2;
    return(res);
}



/* converts LPC vector into LSP frequency vector */
/* all LSP frenquencies are in [0;PI] but are normalized to be in [0;1] */
void lpc2lsp(double lpc_coef[],double *f1,double *f2,double lsp_coef[])
{

    int i,k=1;
    double *fonc,*prev_f,*f_exch;
    double prev_sign1,sign,prev_sign2;
    double *s, *prev_s,*s_exch;
    double lpc_exp[11];

    /* first computes an additional bandwidth expansion on LPC coeffs*/
    for(i=1; i<11; i++)
    {
        lpc_exp[i]=lpc_coef[i]*BandExpTable[i];
    };
    /* computes the F1 and F2 coeffs*/
    f1[0]=f2[0]=1;
    for(i=0; i<5; i++)
    {
        f1[i+1]=lpc_exp[i+1]+lpc_exp[10-i]-f1[i];
        f2[i+1]=lpc_exp[i+1]-lpc_exp[10-i]+f2[i];
    };

    /*find the roots of C(x) alternatively for F1 and F2*/
    fonc=f1;
    prev_f=f2;
    prev_sign1=evalc(1.0,f1);
    prev_sign2=evalc(1.0,f2);
    s=&prev_sign1;
    prev_s=&prev_sign2;
    for(i=1; i<256; i++)
    {
        sign=evalc(CosineTable[i],fonc);
        if ((sign)*(*s)<0)
        {
            /* interpolate to find root*/
            lsp_coef[k]=((double)i-(*s)/(sign-(*s)))/256.0;
            k++;
            /* chek if all roots are found */
            if (k==11) i=257;
            (*s)=sign;
            /* pointers exchange  */
            s_exch=s;
            s=prev_s;
            prev_s=s_exch;
            f_exch=fonc;
            fonc=prev_f;
            prev_f=f_exch;
        }
        else (*s)=sign;
    }
    /* if here all roots are not found , use  lspDC vector */
    if (k!=11)
    {
        for(i=1; i<11; i++)
        {
            lsp_coef[i]=LspDcTable[i];
        };
    };
}


/* converts lsp frequencies to lpc coeffs */

void lsp2lpc(double *lsp_coef,double *lpc_coef)
{
    int i,j=0,index,ok=1;
    double lspcos[11],delta,tmp,p_avg;
    double F1[12],F2[12]; /* begin at indice two*/

    F1[0]=0;
    F1[1]=1;
    F2[0]=0;
    F2[1]=1;
    /* stability check */
    while(ok && (j<11))
    {
        ok=0;
        for(i=1; i<10; i++)
        {
            if( (lsp_coef[i+1]-lsp_coef[i]) < Dmin)
            {
                ok=1;
                p_avg=(lsp_coef[i]+lsp_coef[i+1])/2.0;
                lsp_coef[i]=p_avg-Dmin/2.0;
                lsp_coef[i+1]=p_avg+Dmin/2.0;
            };
        };
        j++;
    }

    /* first converts lsp frequencies to lsp coefficients */
    for (i=1; i<11; i++)
    {
        /* interpolation */
        tmp=lsp_coef[i]*255.0;
        index=(int)tmp;
        delta=CosineTable[index+1]-CosineTable[index];
        lspcos[i]=CosineTable[index]+delta*(tmp-index);
    };

    for(i=2; i<7; i++)
    {
        F1[i]=-2*lspcos[2*i-3]*F1[i-1]+2*F1[i-2];
        F2[i]=-2*lspcos[2*i-2]*F2[i-1]+2*F2[i-2];
        for(j=i-1; j>1; j--)
        {
            F1[j]=F1[j]-2*lspcos[2*i-3]*F1[j-1]+F1[j-2];
            F2[j]=F2[j]-2*lspcos[2*i-2]*F2[j-1]+F2[j-2];
        };
    };
    for(i=6; i>1; i--)
    {
        F1[i]=F1[i]+F1[i-1];
        F2[i]=F2[i]-F2[i-1];
    };
    for(i=2; i<7; i++)
    {
        lpc_coef[i-1]=(F1[i]+F2[i])*0.5;
        lpc_coef[i+4]=(F1[8-i]-F2[8-i])*0.5;
    };
    lpc_coef[0]=1;
}

