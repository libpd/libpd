/*
 *   Pure Data Packet module. Raw packet input
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


#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include "pdp_pd.h"
#include "pdp_debug.h"
#include "pdp_list.h"
#include "pdp_comm.h"
#include "pdp_post.h"
#include "pdp_packet.h"


#define PERIOD 1.0f
#define D if (1)


/* TODO: add flow control 

reminder: pthread condition values for synchro

writing to a queue goes like this:
* pthread_mutex_lock
* atomic op (write)
* pthread_cond_signal
* pthread_mutex_unlock

reading from a queue goes like this:
* pthread_mutex_lock
* while (!CONDITION) pthread_cond_wait
* atomic op (read)
* pthread_mutex_unlock


in this case, there is a reader and a writer, AND a maximum
size of the buffer between them (1 atom) so both reader and writer
might block. compare this to the 'command queue' in libpf, where
only the reader blocks.

so the course of events is:

READER
* wait for data ready (COND_READ) this blocks pd, it is assumed data is always ready in normal operation
* consume
* signal writer (COND_WRITE)


WRITER
* wait for space ready (COND_WRITE)
* write
* signal reader


one remark though: 
is this machinery really necessary?
-----------------------------------

it might be easier to just read in the pd thread.
the only problem this gives is when data really arrives not in a single chunk, but as a stream.
to me that seems a really awkward special case which can be solved using another PROCESS to
do the buffering / dropping.

so, for now, it's just synchronous, no threads for sync reading.

*/



/* raw input from a unix pipe */

typedef struct rawin_struct
{
    /* pd */
    t_object x_obj;
    t_outlet *x_outlet;
    t_outlet *x_sync_outlet;
    t_clock *x_clock;

    /* comm */
    t_pdp_list *x_queue; // packet queue
    int x_pipefd;

    /* thread */
    pthread_mutex_t x_mut;
    pthread_attr_t x_attr;
    pthread_t x_thread;
    

    /* sync */
    int x_mode;    // 1-> sync to input
    int x_giveup;  // 1-> terminate reader thread
    int x_active;  // 1-> reader thread is launched

    /* config */
    t_symbol *x_pipe;
    t_pdp_symbol *x_type;

} t_rawin;


static inline void lock(t_rawin *x){pthread_mutex_lock(&x->x_mut);}
static inline void unlock(t_rawin *x){pthread_mutex_unlock(&x->x_mut);}

static void rawin_close(t_rawin *x);




static int read_packet(t_rawin *x){

    int packet = -1;
    void *data = 0;
    int left = -1;
    int period_sec;
    int period_usec;

    /* create packet */
    if (-1 != packet){
        pdp_post("WARNING: deleting stale packet");
        pdp_packet_mark_unused(packet);
    }
    packet = pdp_factory_newpacket(x->x_type);
    if (-1 == packet){
        pdp_post("ERROR: can't create packet. type = %s", x->x_type->s_name);
        goto close;
    }
    
    /* fill packet */
    data = pdp_packet_data(packet);
    left = pdp_packet_data_size(packet);
    // D pdp_post("packet %d, data %x, size %d", packet, data, left);

    /* inner loop: pipe reads */
    while(left){

        fd_set inset;
	struct timeval tv = {0,10000};

	/* check if we need to stop */
	if (x->x_giveup){
	    pdp_packet_mark_unused(packet);
	    goto close;
	}
	/* select, with timeout */
	FD_ZERO(&inset);
	FD_SET(x->x_pipefd, &inset);
	if (-1 == select(x->x_pipefd+1, &inset, NULL,NULL, &tv)){
	    pdp_post("select error");
	    goto close;
	}

	/* if ready, read, else retry */
	if (FD_ISSET(x->x_pipefd, &inset)){
	    int bytes = read(x->x_pipefd, data, left);
	    if (!bytes){
	        /* if no bytes are read, pipe is closed */
	        goto close;
	    }
	    data += bytes;
	    left -= bytes;
	}
    }
    return packet;
 close:
    return -1;
}


/* reader thread syncs to pipe */
static void *rawin_thread(void *y)
{
    t_rawin *x = (t_rawin *)y;
    int packet = -1;

    /* loop until error or close */
    while (-1 != (packet = read_packet(x))) {
      lock(x);
      pdp_list_add_back(x->x_queue, a_packet, (t_pdp_word)packet);
      unlock(x);
    }

    return 0;
}

