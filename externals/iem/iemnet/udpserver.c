/* udpserver.c
 *
 * listens on a UDP-socket for bi-directional communication
 *
 * copyright (c) 2010 IOhannes m zmölnig, IEM
 * copyright (c) 2006-2010 Martin Peach
 * copyright (c) 2004 Olaf Matthes
 */

/*                                                                              */
/* A server for bidirectional communication from within Pd.                     */
/* Allows to send back data to specific clients connected to the server.        */
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
#include <stdio.h>
#include <stdlib.h>

#define MAX_CONNECT 32 /* maximum number of connections */

/* ----------------------------- udpserver ------------------------- */

static t_class *udpserver_class;
static const char objName[] = "udpserver";

typedef struct _udpserver_sender
{
  struct _udpserver *sr_owner;

  long           sr_host;
  unsigned short sr_port;
  t_int          sr_fd;
  t_iemnet_sender*sr_sender;
} t_udpserver_sender;

typedef struct _udpserver
{
  t_object                    x_obj;
  t_outlet                   *x_msgout;
  t_outlet                   *x_connectout;
  t_outlet                   *x_sockout; // legacy
  t_outlet                   *x_addrout; // legacy
  t_outlet                   *x_statout;

  t_udpserver_sender         *x_sr[MAX_CONNECT]; /* socket per connection */
  t_int                       x_nconnections;

  t_int                       x_connectsocket;    /* socket waiting for new connections */
  t_int                       x_port;
  unsigned char               x_accept; /* whether we accept new connections or not */

  int                         x_defaulttarget; /* the default connection to send to; 0=broadcast; >0 use this client; <0 exclude this client */

  t_iemnet_receiver          *x_receiver;
  t_iemnet_floatlist         *x_floatlist;
} t_udpserver;

static t_udpserver_sender *udpserver_sender_new(t_udpserver *owner,  unsigned long host, unsigned short port)
{
  t_udpserver_sender *x = (t_udpserver_sender *)malloc(sizeof(t_udpserver_sender));
  if(NULL==x) {
    error("%s_sender: unable to allocate %d bytes", objName, (int)sizeof(*x));
    return NULL;
  } else {
    int sockfd = owner->x_connectsocket;
    x->sr_owner=owner;

    x->sr_fd=sockfd;

    x->sr_host=host; //ntohl(addr->sin_addr.s_addr);
    x->sr_port=port; //ntohs(addr->sin_port);

    x->sr_sender=iemnet__sender_create(sockfd);
  }
  return (x);
}

static void udpserver_sender_free(t_udpserver_sender *x)
{
  DEBUG("freeing %x", x);
  if (x != NULL)
    {
      int sockfd=x->sr_fd;
      t_iemnet_sender*sender=x->sr_sender;

      x->sr_owner=NULL;
      x->sr_sender=NULL;

      x->sr_fd=-1;

      free(x);

      if(sender)  iemnet__sender_destroy(sender);

      sys_closesocket(sockfd);
    }
  DEBUG("freeed %x", x);
}

static t_udpserver_sender* udpserver_sender_copy(t_udpserver_sender*x) {
  return udpserver_sender_new(x->sr_owner,x->sr_host, x->sr_port);
}

static int udpserver_socket2index(t_udpserver*x, int sockfd)
{
  int i=0;
  for(i = 0; i < x->x_nconnections; i++) /* check if connection exists */
    {
      if(x->x_sr[i]->sr_fd == sockfd)
        {
          return i;
        }
    }
  return -1;
}

/* checks whether client is a valid (1-based) index
 *  if the id is invalid, returns -1
 *  if the id is valid, return the 0-based index (client-1)
 */
static int udpserver_fixindex(t_udpserver*x, int client)
{
  if(x->x_nconnections <= 0)
    {
      pd_error(x, "[%s]: no clients connected", objName);
      return -1;
    }

  if (!((client > 0) && (client <= x->x_nconnections)))
    {
      pd_error(x, "[%s] client %d out of range [1..%d]", objName, client, (int)(x->x_nconnections));
      return -1;
    }
  return (client-1);
}


