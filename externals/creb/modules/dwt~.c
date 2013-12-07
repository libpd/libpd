/*
 *   dwt.c  - code for discrete wavelet transform 
 *   (symmetric interpolating biorthogonal wavelets using the lifting transform) 
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXORDER 64

typedef enum{DWT,IDWT,DWT16,IDWT16} t_dwttype;

typedef struct dwtctl
{
  t_float c_update[MAXORDER];
  t_float c_predict[MAXORDER];
  t_int c_nupdate;
  t_int c_npredict;
  t_int c_levels;
  t_int c_fakein;
  t_float c_fakeval;
  t_int c_mask;
  char c_name[16];
  t_int *c_clutter;
  t_int *c_unclutter;
  t_int c_permute;
  t_dwttype c_type;
} t_dwtctl;

typedef struct dwt
{
  t_object x_obj;
  t_float x_f;
  t_dwtctl x_ctl;
} t_dwt;


static void dwt_even(t_dwt *x, t_floatarg f)
{
  int k = (int)f;
  int i, j;
  t_float *p = x->x_ctl.c_predict;
  t_float *u = x->x_ctl.c_update;
  t_float  l, xi, xj;

  if ((k>0) && (k<MAXORDER/2))
    {
      for (i=0; i<k; i++){
	l = 1;
	xi = 1+2*i;
	for (j=0; j<k; j++)
	  {
	    xj = 1+2*j;
	    if (i != j)  l /= (1 - ((xi*xi) / (xj*xj)));
	  }
	l *= .5;

	p[k-i-1] = l;
	p[k+i] = l;
	u[k-i-1] = l/2;
	u[k+i] = l/2;
      }

      x->x_ctl.c_npredict = 2*k;
      x->x_ctl.c_nupdate  = 2*k;
    }
}

static void dwt_wavelet(t_dwt *x, t_floatarg f)
{
  int k = (int)f;
  t_float *p = x->x_ctl.c_predict;
  t_float *u = x->x_ctl.c_update;
  t_int *np = &x->x_ctl.c_npredict;
  t_int *nu = &x->x_ctl.c_nupdate;
  
  switch(k)
    {
    default:
    case 1: /* haar */
      *np = *nu = 2; /* actual order is one */
      p[0] = 1;
      p[1] = 0;
      u[0] = 0;
      u[1] = .5;
      break;

    case 2: /* hat */
    case 3:
      *np = *nu = 2;
      p[0] = .5;
      p[1] = .5;
      u[0] = .25;
      u[1] = .25;
      break;

    case 4: /* N = 4, N~ = 4 */
    case 5:
      *np = *nu = 4;
      p[0] = -0.0625;
      p[1] =  0.5625;
      p[2] =  0.5625;
      p[3] = -0.0625;
      u[0] = -0.03125;
      u[1] =  0.28125;
      u[2] =  0.28125;
      u[3] = -0.03125;
      break;

    case 6:
    case 7:
      *np = *nu = 6;
      p[0] =    0.01171875000000;
      p[1] =   -0.09765625000000;
      p[2] =    0.58593750000000;
      p[3] =    0.58593750000000;
      p[4] =   -0.09765625000000;
      p[5] =    0.01171875000000;
      u[0] =    0.00585937500000;
      u[1] =   -0.04882812500000;
      u[2] =    0.29296875000000;
      u[3] =    0.29296875000000;
      u[4] =   -0.04882812500000;
      u[5] =    0.00585937500000;
      break;
    }
}

static inline void dwt_perform_permutation(t_float *S, int n, t_int *f)
{
  t_int k,l;
  t_float swap;
  for(k=0; k<n; k++)
    {
      l = f[k];
      while (l<k) l = f[l];
      swap = S[k];
      S[k] = S[l];
      S[l] = swap;
    }
}

