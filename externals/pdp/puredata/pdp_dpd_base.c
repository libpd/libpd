/*
 *   Pure Data Packet module. DPD base class implementation.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
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
 *
 */


#include "pdp_dpd_base.h"
#include "pdp_internals.h"


#define THIS(b) t_pdp_dpd_base *b = (t_pdp_dpd_base *)x


#ifdef __cplusplus
extern "C"
{
#endif



/* PRIVATE METHODS */




/* dpd packet context input handler */
static void _pdp_dpd_base_context_input(t_pdp_dpd_base *b, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int i;

    //post ("pdp_dpd_base_context_input: got %s %d", s->s_name, p);

    /* sources/sinks have active inlet disabled */
    if (b->b_dpd_active_inlet_disabled) return;

    /* handle inspect message */
    if (s == S_INSPECT){

	/* store packet for inspector */
	b->b_context_packet = p;

      	/* add inspector to pdp queue
	   this is special: it doesn't use a command object */
	pdp_dpd_base_queue_command(b, b, b->b_inspector_method, b->b_inspector_callback, 0);
    }

    /* handle accumulate message */
    if (s == S_ACCUMULATE){

	/* store context for accumulator methods */
	b->b_context_packet = p;

	/* call bang */
	pdp_dpd_base_bang(b);


    }

}

/* default command object (returns self) */
void *_pdp_dpd_base_get_command_object(void *x){return x;}

/* PUBLIC METHODS */


void pdp_dpd_base_queue_command(void *x, void *c, t_pdp_method process, 
				t_pdp_method callback, int *id)
{
    THIS(b);
    t_pdp_procqueue *q = pdp_base_get_queue(x);
    pdp_procqueue_add(q, c, process, callback, id);
    
}

/* bang method (propagate context to outlet) : it is not registered as a pd message by default ! */
void pdp_dpd_base_bang(void *x)
{
    THIS(b);
    int i, id;
    void *cobj;

    /* move passive pdp packets in place */
    pdp_base_movepassive(x);
    
    /* get command object (or use self) */
    cobj = b->b_command_factory_method ? (b->b_command_factory_method)(b) : b;
    //post(" command object is %x. object is %x", cobj, b);


    /* queue acc method & propagate for all outlets */
    for (i=b->b_nb_context_outlets; i--;){

	
	/* propagate the context packet to the outlet */
	if (b->b_outlet_enable[i]){
	    pdp_dpd_base_queue_command(x, cobj, b->b_accum_method[i], b->b_accum_callback[i], 0);
	    outlet_dpd(b->b_context_outlet[i], b->b_context_packet);
	}
	else{
	    //post("outlet %d disabled", i);
	}
    }

    /* queue cleanup method */
    if (b->b_cleanup_method)
	//pdp_procqueue_add(b->b_q, b, b->b_cleanup_method, 0, &b->b_cleanup_queue_id);
	pdp_dpd_base_queue_command(x, cobj, b->b_cleanup_method, b->b_cleanup_callback, 0);

    /* send communication complete notify */
    if (b->b_complete_notify)
	(b->b_complete_notify)(x);

}

/* get/set context packet */
int pdp_dpd_base_get_context_packet(void *x){
    THIS(b);
    return b->b_context_packet;
}
int pdp_dpd_base_move_context_packet(void *x){
    THIS(b);
    int p =  b->b_context_packet;
    b->b_context_packet = -1;
    return p;
}

void pdp_dpd_base_set_context_packet(void *x, int p){
    THIS(b);
    pdp_packet_mark_unused(b->b_context_packet);
    b->b_context_packet = p;
}

/* add a cleanup callback (called after all propagation is finished) for sources/sinks */
void pdp_dpd_base_add_cleanup(void *x, t_pdp_method cleanup_method, t_pdp_method cleanup_callback)
{
    THIS(b);
    b->b_cleanup_method = cleanup_method;
    b->b_cleanup_callback = cleanup_callback;
    //b->b_cleanup_queue_id = -1;
}

/* add a inspector callback */
void pdp_dpd_base_add_inspector(void *x, t_pdp_method inspector_method)
{
    THIS(b);
    b->b_inspector_method = inspector_method;
    //b->b_inspector_queue_id = -1;
}

/* add a context outlet */
t_outlet *pdp_dpd_base_add_outlet(void *x, t_pdp_method accum_method, t_pdp_method accum_callback)
{
    THIS(b);
    int i = b->b_nb_context_outlets;
    if (i < PDP_DPD_MAX_CONTEXT_OUTLETS){
	b->b_context_outlet[i] = outlet_new((t_object *)b, &s_anything);
	b->b_outlet_enable[i] = 1;
	b->b_accum_method[i] = accum_method;
	b->b_accum_callback[i] = accum_callback;
	//b->b_accum_queue_id[i] = -1;
	b->b_nb_context_outlets++;
	return b->b_context_outlet[i];
    }
    else{
	post("pdp_dpd_base_add_outlet: no more free outlet slots");
	return 0;
    }

}


/* destructor */
void pdp_dpd_base_free(void *x)
{
    THIS(b);

    /* free base */
    pdp_base_free(b);
}


void pdp_dpd_base_disable_active_inlet(void *x)
{
    THIS(b);
    b->b_dpd_active_inlet_disabled = 1;
}



void pdp_dpd_base_enable_outlet(void *x, int outlet, int toggle)
{
    THIS(b);
    if (outlet >=0 && outlet < PDP_DPD_MAX_CONTEXT_OUTLETS){
	b->b_outlet_enable[outlet] = toggle;
    }
	
}


void pdp_dpd_base_register_complete_notify(void *x, t_pdp_method method)
{
    THIS(b);
    b->b_complete_notify = method;
}

void pdp_dpd_base_register_command_factory_method(void *x, t_pdp_newmethod command_factory_method)
{
    THIS(b);
    b->b_command_factory_method = command_factory_method;
}


/* init method */
void pdp_dpd_base_init(void *x)
{
    THIS(b);

    /* super init */
    pdp_base_init(b);

    /* disable pdp messages on active inlet (dpd messages are used as sync) */
    pdp_base_disable_active_inlet(b);

    /* init data */
    b->b_nb_context_outlets = 0;
    b->b_context_packet = -1;
    b->b_cleanup_method = 0;
    //b->b_cleanup_queue_id = -1;
    b->b_inspector_method = 0;
    //b->b_inspector_queue_id = -1;
    b->b_dpd_active_inlet_disabled = 0;

    // default notify == none
    b->b_complete_notify = 0;

    // default command object getter
    b->b_command_factory_method = 0;

}


void pdp_dpd_base_setup(t_class *class)
{

    pdp_base_setup(class);
    class_addmethod(class, (t_method)_pdp_dpd_base_context_input, gensym("dpd"), A_SYMBOL, A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
