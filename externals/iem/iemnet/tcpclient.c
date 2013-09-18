/* tcpclient.c
 * copyright (c) 2010 IOhannes m zmölnig, IEM
 * copyright (c) 2006-2010 Martin Peach
 * copyright (c) 2004 Olaf Matthes
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

static t_class *tcpclient_class;
static char objName[] = "tcpclient";


typedef struct _tcpclient
{
  t_object        x_obj;
  t_clock         *x_clock;
  t_outlet        *x_msgout;
  t_outlet        *x_addrout;
  t_outlet        *x_connectout;
  t_outlet        *x_statusout;

  t_iemnet_sender  *x_sender;
  t_iemnet_receiver*x_receiver;

  int              x_serialize;

  int             x_fd; // the socket
  char           *x_hostname; // address we want to connect to as text
  int             x_connectstate; // 0 = not connected, 1 = connected
  int             x_port; // port we're connected to
  long            x_addr; // address we're connected to as 32bit int

  /* multithread stuff */
  pthread_t       x_threadid; /* id of child thread */
  pthread_attr_t  x_threadattr; /* attributes of child thread */

	t_iemnet_floatlist         *x_floatlist;
} t_tcpclient;


static void tcpclient_receive_callback(void *x, t_iemnet_chunk*);

static void tcpclient_info(t_tcpclient *x)
{
  // "server <socket> <IP> <port>"
  // "bufsize <insize> <outsize>"
  static t_atom output_atom[3];
  if(x&&x->x_connectstate) {
    int sockfd = x->x_fd;
    unsigned short port   = x->x_port;
    const char*hostname=x->x_hostname;

    int insize =iemnet__receiver_getsize(x->x_receiver);
    int outsize=iemnet__sender_getsize  (x->x_sender  );

    SETFLOAT (output_atom+0, sockfd);
    SETSYMBOL(output_atom+1, gensym(hostname));
    SETFLOAT (output_atom+2, port);

    outlet_anything( x->x_statusout, gensym("server"), 3, output_atom);

    SETFLOAT (output_atom+0, insize);
    SETFLOAT (output_atom+1, outsize);
    outlet_anything( x->x_statusout, gensym("bufsize"), 2, output_atom);
  }
}

/* connection handling */

static void *tcpclient_child_connect(void *w)
{
  t_tcpclient         *x = (t_tcpclient*) w;
  struct sockaddr_in  server;
  struct hostent      *hp;
  int                 sockfd;

  t_iemnet_sender*sender;
  t_iemnet_receiver*receiver;

  if (x->x_fd >= 0)
    {
      error("%s_connect: already connected", objName);
      return (x);
    }

  /* create a socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DEBUG("send socket %d\n", sockfd);
  if (sockfd < 0)
    {
      sys_sockerror("tcpclient: socket");
      return (x);
    }
  /* connect socket using hostname provided in command line */
  server.sin_family = AF_INET;
  hp = gethostbyname(x->x_hostname);
  if (hp == 0)
    {
      sys_sockerror("tcpclient: bad host?\n");
      return (x);
    }
  memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

  /* assign client port number */
  server.sin_port = htons((u_short)x->x_port);

  /* try to connect */
  if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
      sys_sockerror("tcpclient: connecting stream socket");
      sys_closesocket(sockfd);
      return (x);
    }

  sender=iemnet__sender_create(sockfd);
  receiver=iemnet__receiver_create(sockfd, x,  tcpclient_receive_callback);

  /* update the tcpclient-object (thread safe) */
  sys_lock();
  x->x_fd = sockfd;
  x->x_addr = ntohl(*(long *)hp->h_addr);

  x->x_sender=sender;
  x->x_receiver=receiver;

  x->x_connectstate = 1;

  /* use callback to set outlet in main thread */
  clock_delay(x->x_clock, 0);
  sys_unlock();
  return (x);
}
static void tcpclient_tick(t_tcpclient *x)
{
    outlet_float(x->x_connectout, 1);
}


static void tcpclient_disconnect(t_tcpclient *x);

static void tcpclient_connect(t_tcpclient *x, t_symbol *hostname, t_floatarg fportno)
{
  if(x->x_fd>=0)tcpclient_disconnect(x);
  /* we get hostname and port and pass them on
     to the child thread that establishes the connection */
  x->x_hostname = hostname->s_name;
  x->x_port = fportno;
  x->x_connectstate = 0;
  /* start child thread */
  if(pthread_create(&x->x_threadid, &x->x_threadattr, tcpclient_child_connect, x) < 0)
    error("%s: could not create new thread", objName);
}