static void dwt_permutation(t_dwt *x, t_int n){

  t_dwtctl *ctl = &x->x_ctl;
  t_int k, L=0, l, start, power;
  t_int nsave = n;

  while(nsave>>=1) L++; 

  if (ctl->c_clutter)   free(ctl->c_clutter);
  if (ctl->c_unclutter) free(ctl->c_unclutter);
  
  ctl->c_clutter = (t_int *)malloc(n*sizeof(t_int));
  ctl->c_unclutter = (t_int *)malloc(n*sizeof(t_int));


  for(l = L, start = n/2, power=1; l>0; l--, start /=2, power *=2)
    {
      for(k=0; k<start; k++)
	{
	  ctl->c_unclutter[start+k] = (1 + 2*k) * power;
	}
    }
  ctl->c_unclutter[0] = 0;

  for(k=0; k<n; k++)
    ctl->c_clutter[ctl->c_unclutter[k]] = k;

  return;

  /* debug */
  for(k=0; k<n; k++)
    printf("clutter[%d] = %d\n", (int)k, (int)ctl->c_clutter[k]);
  for(k=0; k<n; k++)
    printf("unclutter[%d] = %d\n", (int)k, (int)ctl->c_unclutter[k]);

  exit(1);
}



static void idwt_coef(t_dwt *x, t_floatarg index, t_floatarg value)
{
  x->x_ctl.c_fakein = (int)index;
  x->x_ctl.c_fakeval = value;
 
}

static void dwt_print(t_dwt *x)
{
  int i;

  printf("%s: predict: [ ", x->x_ctl.c_name);
  for (i=0; i<x->x_ctl.c_npredict; i++) printf("%f ", x->x_ctl.c_predict[i]);
  printf("], ");

  printf("update: [ ");
  for (i=0; i<x->x_ctl.c_nupdate; i++) printf("%f ", x->x_ctl.c_update[i]);
  printf("]\n");


  
}


static void dwt_filter(t_dwt *x,  t_symbol *s, int argc, t_atom *argv)
{
  int invalid_argument = 0;
  int i;
  
  char *name = x->x_ctl.c_name;

  t_float *pfilter = x->x_ctl.c_predict; 
  t_float *ufilter = x->x_ctl.c_update; 
  t_float *mask = NULL;

  t_int *length = NULL;
  t_float sum = 0;

  if (s == gensym("predict"))
    {
      mask = pfilter;
      length = &(x->x_ctl.c_npredict);
    }
  else if (s == gensym("update"))
    {
      mask = ufilter;
      length = &(x->x_ctl.c_nupdate);
    }
  else if (s == gensym("mask"))
    {
      mask = NULL;
    }
  else
    {
      return;
    }

  if (argc >= MAXORDER) post("%s: error, maximum order exceeded.",name);
  else if ((x->x_ctl.c_type == DWT16 || x->x_ctl.c_type == IDWT16 ) && (argc != 16))
    post("%s: error, need to have 16 coefficients.",name);
  else if (argc == 0) post("%s: no arguments given.",name);
  else if (argc & 1) post("%s: error, only an even number of coefficients is allowed.", name);
  else
    {
      for (i=0; i<argc; i++){
	if (argv[i].a_type != A_FLOAT )
	  {
	    invalid_argument = 1;
	    break;
	  }
      }
      
      if (invalid_argument) post("%s: invalid argument, must be a number.", name);
      else
	{
	  if (mask) /* one of update / predict */
	    {
	      for (i=0; i<argc; i++) mask[i] = argv[i].a_w.w_float;
	      *length = argc;
	    }
	  else /* both + normalization */
	    {
	      for (i=0; i<argc; i++) sum +=  argv[i].a_w.w_float;
	      for (i=0; i<argc; i++)
		{
		  pfilter[i] =  argv[i].a_w.w_float / sum;
		  ufilter[i] =  argv[i].a_w.w_float / (sum*2);
		}
	      x->x_ctl.c_npredict = argc;
	      x->x_ctl.c_nupdate = argc;
	    }
	}

    }

}



