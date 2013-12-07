/*
 *   Pure Data Packet - processor queue interface
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


#ifndef PDP_QUEUE_H
#define PDP_QUEUE_H

#include "pdp_pd.h"
#include <pthread.h>

/********************* general purpose pd process queue class *********************/

typedef void (*t_pdpmethod)(void *client);

/* the process queue data record */
typedef struct process_queue_struct
{
  void *x_owner;             /* the object we are dealing with */
  t_pdpmethod x_process;     /* the process method */
  t_pdpmethod x_callback;    /* the function to be called when finished */
  int *x_queue_id;           /* place to store the queue id for task */
} t_process_queue_item;


/* a pd process queue object */
typedef struct _pd_queue
{
    /* clock members */
    t_clock *pdp_clock;
    double deltime;

    /* some bookkeeping vars */
    long long ticks;
    long long packets;

    /* queue members */
    t_process_queue_item *q;    /* queue */
    int mask;
    int head;              /* last entry in queue + 1 */
    int tail;              /* first entry in queque */
    int curr;              /* the object currently processed in other thread */

    /* pthread vars */
    pthread_mutex_t mut;
    pthread_cond_t cond_dataready;
    pthread_cond_t cond_processingdone;
    pthread_t thread_id;
    
    /* toggle for thread usage */
    int use_thread;

} t_pdp_procqueue;


/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif


/* returns 1 if full, 0 if there's space available */
int pdp_procqueue_full(t_pdp_procqueue *q);


void pdp_procqueue_flush(t_pdp_procqueue *q);
void pdp_procqueue_wait(t_pdp_procqueue *q);
void pdp_procqueue_finish(t_pdp_procqueue *q, int indx);
void pdp_procqueue_add(t_pdp_procqueue *q, void *owner, void *process, void *callback, int *queue_id);
void pdp_procqueue_use_thread(t_pdp_procqueue* q, int t);
void pdp_procqueue_init(t_pdp_procqueue *q, double milliseconds, int logsize);

/********************* interface to pdp process queue singleton *********************/

/* processor queue methods, callable from main pd thread */

/* get the default queue */
t_pdp_procqueue *pdp_queue_get_queue(void);



#if 1 

/* add a method to the processing queue */
void  pdp_queue_add(void *owner, void *process, void *callback, int *queue_id);

/* halt main tread until processing is done */
void pdp_queue_wait(void);

/* halt main tread until processing is done and remove 
   callback from queue(for destructors) */
void pdp_queue_finish(int queue_id);

#endif


/* misc signals to pdp */
void pdp_control_notify_drop(int packet);



#ifdef __cplusplus
}
#endif

#endif 
