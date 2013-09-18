/*
 *   fdn.c  - a feedback delay network (reverb tail)
 *   using a housholder reflection feedback matrix (In - 2/n 11T)
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

/* TODO: CLEAN UP THIS MESS

add delay time generation code
add prime calculation routine (for prime delay line lengths)
add more diffuse feedback matrix (hadamard)
check filtering code

*/

#include "extlib_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FDN_MIN_DECAY_TIME .01

/*

#define NBPRIMES
int prime[NBPRIMES];

static int isprime(int n)
{
  int i=1;
  int d,m,p;
  while(1){
    p = prime[i++];
    m = n % p;
    if (m == 0) return 0; // it is a prime
    d = n / p;
    if (d < p) return 1;  // it is not a prime
  }
}

static int initprimes(void)
{
  int i, curprime;
  prime[0] = 1;
  prime[1] = 2;
  curprime = 3;
  
  for(i=2; i<NBPRIMES; i++){
    while (!isprime(curprime)) curprime++;
    prime[i] = curprime;
    //printf("%d, ", curprime);
    curprime++;
  }
  printf("\n");
  return 0;
}
*/


//static int find_nearest_prime(int n){ return n;}

typedef struct fdnctl
{
  t_int c_order;      /* veelvoud van 4 */
  t_int c_maxorder;
  t_float c_leak;
  t_float c_input;
  t_float c_output;
  t_float *c_buf;
  t_float *c_gain_in;
  t_float *c_gain_state;
  t_float c_timehigh;
  t_float c_timelow;
  t_int *c_tap;       /* cirular feed: N+1 pointers: 1 read, (N-1)r/w, 1 write */ 
  t_float *c_length;  /* delay lengths in ms */
  t_int c_bufsize;
  t_float c_fsample;
  t_float *c_vector[2];
  t_float *c_vectorbuffer;
  t_int c_curvector;
} t_fdnctl;

typedef struct fdn
{
  t_object x_obj;
  t_float x_f;
  t_fdnctl x_ctl;
} t_fdn;


static void fdn_order(t_fdn *x, t_int order){
  if (order > x->x_ctl.c_maxorder) {
    post("fdn: this should not happen (panic!) order %d "
	 "is larger than maxorder %d:", 
	 order, x->x_ctl.c_maxorder ); 
    exit(1);
  }
  x->x_ctl.c_order = order;
  x->x_ctl.c_leak = -2./ order;
  x->x_ctl.c_input = 1./ sqrt(order); //????????????????
 
}

static void fdn_print(t_fdn *x)
{
  int i;
  fprintf(stderr, "fdn: delay coefficients (ms)\n");
  for (i=0;i<x->x_ctl.c_order;i++) {
    fprintf(stderr, "%f ", x->x_ctl.c_length[i]);
  }
  fprintf(stderr, "\n");

}


static void fdn_reset(t_fdn *x)
{
  int i;
  if (x->x_ctl.c_buf) 
      memset(x->x_ctl.c_buf, 0, x->x_ctl.c_bufsize 
	     * sizeof(t_float));
  if (x->x_ctl.c_vectorbuffer) 
      memset(x->x_ctl.c_vectorbuffer,
	     0, x->x_ctl.c_maxorder * 2 * sizeof(t_float));
}