static inline void dwtloop(t_float *vector, 
			   int source,
			   int dest,
		     int increment,
		     int backup,
		     int numcoef, 
		     int mask,
		     t_float *filter,
		     int filtlength,
		     t_float sign)
{

  int k,m;
  t_float acc;

  for (k = 0; k < numcoef; k++)
    {
      acc = 0;
      for (m = 0; m < filtlength; m++)
	{

	  acc += filter[m] * vector[source];
	  source += increment;
	  source &= mask;
	}
      vector[dest] += sign * acc;
      dest += increment;
      source -= backup;
      source &= mask;
    }

}

static inline void dwtloop16(t_float *vector, 
		     int source,
		     int dest,
		     int increment,
		     int backup,
		     int numcoef, 
		     int mask,
		     t_float *filter,
		     int filtlength, /* ignored, set to 16 */
		     t_float sign)
{

  int k,m;
  t_float acc;

  for (k = 0; k < numcoef; k++)
    {
      acc = 0;

      acc += filter[0] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[1] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[2] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[3] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[4] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[5] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[6] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[7] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[8] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[9] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[10] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[11] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[12] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[13] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[14] * vector[source];
      source += increment;
      source &= mask;
      
      acc += filter[15] * vector[source];
      source += increment;
      source &= mask;
      
      vector[dest] += sign * acc;
      dest += increment;
      source -= backup;
      source &= mask;
    }

}





static t_int *dwt_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_dwtctl *ctl  = (t_dwtctl *)(w[1]);


  t_int n = (t_int)(w[2]);

  int i;

  int numcoef = n/2;
  /*  int source_u = ((1 - ctl->c_nupdate)/2 - 1);
   *  int source_p = ((1 - ctl->c_npredict)/2);
   */
  int source_u = ((2 - ctl->c_nupdate) - 1);
  int source_p = ((2 - ctl->c_npredict));
  int increment = 2;
  int dest = 1;
  int backup_u = (ctl->c_nupdate-1)*2;
  int backup_p = (ctl->c_npredict-1)*2;

  /* copy input to output */
  if (in != out)
    for (i=0; i<n; i++) out[i]=in[i];


  /* fake input */
  /*    for (i=0; i<n; i++) out[i]=0; out[n/8]=1;*/



  /* backward transform */


  /* iterate over all levels */
  for (i=0; i < ctl->c_levels; i++){


    /* foreward predict */
    dwtloop(out, (source_p & (n-1)), dest, increment, backup_p, numcoef, n-1, ctl->c_predict, ctl->c_npredict, -1);


    /* foreward update */
    dwtloop(out, (source_u & (n-1)), 0,    increment, backup_u, numcoef, n-1, ctl->c_update,  ctl->c_nupdate,  +1);


    /* update control parameters */
    numcoef /= 2;
    source_p *= 2;
    source_u *= 2;
    backup_p *= 2;
    backup_u *= 2;
    increment *= 2;
    dest *= 2;
  }
  
  if (ctl->c_permute) 
    dwt_perform_permutation(out, n, ctl->c_unclutter);


  return (w+5);
}




