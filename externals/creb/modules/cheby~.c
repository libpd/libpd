/*
 *   cheby.c  - chebyshev polynomial evaluation 
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


#define MAX_ORDER 1024
#define DEFAULT_ORDER 4

typedef struct chebyctl
{
  t_float c_gain[MAX_ORDER];
  t_int c_order;
} t_chebyctl;

typedef struct cheby
{
  t_object x_obj;
  t_float x_f;
  t_chebyctl x_ctl;
} t_cheby;

static void cheby_bang(t_cheby *x)
{

}

static void cheby_coef(t_cheby *x, t_floatarg coef, t_floatarg f)
{
  int i = (int)coef;
  if ((i > 0) && (i < x->x_ctl.c_order + 1)){
    x->x_ctl.c_gain[i-1] = f;
  /*   post("cheby: harmonic %d set to %f", i, f); */
  }
}


static t_int *cheby_perform(t_int *w)
{


  t_float *in    = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_chebyctl *ctl  = (t_chebyctl *)(w[1]);
  t_float *gain  = ctl->c_gain;
  t_int i;
  t_int n = (t_int)(w[2]), k;
  t_float x,y,t1,t2,t,acc;

    
  for (i = 0; i < n; i++) 
    {
      x = *in++;
      
      gain = ctl->c_gain;
      t2 = 1;                      /* T_0 */
      t1 = x;                      /* T_1 */
      
      acc = *gain++ * x;           /* a_1 T_1 */
      for (k=2; k<=ctl->c_order; k++){
	t = 2*x*t1 - t2;           /* T_k = 2 x T_{k-1} - T_{k-2} */
	acc += *gain++ * t;        /* a_k T_k */
	t2 = t1;
	t1 = t;
      }

      *out++ = acc;
      
    }
  
    
  return (w+5);
}

static void cheby_dsp(t_cheby *x, t_signal **sp)
{
    dsp_add(cheby_perform, 4, &x->x_ctl, sp[0]->s_n, 
	    sp[0]->s_vec, sp[1]->s_vec);

}                                  
static void cheby_free(void)
{

}

t_class *cheby_class;

static void *cheby_new(t_floatarg order_f)
{
  int i;
  int order = (int)order_f;

    t_cheby *x = (t_cheby *)pd_new(cheby_class);
    outlet_new(&x->x_obj, gensym("signal")); 

    if (order < 1)         order = DEFAULT_ORDER;  /* default */
    if (order > MAX_ORDER) order = MAX_ORDER;      /* maximum */


    //post("cheby: order = %d", order);

    x->x_ctl.c_order = order;
    cheby_coef(x, 1, 1);
    for (i=2; i<order+1; i++){
      cheby_coef(x, 0, i);
    }

    return (void *)x;
}

void cheby_tilde_setup(void)
{
  //post("cheby~ v0.1");
    cheby_class = class_new(gensym("cheby~"), (t_newmethod)cheby_new,
    	(t_method)cheby_free, sizeof(t_cheby), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(cheby_class, t_cheby, x_f); 
    class_addmethod(cheby_class, (t_method)cheby_bang, gensym("bang"), 0);
    class_addmethod(cheby_class, (t_method)cheby_dsp, gensym("dsp"), 0); 
    class_addmethod(cheby_class, (t_method)cheby_coef, gensym("coef"), 
		    A_DEFFLOAT, A_DEFFLOAT, 0); 

}

