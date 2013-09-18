/*
 *   ead.c  -  exponential attack decay envelope 
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

/* pointer to */
t_class *ead_class;


/* state data fpr attack/decay dsp plugin */
typedef struct eadctl
{
  t_float c_attack;
  t_float c_decay;
  t_float c_state;
  t_int c_target;
} t_eadctl;


/* object data structure */
typedef struct ead
{
  t_object x_obj;  
  t_eadctl x_ctl;   
} t_ead;



static void ead_attack(t_ead *x, t_floatarg f)
{
  x->x_ctl.c_attack = milliseconds_2_one_minus_realpole(f);
}

static void ead_decay(t_ead *x, t_floatarg f)
{
  x->x_ctl.c_decay = milliseconds_2_one_minus_realpole(f);
}

static void ead_start(t_ead *x)
{
    /* reset state if necessary to prevent skipping */

    // always reset, seems to be safest
    //if (x->x_ctl.c_target == 1)

    x->x_ctl.c_state = 0.0f; 
    x->x_ctl.c_target = 1;
}


/* dsp callback function, not a method */
static t_int *ead_perform(t_int *w)
{

  /* interprete arguments */
    t_float *out    = (t_float *)(w[3]);
    t_eadctl *ctl  = (t_eadctl *)(w[1]);
    t_float attack  = ctl->c_attack;
    t_float decay   = ctl->c_decay;
    t_float state   = ctl->c_state;
    t_int n = (t_int)(w[2]);

    t_int i;


    /* A/D code */

    for (i = 0; i < n; i++){
	switch(ctl->c_target){
	case 1:
	    /* attack phase */
	    *out++ = state;
	    state += attack*(1 - state);
	    ctl->c_target = (state <= ENVELOPE_MAX);
	    break;
	default:
	    /* decay phase */
	    *out++ = state;
	    state -= decay*state;
	    break;
	}
	
    }

    /* save state */
    ctl->c_state = IS_DENORMAL(state) ? 0 : state;

    return (w+4); 
}


static void ead_dsp(t_ead *x, t_signal **sp)
{
  dsp_add(ead_perform, 3, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec);
}                                  

/* destructor */
static void ead_free(void)
{

}



/* constructor */
static void *ead_new(t_floatarg attack, t_floatarg decay, 
		     t_floatarg sustain, t_floatarg release)
{
    /* create instance */
    t_ead *x = (t_ead *)pd_new(ead_class);
    /* create new inlets, convert incoming message float to attack/decay */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("attack"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("decay"));  
    /* create a dsp outlet */
    outlet_new(&x->x_obj, gensym("signal")); 

    /* initialize */
    x->x_ctl.c_state  = 0;
    x->x_ctl.c_target = 0;
    ead_attack(x, attack);
    ead_decay(x, decay);

    /* return instance */
    return (void *)x;
}

void ead_tilde_setup(void)
{
  //post("ead~ v0.1");

    ead_class = class_new(gensym("ead~"), (t_newmethod)ead_new,
    	(t_method)ead_free, sizeof(t_ead), 0,  A_DEFFLOAT, A_DEFFLOAT, 0);


    class_addmethod(ead_class, (t_method)ead_start, gensym("start"), 0);
    class_addmethod(ead_class, (t_method)ead_start, gensym("bang"), 0);
    class_addmethod(ead_class, (t_method)ead_dsp, gensym("dsp"), 0); 
    class_addmethod(ead_class, (t_method)ead_attack,
		    gensym("attack"), A_FLOAT, 0);
    class_addmethod(ead_class, (t_method)ead_decay,
		    gensym("decay"), A_FLOAT, 0);
}