/* returns 1 if addr1==addr2, 0 otherwise */
static int equal_addr(unsigned long host1, unsigned short port1,  unsigned long host2, unsigned short port2) {
  return (
          ((port1 == port2) &&
           (host1 == host2))
          );
}


static int udpserver__find_sender(t_udpserver*x,  unsigned long host, unsigned short port) {
  int i=0;
  for(i=0; i<x->x_nconnections; i++) {
    if(NULL==x->x_sr[i])return -1;
    if(equal_addr(host, port, x->x_sr[i]->sr_host, x->x_sr[i]->sr_port))return i;
  }
  return -1;
}

/**
 * check whether the sender is already registered
 * if not, add it to the list of registered senders
 */
static t_udpserver_sender* udpserver_sender_add(t_udpserver*x,
						unsigned long host, unsigned short port )
{
  int id=-1;

  if(!x->x_accept)return NULL;

  id=udpserver__find_sender(x, host, port);
  DEBUG("%X:%d -> %d", host, port, id);
  if(id<0) {
#if 1
    /* since udp is a connection-less protocol we have no way of knowing the currently connected clients
     * the following 3 lines assume, that there is only one client connected (the last we got data from
     */
    id=0;
    udpserver_sender_free(x->x_sr[id]);
    x->x_sr[id]=udpserver_sender_new(x, host, port);
    x->x_nconnections=1;
#else
    /* this is a more optimistic approach as above:
     * every client that we received data from is added to the list of receivers
     * the idea is to remove the sender, if it's known to not receive any data
     */
    id=x->x_nconnections;
    /* an unknown address! add it */
    if(id<MAX_CONNECT) {
      x->x_sr[id]=udpserver_sender_new(x, host, port);
      DEBUG("new sender[%d]= %x", id, x->x_sr[id]);
      x->x_nconnections++;
    } else {
      // oops, no more senders!
      id=-1;
    }
#endif
  }
  DEBUG("sender_add: %d", id);
  if(id>=0) {
    return x->x_sr[id];
  }

  return NULL;
}

static void udpserver_sender_remove(t_udpserver*x, int id) {
  if(id>=0 && id<x->x_nconnections && x->x_sr[id]) {
    int i;

    t_udpserver_sender* sdr=x->x_sr[id];
    udpserver_sender_free(sdr);

    // close the gap by shifting the remaining connections to the left
    for(i=id; i<x->x_nconnections; i++) {
      x->x_sr[id]=x->x_sr[id+1];
    }
    x->x_sr[id]=NULL;

    x->x_nconnections--;
  }
}




/* ---------------- udpserver info ---------------------------- */
static void udpserver_info_client(t_udpserver *x, int client)
{
  // "client <id> <socket> <IP> <port>"
  // "bufsize <id> <insize> <outsize>"
  static t_atom output_atom[4];
  if(x&&x->x_sr&&x->x_sr[client]) {
    int sockfd = x->x_sr[client]->sr_fd;
    unsigned short port   = x->x_sr[client]->sr_port;
    long address = x->x_sr[client]->sr_host;
    char hostname[MAXPDSTRING];

    int insize =iemnet__receiver_getsize(x->x_receiver);
    int outsize=iemnet__sender_getsize  (x->x_sr[client]->sr_sender  );

    snprintf(hostname, MAXPDSTRING-1, "%d.%d.%d.%d",
             (unsigned char)((address & 0xFF000000)>>24),
             (unsigned char)((address & 0x0FF0000)>>16),
             (unsigned char)((address & 0x0FF00)>>8),
	     (unsigned char)((address & 0x0FF))
	     );
    hostname[MAXPDSTRING-1]=0;

    SETFLOAT (output_atom+0, client+1);
    SETFLOAT (output_atom+1, sockfd);
    SETSYMBOL(output_atom+2, gensym(hostname));
    SETFLOAT (output_atom+3, port);

    outlet_anything( x->x_statout, gensym("client"), 4, output_atom);

    SETFLOAT (output_atom+0, client+1);
    SETFLOAT (output_atom+1, insize);
    SETFLOAT (output_atom+2, outsize);
    outlet_anything( x->x_statout, gensym("bufsize"), 3, output_atom);
  }
}


