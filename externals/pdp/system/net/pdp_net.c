
#include "pdp_net.h"
#include "pdp_debug.h"
#include "pdp_post.h"
#include "pdp_mem.h"

#define D if (0)   // DEBUG MSG
#define DD if (0)  // DROP DEBUG MSG

/* shared internals */

static int _is_udp_header(t_pdp_udp_header *header, unsigned int size)
{
    if (size < sizeof(t_pdp_udp_header)) return 0;
    if (strcmp(header->signature, "PDP")) return 0;
    if (PDP_UDP_VERSION != header->version) return 0;
    return 1;
}

static void _make_udp_header(t_pdp_udp_header *header)
{
    strcpy(header->signature, "PDP");
    header->version = PDP_UDP_VERSION;
}




/* R E C E I V E R */


/* INTERNALS */

static void _send_packet(t_pdp_udp_receiver *x)
{
    _make_udp_header(&x->x_resend_header);
    PDP_ASSERT(x->x_resend_udp_packet_size <= sizeof(t_pdp_udp_header) + sizeof(x->x_resend_chunks));

    /* send the packet */
    if (-1 == sendto (x->x_socket, &x->x_resend_header, x->x_resend_udp_packet_size, 0, 
		      (struct sockaddr *)&x->x_source_socket, x->x_sslen)){
	pdp_post("pdp_netreceive: send failed");
    }
}

static void _send_ack_new(t_pdp_udp_receiver *x)
{
    /* setup resend header */
    x->x_resend_header.connection_id = x->x_connection_id;
    x->x_resend_header.sequence_number = PDP_UDP_ACKNEW;
    x->x_resend_udp_packet_size = sizeof(t_pdp_udp_header);

    _send_packet(x);
        
}


static int _handle_PDP_UDP_NEW(t_pdp_udp_receiver *x)
{
    /* we've got a PDP_UDP_NEW packet, so prepare to receive the data */
    t_pdp_udp_newpacket *np = (t_pdp_udp_newpacket *)x->x_buf;


    //pdp_post("conn_id    = %x", x->x_header.connection_id);
    //pdp_post("size       = %d", np->data_size);
    //pdp_post("nb_chunks  = %d", np->nb_chunks);
    //pdp_post("chunk_size = %d", np->chunk_size);
    //pdp_post("type       = %s", np->type);

    /* check if it is a resend of the PDP_UDP_NEW packet (if NEW_ACK didn't get through)
       if not, prepare for reception */

    if (x->x_connection_id != x->x_header.connection_id){


	/* prepare for reception : TODO add some more checks here */

	// setup type info
	if (x->x_data_type) pdp_dealloc (x->x_data_type);
	x->x_data_type = pdp_alloc(1 + strlen(np->type));
	strcpy(x->x_data_type, np->type);

	// setup data buffer
	x->x_data_size = np->data_size;
	if (x->x_data) pdp_dealloc (x->x_data);
	x->x_data = pdp_alloc(x->x_data_size);
	memset(x->x_data, 0, x->x_data_size); // clear for debug
	
	// setup connection info
	x->x_connection_id = x->x_header.connection_id;
	x->x_nb_chunks     = np->nb_chunks;
	x->x_chunk_size    = np->chunk_size;

	/* setup chunk list */
	if (x->x_chunk_list) pdp_dealloc(x->x_chunk_list);
	x->x_chunk_list = pdp_alloc(sizeof(unsigned int)*x->x_nb_chunks);
	memset(x->x_chunk_list, 0, sizeof(unsigned int)*x->x_nb_chunks);

	x->x_receive_finished = 0;   // we're in a receiving state
	x->x_packet_transferred = 0; // we didn't pass the packet yet
    }
	
    /* send ACK */
    _send_ack_new(x);
    


    return 1;
}

