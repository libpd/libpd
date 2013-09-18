/*
 *   Pure Data Packet base class header file.
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
  This file contains the specification of the pdp base class. It is derived
  from t_object, the basic pd object (like any other pd extern). Have a look
  at pdp_add, pdp_gain and pdp_noise to see how to use this.

*/


#define MAX_NB_PDP_BASE_INLETS 4
#define MAX_NB_PDP_BASE_OUTLETS 4

#include "pdp_pd.h"
#include "pdp_symbol.h"
#include "pdp_types.h"
#include "pdp_queue.h"
#include "pdp_comm.h"
#include "pdp_compat.h"
#include "pdp_packet.h"

typedef void (*t_pdp_method)(void *);
typedef void* (*t_pdp_newmethod)(void *);


typedef struct
{
    t_object x_obj;

    int b_inlets;           // the number of pdp inlets
    int b_outlets;          // the number of pdp outlets

                            // registers to store incoming packets
    int b_packet[MAX_NB_PDP_BASE_INLETS];
    int b_packet_next[MAX_NB_PDP_BASE_INLETS];
    t_pdp_symbol *b_type_template[MAX_NB_PDP_BASE_INLETS];

    int b_queue_id;         // task id in process queue (for callback cancelling)
    //int b_dropped;          // indicate if a packet was dropped during register_rw cycle

                            // wil the default (left) active inlet accept pdp messages ?
    int b_active_inlet_enabled;
    int b_active_inlet_readonly;
    
                            // the process callbacks
    t_pdp_method b_process_method; // called in pdp thread
    t_pdp_method b_preproc_method; // called before thread (for packet alloc and checking)
    t_pdp_method b_postproc_method; // called after thread (for outlet stuff other than default active packet->out0)
	                     
                            // packet outlets
    t_outlet *b_outlet[MAX_NB_PDP_BASE_OUTLETS];

    u32 b_channel_mask;     // channel mask

    int b_thread_enabled;   // thread enable switch

    t_pdp_procqueue *b_q;   // queue object

} t_pdp_base;



/* setup base class. call this in your derived class setup method */
void pdp_base_setup(t_class *c);


/* base class constructor/destructor. call this in your base class constructor/destructor */
void pdp_base_init(void *x);
void pdp_base_free(void *x);


/* register processing callbacks */
void pdp_base_set_process_method(void *x, t_pdp_method m); //process callback (called from pdp thread)
void pdp_base_set_preproc_method(void *x, t_pdp_method m); //pre-process callback (called before process from pd thread)
void pdp_base_set_postproc_method(void *x, t_pdp_method m); //post-process callback (called after process from pd thread)


/* configure inlets/outlets */
void pdp_base_add_pdp_inlet(void *x);
t_outlet *pdp_base_add_pdp_outlet(void *x);
void pdp_base_disable_active_inlet(void *x); //use this for pdp generators
void pdp_base_readonly_active_inlet(void *x); //use this for pdp converters ("out of place" processing)
void pdp_base_add_gen_inlet(void *x, t_symbol *from, t_symbol *to); // generic inlet


/* bang method */
void pdp_base_bang(void *x);


/* move delayed passive packets in place */
void pdp_base_movepassive(void *x);



/* packet manipulation methods 
   0   active inlet (left) if enabled
   >0  additional pdp inlets created with pdp_base_add_pdp_inlet */
int pdp_base_get_packet(void *x, int inlet);  // get the packet from an inlet
int pdp_base_move_packet(void *x, int inlet); // same as get, but it removes the reference in the base class
void pdp_base_set_packet(void *x, int inlet, int packet); // set (replace) the active packet (will be sent to outlet)


/* getters for base class data */
u32 pdp_base_get_chanmask(void *x);
t_object *pdp_base_get_object(void *x);


/* thread control */
void pdp_base_disable_thread(void *x);

/* type control */
void pdp_base_set_type_template(void *x, int inlet, t_pdp_symbol *type_template);

/* queue control */
void pdp_base_queue_wait(void *x);
void pdp_base_set_queue(void *x, t_pdp_procqueue *q);
t_pdp_procqueue *pdp_base_get_queue(void *x);
