/*
 *   Pure Data Packet module.
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



/* this module sends a pure packet out as an udp packet stream */

#include "pdp_net.h"
#include "pdp.h"
#include "pdp_resample.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>

#define DD if(0)  // print DROP debug info
#define D if(0)  // print extra connection debug info
#define V if(0)  // be verbose (parameter setting feedback)

typedef struct pdp_udp_send_struct
{

    t_object x_obj;
    t_float x_f;

    /* sender object */
    t_pdp_udp_sender *x_sender;

    /* pthread vars */
    pthread_mutex_t x_mut;
    pthread_cond_t x_cond_data_ready;
    pthread_cond_t x_cond_send_done;
    pthread_t x_thread;
    int x_exit_thread;

    // drop info
    unsigned int x_drop;

    t_outlet *x_outlet0;

    // packet queue
    int x_nb_packets;
    int x_read_packet;
    int x_write_packet;
    int *x_packet;    


} t_pdp_udp_send;





/* some synchro code */

static int _wait_for_feeder(t_pdp_udp_send *x)
{

    /* only use locking when there is no data */
    if (x->x_packet[x->x_read_packet] == -1){

	/* signal sending is done */
	pthread_mutex_lock(&x->x_mut);
	pthread_cond_signal(&x->x_cond_send_done);

	/* wait until there is an item in the queue */
	while((x->x_packet[x->x_read_packet] == -1) && (!x->x_exit_thread)){
	    pthread_cond_wait(&x->x_cond_data_ready, &x->x_mut);
	}
	pthread_mutex_unlock(&x->x_mut);

	/* check if we need to stop the thread */
	if (x->x_exit_thread) return 0;

    }

    return !x->x_exit_thread;
}

static void _signal_sender(t_pdp_udp_send *x)
{

    pthread_mutex_lock(&x->x_mut);
    pthread_cond_signal(&x->x_cond_data_ready);
    pthread_mutex_unlock(&x->x_mut);
}

static void _wait_until_done(t_pdp_udp_send *x)
{
    pthread_mutex_lock(&x->x_mut);
    while (x->x_packet[x->x_read_packet] != -1){
	  pthread_cond_wait(&x->x_cond_send_done, &x->x_mut);
    }
    pthread_mutex_unlock(&x->x_mut);
}


static void _remove_packet_from_queue(t_pdp_udp_send *x)
{

}





static void *send_thread(void *threaddata)
{
    t_pdp_udp_send *x = (t_pdp_udp_send *)threaddata;

    /* main thread loop */

    /* get a pdp packet from queue */
    /* send header packet and make sure it has arrived */
    /* send a chunk burst */
    /* send done packet and get the resend list */
    /* repeat until send list is empty */

    while (_wait_for_feeder(x)){
	t_pdp *header;
	void *data;

	/* check if we have a valid pdp packet */
	if ((!(header = pdp_packet_header(x->x_packet[x->x_read_packet])))
	    ||(!(data = pdp_packet_data(x->x_packet[x->x_read_packet])))
	    ||(0 == header->desc)) goto remove; /* nothing to transmit  */

	/* send it */
	pdp_udp_sender_send(x->x_sender, 
			    header->desc->s_name,
			    header->size - PDP_HEADER_SIZE, data); 


      remove:
	/* remove packet from queue */
	pdp_packet_mark_unused(x->x_packet[x->x_read_packet]);
	x->x_packet[x->x_read_packet] = -1;
	x->x_read_packet++;
	x->x_read_packet %= x->x_nb_packets;
   
    }
    return 0;
}


static void pdp_udp_send_input_0(t_pdp_udp_send *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int my_p;
    int transferred = 0;

    if (s== gensym("register_ro")){


	// check if packet can be stored in the queue
	// this is possible if the current write location does not contain a packet

	if (x->x_packet[x->x_write_packet] == -1){

	    // get the packet outside of the lock
	    my_p = pdp_packet_copy_ro(p); 


	    // add to queue (do we really need to lock here?>
	    //pthread_mutex_lock(&x->x_mut); // LOCK
	       x->x_packet[x->x_write_packet] = my_p;
	       x->x_write_packet++;
	       x->x_write_packet %= x->x_nb_packets;
	       transferred = 1;
	    //pthread_mutex_unlock(&x->x_mut); // UNLOCK
	}

	// signal sender if transfer succeded
	if (transferred) _signal_sender(x);

	// else send a float indicating the number of drops so far
	else{
	    x->x_drop++;
	    //outlet_float(x->x_outlet0, (float)x->x_drop);

	    DD post ("pdp_netsend: DROP: queue full");
	}
    }
}