static t_int *fdn_perform(t_int *w)
{


  t_float *in      = (t_float *)(w[3]);
  t_float *outr    = (t_float *)(w[4]);
  t_float *outl    = (t_float *)(w[5]);
  t_fdnctl *ctl    = (t_fdnctl *)(w[1]);
  t_int n          = (t_int)(w[2]);
  t_float input    = ctl->c_input;
  t_float output   = ctl->c_output;
  t_float *gain_in    = ctl->c_gain_in;
  t_float *gain_state = ctl->c_gain_state;
  t_int order      = ctl->c_order;
  t_int *tap       = ctl->c_tap;
  t_float *buf     = ctl->c_buf;
  t_int mask       = ctl->c_bufsize - 1;

  t_int i,j;
  t_float x,y,v,left,right,z;
  t_float filt_in, filt_last;

  t_float *cvec, *lvec;
  
  t_float save;


  for(i=0;i<n;i++){

    x = *in++;
    y = 0;
    left = 0;
    right = 0;

    /* get temporary vector buffers */
    cvec = ctl->c_vector[ctl->c_curvector];
    lvec = ctl->c_vector[ctl->c_curvector ^ 1];
    ctl->c_curvector ^= 1;


    /* read input vector + get sum and left/right output*/
    for(j=0;j<order;) 
      {
	z = buf[tap[j]];
	cvec[j] = z;
	y     += z;
	left  += z;
	right += z;
	j++;

	z = buf[tap[j]];
	cvec[j] = z;
	y     += z;
	left  -= z;
	right += z;
	j++;

	z = buf[tap[j]];
	cvec[j] = z;
	y     += z;
	left  += z;
	right -= z;
	j++;

	z = buf[tap[j]];
	cvec[j] = z;
	y     += z;
	left  -= z;
	right -= z;
	j++;
      }	


    /* write output */
    *outl++ = left;
    *outr++ = right;

    /* y == leak to all inputs */
    y *=  ctl->c_leak;



    /* perform feedback */
    /* todo: decouple feedback & permutation */
    save = cvec[0];
    for (j=0; j<order-1; j++){
      cvec[j] = cvec[j+1] + y + x; 
    }
    cvec[order-1] = save + y + x;



    /* apply gain + store result vector in delay lines + increment taps*/
    tap[0] = (tap[0]+1)&mask;
    for(j=0;j<order;j++) {
	save = gain_in[j] * cvec[j] + gain_state[j] * lvec[j];
	save = IS_DENORMAL(save) ? 0 : save;
	cvec[j] = save;
	buf[tap[j+1]] = save;
	tap[j+1] = (tap[j+1] + 1) & mask;
    }	
  }

    
  return (w+6);
}

static void fdn_dsp(t_fdn *x, t_signal **sp)
{
  
  x->x_ctl.c_fsample = sp[0]->s_sr;
  dsp_add(fdn_perform, 
	  5, 
	  &x->x_ctl, 
	  sp[0]->s_n, 
	  sp[0]->s_vec, 
	  sp[1]->s_vec, 
	  sp[2]->s_vec);


}                                  
static void fdn_free(t_fdn *x)
{
  if ( x->x_ctl.c_tap) free( x->x_ctl.c_tap);
  if ( x->x_ctl.c_length) free( x->x_ctl.c_length);
  if ( x->x_ctl.c_gain_in) free( x->x_ctl.c_gain_in);
  if ( x->x_ctl.c_gain_state) free( x->x_ctl.c_gain_state);
  if ( x->x_ctl.c_buf) free ( x->x_ctl.c_buf);
  if ( x->x_ctl.c_vectorbuffer) free ( x->x_ctl.c_vectorbuffer );
}


/*

each delay line is filtered with a first order iir filter:
(gl: dc gain, gh: ny gain)

H(z) = 2 gl gh / (gl + gh - z^-1 (gl - gh))

this results in the difference equation

yk = (2 gl gh ) / (gl + gh) x + (gl - gh) / (gl + gh) yk-1

*/


