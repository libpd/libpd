/*
 *   xfm.c  -  cross frequency modulation object
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


/*  
coupled fm. osc state equations:

phasor for system i =

                     [  1  -phi ]
(1+phi^2)^(1/2)  *   [ phi   1  ] 
 

with phi = 2*pi*(freq_base + freq_mod * out_other) / sr


ideal phasor would be

[ cos(phi)  - sin(phi) ]
[ sin(phi)    cos(phi) ]


this means frequencies are warped:

2*pi*f_real = atan(2*pi*f)

some (possible) enhancements:
	+ add an integrator to get phase modulation
	+ undo the frequency warping

*/

#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

#define SINSAMPLES 512
#define MYPI 3.141592653589793


#define DISTORTED   0
#define NORMALIZED  1
   

typedef struct xfmctl
{
    //t_float c_sintab[SINSAMPLES + 1];
    t_float c_x1, c_y1; /* state osc 1 */
    t_float c_x2, c_y2; /* state osc 2 */
    t_int c_type; /* type of algo */

} t_xfmctl;

typedef struct xfm
{
    t_object x_obj;
    t_float x_f;
    t_xfmctl x_ctl;
} t_xfm;

void xfm_type(t_xfm *x, t_float f)
{
    int t = (int)f;

    if (t == DISTORTED) x->x_ctl.c_type = t;
    if (t == NORMALIZED) x->x_ctl.c_type = t;

}


static inline t_float xfm_sat(t_float x)
{
  const t_float max =  1;
  const t_float min = -1;

  x = (x > max) ? (max) : (x);
  x = (x < min) ? (min) : (x);

  return(x);
}

static t_int *xfm_perform(t_int *w)
{


  t_float *inA     = (t_float *)(w[3]);
  t_float *inB     = (t_float *)(w[4]);
  t_float *fbA     = (t_float *)(w[5]);
  t_float *fbB     = (t_float *)(w[6]);
  t_float *outA    = (t_float *)(w[7]);
  t_float *outB    = (t_float *)(w[8]);
  t_xfmctl *ctl    = (t_xfmctl *)(w[1]);
  t_int n          = (t_int)(w[2]);
  //t_float *tab     = ctl->c_sintab;

  t_float x1 = ctl->c_x1, y1 = ctl->c_y1,   z1, dx1, dy1, inv_norm1;
  t_float x2 = ctl->c_x2, y2 = ctl->c_y2,   z2, dx2, dy2, inv_norm2;

  t_float scale = 2 * M_PI / sys_getsr();

  t_int i;

  switch(ctl->c_type){
  default:
  case  DISTORTED:

      /* this is a 4 degree of freedom hyperchaotic system */
      /* two coupled saturated unstable oscillators */

      for (i=0; i<n; i++){
	  /* osc 1 */
	  z1 = scale * (x2 * (*fbA++) + (*inA++));
	  
	  dx1 = x1 - z1*y1;
	  dy1 = y1 + z1*x1;
	  
	  x1 = xfm_sat(dx1);
	  y1 = xfm_sat(dy1);
	  
	  /* osc 2*/
	  z2 = scale * (x1 * (*fbB++) + (*inB++));
	      
	  dx2 = x2 - z2*y2;
	  dy2 = y2 + z2*x2;
	  
	  x2 = xfm_sat(dx2);
	  y2 = xfm_sat(dy2);
	  
	  /* output */
	  (*outA++) = x1;
	  (*outB++) = x2;
	  
      }
      break;

  case NORMALIZED:

      /* this is a an effective 2 degree of freedom quasiperiodic system */
      /* two coupled stable oscillators */

      for (i=0; i<n; i++){

	  /* osc 1 */
	  z1 = scale * (x2 * (*fbA++) + (*inA++));
	  
	  dx1 = x1 - z1*y1;
	  dy1 = y1 + z1*x1;
	  inv_norm1 = 1.0f / hypot(dx1, dy1);
	  
	      
	  /* osc 2*/
	  z2 = scale * (x1 * (*fbB++) + (*inB++));
	  
	  dx2 = x2 - z2*y2;
	  dy2 = y2 + z2*x2;
	  inv_norm2 = 1.0f / hypot(dx2, dy2);
	  
	  /* renormalize */
	  x1 = dx1 * inv_norm1;
	  y1 = dy1 * inv_norm1;
	  x2 = dx2 * inv_norm2;
	  y2 = dy2 * inv_norm2;
	  
	  /* output */
	  (*outA++) = x1;
	  (*outB++) = x2;
      }
      break;
  }

  ctl->c_x1 = x1;
  ctl->c_y1 = y1;
  ctl->c_x2 = x2;
  ctl->c_y2 = y2;

  return (w+9);
}

static void xfm_dsp(t_xfm *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  int k;


  dsp_add(xfm_perform, 
	  8, 
	  &x->x_ctl, 
	  sp[0]->s_n, 
	  sp[0]->s_vec, 
	  sp[1]->s_vec, 
	  sp[2]->s_vec,
	  sp[3]->s_vec,
	  sp[4]->s_vec,
	  sp[5]->s_vec);


}                                  
static void xfm_free(t_xfm *x)
{


}




static void xfm_reset(t_xfm *x)
{
    x->x_ctl.c_x1 = 1;
    x->x_ctl.c_y1 = 0;
    x->x_ctl.c_x2 = 1;
    x->x_ctl.c_y2 = 0;

}


t_class *xfm_class;

static void *xfm_new(t_floatarg algotype)
{
    t_xfm *x = (t_xfm *)pd_new(xfm_class);

    /* ins */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  

    /* outs */
    outlet_new(&x->x_obj, gensym("signal")); 
    outlet_new(&x->x_obj, gensym("signal")); 



    /* init data */
    xfm_reset(x);
    xfm_type(x, algotype);

    return (void *)x;
}

void xfm_tilde_setup(void)
{
    //post("xfm~ v0.1");
    xfm_class = class_new(gensym("xfm~"), (t_newmethod)xfm_new,
    	(t_method)xfm_free, sizeof(t_xfm), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(xfm_class, t_xfm, x_f);
    class_addmethod(xfm_class, (t_method)xfm_type, gensym("type"), A_FLOAT, 0);
    class_addmethod(xfm_class, (t_method)xfm_dsp, gensym("dsp"), 0); 
    class_addmethod(xfm_class, (t_method)xfm_reset, gensym("reset"), 0); 


}