/* some flow control hacks */

static void pdp_udp_send_timeout(t_pdp_udp_send *x, float f)
{
    if (f < 0.0f) f = 0.0f;
    pdp_udp_sender_timeout_us(x->x_sender, 1000.0f * f);
}


static void pdp_udp_send_sleepgrain(t_pdp_udp_send *x, float f)
{
    if (f < 0.0f) f = 0.0f;
    pdp_udp_sender_sleepgrain_us(x->x_sender, 1000.0f * f);
}

static void pdp_udp_send_sleepperiod(t_pdp_udp_send *x, float f)
{
    if (f < 0.0f) f = 0.0f;
    pdp_udp_sender_sleepperiod(x->x_sender, f);
}


static void pdp_udp_send_udpsize(t_pdp_udp_send *x, float f)
{
    if (f < 0.0f) f = 0.0f;
    pdp_udp_sender_udp_packet_size(x->x_sender, f);
}

static void pdp_udp_send_connect(t_pdp_udp_send *x, t_symbol *shost, t_float fport)
{
    unsigned int port;
    struct hostent *hp;

    /* suspend until sending thread is finished */
    _wait_until_done(x);

    /* set target address */
    port = (fport == 0.0f) ? 7777 : fport;
    if (shost == gensym("")) shost = gensym("127.0.0.1");

    /* connect */
    pdp_udp_sender_connect(x->x_sender, shost->s_name, port);

}


static void pdp_udp_send_free(t_pdp_udp_send *x)
{
    int i;
    void* retval;
    _wait_until_done(x);  // send all remaining packets
    x->x_exit_thread = 1; // .. and wait for thread to finish
    _signal_sender(x);
    pthread_join(x->x_thread, &retval);

    pdp_udp_sender_free(x->x_sender);

    
    for (i=0; i<x->x_nb_packets; i++) pdp_packet_mark_unused(x->x_packet[i]);
    pdp_dealloc(x->x_packet);

}

t_class *pdp_udp_send_class;



void *pdp_udp_send_new(void)
{
    int i;
    pthread_attr_t attr;

    t_pdp_udp_send *x = (t_pdp_udp_send *)pd_new(pdp_udp_send_class);

    x->x_sender = pdp_udp_sender_new();
    
    //x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_nb_packets = 4;
    x->x_packet = malloc(sizeof(int)*x->x_nb_packets);
    for (i=0; i<x->x_nb_packets; i++) x->x_packet[i] = -1;
    x->x_read_packet = 0;
    x->x_write_packet = 0;

    x->x_drop = 0;



    /* setup thread stuff & create thread */
    x->x_exit_thread = 0;
    pthread_mutex_init(&x->x_mut, NULL);
    pthread_cond_init(&x->x_cond_data_ready, NULL);
    pthread_cond_init(&x->x_cond_send_done, NULL);
    pthread_attr_init(&attr);
    //pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_create(&x->x_thread, &attr, send_thread, x);
    post("pdp_netsend: WARNING: experimental object");


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_udp_send_setup(void)
{

    pdp_udp_send_class = class_new(gensym("pdp_netsend"), (t_newmethod)pdp_udp_send_new,
    	(t_method)pdp_udp_send_free, sizeof(t_pdp_udp_send), 0, A_NULL);


    class_addmethod(pdp_udp_send_class, (t_method)pdp_udp_send_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_udp_send_class, (t_method)pdp_udp_send_sleepgrain, gensym("sleepgrain"), A_FLOAT, A_NULL);
    class_addmethod(pdp_udp_send_class, (t_method)pdp_udp_send_sleepperiod, gensym("sleepperiod"), A_FLOAT, A_NULL);
    class_addmethod(pdp_udp_send_class, (t_method)pdp_udp_send_udpsize, gensym("udpsize"), A_FLOAT, A_NULL);
    class_addmethod(pdp_udp_send_class, (t_method)pdp_udp_send_timeout, gensym("timeout"), A_FLOAT, A_NULL);
    class_addmethod(pdp_udp_send_class, (t_method)pdp_udp_send_connect, gensym("connect"), A_SYMBOL, A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