static void fdn_time(t_fdn *x, t_float timelow, t_float timehigh){
  t_float elow, ehigh;
  t_int i;
  t_float gainlow, gainhigh, gainscale;

  if (timelow < FDN_MIN_DECAY_TIME) timelow = FDN_MIN_DECAY_TIME;
  if (timehigh < FDN_MIN_DECAY_TIME) timehigh = FDN_MIN_DECAY_TIME;

  elow = -.003 / (timelow);
  ehigh = -.003 / (timehigh);

  /* setup gains */
  for(i=0;i<x->x_ctl.c_order;i++){
    gainlow = pow(10, elow * (x->x_ctl.c_length[i]));
    gainhigh = pow(10, ehigh * (x->x_ctl.c_length[i]));
    gainscale = 1.0f / (gainlow + gainhigh);
    x->x_ctl.c_gain_in[i]    = 2.0f * gainlow * gainhigh * gainscale;
    x->x_ctl.c_gain_state[i] = (gainlow - gainhigh) * gainscale;
  }
  x->x_ctl.c_timehigh = timehigh;
  x->x_ctl.c_timelow = timelow;
}

static void fdn_updatedamping(t_fdn *x)
{
  fdn_time(x, x->x_ctl.c_timelow, x->x_ctl.c_timehigh);
}

static void fdn_timelow(t_fdn *x, t_float f){
  x->x_ctl.c_timelow = fabs(f);
  fdn_updatedamping(x);
}
static void fdn_timehigh(t_fdn *x, t_float f){
  x->x_ctl.c_timehigh = fabs(f);
  fdn_updatedamping(x);
}


static void fdn_setupdelayline(t_fdn *x){
  int sum, t, n;
  int mask = x->x_ctl.c_bufsize - 1;
  int start =  x->x_ctl.c_tap[0];
  t_int *tap = x->x_ctl.c_tap;
  t_float *length = x->x_ctl.c_length;
  t_float scale = sys_getsr() * .001f;

  sum = 0;
  tap[0] = (start & mask);
  for (t=1; t<= x->x_ctl.c_order; t++){
    sum += (int)(length[t-1] * scale);
    tap[t]=(start+sum)&mask;
  }

  if (sum > mask){ 
    post("fdn: warning: not enough delay memory, behaviour "
	 "is undefined (this could lead to instability...)");
  }
    

}

static void fdn_list (t_fdn *x,  t_symbol *s, int argc, t_atom *argv){
  int i;
  t_float l;
  int sum=0;

  int order = argc & 0xfffffffc;

  if (order < 4) return;
  if (order > x->x_ctl.c_maxorder) return;


  fdn_order(x, order);
  for(i=0; i<order; i++)
      if (argv[i].a_type == A_FLOAT) x->x_ctl.c_length[i] = argv[i].a_w.w_float;

  fdn_setupdelayline(x);
  fdn_updatedamping(x);
}

static void fdn_linear(t_fdn *x, t_float forder, t_float min, t_float max)
{
    t_int order = ((int)forder) & 0xfffffffc;
    t_float length, inc;
    t_int i;

    if (order < 4) return;
    if (order > x->x_ctl.c_maxorder) return;
    if (min <= 0) return;
    if (max <= 0) return;

    inc = (max - min) / (t_float)(order - 1);
    length = min;
    
    for (i=0; i<order; i++){
	x->x_ctl.c_length[i] = length;
	length += inc;
    }

    fdn_order(x, order);
    fdn_setupdelayline(x);
    fdn_updatedamping(x);
} 



static void fdn_exponential(t_fdn *x, t_float forder, t_float min, t_float max)
{
    t_int order = ((int)forder) & 0xfffffffc;
    t_float length, inc;
    t_int i;

    if (order < 4) return;
    if (order > x->x_ctl.c_maxorder) return;
    if (min <= 0) return;
    if (max <= 0) return;

    inc = pow (max / min, 1.0f / ((t_float)(order - 1)));
    length = min;
    
    for (i=0; i<order; i++){
	x->x_ctl.c_length[i] = length;
	length *= inc;
    }

    fdn_order(x, order);
    fdn_setupdelayline(x);
    fdn_updatedamping(x);
} 





t_class *fdn_class;

