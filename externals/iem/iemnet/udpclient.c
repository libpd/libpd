/* udpclient.c
 * copyright (c) 2010 IOhannes m zmölnig, IEM
 */

/*                                                                              */
/* A client for bidirectional communication from within Pd.                     */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc.,                                                            */
/* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                  */
/*                                                                              */

/* ---------------------------------------------------------------------------- */

#define DEBUGLEVEL 1

#include "iemnet.h"
#include <string.h>

#include <pthread.h>

static t_class *udpclient_class;
static const char objName[] = "udpclient";


typedef struct _udpclient
{
  t_object        x_obj;
  t_clock         *x_clock;
  t_outlet        *x_msgout;
  t_outlet        *x_addrout;
  t_outlet        *x_connectout;
  t_outlet        *x_statusout;

  t_iemnet_sender*x_sender;
  t_iemnet_receiver*x_receiver;


  int             x_fd; // the socket
  char           *x_hostname; // address we want to connect to as text
  int             x_connectstate; // 0 = not connected, 1 = connected
  int             x_port; // port we're connected to
  long            x_addr; // address we're connected to as 32bit int


  /* multithread stuff */
  pthread_t       x_threadid; /* id of child thread */
  pthread_attr_t  x_threadattr; /* attributes of child thread */

	t_iemnet_floatlist         *x_floatlist;
} t_udpclient;


static void udpclient_receive_callback(void *x, t_iemnet_chunk*);



/* connection handling */

static void *udpclient_child_connect(void *w)
{
  t_udpclient         *x = (t_udpclient*) w;

  struct sockaddr_in  server;
  struct hostent      *hp;
  int                 sockfd;
  int                 broadcast = 1;/* nonzero is true */

  if (x->x_sender)
    {
      error("[%s] already connected", objName);
      return (x);
    }

  /* create a socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  DEBUG("send socket %d\n", sockfd);
  if (sockfd < 0)
    {
      sys_sockerror("udpclient: socket");
      return (x);
    }

  /* Based on zmoelnig's patch 2221504:
     Enable sending of broadcast messages (if hostname is a broadcast address)*/
#ifdef SO_BROADCAST
  if( 0 != setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const void *)&broadcast, sizeof(broadcast)))
    {
      error("[%s] couldn't switch to broadcast mode", objName);
    }
#endif /* SO_BROADCAST */
    
  /* connect socket using hostname provided in command line */
  server.sin_family = AF_INET;
  hp = gethostbyname(x->x_hostname);
  if (hp == 0)
    {
      error("[%s] bad host '%s'?", objName, x->x_hostname);
      return (x);
    }
  memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

  /* assign client port number */
  server.sin_port = htons((u_short)x->x_port);

  DEBUG("connecting to port %d", x->x_port);
  /* try to connect. */
  if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
      sys_sockerror("udpclient: connecting stream socket");
      sys_closesocket(sockfd);
      return (x);
    }
  x->x_fd = sockfd;
  x->x_addr = ntohl(*(long *)hp->h_addr);

  x->x_sender=iemnet__sender_create(sockfd);
  x->x_receiver=iemnet__receiver_create(sockfd, x,  udpclient_receive_callback);

  x->x_connectstate = 1;

  clock_delay(x->x_clock, 0);
  return (x);
}
static void udpclient_tick(t_udpclient *x)
{
  outlet_float(x->x_connectout, 1);
}


static void udpclient_disconnect(t_udpclient *x)
{
  if (x->x_fd >= 0)
    {

      DEBUG("disconnect %x %x", x->x_sender, x->x_receiver);
      if(x->x_receiver)iemnet__receiver_destroy(x->x_receiver); x->x_receiver=NULL;
      if(x->x_sender)iemnet__sender_destroy(x->x_sender); x->x_sender=NULL;

      sys_closesocket(x->x_fd);
      x->x_fd = -1;
      x->x_connectstate = 0;
      outlet_float(x->x_connectout, 0);
    }
  else pd_error(x, "[%s] not connected", objName);
}


