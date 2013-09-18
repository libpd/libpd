/*
 *   Pure Data Packet - processor queue module.
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
   this is a the processor queue pdp system module 
   it receives tasks from objects that are schedules to 
   be computed in another thread. the object is signalled back
   when the task is completed, using a polling mechanism
   based on a pd clock.

   the queue object can be reused. the pdp system however only
   has one instance (one pdp queue. pdp remains a serial program, though
   it can run in a separate thread)

 */


#include <string.h>

#include "pdp_queue.h"
#include "pdp_mem.h"


#define D if (0)

#ifdef __cplusplus
extern "C"
{
#endif

#define PDP_QUEUE_LOGSIZE 10
#define PDP_QUEUE_DELTIME 10.0f


/* there are 3 synchro methods, which can be used i.e. to ensure
   all processing is done before shared resources are freed.

   all 3 wait for the processing thread to finish, and

   _wait:   leaves callback queue untouched
   _finish: clears the queue_id item in the callback queue
   _flush:  waits for thread and calls callbacks
            and loops until callback list is empty

*/
   


/********************* general purpose pd process queue class *********************/

void pdp_procqueue_wait(t_pdp_procqueue *q)
{
    D post("pdp_procqueue_wait(%x): waiting for pdp_queue_thread to finish processing", q);
    pthread_mutex_lock(&q->mut);
    while(((q->curr - q->head) & q->mask) != 0){

	  pthread_cond_wait(&q->cond_processingdone, &q->mut);
    }
    pthread_mutex_unlock(&q->mut);
    D post("pdp_procqueue_wait(%x): pdp_procqueue_thread has finished processing", q);

}
void pdp_procqueue_finish(t_pdp_procqueue *q, int indx)
{

  if (-1 == indx) {
      //post("pdp_pq_remove: index == -1");
      return;
  }
  /* wait for processing thread to finish*/
  pdp_procqueue_wait(q);

  /* invalidate callback at index */
  q->q[indx & q->mask].x_callback = 0;
  q->q[indx & q->mask].x_queue_id = 0;

}

static void pdp_procqueue_callback (t_pdp_procqueue *q);

void pdp_procqueue_flush(t_pdp_procqueue *q)
{
    /* wait once */
    pdp_procqueue_wait(q);

    do {

	/* process callbacks and wait again
	   in case the callbacks introduced new tasks */
	pdp_procqueue_callback(q);
	pdp_procqueue_wait(q);
	
    }
    /* repeat if callback list is not empty */
    while ((q->curr - q->head) & q->mask);

    D post("pdp_procqueue_flush: done");
}

static void pdp_procqueue_signal_processor(t_pdp_procqueue *q)
{

    //NOTE: uncommenting these post statements causes a libc crash
    //in mutex lock in putc
    //D post("pdp_procqueue_signal_processor(%x): signalling process thread", q);
    pthread_mutex_lock(&q->mut);
    pthread_cond_signal(&q->cond_dataready);
    pthread_mutex_unlock(&q->mut);
    //D post("pdp_procqueue_signal_processor(%x): signalling done", q);


}

static void pdp_procqueue_wait_for_feeder(t_pdp_procqueue *q)
{


    /* only use locking when there is no data */
    if(((q->curr - q->head) & q->mask) == 0){

	/* signal processing done */
	D post("pdp_procqueue_wait_for_feeder(%x): signalling processing is done", q);
	pthread_mutex_lock(&q->mut);
	pthread_cond_signal(&q->cond_processingdone);

	/* wait until there is an item in the queue */
	while(((q->curr - q->head) & q->mask) == 0){
	    pthread_cond_wait(&q->cond_dataready, &q->mut);
	}

	pthread_mutex_unlock(&q->mut);
	D post("pdp_procqueue_wait_for_feeder(%x): waiting done", q);

    }
}


int pdp_procqueue_full(t_pdp_procqueue *q)
{
    return (1 == ((q->tail - q->head) & q->mask));
}


void pdp_procqueue_add(t_pdp_procqueue *q, void *owner, void *process, void *callback, int *queue_id)
{
    int i;

    /* if processing is in not in thread, just call the funcs */
    if (!q->use_thread){
	D post("pdp_procqueue_add(%q): calling processing routine directly", q);
	if (queue_id) *queue_id = -1;
	if (process)  ((t_pdpmethod) process)(owner);
	if (callback) ((t_pdpmethod) callback)(owner);
	return;
    }
	

    /* if queue is full, print an error message and return */
    if (pdp_procqueue_full(q)) {
	post("pdp_procqueue_add: WARNING: processing queue (%x) is full.\n", q);
	post("pdp_procqueue_add: WARNING: tail %08x, head %08x (%08x), mask %08x.\n", q->tail, q->head, q->head & q->mask, q->mask);
	post("pdp_procqueue_add: WARNING: skipping process method, calling callback directly.\n");
	if (queue_id) *queue_id = -1;
	if (callback) ((t_pdpmethod) callback)(owner);
	return;
	//exit(1);
    }

    /* schedule method in thread queue */
    i = q->head & q->mask;
    q->q[i].x_owner = owner;
    q->q[i].x_process = process;
    q->q[i].x_callback = callback;
    q->q[i].x_queue_id = queue_id;
    if (queue_id) *queue_id = i;
    //post("pdp_queue_add: added method to queue, index %d", i);

      
    // increase the packet count
    q->packets++;
  
    // move head forward
    q->head++;

    pdp_procqueue_signal_processor(q);

}


/* processing thread */
static void *pdp_procqueue_thread(void *vq)
{
    t_pdp_procqueue *q = (t_pdp_procqueue *)vq;

    D post("pdp_procqueue_thread(%x): thread started", q);

    while(1){
	t_process_queue_item *p;


	D post("pdp_procqueue_thread(%x): waiting for feeder", q);

	/* wait until there is data available */
	pdp_procqueue_wait_for_feeder(q);      


	D post("pdp_procqueue_thread(%x): processing %d", q, q->curr & q->mask);

	
	/* call the process routine */
	p = &q->q[q->curr & q->mask];
	if (p->x_process) 
	    (p->x_process)(p->x_owner);

	/* advance */
	q->curr++;


    }
    return 0;
}


/* call back all the callbacks */
static void pdp_procqueue_callback (t_pdp_procqueue *q)
{

  /* call callbacks for finished packets */
  while(0 != ((q->curr - q->tail) & q->mask))
    {
      int i = q->tail & q->mask;
      /* invalidate queue id */
      if(q->q[i].x_queue_id) *q->q[i].x_queue_id = -1;
      /* call callback */
      if(q->q[i].x_callback) (q->q[i].x_callback)(q->q[i].x_owner);
      //else post("pdp_pq_tick: callback %d is disabled",i );
      q->tail++;
    }

}

/* the clock method */
static void pdp_procqueue_tick (t_pdp_procqueue *q)
{
  /* do work */
  //if (!(ticks % 1000)) post("pdp tick %d", ticks);

  if (!q->use_thread) return;

  /* call callbacks */
  pdp_procqueue_callback(q);

  /* increase counter */
  q->ticks++;

  /* set clock for next update */
  clock_delay(q->pdp_clock, q->deltime);
}



void pdp_procqueue_use_thread(t_pdp_procqueue* q, int t)
{
    /* if thread usage is being disabled, 
       wait for thread to finish processing first */
    if (t == 0) {
	pdp_procqueue_wait(q);
	q->use_thread = 0;
	pdp_procqueue_callback(q);
	clock_unset(q->pdp_clock);
    }
    else {
	clock_unset(q->pdp_clock);
	clock_delay(q->pdp_clock, q->deltime);
	q->use_thread = 1;
    }

}

void pdp_procqueue_init(t_pdp_procqueue *q, double milliseconds, int logsize)
{
    pthread_attr_t attr;
    int size = 1 << logsize;

    /* setup pdp queue processor object */
    q->ticks = 0;
    q->deltime = milliseconds;

    /* setup queue data */
    q->mask = size - 1;
    q->head = 0;
    q->tail = 0;
    q->curr = 0;
    q->q = pdp_alloc(size * sizeof(t_process_queue_item));
    memset(q->q, 0, size * sizeof(t_process_queue_item));

    /* enable threads */
    q->use_thread = 1;

    /* setup synchro stuff */
    pthread_mutex_init(&q->mut, NULL);
    pthread_cond_init(&q->cond_dataready, NULL);
    pthread_cond_init(&q->cond_processingdone, NULL);

 
    /* allocate the clock */
    q->pdp_clock = clock_new(q, (t_method)pdp_procqueue_tick);

    /* set the clock */
    clock_delay(q->pdp_clock, 0);

    /* start processing thread */

    /* glibc doc says SCHED_OTHER is default,
       but it seems not to be when initiated from a RT thread
       so we explicitly set it here */
    pthread_attr_init (&attr);
    //pthread_attr_setschedpolicy(&attr, SCHED_FIFO); 
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER); 

    D post("pdp_procqueue_init(%x): starting thread", q);
    pthread_create(&q->thread_id, &attr, pdp_procqueue_thread, (void *)q);
    D post("pdp_procqueue_init(%x): back in pd thread", q);

    /* wait for processing thread to finish */
    //pdp_procqueue_wait(q);

    /* set default disable/enable thread here */
    //post("pdp_queue: THREAD PROCESSING ON BY DEFAULT!!");
    pdp_procqueue_use_thread(q,0);

}




/* the (static) pdp queue object */
static t_pdp_procqueue pdp_queue;


/* get the default queue */
t_pdp_procqueue *pdp_queue_get_queue(void){return &pdp_queue;}


#if 1
/* default pdp queue shortcut methods */
void pdp_queue_wait() {pdp_procqueue_wait(&pdp_queue);}
void pdp_queue_finish(int indx) { pdp_procqueue_finish(&pdp_queue, indx);}
void pdp_queue_add(void *owner, void *process, void *callback, int *queue_id) {
    pdp_procqueue_add(&pdp_queue, owner, process, callback, queue_id);
}
void pdp_queue_use_thread(int t) {pdp_procqueue_use_thread(&pdp_queue, t);}
void pdp_queue_setup(void){
    pdp_procqueue_init(&pdp_queue, PDP_QUEUE_DELTIME, PDP_QUEUE_LOGSIZE);
    pdp_procqueue_use_thread(&pdp_queue,0);
}
#endif







#ifdef __cplusplus
}
#endif