static void _handle_PDP_UDP_DONE(t_pdp_udp_receiver *x)
{
    unsigned int chunk;
    unsigned int missing;
    unsigned int i;
    unsigned int resend_packet_size;


    /* check the connection id */
    if (x->x_connection_id != x->x_header.connection_id) return;

    /* determine how many packets are missing */
    missing = 0;
    for (i=0; i<x->x_nb_chunks; i++) 
	if (!x->x_chunk_list[i]) missing++;

    D pdp_post ("last packet %x had %d/%d dropped chunks", x->x_connection_id, missing, x->x_nb_chunks);


    /* build the resend request (chunk list )*/
    if (missing > RESEND_MAX_CHUNKS) missing = RESEND_MAX_CHUNKS;
    chunk = 0;
    i = missing;
    while(i--){
	while (x->x_chunk_list[chunk]) chunk++; // find next missing chunk
	x->x_resend_chunks[i] = chunk++;        // store it in list
    } 
	       
    /* set the packet size to include the list */
    x->x_resend_udp_packet_size = sizeof(t_pdp_udp_header) 
	+ missing * sizeof(unsigned int);
    
    /* setup resend header */
    strcpy((char *)&x->x_resend_header, "PDP");
    x->x_resend_header.version = PDP_UDP_VERSION;
    x->x_resend_header.connection_id = x->x_connection_id;
    x->x_resend_header.sequence_number = PDP_UDP_RESEND;
    
    D pdp_post("pdp_netreceive: sending RESEND response for %u chunks",  missing);

    /* send out */
    _send_packet(x);

    /* indicate we're done if there's no chunks missing */
    if (!missing) x->x_receive_finished = 1;

}


static int _handle_UDP_DATA(t_pdp_udp_receiver *x)
{
    unsigned int seq = x->x_header.sequence_number;
    unsigned int offset = x->x_chunk_size * seq;

    /* ignore the packet if we're not expecting it */
    if ((!x->x_connection_id) || (x->x_connection_id != x->x_header.connection_id)){
	//pdp_post("pdp_netreceive: got invalid data packet: transmission id %x is not part of current transmisson %x",
	//     x->x_header.connection_id, x->x_connection_id); 
	return 0;
    }

    /* check if it is valid */
    if (seq >= x->x_nb_chunks){
	pdp_post("pdp_netreceive: got invalid data packet: sequence number %u out of bound (nb_chunks=%u)",
	     seq, x->x_nb_chunks);
	return 0;
    }

    /* final check */
    PDP_ASSERT(offset + x->x_buf_size <= x->x_data_size);

    /* write & log it */
    memcpy(x->x_data + offset, x->x_buf, x->x_buf_size);
    x->x_chunk_list[seq] = 1;
    return 1;

}

/* INTERFACE */

/* setup */
t_pdp_udp_receiver *pdp_udp_receiver_new(int port)
{
    t_pdp_udp_receiver *x = pdp_alloc(sizeof(*x));
    memset(x, 0, sizeof(*x));

    /* init */
    x->x_data = 0;
    x->x_data_type = 0;
    x->x_data_size = 0;
    x->x_chunk_list = 0;
    x->x_receive_finished = 0;
    x->x_packet_transferred = 0;
    x->x_zero_terminator = 0;

    x->x_socket = socket(PF_INET, SOCK_DGRAM, 0);
    x->x_connection_id = 0; /* zero for bootstrap (0 == an invalid id) */
    x->x_sslen = sizeof(struct sockaddr_in);

    /* bind socket */
    x->x_sa.sin_port = htons(port);
    x->x_sa.sin_addr.s_addr = 0;
    if (-1 != bind (x->x_socket, (struct sockaddr *)&x->x_sa, 
		    sizeof(struct sockaddr_in))) return x;
    
    /* suicide if find failed */
    else {
	pdp_dealloc(x);
	return 0;
    }
}
void pdp_udp_receiver_free(t_pdp_udp_receiver *x)
{
    if (!x) return;
    if (x->x_socket != 1) close (x->x_socket);
    if (x->x_data) pdp_dealloc(x->x_data);
    if (x->x_data_type) pdp_dealloc (x->x_data_type);
    if (x->x_chunk_list) pdp_dealloc (x->x_chunk_list);
}

void pdp_udp_receiver_reset(t_pdp_udp_receiver *x)
{
    x->x_connection_id = 0;
}