static void udpserver_info(t_udpserver *x) {
  static t_atom output_atom[4];
  int sockfd=x->x_connectsocket;


  int port=x->x_port;

  if(sockfd<0) {
    // no open port
    error("[%s] no valid sock", objName);
  }


  if(x->x_port<=0) {
    struct sockaddr_in  server;
    socklen_t           serversize=sizeof(server);
    if(!getsockname(sockfd, (struct sockaddr *)&server, &serversize)) {
      x->x_port=ntohs(server.sin_port);
      port=x->x_port;
    } else {
      error("[%s] gesockname failed for %d", objName, sockfd);
    }
  }

  SETFLOAT (output_atom+0, port);
  outlet_anything( x->x_statout, gensym("port"), 1, output_atom);
}


static void udpserver_info_connection(t_udpserver *x, t_udpserver_sender*y)
{
  iemnet__addrout(x->x_statout, x->x_addrout, y->sr_host, y->sr_port);
  //  outlet_float(x->x_sockout, y->sr_fd);
}

/* ---------------- main udpserver (send) stuff --------------------- */
static void udpserver_disconnect_socket(t_udpserver *x, t_floatarg fsocket);
static void udpserver_send_bytes(t_udpserver*x, int client, t_iemnet_chunk*chunk)
{
  DEBUG("send_bytes to %x -> %x[%d]", x, x->x_sr, client);
  if(x->x_sr)DEBUG("client %X", x->x_sr[client]);
  if(x && x->x_sr && x->x_sr[client]) {
    t_atom                  output_atom[3];
    int size=0;

    t_iemnet_sender*sender=sender=x->x_sr[client]->sr_sender;
    int sockfd = x->x_sr[client]->sr_fd;

    chunk->addr=x->x_sr[client]->sr_host;
    chunk->port=x->x_sr[client]->sr_port;

    if(sender) {
      size=iemnet__sender_send(sender, chunk);
    }

    SETFLOAT(&output_atom[0], client+1);
    SETFLOAT(&output_atom[1], size);
    SETFLOAT(&output_atom[2], sockfd);
    outlet_anything( x->x_statout, gensym("sent"), 3, output_atom);

    if(size<0) {
      // disconnected!
      udpserver_disconnect_socket(x, sockfd);
    }
  }
}



/* broadcasts a message to all connected clients but the given one */
static void udpserver_send_butclient(t_udpserver *x, int but, int argc, t_atom *argv)
{
  int client=0;
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);

  /* enumerate through the clients and send each the message */
  for(client = 0; client < x->x_nconnections; client++)	/* check if connection exists */
    {
      /* socket exists for this client */
      if(client!=but)udpserver_send_bytes(x, client, chunk);
    }
  iemnet__chunk_destroy(chunk);
}
/* sends a message to a given client */
static void udpserver_send_toclient(t_udpserver *x, int client, int argc, t_atom *argv)
{
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);
  udpserver_send_bytes(x, client, chunk);
  iemnet__chunk_destroy(chunk);
}



/* send message to client using client number
   note that the client numbers might change in case a client disconnects! */
/* clients start at 1 but our index starts at 0 */
static void udpserver_send_client(t_udpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int client=0;

  if (argc > 0)
    {
      client=udpserver_fixindex(x, atom_getint(argv));
      if(client<0)return;
      if(argc==1) {
        udpserver_info_client(x, client);
      } else {
        udpserver_send_toclient(x, client, argc-1, argv+1);
      }
      return;
    }
  else
    {
      for(client=0; client<x->x_nconnections; client++)
        udpserver_info_client(x, client);
    }
}

