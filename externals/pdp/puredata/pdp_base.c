/*
 *   Pure Data Packet base class implementation.
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


/*

  This file contains the pdp base class object.
  This is really nothing more than an attempt to stay away from c++
  as far as possible, while having some kind of base class functionality
  for pdp (tucking away the communication & thread protocol).

*/

#include "pdp_base.h"
#include <stdarg.h>


static void pdp_base_debug(t_pdp_base *b, t_floatarg f)
{
    int i;
    post("debug");
    post("inlets: %d", b->b_inlets);
    post("\tpacket\tnext_packet");
    for (i=0; i<b->b_inlets; i++)
	post("\t%d\t%d", b->b_packet[i], b->b_packet_next[i]);
    //post("outlets: %d", b->b_inlets);
}

static void pdp_base_thread(t_pdp_base *b, t_floatarg f)
{
    int i = (int)f;
    if ((i == 0) || (i == 1)) b->b_thread_enabled = i;
}

static void pdp_base_process(t_pdp_base *b)
{

    if (b->b_process_method)
	(*b->b_process_method)(b);
}

/* this method is called after the thread has finished processing */
static void pdp_base_postprocess(t_pdp_base *b)
{
    /* call the derived class postproc callback if there is any */
    if (b->b_postproc_method)
	(*b->b_postproc_method)(b);

    /* unregister (mark unused) packet and propagate if packet is valid */
    if (b->b_outlet[0])
	pdp_pass_if_valid(b->b_outlet[0], &b->b_packet[0]);
}


/* move the passive packets in place */
void pdp_base_movepassive(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    int i;

    /* if a cold packet was received in the meantime
       swap it in, else keep the old one */
    for (i=1; i<b->b_inlets; i++){
	pdp_replace_if_valid(&b->b_packet[i], &b->b_packet_next[i]);
    }
    

}

/* the standard bang method */
void pdp_base_bang(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    int i;

    /* if pdp thread is still processing, do nothing */
    if (-1 != b->b_queue_id) return;

    /* move packets in place */
    pdp_base_movepassive(x);
    

    /* if there is a preproc method defined, call it inside
       the pd thread. (mainly for allocations) */
    if (b->b_preproc_method)
	(*b->b_preproc_method)(b);

    /* check if we need to use pdp queue */
    if (b->b_thread_enabled){

	/* add the process method and callback to the process queue */
	pdp_procqueue_add(b->b_q, b, pdp_base_process, pdp_base_postprocess, &b->b_queue_id);
    }
    else{
	/* call both methods directly */
	pdp_base_process(b);
	pdp_base_postprocess(b);
    }
}

/* hot packet input handler */
void pdp_base_input_hot(t_pdp_base *b, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    /* dont register if active inlet is disabled */
    if (!b->b_active_inlet_enabled) return;

    /* register the packet (readonly or read/write)
       or drop it if we have an active packet 
       if type template is not null, packet will be converted */


    if (b->b_active_inlet_readonly){
	if (s == S_REGISTER_RO){
	    if (b->b_type_template[0]){
		pdp_packet_convert_ro_or_drop(&b->b_packet[0], p, b->b_type_template[0]);
	    }
	    else{
		pdp_packet_copy_ro_or_drop(&b->b_packet[0], p);
	    }
	} 
    }
    else{
	if (s == S_REGISTER_RW) {
	    if (b->b_type_template[0]){
		pdp_packet_convert_rw_or_drop(&b->b_packet[0], p, b->b_type_template[0]);
	    }
	    else{
		pdp_packet_copy_rw_or_drop(&b->b_packet[0], p);
	    }
	}
    }

    /* start processing if there is an active packet to process
       and the processing method is not active */

    if ((s == S_PROCESS) && (-1 != b->b_packet[0]) && (-1 == b->b_queue_id)){
	pdp_base_bang(b);
    }
    //if ((pdp_sym_prc() == s) && (-1 != b->b_packet[0]) && (!b->b_dropped))	pdp_base_bang(b);

}

/* cold packet input handlers */
void pdp_base_input_cold(t_pdp_base *b, t_symbol *s, int ac, t_atom *av)
{

    int p;
    int i;
    char msg[] = "pdp1";
    char *c;

    int inlet;

    //post("pdp_base_input_cold: got packet");

    /* do cheap tests first */
    if (ac != 2) return;
    if (av[0].a_type != A_SYMBOL) return;
    if (av[0].a_w.w_symbol != S_REGISTER_RO) return;
    if (av[1].a_type != A_FLOAT) return;
    p = (int)av[1].a_w.w_float;


    /* check if it's a pdp message
       and determine inlet */
    for (i=1; i<MAX_NB_PDP_BASE_INLETS; i++){
	if (s == gensym(msg)){
	    inlet = i;
	    goto found;
	}
	else{
	    msg[3]++;
	}
    }
    return;
    

 found:

    /* store the packet and trow away 
       the old one, if there is any */

    pdp_packet_copy_ro_or_drop(&b->b_packet_next[inlet], p);
}


void pdp_base_set_process_method(void *x, t_pdp_method m)
{
    t_pdp_base *b = (t_pdp_base *)x;
    b->b_process_method = m;
}

void pdp_base_set_preproc_method(void *x, t_pdp_method m)
{
    t_pdp_base *b = (t_pdp_base *)x;
    b->b_preproc_method = m;
}


