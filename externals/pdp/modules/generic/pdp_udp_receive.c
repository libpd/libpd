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


/* this module sends receives an udp packet stream and converts to pdp packet */

#include "pdp_net.h"
#include "pdp.h"
#include "pdp_resample.h"

#define D if(0)

typedef struct pdp_udp_receive_struct
{

    t_object x_obj;
    t_float x_f;

    /* receiver object */
    t_pdp_udp_receiver *x_receiver;


    /* thread vars */
    pthread_attr_t x_attr;
    pthread_t x_thread;
    int x_exit_thread;

    /* packet queue */
    int x_index;
    int x_packet[2];    

    /* polling clock */
    t_clock *x_clock;
    /* outlet */
    t_outlet *x_outlet0;

} t_pdp_udp_receive;


static void clock_tick(t_pdp_udp_receive *x)
{
    /* poll for new packet */
  
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet[!x->x_index]);
    clock_delay(x->x_clock, 1.0f);
}




static void *receive_thread(void *threaddata)
{
    t_pdp_udp_receive *x = (t_pdp_udp_receive *)threaddata;
    t_pdp *pdp_header = 0;
    void *pdp_data = 0;
    int tmp_packet = -1;
    char *type = 0;
    unsigned int size = 0;

    /* listen for packets */
    while (!x->x_exit_thread){
	

	switch(pdp_udp_receiver_receive(x->x_receiver, 100)){
	case -1:
	    /* error */
	    goto exit;
	case 0:
	    /* timeout */
	    continue;
	case 1:
	    /* data ready */
	    break;
	}

        /* create a new packet */
	type = pdp_udp_receiver_type(x->x_receiver);
        tmp_packet = pdp_factory_newpacket(pdp_gensym(type));
        pdp_header = pdp_packet_header(tmp_packet);
        pdp_data = pdp_packet_data(tmp_packet);

        /* check if we were able to create the pdp packet */
        if (!(pdp_header && pdp_data)){
            post("pdp_netreceive: can't create packet (type %s)", type);
	    pdp_udp_receiver_reset(x->x_receiver);
	    continue;
        }

	/* check size */
	size = pdp_udp_receiver_size(x->x_receiver);
	if ((pdp_header->size - PDP_HEADER_SIZE) != size){
	    pdp_packet_mark_unused(tmp_packet);
	    tmp_packet = -1;
            post("pdp_netreceive: invalid packet size %d (pdp packet size = %d)", 
		 size, pdp_header->size - PDP_HEADER_SIZE);
	    continue;
	}

	/* copy the data */
	memcpy(pdp_data, pdp_udp_receiver_data(x->x_receiver), size);

	/* copy the packet into queue */
	x->x_index ^= 1;
        pdp_packet_mark_unused(x->x_packet[x->x_index]);
	x->x_packet[x->x_index] = tmp_packet;
    

    }

 exit:
    post("thread exiting");
    return 0;
}


static void pdp_udp_receive_free(t_pdp_udp_receive *x)
{
    int i;
    void* retval;
    x->x_exit_thread = 1; // wait for thread to finish
    pthread_join(x->x_thread, &retval);
    
    pdp_udp_receiver_free(x->x_receiver);

    pdp_packet_mark_unused(x->x_packet[0]);
    pdp_packet_mark_unused(x->x_packet[1]);

}

t_class *pdp_udp_receive_class;



void *pdp_udp_receive_new(t_floatarg fport)
{
    int i;
    int port;
    struct hostent *hp;

    t_pdp_udp_receive *x = (t_pdp_udp_receive *)pd_new(pdp_udp_receive_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet[0] = -1;
    x->x_packet[1] = -1;
    x->x_index = 0;

    port = (fport == 0.0f) ? 7777 : fport;
    x->x_receiver = pdp_udp_receiver_new(port);

    /* setup thread stuff & create thread */
    x->x_exit_thread = 0;
    pthread_attr_init(&x->x_attr);
    pthread_attr_setschedpolicy(&x->x_attr, SCHED_OTHER); 
    pthread_create(&x->x_thread, &x->x_attr, receive_thread, x);


    /* setup the clock */
    x->x_clock = clock_new(x, (t_method)clock_tick);
    clock_delay(x->x_clock, 0);

    post("pdp_netreceive: WARNING: experimental object");

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_udp_receive_setup(void)
{


    pdp_udp_receive_class = class_new(gensym("pdp_netreceive"), (t_newmethod)pdp_udp_receive_new,
    	(t_method)pdp_udp_receive_free, sizeof(t_pdp_udp_receive), 0, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