/* broadcasts a message to all connected clients */
static void udpserver_broadcast(t_udpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int     client;
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);

  DEBUG("broadcasting to %d clients", x->x_nconnections);

  /* enumerate through the clients and send each the message */
  for(client = 0; client < x->x_nconnections; client++)	/* check if connection exists */
    {
      /* socket exists for this client */
      udpserver_send_bytes(x, client, chunk);
    }
  iemnet__chunk_destroy(chunk);
}

/* broadcasts a message to all connected clients */
static void udpserver_broadcastbut(t_udpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int but=-1;

  if(argc<2) {
    return;
  }
  if((but=udpserver_fixindex(x, atom_getint(argv)))<0)return;
  udpserver_send_butclient(x, but, argc-1, argv+1);
}

static void udpserver_defaultsend(t_udpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int client=-1;
  int sockfd=x->x_defaulttarget;
  DEBUG("sending to sockfd: %d", sockfd);
  if(0==sockfd)
    udpserver_broadcast(x, s, argc, argv);
  else if(sockfd>0) {
    client=udpserver_socket2index(x, sockfd);
    udpserver_send_toclient(x, client, argc, argv);
  } else if(sockfd<0) {
    client=udpserver_socket2index(x, -sockfd);
    udpserver_send_butclient(x, client, argc, argv);
  }
}
static void udpserver_defaulttarget(t_udpserver *x, t_floatarg f)
{
  int sockfd=0;
  int rawclient=f;
  int client=(rawclient<0)?(-rawclient):rawclient;

  if(client > x->x_nconnections) {
    error("[%s] target %d out of range [0..%d]", objName, client, (int)(x->x_nconnections));
    return;
  }

  // map the client to a persistant socket
  if(client>0) {
    sockfd=x->x_sr[client-1]->sr_fd;
  }

  if(rawclient<0)sockfd=-sockfd;

  x->x_defaulttarget=sockfd;
}
static void udpserver_targetsocket(t_udpserver *x, t_floatarg f)
{
  int sockfd=f;
  x->x_defaulttarget=sockfd;
}



/* send message to client using socket number */
static void udpserver_send_socket(t_udpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int     client = -1;
  t_iemnet_chunk*chunk=NULL;
  if(argc) {
    client = udpserver_socket2index(x, atom_getint(argv));
    if(client<0)return;
  } else {
    pd_error(x, "%s_send: no socket specified", objName);
    return;
  }

  /* get socket number of connection (first element in list) */
  if(argc && argv->a_type == A_FLOAT)
    {
      int sockfd=atom_getint(argv);
      client = udpserver_socket2index(x, sockfd);
      if(client < 0)
        {
          error("[%s]: no connection on socket %d", objName, sockfd);
          return;
        }
    }
  else
    {
      error("[%s]: no socket specified", objName);
      return;
    }

  chunk=iemnet__chunk_create_list(argc-1, argv+1);
  udpserver_send_bytes(x, client, chunk);
  iemnet__chunk_destroy(chunk);
}

static void udpserver_disconnect(t_udpserver *x, int client)
{
  t_udpserver_sender*sdr;
  int conns;
  DEBUG("disconnect %x %d", x, client);

  if(client<0 || client >= x->x_nconnections)return;

  sdr=udpserver_sender_copy(x->x_sr[client]);

  udpserver_sender_remove(x, client);
  conns=x->x_nconnections;


  udpserver_info_connection(x, sdr);
  outlet_float(x->x_connectout, conns);
}


/* disconnect a client by number */
static void udpserver_disconnect_client(t_udpserver *x, t_floatarg fclient)
{
  int client = udpserver_fixindex(x, fclient);

  if(client<0)return;
  udpserver_disconnect(x, client);
}


/* disconnect a client by socket */
static void udpserver_disconnect_socket(t_udpserver *x, t_floatarg fsocket)
{
  int id=udpserver_socket2index(x, (int)fsocket);
  if(id>=0)
    udpserver_disconnect_client(x, id+1);
}



