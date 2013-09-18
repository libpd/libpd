/*
 *   Pure Data Packet module. Raw packet output.
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
  This is the most straightforward way to get data out of pdp.
  The internals follow the simple reader/writer pattern
  * writer: runs in pd thread, accepts packets from inlet and stores in queue
  * reader: runs in own thread, reads packets from queue and writes to pipe

  Since there is no communication from reader to writer, we need a watchdog
  timer to check the status of the writer. Mainly to close when necessary.

  To enable audio recording, pdp_rawout will also produce interleaved 16bit
  audio. You will need to instantiate it with [pdp_rawout~ nbchans]



*/

/* TODO:

   make audio buffer smaller (128 bytes writes is too heavy)

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
#include <errno.h>
#include <signal.h>
#include "pdp_pd.h"
#include "pdp_debug.h"
#include "pdp_list.h"
#include "pdp_comm.h"
#include "pdp_post.h"
#include "pdp_packet.h"


#define D if (1)
#define DEF_QUEUESIZE 100
#define PIPE_BLOCKSIZE 4096
#define POLLTIME 20


/* raw input from a unix pipe */

typedef struct rawout_struct
{
    /* pd */
    t_object x_obj;
    t_float  x_f;

    //t_outlet *x_outlet;
    t_outlet *x_sync_outlet;

    /* comm */
    t_pdp_list *x_queue;   // packet queue
    t_clock    *x_clock;   // watchdog timer
    t_float     x_deltime; // watchdog period
    int         x_verbose;
    int         x_packet;    // dsp fillup state
    int         x_countdown; // amount of packet filled up
    short int * x_needle;    // start writing here

    /* thread */
    pthread_mutex_t x_mut;
    pthread_attr_t x_attr;
    pthread_t x_thread;

    /* sync */
    int x_giveup;  // 1-> terminate writer thread
    int x_active;  // 1-> writer thread is launched
    int x_done;    // 1-> writer thread has exited

    /* config */
    t_symbol      *x_pipe;
    int           x_chans;            // nb audio channels
    t_pdp_symbol *x_tmptype;
    int           x_max_queuesize;    // buffer size ( < 0 = infty )

} t_rawout;


static inline void lock(t_rawout *x){pthread_mutex_lock(&x->x_mut);}
static inline void unlock(t_rawout *x){pthread_mutex_unlock(&x->x_mut);}

static void rawout_close(t_rawout *x);

/* READER THREAD: reads from queue, writes to pipe */

static void *rawout_thread(void *y)
{
    int pipefd;
    int packet = -1;
    t_rawout *x = (t_rawout *)y;
    int period_sec;
    int period_usec;
    sigset_t _sigvec; /* signal handling  */

    char me[1024];
    sprintf(me, "pdp_rawout: %s", x->x_pipe->s_name);

    /* ignore pipe signal */
    sigemptyset(&_sigvec);
    sigaddset(&_sigvec,SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &_sigvec, 0);

    //D pdp_post("pipe: %s", x->x_pipe->s_name);
    //D pdp_post("type: %s", x->x_type->s_name);


    /* open pipe */
    if (-1 == (pipefd = open(x->x_pipe->s_name, O_WRONLY|O_NONBLOCK|O_APPEND))){
      if (-1 == (pipefd = open(x->x_pipe->s_name, O_WRONLY|O_CREAT, 0600))){
	    perror(me);
	    goto exit;
	}
    }

    pdp_post("pdp_rawout: opened %s", x->x_pipe->s_name);


    /* main loop (packets) */
    while(1){
	void *data = 0;
	int left = -1;

	/* try again if queue is empty */
	if (!x->x_queue->elements){
	    /* check if we need to stop */
	    if (x->x_giveup){
		goto close;
	    }
	    else {
		usleep(1000.0f); // sleep before polling again
		continue;
	    }
	}
	/* get packet from queue */
	lock(x);
	packet = pdp_list_pop(x->x_queue).w_packet;
	unlock(x);

	/* send packet */
	//t_pdp *h = pdp_packet_header(packet);
	//fprintf(stderr, "users %d\n", h->users);
	
	data = pdp_packet_data(packet);
	left = pdp_packet_data_size(packet);
	//int i; for (i = 0; i<left/2; i++) fprintf(stderr, "%06x ", ((short int *)data)[i]); fprintf(stderr, "\n");


	/* inner loop: pipe reads */
	while(left){

	    fd_set outset;
	    struct timeval tv = {0,10000};

	    /* check if we need to stop */
	    if (x->x_giveup){
		pdp_packet_mark_unused(packet);
		goto close;
	    }

	    /* select, with timeout */
	    FD_ZERO(&outset);
	    FD_SET(pipefd, &outset);
	    if (-1 == select(pipefd+1, NULL, &outset, NULL, &tv)){
		pdp_post("select error");
		goto close;
	    }

	    /* if ready, read, else retry */
	    if (FD_ISSET(pipefd, &outset)){

		int bytes = write(pipefd, data, left);
		/* handle errors */
		if (bytes <= 0){
		    perror(me);
		    if (errno != EAGAIN) goto close;
		}
		/* or update pointers */
		else{
		    data += bytes;
		    left -= bytes;
		    //pdp_post("left %d", left);
		}
	    }
	    else {
		//pdp_post("retrying write");
	    }
	}
		   
	/* discard packet */
	pdp_packet_mark_unused(packet);

    
    }

  close:
    /* close pipe */
    close(pipefd);
	
	
  exit:
    x->x_done = 1;
    return 0;
}


