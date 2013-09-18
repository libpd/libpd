/*
 *   Pure Data Packet header file. DPD base class
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


/* dpd base class for context based processors (for pd-fying standard stack based serial languages)
   not all of pdp is used here, but the class is derived from the pdp base class to enable mixing
   standard pdp (data flow packet processing) and dpd (context based serial languages)
   
   dpd is short for "upside down pdp". the name stems from the observation
   of opengl code in pd (like in gem) having an upside down feel when looking
   through dataflow eyes. i.e.: the target (window context) is on top, while
   the objects (geos) are at the bottom connected by a chain of geometric transforms

   the principles of dpd are simple:

   * there is only one main packet, received on the left inlet.
   * this packet is called the "context" and is produced by and returned to a top context source/sink
   * additional pd messages and pdp packets can be received on the cold inlets
   * as opposed to pdp, no copies are made of this context packet, all operations on it are accumulative.
   * the protocol is different because fanout is prohibited (so no ro/rw registering)
   * the only exception for fanout are inspectors, which have precedence over normal processors
   * all processors and inspectors for a single context type must use the pdp thread, to preserve execution order
   

*/



#include "pdp_base.h"

#define PDP_DPD_MAX_CONTEXT_OUTLETS 4

typedef struct pdp_dpd_base_struct
{
    t_pdp_base b_base; /* pdp base class */

    int b_nb_context_outlets;
    t_outlet *b_context_outlet[PDP_DPD_MAX_CONTEXT_OUTLETS];  /* dpd outlets */
    int b_outlet_enable[PDP_DPD_MAX_CONTEXT_OUTLETS];  /* dpd outlets */
    t_pdp_method b_accum_method[PDP_DPD_MAX_CONTEXT_OUTLETS]; /* accumulation methods for each outlet */
    t_pdp_method b_accum_callback[PDP_DPD_MAX_CONTEXT_OUTLETS]; /* pd callback methods for each outlet */
    //int b_accum_queue_id[PDP_DPD_MAX_CONTEXT_OUTLETS];        /* accumulator queue id's */

    t_pdp_method b_inspector_method;         /* pdp thread inspector callback */
    t_pdp_method b_inspector_callback;       /* main thread inspector callback */
    //int b_inspector_queue_id;
    
    t_pdp_method b_cleanup_method;         /* queued after propagation is done  */
    t_pdp_method b_cleanup_callback;       /* queued after propagation is done  */
    //int b_cleanup_queue_id;

    t_pdp_method b_complete_notify;         /* method called after packet output is done */
    t_pdp_newmethod b_command_factory_method;      /* command factory method */


    int b_context_packet; /* the current context packet */

    int b_dpd_active_inlet_disabled;


} t_pdp_dpd_base;

#ifdef __cplusplus
extern "C"
{
#endif

/* bang method (propagate context to outlets & register callbacks)
   mainly for context source/sinks
   it is not registered as a pd message by default ! */
void pdp_dpd_base_bang(void *x);

/* get/set the context packet */
int pdp_dpd_base_get_context_packet(void *x);
void pdp_dpd_base_set_context_packet(void *x, int p);
int pdp_dpd_base_move_context_packet(void *x);

/* add a context outlet and it's corresponding accumulation (process) and callback method */
t_outlet *pdp_dpd_base_add_outlet(void *x, t_pdp_method accum_method, t_pdp_method accum_callback);

/* add a cleanup callback (called after all propagation is finished) for sources/sinks */
void pdp_dpd_base_add_cleanup(void *x, t_pdp_method cleanup_method, t_pdp_method accum_callback);

/* add an inspector callback */
void pdp_dpd_base_add_inspector(void *x, t_pdp_method inspector_method);


/* destructor */
void pdp_dpd_base_free(void *x);

/* init method */
void pdp_dpd_base_init(void *x);

/* disable dpd active inlet */
void pdp_dpd_base_disable_active_inlet(void *x);

/* enable/disable outlet */
void pdp_dpd_base_enable_outlet(void *x, int outlet, int toggle);

/* register notify method (called from the end of pdp_dpd_base_bang) */
void pdp_dpd_base_register_complete_notify(void *x, t_pdp_method method);

/* register a command init (factory) method 
   this method should return a command object to place in the queue */
void pdp_dpd_base_register_command_factory_method(void *x, t_pdp_newmethod command_factory_method);

/* class setup method */
void pdp_dpd_base_setup(t_class *class);

/* add a command to the process queue */
void pdp_dpd_base_queue_command(void *x, void *c, t_pdp_method process, 
				t_pdp_method callback, int *id);


/* get/set the queue instance (thread) used for scheduling */
#define pdp_dpd_base_set_queue   pdp_base_set_queue
#define pdp_dpd_base_get_queue   pdp_base_get_queue
#define pdp_dpd_base_queue_wait  pdp_base_queue_wait



#ifdef __cplusplus
}
#endif
