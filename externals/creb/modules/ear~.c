/*
 *   ear.c  -  exponential attack release envelope 
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

#include "extlib_util.h"

typedef struct earctl
{
  t_float c_attack;
  t_float c_release;
  t_float c_state;
  t_float c_target;
} t_earctl;

typedef struct ear
{
  t_object x_obj;
  t_float x_sr;
  t_earctl x_ctl;
} t_ear;

void ear_attack(t_ear *x, t_floatarg f)
{
  x->x_ctl.c_attack = milliseconds_2_one_minus_realpole(f);
}

void ear_release(t_ear *x, t_floatarg f)
{
  x->x_ctl.c_release = milliseconds_2_one_minus_realpole(f);
}

void ear_start(t_ear *x)
{
    x->x_ctl.c_target = 1;
    x->x_ctl.c_state = 0.0f;

}

void ear_stop(t_ear *x)
{
    x->x_ctl.c_target = 0;

}

void ear_float(t_ear *x, t_floatarg f)
{
    if (f == 0.0f) ear_stop(x);
    else ear_start(x);
}

static t_int *ear_perform(t_int *w)
{
    t_float *out   = (t_float *)(w[3]);
    t_earctl *ctl = (t_earctl *)(w[1]);
    t_float attack = ctl->c_attack;
    t_float release  = ctl->c_release;
    t_float state  = ctl->c_state;
    t_float target = ctl->c_target;
    t_int n = (t_int)(w[2]);

    t_int i;

    if (target) /* attack phase */
      for (i = 0; i < n; i++)
	{
	  *out++ = state;
	  state += attack*(1 - state);
	} 
    else /* release phase */
      for (i = 0; i < n; i++)
	{
	  *out++ = state;
	  state -= release*state;
	}

    ctl->c_state = IS_DENORMAL(state) ? 0 : state;
    return (w+4);
}

static void ear_dsp(t_ear *x, t_signal **sp)
{
    x->x_sr = sp[0]->s_sr;
    dsp_add(ear_perform, 3, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec);

}                                  
void ear_free(void)
{

}


t_class *ear_class;  /* attack - release */

void *ear_new(t_floatarg attack, t_floatarg release)
{
    t_ear *x = (t_ear *)pd_new(ear_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("attack"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("release"));
    outlet_new(&x->x_obj, gensym("signal")); 

    ear_attack(x,attack);
    ear_release(x,release);
    x->x_ctl.c_state  = 0;
    x->x_ctl.c_target = 0;

    return (void *)x;
}


void ear_tilde_setup(void)
{
    //post("ear~ v0.1");
    ear_class = class_new(gensym("ear~"), (t_newmethod)ear_new,
    	(t_method)ear_free, sizeof(t_ear), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(ear_class, (t_method)ear_float, gensym("float"), A_FLOAT, 0);
    class_addmethod(ear_class, (t_method)ear_start, gensym("start"), 0);
    class_addmethod(ear_class, (t_method)ear_start, gensym("bang"), 0);
    class_addmethod(ear_class, (t_method)ear_stop, gensym("stop"), 0);
    class_addmethod(ear_class, (t_method)ear_dsp, gensym("dsp"), 0); 
    class_addmethod(ear_class, 
		    (t_method)ear_attack, gensym("attack"), A_FLOAT, 0);
    class_addmethod(ear_class, 
		    (t_method)ear_release, gensym("release"), A_FLOAT, 0);


}