/* DSP INPUT */
#define DSP_ARG(type, name, source) type name = (type)source

static t_int *rawout_perform(t_int *w);
static void rawout_dsp(t_rawout *x, t_signal **sp){
    int nargs = 2 + x->x_chans;
    t_int args[nargs];
    args[0] = (t_int)x;
    args[1] = (t_int)sp[0]->s_n;
    float **in = (float **)(args+2);
    int i;
    for (i=0; i<x->x_chans; i++) in[i] = sp[i]->s_vec;
    dsp_addv(rawout_perform, nargs, args);
}

static t_int *rawout_perform(t_int *w)
{
    DSP_ARG(t_rawout*,  x,     w[1]);
    DSP_ARG(t_int,      n,     w[2]);
    DSP_ARG(t_float**,  in,    &w[3]);

    short int *out;
    int i,c,k;
    

    if (x->x_queue->elements >= x->x_max_queuesize){
	// drop
	if (x->x_verbose && x->x_active) pdp_post_n(".");
    }
    else {

	// create packet
	if (x->x_countdown) {
	    out = x->x_needle;
	}
	else {
	    int p = pdp_factory_newpacket(x->x_tmptype);
	    pdp_packet_mark_unused(x->x_packet);
	    // if (-1 == p) pdp_post("pdp_rawout~: can't create packet");
	    x->x_needle = out = (short int *)pdp_packet_data(p);
	    x->x_packet = p;
	    x->x_countdown = pdp_packet_data_size(p) / 2;
	}

	//pdp_post("data size = %d bytes", pdp_packet_data_size(p));

	//memset(out, 0, pdp_packet_data_size(p));


	// convert & interleave
	for (k=0,i=0; i<n; i++){
	    for (c=0; c<x->x_chans; c++,k++){
		float val = (in[c])[i];
		val *= (float)((1<<15)-1);
		out[k] = (short int)(val);
		//out[k] = 0x1234;
		//fprintf(stderr, "(%d,%d,%d) %d\n", c, i, k, (int)out[k]);
	    }
	}

	x->x_needle += k;
	x->x_countdown -= k;
	if (!x->x_countdown){
	    // transfer
	    lock(x);
	    pdp_list_add_back(x->x_queue, a_packet, (t_pdp_word)x->x_packet);
	    x->x_packet = -1;
	    unlock(x);
	}
    }

    return w+3+x->x_chans;
}


/* PACKET INPUT */


static void pdp_in(t_rawout *x, t_symbol *s, t_float f)
{
    /* save packet to pdp queue, if size is smaller than maxsize */
    if (s == S_REGISTER_RO){
	if (x->x_queue->elements < x->x_max_queuesize){
	    int p = (int)f;
	    p = pdp_packet_copy_ro(p);
	    if (p != -1){
		lock(x);
		pdp_list_add_back(x->x_queue, a_packet, (t_pdp_word)p);
		unlock(x);
	    }
	}
	else {
	    //pdp_post("pdp_rawout: dropping packet: (queue full)", x->x_max_queuesize);
	    if (x->x_active && x->x_verbose) pdp_post_n(".");
	}
	    
    }

    /* check if thread is done */
    if (x->x_done) rawout_close(x);

}

/* CONTROL */

//static void rawout_type(t_rawout *x, t_symbol *type)
//{
    //x->x_type = pdp_gensym(type->s_name);
//}

static void clear_queue(t_rawout *x);
static void rawout_open(t_rawout *x, t_symbol *spipe)
{
    /* save pipe name if not empty */
    if (spipe->s_name[0]) {x->x_pipe = spipe;}

    if (x->x_active) {
	pdp_post("already open");
	return;
    }
    /* start thread */
    x->x_giveup = 0;
    x->x_done = 0;
    clear_queue(x);
    pthread_create(&x->x_thread, &x->x_attr, rawout_thread , x);
    x->x_active = 1;
}

static void rawout_stopthread(t_rawout *x){
    if (!x->x_active) return;

    /* stop thread: set giveup + wait */
    x->x_giveup = 1;
    pthread_join(x->x_thread, NULL);
    x->x_active = 0;
    clear_queue(x);
}

static void rawout_close(t_rawout *x)
{
    if (!x->x_active) return;
    rawout_stopthread(x);

    /* notify */
    outlet_bang(x->x_sync_outlet);
    pdp_post("pdp_rawout: closed %s", x->x_pipe->s_name);
    
}