/* disconnect a client by socket */
static void udpserver_disconnect_all(t_udpserver *x)
{
  int id=x->x_nconnections;
  while(--id>=0) {
    udpserver_disconnect(x, id);
  }
}

/* whether we should accept new connections */
static void udpserver_accept(t_udpserver *x, t_float f) {
  x->x_accept=(unsigned char)f;
}


/* ---------------- main udpserver (receive) stuff --------------------- */
static void udpserver_receive_callback(void *y, t_iemnet_chunk*c) {
  t_udpserver*x=(t_udpserver*)y;
  if(NULL==y)return;

  if(c) {
    int conns = x->x_nconnections;
    t_udpserver_sender*sdr=NULL;
    DEBUG("add new sender from %d", c->port);
    sdr=udpserver_sender_add(x, c->addr, c->port);
    DEBUG("added new sender from %d", c->port);
    if(sdr) {
      udpserver_info_connection(x, sdr);
      x->x_floatlist=iemnet__chunk2list(c, x->x_floatlist); // gets destroyed in the dtor

      /* here we might have a reentrancy problem */
      if(conns!=x->x_nconnections) {
        outlet_float(x->x_connectout, x->x_nconnections);
      }
      outlet_list(x->x_msgout, gensym("list"), x->x_floatlist->argc, x->x_floatlist->argv);
    }
  } else {
    // disconnection never happens with a connectionless protocol like UDP
    pd_error(x, "[%s] received disconnection event", objName);
  }
}


// this get's never called
static void udpserver_connectpoll(t_udpserver *x)
{
  struct sockaddr_in  incomer_address;
  socklen_t           sockaddrl = sizeof( struct sockaddr );
  int                 fd = -1;
  int                 i;

  // TODO: provide a way to not accept connection
  // idea: add a message "accept $1" to turn off/on acceptance of new connections
  fd = accept(x->x_connectsocket, (struct sockaddr*)&incomer_address, &sockaddrl);

  bug("connectpoll");

  if (fd < 0) error("[%s] accept failed", objName);
  else
    {
      unsigned long host  = ntohl(incomer_address.sin_addr.s_addr);
      unsigned short port = ntohs(incomer_address.sin_port);

      t_udpserver_sender *y = udpserver_sender_new(x, host, port);
      if (!y)
        {
          sys_closesocket(fd);
          return;
        }
      x->x_nconnections++;
      i = x->x_nconnections - 1;
      x->x_sr[i] = y;

      udpserver_info_connection(x, y);
    }

  outlet_float(x->x_connectout, x->x_nconnections);
}

static void udpserver_port(t_udpserver*x, t_floatarg fportno)
{
  static t_atom ap[1];
  int                 portno = fportno;
  struct sockaddr_in  server;
  socklen_t           serversize=sizeof(server);
  int sockfd = x->x_connectsocket;
  SETFLOAT(ap, -1);
  if(x->x_port == portno) {
    return;
  }

  /* cleanup any open ports */
  if(sockfd>=0) {
    //sys_rmpollfn(sockfd);
    sys_closesocket(sockfd);
    x->x_connectsocket=-1;
    x->x_port=-1;
  }


  sockfd = socket(AF_INET, SOCK_DGRAM, 0);


  server.sin_family = AF_INET;

  /* LATER allow setting of inaddr */
  server.sin_addr.s_addr = INADDR_ANY;

  /* assign server port number */
  server.sin_port = htons((u_short)portno);
  /* name the socket */
  if (bind(sockfd, (struct sockaddr *)&server, serversize) < 0)
    {
      sys_sockerror("udpserver: bind");
      sys_closesocket(sockfd);
      outlet_anything(x->x_statout, gensym("port"), 1, ap);
      return;
    }

  x->x_receiver=iemnet__receiver_create(sockfd,
					x,
					udpserver_receive_callback);

  x->x_connectsocket = sockfd;
  x->x_port = portno;


  // find out which port is actually used (useful when assigning "0")
  if(!getsockname(sockfd, (struct sockaddr *)&server, &serversize)) {
    x->x_port=ntohs(server.sin_port);
  }


  SETFLOAT(ap, x->x_port);
  outlet_anything(x->x_statout, gensym("port"), 1, ap);
}