static t_int *idwt_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_dwtctl *ctl  = (t_dwtctl *)(w[1]);


  t_int n = (t_int)(w[2]);

  int i;

  int numcoef = 1;
  int source_u = ((2 - ctl->c_nupdate) - 1) * (n/2);
  int source_p = ((2 - ctl->c_npredict)) * (n/2);
  int increment = n;
  int dest = n/2;
  int backup_u = (ctl->c_nupdate-1)*n;
  int backup_p = (ctl->c_npredict-1)*n;
  int fake_in = ctl->c_fakein;
  t_float fake_val = ctl->c_fakeval;

  /* copy input to output */
  if (in != out)
    for (i=0; i<n; i++) out[i]=in[i];


  /* fake input */
  
  if ((fake_in >= 0) && (fake_in<n)){
    for (i=0; i<n; i++) out[i]=0; 
    out[fake_in]=fake_val;
  }


  if (ctl->c_permute) 
    dwt_perform_permutation(out, n, ctl->c_clutter);


  /* backward transform */


  /* iterate over all levels */
  for (i=0; i < ctl->c_levels; i++){

    /* backward update */
    dwtloop(out, (source_u & (n-1)), 0,    increment, backup_u, numcoef, n-1, ctl->c_update,  ctl->c_nupdate,  -1);


    /* backward predict */
    dwtloop(out, (source_p & (n-1)), dest, increment, backup_p, numcoef, n-1, ctl->c_predict, ctl->c_npredict, +1);

    /* update control parameters */
    numcoef *= 2;
    source_p /= 2;
    source_u /= 2;
    backup_p /= 2;
    backup_u /= 2;
    increment /= 2;
    dest /= 2;
  }
  


  return (w+5);
}

static t_int *dwt16_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_dwtctl *ctl  = (t_dwtctl *)(w[1]);


  t_int n = (t_int)(w[2]);

  int i;

  int numcoef = n/2;
  /*  int source_u = ((1 - ctl->c_nupdate)/2 - 1);
   *  int source_p = ((1 - ctl->c_npredict)/2);
   */
  int source_u = ((2 - ctl->c_nupdate) - 1);
  int source_p = ((2 - ctl->c_npredict));
  int increment = 2;
  int dest = 1;
  int backup_u = (ctl->c_nupdate-1)*2;
  int backup_p = (ctl->c_npredict-1)*2;

  /* copy input to output */
  if (in != out)
    for (i=0; i<n; i++) out[i]=in[i];


  /* fake input */
  /*    for (i=0; i<n; i++) out[i]=0; out[n/8]=1;*/



  /* backward transform */


  /* iterate over all levels */
  for (i=0; i < ctl->c_levels; i++){


    /* foreward predict */
    dwtloop16(out, (source_p & (n-1)), dest, increment, backup_p, numcoef, n-1, ctl->c_predict, 16, -1);


    /* foreward update */
    dwtloop16(out, (source_u & (n-1)), 0,    increment, backup_u, numcoef, n-1, ctl->c_update,  16,  +1);


    /* update control parameters */
    numcoef /= 2;
    source_p *= 2;
    source_u *= 2;
    backup_p *= 2;
    backup_u *= 2;
    increment *= 2;
    dest *= 2;
  }
  
  if (ctl->c_permute) 
    dwt_perform_permutation(out, n, ctl->c_unclutter);


  return (w+5);
}




static t_int *idwt16_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_dwtctl *ctl  = (t_dwtctl *)(w[1]);


  t_int n = (t_int)(w[2]);

  int i;

  int numcoef = 1;
  int source_u = ((2 - ctl->c_nupdate) - 1) * (n/2);
  int source_p = ((2 - ctl->c_npredict)) * (n/2);
  int increment = n;
  int dest = n/2;
  int backup_u = (ctl->c_nupdate-1)*n;
  int backup_p = (ctl->c_npredict-1)*n;
  int fake_in = ctl->c_fakein;
  t_float fake_val = ctl->c_fakeval;

  /* copy input to output */
  if (in != out)
    for (i=0; i<n; i++) out[i]=in[i];


  /* fake input */
  
  if ((fake_in >= 0) && (fake_in<n)){
    for (i=0; i<n; i++) out[i]=0; 
    out[fake_in]=fake_val;
  }


  if (ctl->c_permute) 
    dwt_perform_permutation(out, n, ctl->c_clutter);


  /* backward transform */


  /* iterate over all levels */
  for (i=0; i < ctl->c_levels; i++){

    /* backward update */
    dwtloop16(out, (source_u & (n-1)), 0,    increment, backup_u, numcoef, n-1, ctl->c_update,  16,  -1);


    /* backward predict */
    dwtloop16(out, (source_p & (n-1)), dest, increment, backup_p, numcoef, n-1, ctl->c_predict, 16, +1);

    /* update control parameters */
    numcoef *= 2;
    source_p /= 2;
    source_u /= 2;
    backup_p /= 2;
    backup_u /= 2;
    increment /= 2;
    dest /= 2;
  }
  


  return (w+5);
}