void pdp_base_set_postproc_method(void *x, t_pdp_method m)
{
    t_pdp_base *b = (t_pdp_base *)x;
    b->b_postproc_method = m;
}


void pdp_base_queue_wait(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    pdp_procqueue_wait(b->b_q);
}

void pdp_base_set_queue(void *x, t_pdp_procqueue *q)
{
    t_pdp_base *b = (t_pdp_base *)x;
    pdp_base_queue_wait(x);
    b->b_q = q;
}

t_pdp_procqueue *pdp_base_get_queue(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    return b->b_q;
}

void pdp_base_setup(t_class *c)
{

     /* add pdp base class methods */
    class_addmethod(c, (t_method)pdp_base_thread, gensym("thread"), A_FLOAT, A_NULL);
    class_addmethod(c, (t_method)pdp_base_debug, gensym("debug"), A_NULL);

    /* hot packet handler */
    class_addmethod(c, (t_method)pdp_base_input_hot, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    /* cold packet handler */
    class_addanything(c, (t_method)pdp_base_input_cold);
}

/* pdp base instance constructor */
void pdp_base_init(void *x)
{
    int i;
    t_pdp_base *b = (t_pdp_base *)x;

    b->b_channel_mask = -1;

    for(i=0; i<MAX_NB_PDP_BASE_INLETS; i++){
	b->b_packet[i] = -1;
	b->b_packet_next[i] = -1;
	b->b_type_template[i] = 0;
    } 

    b->b_queue_id = -1;
    //b->b_dropped = 0;
    b->b_process_method = 0;
    b->b_preproc_method = 0;
    b->b_inlets = 1;
    b->b_outlets = 0;
    b->b_active_inlet_enabled = 1;
    b->b_active_inlet_readonly = 0;
    b->b_thread_enabled = 1;

    // default queue is pdp queue
    b->b_q = pdp_queue_get_queue();

}

/* base instance destructor */
void pdp_base_free(void *x)
{
    int i;
    t_pdp_base *b = (t_pdp_base *)x;
    /* remove process method from queue before deleting data */
    pdp_procqueue_finish(b->b_q, b->b_queue_id);

    /* delete stuff */
    for(i=0; i<MAX_NB_PDP_BASE_INLETS; i++){
	pdp_packet_mark_unused(b->b_packet[i]);
	pdp_packet_mark_unused(b->b_packet_next[i]);
    } 

}

void pdp_base_readonly_active_inlet(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    b->b_active_inlet_readonly = 1;
}

void pdp_base_disable_active_inlet(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    b->b_active_inlet_enabled = 0;
}


/* add an inlet */
void pdp_base_add_pdp_inlet(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    char s[] = "pdp0";
    s[3] += b->b_inlets;

    if (b->b_inlets < MAX_NB_PDP_BASE_INLETS){
	inlet_new(&b->x_obj, &b->x_obj.ob_pd, gensym("pdp"), gensym(s));
	b->b_inlets++;
    }
    else {
	post("pdp_base_add_pdp_inlet: only %d pdp inlets allowed. ignoring.", MAX_NB_PDP_BASE_INLETS);
    }
}


/* add an outlet: only one allowed */
t_outlet *pdp_base_add_pdp_outlet(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    t_outlet *outlet =  outlet_new(&b->x_obj, &s_anything);

    
    if (b->b_outlets < MAX_NB_PDP_BASE_OUTLETS){
	b->b_outlet[b->b_outlets] = outlet; 
	b->b_outlets++;
    }

    return outlet;

}

void pdp_base_set_packet(void *x, int inlet, int packet)
{
    t_pdp_base *b = (t_pdp_base *)x;

    if (inlet < b->b_inlets){
	//post("%d %d", b->b_packet[inlet], b->b_packet_next[inlet]);
	pdp_packet_mark_unused(b->b_packet[inlet]);
	b->b_packet[inlet] = packet;
    }
} 


int pdp_base_get_packet(void *x, int inlet)
{
    t_pdp_base *b = (t_pdp_base *)x;

    if (inlet < b->b_inlets){
	//post("%d %d", b->b_packet[inlet], b->b_packet_next[inlet]);
	return (b->b_packet[inlet]);
    }

    return -1;
} 

int pdp_base_move_packet(void *x, int inlet)
{
    t_pdp_base *b = (t_pdp_base *)x;
    int p;

    if (inlet < b->b_inlets){
	p = b->b_packet[inlet];
	b->b_packet[inlet] = -1;
	return (p);
    }

    return -1;
} 



t_object *pdp_base_get_object(void *x)
{
    return (t_object *)x;
}

void pdp_base_add_gen_inlet(void *x, t_symbol *from, t_symbol *to)
{
    t_object *o = (t_object *)x;
    inlet_new(o, &o->ob_pd, from, to);
}

void pdp_base_disable_thread(void *x)
{
    
    t_pdp_base *b = (t_pdp_base *)x;
    b->b_thread_enabled = 0;
}

void pdp_base_set_type_template(void *x, int inlet, t_pdp_symbol *type_template)
{
    t_pdp_base *b = (t_pdp_base *)x;
    if (inlet < b->b_inlets){
	b->b_type_template[inlet] = type_template;
    }
}