static void udpclient_connect(t_udpclient *x, t_symbol *hostname, t_floatarg fportno)
{
  if(x->x_fd>=0)udpclient_disconnect(x);
  /* we get hostname and port and pass them on
     to the child thread that establishes the connection */
  x->x_hostname = hostname->s_name;
  x->x_port = fportno;
  x->x_connectstate = 0;
  /* start child thread */
  if(pthread_create(&x->x_threadid, &x->x_threadattr, udpclient_child_connect, x) < 0)
    error("%s: could not create new thread", objName);
}

/* sending/receiving */

static void udpclient_send(t_udpclient *x, t_symbol *s, int argc, t_atom *argv)
{
  int size=0;
  t_atom output_atom;
  t_iemnet_sender*sender=x->x_sender;

  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);
  if(sender && chunk) {
    size=iemnet__sender_send(sender, chunk);
  }
  iemnet__chunk_destroy(chunk);

  SETFLOAT(&output_atom, size);
  outlet_anything( x->x_statusout, gensym("sent"), 1, &output_atom);
}

static void udpclient_receive_callback(void*y, t_iemnet_chunk*c) {
  t_udpclient *x=(t_udpclient*)y;

  if(c) {
    iemnet__addrout(x->x_statusout, x->x_addrout, x->x_addr, x->x_port);
    x->x_floatlist=iemnet__chunk2list(c, x->x_floatlist); // gets destroyed in the dtor
    outlet_list(x->x_msgout, gensym("list"),x->x_floatlist->argc, x->x_floatlist->argv);
  } else {
    // disconnected
    DEBUG("disconnected");
    if(x->x_fd >= 0) {
      udpclient_disconnect(x);
    }
  }
}

/* constructor/destructor */

static void *udpclient_new(void)
{
  t_udpclient *x = (t_udpclient *)pd_new(udpclient_class);
  x->x_msgout = outlet_new(&x->x_obj, 0);	/* received data */
  x->x_addrout = outlet_new(&x->x_obj, gensym("list"));
  x->x_connectout = outlet_new(&x->x_obj, gensym("float"));	/* connection state */
  x->x_statusout = outlet_new(&x->x_obj, 0);/* last outlet for everything else */

  x->x_fd = -1;
  x->x_addr = 0L;
  x->x_port = 0;

  x->x_sender=NULL;
  x->x_receiver=NULL;

  x->x_clock = clock_new(x, (t_method)udpclient_tick);

  x->x_floatlist=iemnet__floatlist_create(1024);

  /* prepare child thread */
  if(pthread_attr_init(&x->x_threadattr) < 0)
    verbose(1, "[%s] warning: could not prepare child thread", objName);
  if(pthread_attr_setdetachstate(&x->x_threadattr, PTHREAD_CREATE_DETACHED) < 0)
    verbose(1, "[%s] warning: could not prepare child thread", objName);
    

  return (x);
}

static void udpclient_free(t_udpclient *x)
{
  udpclient_disconnect(x);
  if(x->x_clock)clock_free(x->x_clock);x->x_clock=NULL;
	if(x->x_floatlist)iemnet__floatlist_destroy(x->x_floatlist);x->x_floatlist=NULL;
}

IEMNET_EXTERN void udpclient_setup(void)
{
  if(!iemnet__register(objName))return;
  udpclient_class = class_new(gensym(objName), (t_newmethod)udpclient_new,
                              (t_method)udpclient_free,
                              sizeof(t_udpclient), 0, A_DEFFLOAT, 0);
  class_addmethod(udpclient_class, (t_method)udpclient_connect, gensym("connect")
                  , A_SYMBOL, A_FLOAT, 0);
  class_addmethod(udpclient_class, (t_method)udpclient_disconnect, gensym("disconnect"), 0);
  class_addmethod(udpclient_class, (t_method)udpclient_send, gensym("send"), A_GIMME, 0);
  class_addlist(udpclient_class, (t_method)udpclient_send);

  DEBUGMETHOD(udpclient_class);
}


IEMNET_INITIALIZER(udpclient_setup);

/* end of udpclient.c */
