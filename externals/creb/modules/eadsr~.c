/*
 *   eadsr.c  -  exponential attack decay sustain release envelope 
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

typedef struct eadsrctl
{
  t_float c_attack;
  t_float c_decay;
  t_float c_sustain;
  t_float c_release;
  t_float c_state;
  t_float c_target;
} t_eadsrctl;

typedef struct eadsr
{
  t_object x_obj;
  t_float x_sr;
  t_eadsrctl x_ctl;
} t_eadsr;

void eadsr_attack(t_eadsr *x, t_floatarg f)
{
  x->x_ctl.c_attack = milliseconds_2_one_minus_realpole(f);
}

void eadsr_decay(t_eadsr *x, t_floatarg f)
{
  x->x_ctl.c_decay = milliseconds_2_one_minus_realpole(f);
}

void eadsr_sustain(t_eadsr *x, t_floatarg f)
{
  if (f>ENVELOPE_MAX) f = ENVELOPE_MAX;
  if (f<ENVELOPE_MIN) f = ENVELOPE_MIN;

  x->x_ctl.c_sustain = f;
}

void eadsr_release(t_eadsr *x, t_floatarg f)
{
  x->x_ctl.c_release = milliseconds_2_one_minus_realpole(f);

}

void eadsr_start(t_eadsr *x)
{
    x->x_ctl.c_target = 1;
    x->x_ctl.c_state = 0.0f;

}

void eadsr_stop(t_eadsr *x)
{
    x->x_ctl.c_target = 0;

}

void eadsr_float(t_eadsr *x, t_floatarg f)
{
    if (f == 0.0f) eadsr_stop(x);
    else eadsr_start(x);
}

static t_int *eadsr_perform(t_int *w)
{
    t_float *out    = (t_float *)(w[3]);
    t_eadsrctl *ctl  = (t_eadsrctl *)(w[1]);
    t_float attack  = ctl->c_attack;
    t_float decay   = ctl->c_decay;
    t_float sustain = ctl->c_sustain;
    t_float release = ctl->c_release;
    t_float state   = ctl->c_state;
    t_float target  = ctl->c_target;
    t_int n = (t_int)(w[2]);

    t_int i;


    for (i = 0; i < n; i++){
	if (target == 1.0f){
	    /* attack */
	    *out++ = state;
	    state += attack*(1 - state);
	    target = (state > ENVELOPE_MAX) ? sustain : 1.0f;
	}
	else if (target == 0.0f){
	    /* release */
	    *out++ = state;
	    state -= release*state;
	}
	else{
	    /* decay */
	    *out++ = state;
	    state -= decay*(state-sustain);
	}
    }

    /* save state */
    ctl->c_state = IS_DENORMAL(state) ? 0 : state;
    ctl->c_target = target;
    return (w+4);
}

static void eadsr_dsp(t_eadsr *x, t_signal **sp)
{
    x->x_sr = sp[0]->s_sr;
    dsp_add(eadsr_perform, 3, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec);

}                                  
void eadsr_free(void)
{

}

t_class *eadsr_class;

void *eadsr_new(t_floatarg attack, t_floatarg decay, 
		t_floatarg sustain, t_floatarg release)
{
    t_eadsr *x = (t_eadsr *)pd_new(eadsr_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("attack"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("decay"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("sustain"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("release"));
    outlet_new(&x->x_obj, gensym("signal")); 

    x->x_ctl.c_state  = 0;
    x->x_ctl.c_target = 0;
    eadsr_attack(x, attack);
    eadsr_decay(x, decay);
    eadsr_sustain(x, sustain);
    eadsr_release(x, release);


    return (void *)x;
}

void eadsr_tilde_setup(void)
{
    //post("eadsr~ v0.1");
    eadsr_class = class_new(gensym("eadsr~"), (t_newmethod)eadsr_new,
    	(t_method)eadsr_free, sizeof(t_eadsr), 0,  A_DEFFLOAT, 
			    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(eadsr_class, (t_method)eadsr_float,
		    gensym("float"), A_FLOAT, 0);
    class_addmethod(eadsr_class, (t_method)eadsr_start,
		    gensym("start"), 0);
    class_addmethod(eadsr_class, (t_method)eadsr_start, gensym("bang"), 0);
    class_addmethod(eadsr_class, (t_method)eadsr_stop, gensym("stop"), 0);
    class_addmethod(eadsr_class, (t_method)eadsr_dsp, gensym("dsp"), 0); 
    class_addmethod(eadsr_class, (t_method)eadsr_attack,
		    gensym("attack"), A_FLOAT, 0);
    class_addmethod(eadsr_class, (t_method)eadsr_decay,
		    gensym("decay"), A_FLOAT, 0);
    class_addmethod(eadsr_class, (t_method)eadsr_sustain,
		    gensym("sustain"), A_FLOAT, 0);
    class_addmethod(eadsr_class, (t_method)eadsr_release,
		    gensym("release"), A_FLOAT, 0);


}

