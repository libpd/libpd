/*
 *   bfft.c - Code for some fourrier transform variants and utility
 *   functions. Data organization is in (real, imag) pairs the first 2
 *   components are (DC, NY)
 *
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

#define MAXORDER 64

typedef struct bfftctl
{
    t_int c_levels;
    char c_name[16];
    t_int *c_clutter;
    t_int *c_unclutter;
    t_int c_kill_DC;
    t_int c_kill_NY;
} t_bfftctl;

typedef struct bfft
{
    t_object x_obj;
    t_float x_f;
    t_bfftctl x_ctl;
} t_bfft;

t_class *bfft_class, *ibfft_class, *fht_class;


static inline void bfft_perform_permutation(t_float *S, int n, t_int *f)
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

static void bfft_permutation(t_bfft *x, t_int n){

  t_bfftctl *ctl = &x->x_ctl;
  int i;

  if (ctl->c_clutter)   free(ctl->c_clutter);
  if (ctl->c_unclutter) free(ctl->c_unclutter);
  
  ctl->c_clutter = (t_int *)malloc(n*sizeof(t_int));
  ctl->c_unclutter = (t_int *)malloc(n*sizeof(t_int));


  ctl->c_unclutter[0] = 0;
  ctl->c_unclutter[1] = n/2;
  for (i=1; i<n/2; i++){
    ctl->c_unclutter[2*i] = i;
    ctl->c_unclutter[2*i+1] = n-i;
  }

  for(i=0; i<n; i++)
    ctl->c_clutter[ctl->c_unclutter[i]] = i;

  return;

  /* debug */
  /*  for(k=0; k<n; k++)
  **    printf("clutter[%d] = %d\n", k, ctl->c_clutter[k]);
  **  for(k=0; k<n; k++)
  **    printf("unclutter[%d] = %d\n", k, ctl->c_unclutter[k]);
  **
  ** exit(1);
  */
}



static t_int *bfft_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_bfftctl *ctl  = (t_bfftctl *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_float scale = sqrt(1.0f / (t_float)(n));

  mayer_fht(out, n);
  bfft_perform_permutation(out, n, ctl->c_unclutter);

  while (n--) *out++ *= scale;

  return (w+5);
}




static t_int *ibfft_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_bfftctl *ctl  = (t_bfftctl *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_float scale = sqrt(1.0f / (t_float)(n));


  if (ctl->c_kill_DC) {out[0] = 0.0f;}
  if (ctl->c_kill_NY) {out[1] = 0.0f;}

  bfft_perform_permutation(out, n, ctl->c_clutter);
  mayer_fht(out, n);


  while (n--) *out++ *= scale;

  

  return (w+5);
}


static t_int *fht_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_bfftctl *ctl  = (t_bfftctl *)(w[1]);


  t_int n = (t_int)(w[2]);

  mayer_fht(out, n);

  return (w+5);
}


static void bfft_dsp(t_bfft *x, t_signal **sp)
{

  int n = sp[0]->s_n;
  t_float *in = sp[0]->s_vec;
  t_float *out = sp[1]->s_vec;

  bfft_permutation(x, n);

  if (in != out)
    {
      dsp_add_copy(in,out,n);
      in = out;
    }

  dsp_add(bfft_perform, 4, &x->x_ctl, n, in, out);

}

static void ibfft_dsp(t_bfft *x, t_signal **sp)
{

  int n = sp[0]->s_n;
  t_float *in = sp[0]->s_vec;
  t_float *out = sp[1]->s_vec;

  bfft_permutation(x, n);

  if (in != out)
    {
      dsp_add_copy(in,out,n);
      in = out;
    }

  dsp_add(ibfft_perform, 4, &x->x_ctl, n, in, out);

}

static void fht_dsp(t_bfft *x, t_signal **sp)
{

  int n = sp[0]->s_n;
  t_float *in = sp[0]->s_vec;
  t_float *out = sp[1]->s_vec;


  if (in != out)
    {
      dsp_add_copy(in,out,n);
      in = out;
    }

  dsp_add(fht_perform, 4, &x->x_ctl, n, in, out);

}


                                  
static void bfft_free(t_bfft *x)
{

  if (x->x_ctl.c_clutter)   free(x->x_ctl.c_clutter);
  if (x->x_ctl.c_unclutter) free(x->x_ctl.c_unclutter);

}




static void *bfft_new(void)
{
    t_bfft *x = (t_bfft *)pd_new(bfft_class);
    int i;

    outlet_new(&x->x_obj, gensym("signal")); 


    sprintf(x->x_ctl.c_name,"bfft");

    x->x_ctl.c_clutter = NULL;
    x->x_ctl.c_unclutter = NULL;

    return (void *)x;


}

static void *ibfft_new(t_symbol *s)
{
    t_bfft *x = (t_bfft *)pd_new(ibfft_class);
    int i;

    outlet_new(&x->x_obj, gensym("signal")); 

    if (s == gensym("killDCNY")){
	x->x_ctl.c_kill_DC = 1;
	x->x_ctl.c_kill_NY = 1;
	post("ibfft: removing DC and NY components.");
    }
    else{
	x->x_ctl.c_kill_DC = 0;
	x->x_ctl.c_kill_NY = 0;
    }

    x->x_ctl.c_clutter = NULL;
    x->x_ctl.c_unclutter = NULL;

    sprintf(x->x_ctl.c_name,"ibfft");

    return (void *)x;
}

static void *fht_new(void)
{
    t_bfft *x = (t_bfft *)pd_new(fht_class);
    int i;

    outlet_new(&x->x_obj, gensym("signal")); 


    x->x_ctl.c_clutter = NULL;
    x->x_ctl.c_unclutter = NULL;

    sprintf(x->x_ctl.c_name,"fht");

    return (void *)x;
}




void bfft_tilde_setup(void)
{
  //post("bfft~ v0.1");
    bfft_class = class_new(gensym("bfft~"), (t_newmethod)bfft_new,
    	(t_method)bfft_free, sizeof(t_bfft), 0, 0);
    CLASS_MAINSIGNALIN(bfft_class, t_bfft, x_f);
    class_addmethod(bfft_class, (t_method)bfft_dsp, gensym("dsp"), 0); 



    ibfft_class = class_new(gensym("ibfft~"), (t_newmethod)ibfft_new,
    	(t_method)bfft_free, sizeof(t_bfft), 0, A_DEFSYMBOL, A_NULL);

    /* add the more logical bifft~ alias */
    class_addcreator((t_newmethod)ibfft_new, 
		     gensym("bifft~"), 0, A_DEFSYMBOL, A_NULL);

    CLASS_MAINSIGNALIN(ibfft_class, t_bfft, x_f);
    class_addmethod(ibfft_class, (t_method)ibfft_dsp, gensym("dsp"), 0); 

 

   fht_class = class_new(gensym("fht~"), (t_newmethod)fht_new,
    	(t_method)bfft_free, sizeof(t_bfft), 0, 0);

    CLASS_MAINSIGNALIN(fht_class, t_bfft, x_f);
    class_addmethod(fht_class, (t_method)fht_dsp, gensym("dsp"), 0); 



}