/* receive loop, returns 1 on success, -1 on error, 0 on timeout */
int pdp_udp_receiver_receive(t_pdp_udp_receiver *x, unsigned int timeout_ms)
{
    /* listen for packets */

    unsigned int size;
    struct timeval tv = {0,1000 * timeout_ms};
    fd_set inset;
    FD_ZERO(&inset);
    FD_SET(x->x_socket, &inset);
    switch(select (x->x_socket+1, &inset, NULL, NULL, &tv)){
	case -1:
	    return -1; /* select error */
	case 0:
	    return 0;  /* select time out */
	default:
	    break;     /* data ready */
    }

    /* this won't block, since there's data available */
    if (-1 == (int)(size = recvfrom(x->x_socket, (void *)&x->x_header, 
				    PDP_UDP_BUFSIZE+sizeof(x->x_header), 0,
				    (struct sockaddr *)&x->x_source_socket, &x->x_sslen))) return -1;

    /* store the data size of the packet */
    x->x_buf_size = size - sizeof(t_pdp_udp_header);
    
    /* parse the udp packet */
    if (_is_udp_header(&x->x_header, size)){

	/* it is a control packet */
	if ((int)x->x_header.sequence_number < 0){
			
	    switch (x->x_header.sequence_number){
	    case PDP_UDP_NEW:
		_handle_PDP_UDP_NEW(x);
		break;
		
	    case PDP_UDP_DONE:
		_handle_PDP_UDP_DONE(x);

		/* check if we got a complete packet
		   and signal arrival if we haven't done this already */
		if (x->x_receive_finished && !x->x_packet_transferred){
		    x->x_packet_transferred = 1;
		    return 1; // data complete, please receive
		}
		break;
		
	    default:
		pdp_post("got unknown msg");
		break;
	    }
	}
	
	/* it is a data packet */
	else {
	    _handle_UDP_DATA(x);
	}


    }
    
    else {
	pdp_post("pdp_netreceive: got invalid UDP packet (size = %d)", size);
    }

    return 0; //no major event, please poll again
	
}

/* get meta & data */
char *pdp_udp_receiver_type(t_pdp_udp_receiver *x){return x->x_data_type;}
unsigned int pdp_udp_receiver_size(t_pdp_udp_receiver *x){return x->x_data_size;}
void *pdp_udp_receiver_data(t_pdp_udp_receiver *x){return x->x_data;}


/* S E N D E R */

/* INTERNALS */

static void _sleep(t_pdp_udp_sender *x)
{
    int sleep_period = x->x_sleep_period;
    
    if (sleep_period) {
	if (!x->x_sleep_count++) usleep(x->x_sleepgrain_us);
	x->x_sleep_count %= sleep_period;
    }
}

static void _send(t_pdp_udp_sender *x)
{
    //post("sending %u data bytes", x->x_buf_size);

    _make_udp_header(&x->x_header);

    PDP_ASSERT (x->x_buf_size <= PDP_UDP_BUFSIZE);

    if (-1 == sendto (x->x_socket, &x->x_header, x->x_buf_size + sizeof(t_pdp_udp_header),
		      0, (struct sockaddr *)&x->x_sa, sizeof(struct sockaddr_in)))
       pdp_post("pdp_netsend: send FAILED");

    _sleep(x);

}


static void _prepare_for_new_transmission(t_pdp_udp_sender *x, char *type, unsigned int size, void *data)
{
    unsigned int i;

    /* setup data for transmission */
    x->x_data_type = type;
    x->x_data_size = size;
    x->x_data = data;
    x->x_chunk_size = x->x_udp_payload_size;
    x->x_nb_chunks = (x->x_data_size - 1) / x->x_chunk_size + 1;

    /* generate a connection id (non-zero) */
    while (!(x->x_connection_id = rand()));

    /* setup chunk list to contain all chunks */
    if (x->x_chunk_list) free (x->x_chunk_list);
    x->x_chunk_list_size = x->x_nb_chunks;
    x->x_chunk_list = malloc(sizeof(unsigned int)*x->x_chunk_list_size);
    for (i=0; i<x->x_chunk_list_size; i++) x->x_chunk_list[i] = i;
    
}

static void _send_header_packet(t_pdp_udp_sender *x)
{
    t_pdp_udp_newpacket *np = (t_pdp_udp_newpacket *)x->x_buf; /* buf contains the PDP_UDP_NEW body */

    /* init packet */
    x->x_header.sequence_number = PDP_UDP_NEW;
    x->x_header.connection_id = x->x_connection_id;
    np->data_size  = x->x_data_size;
    np->nb_chunks  = x->x_nb_chunks;
    np->chunk_size = x->x_chunk_size;
    strcpy(np->type, x->x_data_type);
    x->x_buf_size = sizeof(*np) + strlen(np->type) + 1;
    PDP_ASSERT(x->x_buf_size <= PDP_UDP_BUFSIZE);

    /* send the packet */
    _send(x);
}