static void dwt_dsp(t_dwt *x, t_signal **sp)
{

  int n = sp[0]->s_n;
  int ln = 0;

  dwt_permutation(x, n);

  x->x_ctl.c_mask = n-1;
  while (n >>= 1) ln++;     
  x->x_ctl.c_levels = ln;

  switch(x->x_ctl.c_type){
  case DWT:
    dsp_add(dwt_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    break;
  case IDWT:
    dsp_add(idwt_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    break;
  case DWT16:
    dsp_add(dwt16_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    break;
  case IDWT16:
    dsp_add(idwt16_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    break;


  }
}

                                  
void dwt_free(t_dwt *x)
{

  if (x->x_ctl.c_clutter)   free(x->x_ctl.c_clutter);
  if (x->x_ctl.c_unclutter) free(x->x_ctl.c_unclutter);


}

t_class *dwt_class, *idwt_class, *dwt16_class, *idwt16_class;


static void dwt_reset(t_dwt *x)
{
  bzero(x->x_ctl.c_update, 16*sizeof(t_float));
  bzero(x->x_ctl.c_predict, 16*sizeof(t_float));

    x->x_ctl.c_update[7] = .25;
    x->x_ctl.c_update[8] = .25;
    x->x_ctl.c_nupdate = 16;

    x->x_ctl.c_predict[7] = .5;
    x->x_ctl.c_predict[8] = .5;
    x->x_ctl.c_npredict = 16;
    
    x->x_ctl.c_fakein = -1;
    x->x_ctl.c_fakeval = 0;

}


static void *dwt_new_common(t_floatarg permute)
{
    t_dwt *x = (t_dwt *)pd_new(dwt_class);
    int i;

    outlet_new(&x->x_obj, gensym("signal")); 

    /* init data */
    dwt_reset(x);

    x->x_ctl.c_clutter = NULL;
    x->x_ctl.c_unclutter = NULL;
    x->x_ctl.c_permute = (t_int) permute;

    return (void *)x;


}

static void *dwt_new(t_floatarg permute)
{
    t_dwt *x = dwt_new_common(permute);
    sprintf(x->x_ctl.c_name,"dwt");
    x->x_ctl.c_type = DWT;
    return (void *)x;
}


static void *idwt_new(t_floatarg permute)
{
    t_dwt *x = dwt_new_common(permute);
    sprintf(x->x_ctl.c_name,"idwt");
    x->x_ctl.c_type = IDWT;
    return (void *)x;
}

static void *dwt16_new(t_floatarg permute)
{
    t_dwt *x = dwt_new_common(permute);
    sprintf(x->x_ctl.c_name,"dwt16");
    x->x_ctl.c_type = DWT16;
    return (void *)x;
}


static void *idwt16_new(t_floatarg permute)
{
    t_dwt *x = dwt_new_common(permute);
    sprintf(x->x_ctl.c_name,"idwt16");
    x->x_ctl.c_type = IDWT16;
    return (void *)x;
}


void dwt_tilde_setup(void)
{
  //post("dwt~ v0.1");


    dwt_class = class_new(gensym("dwt~"), (t_newmethod)dwt_new,
    	(t_method)dwt_free, sizeof(t_dwt), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(dwt_class, t_dwt, x_f);
    class_addmethod(dwt_class, (t_method)dwt_print, gensym("print"), 0);
    class_addmethod(dwt_class, (t_method)dwt_reset, gensym("reset"), 0);
    class_addmethod(dwt_class, (t_method)dwt_dsp, gensym("dsp"), 0); 

    class_addmethod(dwt_class, (t_method)dwt_filter, gensym("predict"), A_GIMME, 0); 
    class_addmethod(dwt_class, (t_method)dwt_filter, gensym("update"), A_GIMME, 0); 
    class_addmethod(dwt_class, (t_method)dwt_filter, gensym("mask"), A_GIMME, 0);

    class_addmethod(dwt_class, (t_method)dwt_even, gensym("even"), A_DEFFLOAT, 0);
    class_addmethod(dwt_class, (t_method)idwt_coef, gensym("coef"), A_DEFFLOAT, A_DEFFLOAT, 0); 

    

    /*class_addmethod(dwt_class, (t_method)dwt_wavelet, gensym("wavelet"), A_DEFFLOAT, 0); */


    idwt_class = class_new(gensym("idwt~"), (t_newmethod)idwt_new,
    	(t_method)dwt_free, sizeof(t_dwt), 0, A_DEFFLOAT, 0);

    CLASS_MAINSIGNALIN(idwt_class, t_dwt, x_f);
    class_addmethod(idwt_class, (t_method)dwt_print, gensym("print"), 0);
    class_addmethod(idwt_class, (t_method)dwt_dsp, gensym("dsp"), 0); 

    class_addmethod(idwt_class, (t_method)dwt_filter, gensym("predict"), A_GIMME, 0); 
    class_addmethod(idwt_class, (t_method)dwt_filter, gensym("update"), A_GIMME, 0); 
    class_addmethod(idwt_class, (t_method)dwt_filter, gensym("mask"), A_GIMME, 0); 

    class_addmethod(idwt_class, (t_method)idwt_coef, gensym("coef"), A_DEFFLOAT, A_DEFFLOAT, 0); 

    class_addmethod(idwt_class, (t_method)dwt_even, gensym("even"), A_DEFFLOAT, 0);



    dwt16_class = class_new(gensym("dwt16~"), (t_newmethod)dwt16_new,
    	(t_method)dwt_free, sizeof(t_dwt), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(dwt16_class, t_dwt, x_f);
    class_addmethod(dwt16_class, (t_method)dwt_print, gensym("print"), 0);
    class_addmethod(dwt16_class, (t_method)dwt_reset, gensym("reset"), 0);
    class_addmethod(dwt16_class, (t_method)dwt_dsp, gensym("dsp"), 0); 

    class_addmethod(dwt16_class, (t_method)dwt_filter, gensym("predict"), A_GIMME, 0); 
    class_addmethod(dwt16_class, (t_method)dwt_filter, gensym("update"), A_GIMME, 0); 
    class_addmethod(dwt16_class, (t_method)dwt_filter, gensym("mask"), A_GIMME, 0);




    idwt16_class = class_new(gensym("idwt16~"), (t_newmethod)idwt16_new,
    	(t_method)dwt_free, sizeof(t_dwt), 0, A_DEFFLOAT, 0);

    CLASS_MAINSIGNALIN(idwt16_class, t_dwt, x_f);
    class_addmethod(idwt16_class, (t_method)dwt_print, gensym("print"), 0);
    class_addmethod(idwt16_class, (t_method)dwt_dsp, gensym("dsp"), 0); 

    class_addmethod(idwt16_class, (t_method)dwt_filter, gensym("predict"), A_GIMME, 0); 
    class_addmethod(idwt16_class, (t_method)dwt_filter, gensym("update"), A_GIMME, 0); 
    class_addmethod(idwt16_class, (t_method)dwt_filter, gensym("mask"), A_GIMME, 0); 

    class_addmethod(idwt16_class, (t_method)idwt_coef, gensym("coef"), A_DEFFLOAT, A_DEFFLOAT, 0); 




}