static void *udpserver_new(t_floatarg fportno)
{
  t_udpserver         *x;
  int                 i;

  x = (t_udpserver *)pd_new(udpserver_class);

  x->x_msgout = outlet_new(&x->x_obj, 0); /* 1st outlet for received data */
  x->x_connectout = outlet_new(&x->x_obj, gensym("float")); /* 2nd outlet for number of connected clients */
  x->x_sockout = outlet_new(&x->x_obj, gensym("float"));
  x->x_addrout = outlet_new(&x->x_obj, gensym("list" ));
  x->x_statout = outlet_new(&x->x_obj, 0);/* 5th outlet for everything else */

  x->x_connectsocket = -1;
  x->x_port = -1;
  x->x_nconnections = 0;

  for(i = 0; i < MAX_CONNECT; i++)
    {
      x->x_sr[i] = NULL;
    }

  x->x_defaulttarget=0;
  x->x_floatlist=iemnet__floatlist_create(1024);

  udpserver_port(x, fportno);

  x->x_accept=1;

  return (x);
}

static void udpserver_free(t_udpserver *x)
{
  int     i;

  for(i = 0; i < MAX_CONNECT; i++)
    {
      if (NULL!=x->x_sr[i]) {
        DEBUG("[%s] free %x", objName, x);
        udpserver_sender_free(x->x_sr[i]);
        x->x_sr[i]=NULL;
      }
    }
  if (x->x_connectsocket >= 0)
    {
      //sys_rmpollfn(x->x_connectsocket);
      sys_closesocket(x->x_connectsocket);
    }
	if(x->x_floatlist)iemnet__floatlist_destroy(x->x_floatlist);x->x_floatlist=NULL;
}

IEMNET_EXTERN void udpserver_setup(void)
{
  if(!iemnet__register(objName))return;
  error("[%s] does not work yet", objName);

  udpserver_class = class_new(gensym(objName),(t_newmethod)udpserver_new, (t_method)udpserver_free,
                              sizeof(t_udpserver), 0, A_DEFFLOAT, 0);
  class_addmethod(udpserver_class, (t_method)udpserver_disconnect_client, gensym("disconnectclient"), A_DEFFLOAT, 0);
  class_addmethod(udpserver_class, (t_method)udpserver_disconnect_socket, gensym("disconnectsocket"), A_DEFFLOAT, 0);
  class_addmethod(udpserver_class, (t_method)udpserver_disconnect_all, gensym("disconnect"), 0);

  class_addmethod(udpserver_class, (t_method)udpserver_accept, gensym("accept"), A_FLOAT, 0);

  class_addmethod(udpserver_class, (t_method)udpserver_send_socket, gensym("send"), A_GIMME, 0);
  class_addmethod(udpserver_class, (t_method)udpserver_send_client, gensym("client"), A_GIMME, 0);

  class_addmethod(udpserver_class, (t_method)udpserver_broadcast, gensym("broadcast"), A_GIMME, 0);

  class_addmethod(udpserver_class, (t_method)udpserver_defaulttarget, gensym("target"), A_DEFFLOAT, 0);
  class_addmethod(udpserver_class, (t_method)udpserver_targetsocket, gensym("targetsocket"), A_DEFFLOAT, 0);
  class_addlist  (udpserver_class, (t_method)udpserver_defaultsend);

  class_addmethod(udpserver_class, (t_method)udpserver_port, gensym("port"), A_DEFFLOAT, 0);
  class_addbang  (udpserver_class, (t_method)udpserver_info);

  DEBUGMETHOD(udpserver_class);
}

IEMNET_INITIALIZER(udpserver_setup);


/* end of udpserver.c */
