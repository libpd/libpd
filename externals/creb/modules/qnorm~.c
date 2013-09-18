/*
 *   qnorm.c  - quaternion normalization dsp object 
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


typedef struct qnormctl
{
  t_float *c_input[4];
  t_float *c_output[4];
} t_qnormctl;

typedef struct qnorm
{
  t_object x_obj;
  t_float x_f;
  t_qnormctl x_ctl;
} t_qnorm;


static t_int *qnorm_perform(t_int *word)
{



  t_qnormctl *ctl     = (t_qnormctl *)(word[1]);
  t_int n             = (t_int)(word[2]);
  t_int i;

  t_float *in0        = ctl->c_input[0];
  t_float *in1        = ctl->c_input[1];
  t_float *in2        = ctl->c_input[2];
  t_float *in3        = ctl->c_input[3];

  t_float *out0       = ctl->c_output[0];
  t_float *out1       = ctl->c_output[1];
  t_float *out2       = ctl->c_output[2];
  t_float *out3       = ctl->c_output[3];

  t_float w, x, y, z;
  t_float norm;
  t_float inorm;

  for (i=0;i<n;i++)
    {

	/* read input */
	w = *in0++;
	x = *in1++;
	y = *in2++;
	z = *in3++;

	/* transform */
	norm = w * w;
	norm += x * x;
	norm += y * y;
	norm += z * z;
	
	inorm = (norm == 0.0f) ? (0.0f) : (1.0f / sqrt(norm));

	/* write output */
	*out0++ = w * inorm;
	*out1++ = x * inorm;
	*out2++ = y * inorm;
	*out3++ = z * inorm;
    }

   
  return (word+3);
}



static void qnorm_dsp(t_qnorm *x, t_signal **sp)
{

    int i;
  for (i=0;i<4;i++)
    {
      x->x_ctl.c_input[i] = sp[i]->s_vec;
      x->x_ctl.c_output[i] = sp[i+4]->s_vec;
    }

  dsp_add(qnorm_perform, 2, &x->x_ctl, sp[0]->s_n);


}                                  


static void qnorm_free(t_qnorm *x)
{

}

t_class *qnorm_class;

static void *qnorm_new(t_floatarg channels)
{
    int i;
    t_qnorm *x = (t_qnorm *)pd_new(qnorm_class);

    for (i=1;i<4;i++) inlet_new(&x->x_obj, &x->x_obj.ob_pd,
				gensym("signal"), gensym("signal")); 
    for (i=0;i<4;i++) outlet_new(&x->x_obj, gensym("signal")); 

    return (void *)x;
}

void qnorm_tilde_setup(void)
{
  //post("qnorm~ v0.1");
    qnorm_class = class_new(gensym("qnorm~"), (t_newmethod)qnorm_new,
    	(t_method)qnorm_free, sizeof(t_qnorm), 0, 0);
    CLASS_MAINSIGNALIN(qnorm_class, t_qnorm, x_f); 
    class_addmethod(qnorm_class, (t_method)qnorm_dsp, gensym("dsp"), 0); 

}