/* saend the chunks in the chunk list */
static void _send_chunks(t_pdp_udp_sender *x){
    unsigned int i;
    unsigned int count = 0;

    /* send chunks: this requires header is setup ok (sig,ver,connid)*/
    for (i=0; i<x->x_chunk_list_size; i++){
	unsigned int offset;
	unsigned int current_chunk_size;
	unsigned int seq = x->x_chunk_list[i];

	PDP_ASSERT(seq < x->x_nb_chunks);
	x->x_header.sequence_number = seq; // store chunk number

	/* get current chunk offset */
	offset = seq * x->x_chunk_size;
	PDP_ASSERT(offset < x->x_data_size);


	/* get current chunk size */
	current_chunk_size = (offset + x->x_chunk_size > x->x_data_size) ? 
	    (x->x_data_size - offset) : x->x_chunk_size;
	x->x_buf_size = current_chunk_size;
	PDP_ASSERT(x->x_buf_size <= PDP_UDP_BUFSIZE);

	/* copy chunk to transmission buffer & send */
	PDP_ASSERT(offset + current_chunk_size <= x->x_data_size);
	memcpy(x->x_buf, x->x_data + offset, current_chunk_size);


	/* send the chunk */
	_send(x);
	count++;
	    
    }
    D pdp_post("sent %d chunks, id=%x", count,x->x_connection_id);
}

/* send a DONE packet */
static void _send_done(t_pdp_udp_sender *x){
    x->x_header.sequence_number = PDP_UDP_DONE;
    x->x_buf_size = 0;
    _send(x);
}
static int _receive_packet(t_pdp_udp_sender *x, int desired_type) 
/* 0 == timeout, -1 == error, 1 == got packet */
{
    unsigned int size;
    int type;

    struct timeval tv;
    fd_set inset;
    int sr;


    while (1){
	int retval;

	/* wait for incoming */
	tv.tv_sec = 0;
	tv.tv_usec = x->x_timeout_us;
	FD_ZERO(&inset);
	FD_SET(x->x_socket, &inset);
	switch (select (x->x_socket+1, &inset, NULL, NULL, &tv)){
	case -1:
	    return -1; /* select error */
	case 0:
	    return 0;  /* select time out */
	default:
	    break;     /* data ready */
	}

	/* read packet */
	if (-1 == (int)(size = recv(x->x_socket, (void *)&x->x_resend_header, MAX_UDP_PACKET, 0))){
	    pdp_post("pdp_netsend: error while reading from socket");
	    return -1;
	}

	/* check if it is a valid PDP_UDP packet */
	if (!_is_udp_header(&x->x_resend_header, size)){
	    pdp_post("pdp_netsend: ignoring invalid UDP packet (size = %u)", size);
	    continue;
	}


	/* check connection id */
	if (x->x_connection_id != x->x_resend_header.connection_id){
	    D pdp_post("pdp_netsend: ignoring ghost packet id=%x, current id=%x",
		 x->x_resend_header.connection_id, x->x_connection_id);
	    continue;
	}

	/* check type */
	type = x->x_resend_header.sequence_number;
	if (type != desired_type) continue;


	/* setup data buffer for known packets */
	switch(type){
	case PDP_UDP_RESEND:
	    x->x_resend_items = (size - sizeof(t_pdp_udp_header)) / sizeof(unsigned int);
	    break;
	default:
	    break;
	}

	return 1;
    }

}

/* get the resend list */
static int _need_resend(t_pdp_udp_sender *x) {

    int retries = 3;
    int retval;
    while (retries--){

	/* send a DONE msg */
	_send_done(x);

	/* wait for ACK */
	switch(_receive_packet(x, PDP_UDP_RESEND)){
	case 0:
	    /* timeout, retry */
	    continue; 
	case -1:
	    /* error */
	    goto move_on;

	default:
	    /* got PDP_UDP_RESEND packet: setup resend list */
	    if (x->x_resend_items  > x->x_nb_chunks){
		pdp_post("pdp_netsend: WARNING: chunk list size (%d) is too big, ignoring RESEND request",
		     x->x_resend_items);
		x->x_resend_items = 0;
		continue;
	    }
	    x->x_chunk_list_size = x->x_resend_items;

	    memcpy(x->x_chunk_list, x->x_resend_chunks, sizeof(unsigned int) * x->x_resend_items);
	    D pdp_post("got RESEND request for %d chunks (id %x)", x->x_resend_items,x->x_connection_id);

	    return x->x_chunk_list_size > 0;
	}
	
    }
    
    /* timeout */
 move_on:
    x->x_chunk_list_size = 0;
    return 0;


}


/* INTERFACE */