static void rawout_verbose(t_rawout *x, t_float fverbose){
    x->x_verbose = (int)fverbose;
}
static void rawout_tick(t_rawout *x){
    if (x->x_done) rawout_close(x);
    clock_delay(x->x_clock, x->x_deltime);
}

static void clear_queue(t_rawout *x){
    lock(x);
    while(x->x_queue->elements){
	pdp_packet_mark_unused(pdp_list_pop(x->x_queue).w_packet);
    }
    unlock(x);
}

#define MAXINT (0x80 << ((sizeof(int)-1)*8))
static void rawout_queuesize(t_rawout *x, t_float fbufsize){
    int bufsize = (int)fbufsize;
    if (fbufsize < 1) fbufsize = MAXINT;
    x->x_max_queuesize = bufsize;
}

static void rawout_free(t_rawout *x)
{
    // terminate thread
    rawout_stopthread(x);
    
    // cleanup
    clock_unset(x->x_clock);
    clear_queue(x);
    pdp_tree_free(x->x_queue);
    pdp_packet_mark_unused(x->x_packet);
}

t_class *rawout_class;
t_class *rawout_dsp_class;

// shared stuff
static void rawout_init(t_rawout *x, t_symbol *spipe){
    //x->x_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_sync_outlet = outlet_new(&x->x_obj, &s_anything);
    x->x_queue = pdp_list_new(0);
    x->x_active = 0;
    x->x_giveup = 0;
    x->x_done = 0;
    x->x_packet = -1;
    x->x_countdown = 0;
    x->x_max_queuesize = DEF_QUEUESIZE;
    //x->x_type = pdp_gensym("image/YCrCb/320x240"); //default
    x->x_pipe = gensym("/tmp/pdpraw"); // default
    x->x_deltime = POLLTIME;
    x->x_clock = clock_new(x, (t_method)rawout_tick);
    pthread_attr_init(&x->x_attr);
    pthread_mutex_init(&x->x_mut, NULL);

    /* args */
    //rawout_type(x, type);
    if (spipe && spipe->s_name[0]) x->x_pipe = spipe; 
    rawout_tick(x);
    rawout_verbose(x,0);
}

// [pdp_rawout]
static void *rawout_new(t_symbol *spipe /* , t_symbol *type */)
{
    t_rawout *x;
    /* allocate & init */
    x = (t_rawout *)pd_new(rawout_class);
    rawout_init(x, spipe);
    return (void *)x;

}

// [pdp_rawout~]


// HUH??? why do i get the symbol first, then the float????
// see http://lists.puredata.info/pipermail/pd-dev/2003-09/001618.html
//static void *rawout_dsp_new(t_float fchans, t_symbol *spipe){
static void *rawout_dsp_new(t_symbol *spipe, t_float fchans){
    int chans = (int)fchans;
    if (chans < 1)  chans = 1;
    if (chans > 64) return 0; // this is just a safety measure

    t_rawout *x = (t_rawout *)pd_new(rawout_dsp_class);
    rawout_init(x, spipe);

    // hack: temp packet
    char temp_packet[1024];
    sprintf(temp_packet, "image/grey/256x%d", 8 * chans);
    pdp_post("pdp_rawout: using fake packet %s", temp_packet);
    x->x_tmptype = pdp_gensym(temp_packet);

    // create audio inlets
    x->x_chans = chans;
    while (--chans) inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
    
    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



#define COMMON(base_class)\
    class_addmethod(base_class, (t_method)rawout_open, gensym("open"), A_DEFSYMBOL, A_NULL);\
    class_addmethod(base_class, (t_method)rawout_close, gensym("close"), A_NULL);\
    class_addmethod(base_class, (t_method)rawout_verbose, gensym("verbose"), A_FLOAT, A_NULL);\
    class_addmethod(base_class, (t_method)rawout_queuesize, gensym("bufsize"), A_FLOAT, A_NULL);

void pdp_rawout_setup(void){

    int i;

    /* PACKETS */

    /* create a standard pd class: [pdp_rawout pipe type] */
    rawout_class = class_new(gensym("pdp_rawout"), (t_newmethod)rawout_new,
   	(t_method)rawout_free, sizeof(t_rawout), 0, A_DEFSYMBOL, A_NULL);

    /* add global message handler */
    class_addmethod(rawout_class, (t_method)pdp_in, gensym("pdp"), A_SYMBOL, A_FLOAT, A_NULL);
    COMMON(rawout_class);

    /* DSP */

    /* create a standard pd class: [pdp_rawout pipe type] */
    rawout_dsp_class = class_new(gensym("pdp_rawout~"), (t_newmethod)rawout_dsp_new,
   	(t_method)rawout_free, sizeof(t_rawout), 0, A_DEFFLOAT, A_DEFSYMBOL, A_NULL);

    /* add signal input */
    CLASS_MAINSIGNALIN(rawout_dsp_class, t_rawout, x_f); 
    class_addmethod(rawout_dsp_class, (t_method)rawout_dsp, gensym("dsp"), 0);   
    COMMON(rawout_dsp_class);

}

    





#ifdef __cplusplus
}
#endif
