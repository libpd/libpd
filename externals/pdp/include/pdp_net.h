#ifndef __PDP_UDP_H_
#define __PDP_UDP_H_

/*
 *   Pure Data Packet header: UDP protocol for raw packets
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

     This file contains the specification of the pdp UDP transport protocol.
     It is a very basic binary protocol, not very fool proof.

     The protocol:

     A header packet is transmitted first. This contains mime type information,
     and the size of and number of packets to be received.

     The connection id:

     Currently it is just a random number from the libc rand() function
     this should be accurate enough.


*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* some defs */
#define MAX_UDP_PACKET 1472
#define RESEND_MAX_CHUNKS ((MAX_UDP_PACKET - sizeof(t_pdp_udp_header))/sizeof(unsigned int))

#define PDP_UDP_VERSION 1

/* a pdp udp packet is prepended with this header */

typedef struct _pdp_udp_header
{
    char signature[4];                  /* must be "PDP" */;
    unsigned int version;               /* protocol version */
    unsigned int connection_id;         /* the 'connection' id */ 
    unsigned int sequence_number;       /* sequence number. negative: control packet: body contains meta info */

} t_pdp_udp_header;

/* the data part for a new connection */

#define PDP_UDP_NEW -1 /* start a new transmission */
typedef struct _pdp_udp_new
{
    unsigned int data_size;             /* the size of the packets */
    unsigned int nb_chunks;             /* number of chunks in pdp to be transmitted */
    unsigned int chunk_size;            /* the maximum chunk size */
    char type[0];                       /* the packet type */

    // the tail part is the mime type, for creation and reassembly
} t_pdp_udp_newpacket;

#define PDP_UDP_DONE   -2 /* transmission is done */

#define PDP_UDP_RESEND -3 /* request retransmission of certain chunks. empty: transmission ok */
#define PDP_UDP_ACKNEW -4 /* acknowledge reception of new packet header */


/* receiver and sender classes (transport layer) */

#define PDP_UDP_BUFSIZE 0xF000

/* RECEIVER */
typedef struct _pdp_udp_receiver
{

    // buffer for receiving
    t_pdp_udp_header x_header;  //pdp over udp header
    char x_buf[PDP_UDP_BUFSIZE];        //send buffer
    unsigned int x_zero_terminator; // to prevent runaway strings
    unsigned int x_buf_size;    //size of the received data in the buffer (excluding the header)

    // buffer for sending
    t_pdp_udp_header x_resend_header;                   // header of the resend packet
    unsigned int x_resend_chunks[RESEND_MAX_CHUNKS];    // body contains the chunks to resend
    unsigned int x_resend_udp_packet_size;

    // transmission info
    unsigned int x_connection_id;
    unsigned int x_nb_chunks;
    unsigned int x_chunk_size;
    unsigned int *x_chunk_list;
    char *x_data_type;
    unsigned int x_data_size;
    void *x_data;
    struct sockaddr_in x_source_socket;
    socklen_t x_sslen;
    int x_receive_finished;
    int x_packet_transferred;

    int x_socket;             //socket used for sending
    struct sockaddr_in x_sa;  //address struct
    
} t_pdp_udp_receiver;

/* setup */
t_pdp_udp_receiver *pdp_udp_receiver_new(int port);
void pdp_udp_receiver_free(t_pdp_udp_receiver *x);

/* reset connection (wait for new packet) */
void pdp_udp_receiver_reset(t_pdp_udp_receiver *x);

/* receive, returns 1 on success, 0 on timeout, -1 on error */
int pdp_udp_receiver_receive(t_pdp_udp_receiver *x, unsigned int timeout_ms);

/* get meta & data */
char *pdp_udp_receiver_type(t_pdp_udp_receiver *x);
unsigned int pdp_udp_receiver_size(t_pdp_udp_receiver *x);
void *pdp_udp_receiver_data(t_pdp_udp_receiver *x);



/* SENDER */
typedef struct _pdp_udp_sender
{
    // desired udp packet size
    unsigned int x_udp_payload_size;

    // current packet && communication info
    unsigned int x_connection_id;
    char *x_data_type;
    void *x_data;
    unsigned int x_data_size;
    unsigned int x_chunk_size;
    unsigned int *x_chunk_list;
    unsigned int x_nb_chunks;
    unsigned int x_chunk_list_size;

    // connection data
    int x_socket;             //socket used for sending
    struct sockaddr_in x_sa;  //address struct
    unsigned int x_sleepgrain_us;      //pause between sends (the poor man's flow control) (0 == no sleep)
    unsigned int x_sleep_count;
    unsigned int x_sleep_period;
    unsigned int x_timeout_us;

    // temp buffer for sending
    t_pdp_udp_header x_header;                          
    char x_buf[PDP_UDP_BUFSIZE];                                
    unsigned int x_buf_size;

    // temp buffer for receiving
    t_pdp_udp_header x_resend_header;                   
    unsigned int x_resend_chunks[RESEND_MAX_CHUNKS];
    unsigned int x_resend_items;


} t_pdp_udp_sender;

/* some flow control variables */
void pdp_udp_sender_timeout_us(t_pdp_udp_sender *x, unsigned int timeout_us);
void pdp_udp_sender_sleepgrain_us(t_pdp_udp_sender *x, unsigned int sleepgrain_us);
void pdp_udp_sender_sleepperiod(t_pdp_udp_sender *x, unsigned int sleepperiod);
void pdp_udp_sender_udp_packet_size(t_pdp_udp_sender *x, unsigned int udp_packet_size);

/* setup */
t_pdp_udp_sender *pdp_udp_sender_new(void);
void pdp_udp_sender_free(t_pdp_udp_sender *x);

/* connect */
void pdp_udp_sender_connect(t_pdp_udp_sender *x, char *host, unsigned int port);

/* send, returns 1 on success, 0 on error */
int pdp_udp_sender_send(t_pdp_udp_sender *x, char* type, unsigned int size, void *data);



#endif