static void tcpclient_disconnect(t_tcpclient *x)
{
  if (x->x_fd >= 0)
    {
      int fd=x->x_fd;
      x->x_fd = -1;

      DEBUG("disconnecting %x", x);
      if(x->x_sender)iemnet__sender_destroy(x->x_sender); x->x_sender=NULL;
      if(x->x_receiver)iemnet__receiver_destroy(x->x_receiver); x->x_receiver=NULL;
      DEBUG("disconnect cleaning up %x", x);
      sys_closesocket(fd);


      x->x_connectstate = 0;
      outlet_float(x->x_connectout, 0);
    }
  else pd_error(x, "%s: not connected", objName);
}

/* sending/receiving */

static void tcpclient_send(t_tcpclient *x, t_symbol *s, int argc, t_atom *argv)
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
  if(size<0) {
    tcpclient_disconnect(x);
  }
}

static void tcpclient_receive_callback(void*y, t_iemnet_chunk*c) {
  t_tcpclient *x=(t_tcpclient*)y;

  if(c) {
    iemnet__addrout(x->x_statusout, x->x_addrout, x->x_addr, x->x_port);
	  x->x_floatlist=iemnet__chunk2list(c, x->x_floatlist); // get's destroyed in the dtor
    iemnet__streamout(x->x_msgout, x->x_floatlist->argc, x->x_floatlist->argv, x->x_serialize);
  } else {
    // disconnected
    tcpclient_disconnect(x);
  }
}

static void tcpclient_serialize(t_tcpclient *x, t_floatarg doit) {
  x->x_serialize=doit;
}


/* constructor/destructor */

static void *tcpclient_new(void)
{
  t_tcpclient *x = (t_tcpclient *)pd_new(tcpclient_class);
  x->x_msgout = outlet_new(&x->x_obj, 0);	/* received data */
  x->x_addrout = outlet_new(&x->x_obj, gensym("list"));
  x->x_connectout = outlet_new(&x->x_obj, gensym("float"));	/* connection state */
  x->x_statusout = outlet_new(&x->x_obj, 0);/* last outlet for everything else */

  x->x_serialize=1;

  x->x_fd = -1;

  x->x_addr = 0L;
  x->x_port = 0;

  x->x_sender=NULL;
  x->x_receiver=NULL;

  x->x_clock = clock_new(x, (t_method)tcpclient_tick);
  x->x_floatlist=iemnet__floatlist_create(1024);

  /* prepare child thread */
  if(pthread_attr_init(&x->x_threadattr) < 0)
    error("%s: warning: could not prepare child thread", objName);
  if(pthread_attr_setdetachstate(&x->x_threadattr, PTHREAD_CREATE_DETACHED) < 0)
    error("%s: warning: could not prepare child thread", objName);
    

  return (x);
}

static void tcpclient_free(t_tcpclient *x)
{
  tcpclient_disconnect(x);
  if(x->x_clock)clock_free(x->x_clock);x->x_clock=NULL;
	if(x->x_floatlist)iemnet__floatlist_destroy(x->x_floatlist);x->x_floatlist=NULL;
}

IEMNET_EXTERN void tcpclient_setup(void)
{
  if(!iemnet__register(objName))return;
  tcpclient_class = class_new(gensym(objName), (t_newmethod)tcpclient_new,
                              (t_method)tcpclient_free,
                              sizeof(t_tcpclient), 0, A_DEFFLOAT, 0);
  class_addmethod(tcpclient_class, (t_method)tcpclient_connect, gensym("connect")
                  , A_SYMBOL, A_FLOAT, 0);
  class_addmethod(tcpclient_class, (t_method)tcpclient_disconnect, gensym("disconnect"), 0);

  class_addmethod(tcpclient_class, (t_method)tcpclient_serialize, gensym("serialize"), A_FLOAT, 0);

  class_addmethod(tcpclient_class, (t_method)tcpclient_send, gensym("send"), A_GIMME, 0);
  class_addlist(tcpclient_class, (t_method)tcpclient_send);

  class_addbang(tcpclient_class, (t_method)tcpclient_info);
  DEBUGMETHOD(tcpclient_class);
}


IEMNET_INITIALIZER(tcpclient_setup);




/* end of tcpclient.c */