/* sync to stream:
   tick polls the receive queue */
static void rawin_tick(t_rawin *x)
{
    int p = -1;
 
    /* send all packets in queue to outlet */
    while (x->x_queue->elements){
        lock(x);
	//pdp_post("%d", x->x_queue->elements);
        p = pdp_list_pop(x->x_queue).w_packet;
        unlock(x);
	//pdp_post_n("%d ", p); 
	pdp_packet_pass_if_valid(x->x_outlet, &p);
	//pdp_post("%d",p);
    }
    clock_delay(x->x_clock, PERIOD);

}

/* sync to bang:
   this runs the reader in the pd thread
 */
static void rawin_bang(t_rawin *x){
  if (!x->x_active) return;
  if (x->x_mode) return;
  int packet = read_packet(x);
  if (-1 == packet) rawin_close(x); // close on error
  pdp_packet_pass_if_valid(x->x_outlet, &packet);
  
}





static void rawin_type(t_rawin *x, t_symbol *type)
{
    x->x_type = pdp_gensym(type->s_name);
}

static void rawin_close(t_rawin *x)
{

    if (!x->x_active) return;

    /* stop thread: set giveup + wait */
    x->x_giveup = 1;
    if (x->x_mode) pthread_join(x->x_thread, NULL);
    x->x_active = 0;

    /* close pipe */
    close(x->x_pipefd);

    /* notify */
    outlet_bang(x->x_sync_outlet);
    pdp_post("pdp_rawin: connection to %s closed", x->x_pipe->s_name);
    
}


static void rawin_open(t_rawin *x, t_symbol *spipe)
{
    /* save pipe name if not empty */
    if (spipe->s_name[0]) {x->x_pipe = spipe;}

    if (x->x_active) {
	pdp_post("pdp_rawin: already open");
	return;
    }

    /* open pipe */
    if (-1 == (x->x_pipefd = open(x->x_pipe->s_name, O_RDONLY|O_NONBLOCK))){
	perror(x->x_pipe->s_name);
	return;
    }

    /* thread control vars */
    x->x_giveup = 0;
    x->x_active = 1;

    /* start thread if sync mode */
    if (x->x_mode) 
      pthread_create(&x->x_thread, &x->x_attr, rawin_thread , x);
    
}


static void rawin_sync(t_rawin *x, t_float fmode){
    rawin_close(x);
    x->x_mode = (int)fmode;
}


static void rawin_free(t_rawin *x)
{
    rawin_close(x);
    clock_free(x->x_clock);
    pdp_tree_strip_packets(x->x_queue);
    pdp_tree_free(x->x_queue);
}

t_class *rawin_class;


static void *rawin_new(t_symbol *spipe, t_symbol *type)
{
    t_rawin *x;

    pdp_post("%s %s", spipe->s_name, type->s_name);

    /* allocate & init */
    x = (t_rawin *)pd_new(rawin_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_sync_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_clock = clock_new(x, (t_method)rawin_tick);
    x->x_queue = pdp_list_new(0);
    x->x_active = 0;
    x->x_giveup = 0;
    x->x_mode = 0;
    x->x_type = pdp_gensym("image/YCrCb/320x240"); //default
    x->x_pipe = gensym("/tmp/pdpraw"); // default
    pthread_attr_init(&x->x_attr);
    pthread_mutex_init(&x->x_mut, NULL);
    clock_delay(x->x_clock, PERIOD);

    /* args */
    rawin_type(x, type);
    if (spipe->s_name[0]) x->x_pipe = spipe; 

    return (void *)x;

}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_rawin_setup(void)
{
    int i;

    /* create a standard pd class: [pdp_rawin pipe type] */
    rawin_class = class_new(gensym("pdp_rawin"), (t_newmethod)rawin_new,
   	(t_method)rawin_free, sizeof(t_rawin), 0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);

    /* add global message handlers */
    class_addmethod(rawin_class, (t_method)rawin_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(rawin_class, (t_method)rawin_open, gensym("open"), A_DEFSYMBOL, A_NULL);
    class_addmethod(rawin_class, (t_method)rawin_close, gensym("close"), A_NULL);
    class_addmethod(rawin_class, (t_method)rawin_sync, gensym("sync"), A_FLOAT, A_NULL);
    class_addmethod(rawin_class, (t_method)rawin_bang, gensym("bang"), A_NULL);


}

#ifdef __cplusplus
}
#endif
