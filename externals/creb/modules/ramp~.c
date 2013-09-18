/*
 *   ramp.c  - retriggerable counter for dsp signals 
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


typedef struct rampctl
{
    t_float c_offset;
    t_int c_blockscale;
} t_rampctl;

typedef struct ramp
{
    t_object x_obj;
    t_float x_f;
    t_rampctl x_ctl;
} t_ramp;


void ramp_offset(t_ramp *x,  t_floatarg f)
{
    x->x_ctl.c_offset = f;
}


void ramp_bang(t_ramp *x)
{
    ramp_offset(x, 0);
}



static t_int *ramp_perform(t_int *w)
{
    t_float *out    = (t_float *)(w[3]);
    t_rampctl *ctl  = (t_rampctl *)(w[1]);
    t_int i;
    t_int n = (t_int)(w[2]);
    t_float x;

    t_float scale = ctl->c_blockscale ? 1.0 / (t_float)n : 1.0;


    x = ctl->c_offset;
    
    for (i = 0; i < n; i++)
	{
	    *out++ = ((t_float)x++) * scale;
	}

    ctl->c_offset = x; /* save state */

    
    return (w+4);
}

static void ramp_dsp(t_ramp *x, t_signal **sp)
{
    dsp_add(ramp_perform, 3, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec);

}                                  
void ramp_free(void)
{

}

t_class *ramp_class;

void *ramp_new(void)
{
    t_ramp *x = (t_ramp *)pd_new(ramp_class);
    outlet_new(&x->x_obj, gensym("signal")); 
    x->x_ctl.c_blockscale = 0;
    ramp_bang(x);
    return (void *)x;
}

void *blockramp_new(void)
{
    t_ramp *x = (t_ramp *)ramp_new();
    x->x_ctl.c_blockscale = 1;
    return (void *)x;
}

void ramp_tilde_setup(void)
{
  //post("ramp~ v0.1");
    ramp_class = class_new(gensym("ramp~"), (t_newmethod)ramp_new,
    	(t_method)ramp_free, sizeof(t_ramp), 0, 0);

    class_addcreator((t_newmethod)blockramp_new, gensym("blockramp~"), A_NULL);

    class_addmethod(ramp_class, (t_method)ramp_bang, gensym("bang"), 0);
    class_addmethod(ramp_class, (t_method)ramp_dsp, gensym("dsp"), 0); 
    class_addfloat(ramp_class, (t_method)ramp_offset); 
}

