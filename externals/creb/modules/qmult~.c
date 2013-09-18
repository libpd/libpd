/*
 *   qmult.c  - quaternion multiplication dsp object 
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


typedef struct qmultctl
{
  t_float *c_inputleft[4];
  t_float *c_inputright[4];
  t_float *c_output[4];
} t_qmultctl;

typedef struct qmult
{
  t_object x_obj;
  t_float x_f;
  t_qmultctl x_ctl;
} t_qmult;


static t_int *qmult_perform(t_int *word)
{



  t_qmultctl *ctl     = (t_qmultctl *)(word[1]);
  t_int n             = (t_int)(word[2]);
  t_int i;

  t_float *in0l        = ctl->c_inputleft[0];
  t_float *in1l        = ctl->c_inputleft[1];
  t_float *in2l        = ctl->c_inputleft[2];
  t_float *in3l        = ctl->c_inputleft[3];

  t_float *in0r        = ctl->c_inputright[0];
  t_float *in1r        = ctl->c_inputright[1];
  t_float *in2r        = ctl->c_inputright[2];
  t_float *in3r        = ctl->c_inputright[3];

  t_float *out0       = ctl->c_output[0];
  t_float *out1       = ctl->c_output[1];
  t_float *out2       = ctl->c_output[2];
  t_float *out3       = ctl->c_output[3];

  t_float wl, xl, yl, zl;
  t_float wr, xr, yr, zr;
  t_float w, x, y, z;

  for (i=0;i<n;i++)
    {

	/* read input quaternions */
	wl = *in0l++;
	xl = *in1l++;
	yl = *in2l++;
	zl = *in3l++;

	wr = *in0r++;
	xr = *in1r++;
	yr = *in2r++;
	zr = *in3r++;


	/* multiply quaternions */
	w = wl * wr;
	x = wl * xr;
	y = wl * yr;
	z = wl * zr;

	w -= xl * xr;
	x += xl * wr;
	y -= xl * zr;
	z += xl * yr;

	w -= yl * yr;
	x += yl * zr;
	y += yl * wr;
	z -= yl * xr;

	w -= zl * zr;
	x -= zl * yr;
	y += zl * xr;
	z += zl * wr;



	/* write output quaternion */
	*out0++ = w;
	*out1++ = x;
	*out2++ = y;
	*out3++ = z;
    }

   
  return (word+3);
}



static void qmult_dsp(t_qmult *x, t_signal **sp)
{

    int i;
  for (i=0;i<4;i++)
    {
      x->x_ctl.c_inputleft[i] = sp[i]->s_vec;
      x->x_ctl.c_inputright[i] = sp[i+4]->s_vec;
      x->x_ctl.c_output[i] = sp[i+8]->s_vec;
    }

  dsp_add(qmult_perform, 2, &x->x_ctl, sp[0]->s_n);


}                                  


static void qmult_free(t_qmult *x)
{

}

t_class *qmult_class;

static void *qmult_new(t_floatarg channels)
{
    int i;
    t_qmult *x = (t_qmult *)pd_new(qmult_class);

    for (i=1;i<8;i++) inlet_new(&x->x_obj, &x->x_obj.ob_pd,
				gensym("signal"), gensym("signal")); 
    for (i=0;i<4;i++) outlet_new(&x->x_obj, gensym("signal")); 

    return (void *)x;
}

void qmult_tilde_setup(void)
{
  //post("qmult~ v0.1");
    qmult_class = class_new(gensym("qmult~"), (t_newmethod)qmult_new,
    	(t_method)qmult_free, sizeof(t_qmult), 0, 0);
    CLASS_MAINSIGNALIN(qmult_class, t_qmult, x_f); 
    class_addmethod(qmult_class, (t_method)qmult_dsp, gensym("dsp"), 0); 

}