static void *fdn_new(t_floatarg maxiorder, t_floatarg maxibufsize)
{
  t_int order = maxiorder;
  t_int bufround;
  t_fdn *x = (t_fdn *)pd_new(fdn_class);
  t_float scale = sys_getsr() * .001f;
  t_int bufsize = (t_int)(scale * maxibufsize);


  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("timelow"));  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("timehigh"));  
  outlet_new(&x->x_obj, gensym("signal")); 
  outlet_new(&x->x_obj, gensym("signal")); 
  
  /* init data */
  if (order < 4) order = 8;
  if (bufsize < 64) bufsize = 65536;
  
  bufround = 1;
  while (bufround < bufsize) bufround *= 2;
  bufsize = bufround;
  

  post("fdn: maximum nb of delay lines %d, total buffer "
       "size %d samples (%f seconds)", 
       order, bufsize, ((t_float)bufsize) / sys_getsr());
  
  
  x->x_ctl.c_maxorder = order;
  x->x_ctl.c_buf = (t_float *)malloc(sizeof(t_float) * bufsize);
  x->x_ctl.c_bufsize = bufsize;
  x->x_ctl.c_fsample = sys_getsr();
  x->x_ctl.c_tap = (t_int *)malloc((order + 1) * sizeof(t_int)); 
  x->x_ctl.c_length = (t_float *)malloc(order * sizeof(t_int)); 
  x->x_ctl.c_gain_in = (t_float *)malloc(order * sizeof(t_float));
  x->x_ctl.c_gain_state = (t_float *)malloc(order * sizeof(t_float));
  x->x_ctl.c_vectorbuffer = (t_float *)malloc(order * 2 * sizeof(t_float));
  memset(x->x_ctl.c_vectorbuffer, 0, order * 2 * sizeof(t_float));
  x->x_ctl.c_curvector = 0;
  x->x_ctl.c_vector[0] = &x->x_ctl.c_vectorbuffer[0];
  x->x_ctl.c_vector[1] = &x->x_ctl.c_vectorbuffer[order];
  
  /* preset */
  fdn_order(x,8);
  x->x_ctl.c_length[0]= 29.0f;
  x->x_ctl.c_length[1]= 31.0f;
  x->x_ctl.c_length[2]= 37.0f;
  x->x_ctl.c_length[3]= 67.0f;
  x->x_ctl.c_length[4]= 82.0f;
  x->x_ctl.c_length[5]= 110.0f;
  x->x_ctl.c_length[6]= 172.0f;
  x->x_ctl.c_length[7]= 211.0f;
  fdn_setupdelayline(x);
  fdn_time(x, 4, 1);

  /* reset delay memory to zero */
  fdn_reset(x);

  
  return (void *)x;
}


void fdn_tilde_setup(void)
{
  //post("fdn~ v0.1");
    fdn_class = class_new(gensym("fdn~"), (t_newmethod)fdn_new,
    	(t_method)fdn_free, sizeof(t_fdn), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(fdn_class, t_fdn, x_f); 
    class_addmethod(fdn_class, (t_method)fdn_print, gensym("print"), 0);
    class_addmethod(fdn_class, (t_method)fdn_reset, gensym("reset"), 0);
    class_addmethod(fdn_class, (t_method)fdn_timehigh,
		    gensym("timehigh"), A_DEFFLOAT, 0);
    class_addmethod(fdn_class, (t_method)fdn_timelow,
		    gensym("timelow"), A_DEFFLOAT, 0);
    class_addmethod(fdn_class, (t_method)fdn_list, gensym("lines"), A_GIMME, 0);
    class_addmethod(fdn_class, (t_method)fdn_dsp, gensym("dsp"), 0); 
    class_addmethod(fdn_class, (t_method)fdn_linear,
		    gensym("linear"), A_FLOAT, A_FLOAT, A_FLOAT, 0); 
    class_addmethod(fdn_class, (t_method)fdn_exponential,
		    gensym("exponential"), A_FLOAT, A_FLOAT, A_FLOAT, 0); 

}