/* some flow control hacks */

void pdp_udp_sender_timeout_us(t_pdp_udp_sender *x, unsigned int timeout_us)
{
    x->x_timeout_us = timeout_us;
}


void pdp_udp_sender_sleepgrain_us(t_pdp_udp_sender *x, unsigned int sleepgrain_us)
{
    x->x_sleepgrain_us = sleepgrain_us;
}

void pdp_udp_sender_sleepperiod(t_pdp_udp_sender *x, unsigned int sleepperiod)
{
    x->x_sleep_period = sleepperiod;
}


void pdp_udp_sender_udp_packet_size(t_pdp_udp_sender *x, unsigned int udp_packet_size)
{
    int i = (int)udp_packet_size - sizeof(t_pdp_udp_header);
    // if (i < 1024) i = 1024;
    if (i > PDP_UDP_BUFSIZE) i = PDP_UDP_BUFSIZE;
    x->x_udp_payload_size = i;
}

void pdp_udp_sender_connect(t_pdp_udp_sender *x, char *host, unsigned int port)
{
    struct hostent *hp;

    hp = gethostbyname(host);
    if (!hp){
	pdp_post("pdp_udp_sender: host %s not found", host);
    }
    else{
	/* host ok, setup address */
	x->x_sa.sin_family = AF_INET;
	x->x_sa.sin_port = htons(port);
	memcpy((char *)&x->x_sa.sin_addr, (char *)hp->h_addr, hp->h_length);

	/* create the a socket if necessary */
	if (x->x_socket == -1){
	    if (-1 == (x->x_socket = socket(PF_INET, SOCK_DGRAM, 0))){
		pdp_post("pdp_udp_sender: can't create socket");
	    }
	    if (1){
		int on = 1;
		if (setsockopt(x->x_socket,SOL_SOCKET,SO_BROADCAST,(char *)&on,sizeof(on))<0)
		    pdp_post("pdp_udp_sender: can't set broadcast flag");
	    }
	}
    }
}

/* setup */
t_pdp_udp_sender *pdp_udp_sender_new(void)
{
    t_pdp_udp_sender *x = pdp_alloc(sizeof(*x));
    memset(x,0,sizeof(*x));

    x->x_chunk_list = 0;

    /* no connection */
    x->x_socket = -1;


    /* set flow control */
    pdp_udp_sender_timeout_us(x, 50000);
    x->x_sleep_count = 0;
    pdp_udp_sender_sleepgrain_us(x, 0);
    pdp_udp_sender_sleepperiod(x, 50);
    pdp_udp_sender_udp_packet_size(x, 1472); //optimal udp packet size (ip: 1500 = 28 + 1472)


    return x;
}

void pdp_udp_sender_free(t_pdp_udp_sender *x)
{
    int i;
    void* retval;
    if (x->x_socket != -1) close(x->x_socket);
    if (x->x_chunk_list) free (x->x_chunk_list);
}

/* send, returns 1 on success, 0 on error */
int pdp_udp_sender_send(t_pdp_udp_sender *x, char* type, unsigned int size, void *data)
{

    /* SEND A PACKET */

    /* get the type and data from caller */
    /* send header packet and make sure it has arrived */
    /* send a chunk burst */
    /* send done packet and get the resend list */
    /* repeat until send list is empty */


    int hs_retry = 5; // retry count for initial handshake
    int rs_retry = 5; // retry count for resends

    /* check if we have a target */
    if (-1 == x->x_socket) goto transerror;

    /* setup internal state */
    _prepare_for_new_transmission(x,type,size,data);

    /* handshake a new transmission */
    do {
	if (!(hs_retry--)) break;
	//    pdp_post("handshake retry %d for packet %x", hscount, x->x_connection_id);
	_send_header_packet(x);
    } while (!_receive_packet(x, PDP_UDP_ACKNEW));
    

    /* exit if no handshake was possible */
    if (hs_retry < 0){
	DD pdp_post("pdp_netsend: DROP: receiver does not accept new transmission");
	goto transerror;
    }

    /* transmission loop */
    do {
	if (!(rs_retry--)) break;
	_send_chunks(x);
    } while (_need_resend(x));

    /* exit if transmission was not successful */
    if (rs_retry < 0){
	DD pdp_post("pdp_netsend: DROP: receiver did not confirm reception");
	goto transerror;
    }	

    /* send successful */
    return 1;

  transerror:
    /* transmission error */
    return 0;
}
