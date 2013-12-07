/* tcpserver.c
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
#include "iemnet_data.h"
#include <stdio.h>

#define MAX_CONNECT 32 /* maximum number of connections */

/* ----------------------------- tcpserver ------------------------- */

static t_class *tcpserver_class;
static const char objName[] = "tcpserver";

typedef struct _tcpserver_socketreceiver
{
  struct _tcpserver *sr_owner;

  long           sr_host;
  unsigned short sr_port;
  t_int          sr_fd;
  t_iemnet_sender*sr_sender;
  t_iemnet_receiver*sr_receiver;
} t_tcpserver_socketreceiver;

typedef struct _tcpserver
{
  t_object                    x_obj;
  t_outlet                    *x_msgout;
  t_outlet                    *x_connectout;
  t_outlet                    *x_sockout; // legacy
  t_outlet                    *x_addrout; // legacy
  t_outlet                    *x_statout; 

  int                          x_serialize;

  t_tcpserver_socketreceiver  *x_sr[MAX_CONNECT]; /* socket per connection */
  t_int                       x_nconnections;

  t_int                       x_connectsocket;    /* socket waiting for new connections */
  t_int                       x_port;

  int                         x_defaulttarget; /* the default connection to send to; 0=broadcast; >0 use this client; <0 exclude this client */
	t_iemnet_floatlist         *x_floatlist;
} t_tcpserver;

static void tcpserver_receive_callback(void*x, t_iemnet_chunk*);

static t_tcpserver_socketreceiver *tcpserver_socketreceiver_new(t_tcpserver *owner, int sockfd, struct sockaddr_in*addr)
{
  t_tcpserver_socketreceiver *x = (t_tcpserver_socketreceiver *)getbytes(sizeof(*x));
  if(NULL==x) {
    error("%s_socketreceiver: unable to allocate %d bytes", objName, (int)sizeof(*x));
    return NULL;
  } else {
    x->sr_owner=owner;

    x->sr_fd=sockfd;

    x->sr_host=ntohl(addr->sin_addr.s_addr);
    x->sr_port=ntohs(addr->sin_port);

    x->sr_sender=iemnet__sender_create(sockfd);
    x->sr_receiver=iemnet__receiver_create(sockfd, x, tcpserver_receive_callback);
  }
  return (x);
}

static void tcpserver_socketreceiver_free(t_tcpserver_socketreceiver *x)
{
  DEBUG("freeing %x", x);
  if (x != NULL)
    {
      int sockfd=x->sr_fd;
      t_iemnet_sender*sender=x->sr_sender;
      t_iemnet_receiver*receiver=x->sr_receiver;

      x->sr_owner=NULL;
      x->sr_sender=NULL;
      x->sr_receiver=NULL;

      x->sr_fd=-1;
		
		
	  if(sender)  iemnet__sender_destroy(sender);
	  if(receiver)iemnet__receiver_destroy(receiver);
		
	   sys_closesocket(sockfd);
		

      freebytes(x, sizeof(*x));
    }
  DEBUG("freeed %x", x);
}

static int tcpserver_socket2index(t_tcpserver*x, int sockfd)
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
static int tcpserver_fixindex(t_tcpserver*x, int client)
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


/* ---------------- tcpserver info ---------------------------- */
static void tcpserver_info_client(t_tcpserver *x, int client)
{
  // "client <id> <socket> <IP> <port>"
  // "bufsize <id> <insize> <outsize>"
  static t_atom output_atom[4];
  if(x&&x->x_sr&&x->x_sr[client]) {
    int sockfd = x->x_sr[client]->sr_fd;
    unsigned short port   = x->x_sr[client]->sr_port;
    long address = x->x_sr[client]->sr_host;
    char hostname[MAXPDSTRING];

    int insize =iemnet__receiver_getsize(x->x_sr[client]->sr_receiver);
    int outsize=iemnet__sender_getsize  (x->x_sr[client]->sr_sender  );

    snprintf(hostname, MAXPDSTRING-1, "%d.%d.%d.%d", 
             (unsigned char)((address & 0xFF000000)>>24),
             (unsigned char)((address & 0x0FF0000)>>16),
             (unsigned char)((address & 0x0FF00)>>8),
             (unsigned char)((address & 0x0FF)));
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


static void tcpserver_info(t_tcpserver *x) {
  static t_atom output_atom[4];
  int sockfd=x->x_connectsocket;


  int port=x->x_port;

  if(sockfd<0) {
    // no open port
    post("no valid sock");
  }

  if(x->x_port<=0) {
    struct sockaddr_in  server;
    socklen_t           serversize=sizeof(server);
    if(!getsockname(sockfd, (struct sockaddr *)&server, &serversize)) {
      x->x_port=ntohs(server.sin_port);
      port=x->x_port;
    } else {
      post("gesockname failed for %d", sockfd);
    }
  }

  SETFLOAT (output_atom+0, port);
  outlet_anything( x->x_statout, gensym("port"), 1, output_atom);
}


static void tcpserver_info_connection(t_tcpserver *x, t_tcpserver_socketreceiver*y)
{
  iemnet__addrout(x->x_statout, x->x_addrout, y->sr_host, y->sr_port);
  outlet_float(x->x_sockout, y->sr_fd);
}

/* ---------------- main tcpserver (send) stuff --------------------- */
static void tcpserver_disconnect_socket(t_tcpserver *x, t_floatarg fsocket);
static void tcpserver_send_bytes(t_tcpserver*x, int client, t_iemnet_chunk*chunk)
{
  DEBUG("send_bytes to %x -> %x[%d]", x, x->x_sr, client);
  if(x->x_sr)DEBUG("client %X", x->x_sr[client]);
  if(x && x->x_sr && x->x_sr[client]) {
    t_atom                  output_atom[3];
    int size=-1;

    t_iemnet_sender*sender=sender=x->x_sr[client]->sr_sender;
    int sockfd = x->x_sr[client]->sr_fd;

    if(sender) {
      size=iemnet__sender_send(sender, chunk);
    }

    SETFLOAT(&output_atom[0], client+1);
    SETFLOAT(&output_atom[1], size);
    SETFLOAT(&output_atom[2], sockfd);
    outlet_anything( x->x_statout, gensym("sent"), 3, output_atom);

    if(size<0) {
      // disconnected!
      tcpserver_disconnect_socket(x, sockfd);
    }
  }
}



/* broadcasts a message to all connected clients but the given one */
static void tcpserver_send_butclient(t_tcpserver *x, int but, int argc, t_atom *argv)
{
  int client=0;
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);

  /* enumerate through the clients and send each the message */
  for(client = 0; client < x->x_nconnections; client++)	/* check if connection exists */
    {
      /* socket exists for this client */
      if(client!=but)tcpserver_send_bytes(x, client, chunk);
    }
  iemnet__chunk_destroy(chunk);
}
/* sends a message to a given client */
static void tcpserver_send_toclient(t_tcpserver *x, int client, int argc, t_atom *argv)
{
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);
  tcpserver_send_bytes(x, client, chunk);
  iemnet__chunk_destroy(chunk);
}



/* send message to client using client number
   note that the client numbers might change in case a client disconnects! */
/* clients start at 1 but our index starts at 0 */
static void tcpserver_send_client(t_tcpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int client=0;
      
  if (argc > 0)
    {
      client=tcpserver_fixindex(x, atom_getint(argv));
      if(client<0)return;
      if(argc==1) {
        tcpserver_info_client(x, client);
      } else {
        tcpserver_send_toclient(x, client, argc-1, argv+1);
      }
      return;
    }
  else 
    {
      for(client=0; client<x->x_nconnections; client++)
        tcpserver_info_client(x, client);
    }
}

/* broadcasts a message to all connected clients */
static void tcpserver_broadcast(t_tcpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int     client;
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);

  /* enumerate through the clients and send each the message */
  for(client = 0; client < x->x_nconnections; client++)	/* check if connection exists */
    {
      /* socket exists for this client */
      tcpserver_send_bytes(x, client, chunk);
    }
  iemnet__chunk_destroy(chunk);
}

/* broadcasts a message to all connected clients */
static void tcpserver_broadcastbut(t_tcpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int but=-1;
  //int client=0;
  //  t_iemnet_chunk*chunk=NULL;

  if(argc<2) {
    return;
  }
  if((but=tcpserver_fixindex(x, atom_getint(argv)))<0)return;
  tcpserver_send_butclient(x, but, argc-1, argv+1);
}

static void tcpserver_defaultsend(t_tcpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int client=-1;
  int sockfd=x->x_defaulttarget;
  if(0==sockfd) 
    tcpserver_broadcast(x, s, argc, argv);
  else if(sockfd>0) {
    client=tcpserver_socket2index(x, sockfd);
    tcpserver_send_toclient(x, client, argc, argv);
  } else if(sockfd<0) {
    client=tcpserver_socket2index(x, -sockfd);
    tcpserver_send_butclient(x, client, argc, argv);     
  }
}
static void tcpserver_defaulttarget(t_tcpserver *x, t_floatarg f)
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
static void tcpserver_targetsocket(t_tcpserver *x, t_floatarg f)
{
  int sockfd=f;
  x->x_defaulttarget=sockfd;
}



/* send message to client using socket number */
static void tcpserver_send_socket(t_tcpserver *x, t_symbol *s, int argc, t_atom *argv)
{
  int     client = -1;
  t_iemnet_chunk*chunk=NULL;
  if(argc) {
    client = tcpserver_socket2index(x, atom_getint(argv));
    if(client<0)return;
  } else {
    pd_error(x, "%s_send: no socket specified", objName);
    return;
  }

  /* get socket number of connection (first element in list) */
  if(argc && argv->a_type == A_FLOAT)
    {
      int sockfd=atom_getint(argv);
      client = tcpserver_socket2index(x, sockfd);
      if(client < 0)
        {
          post("%s_send: no connection on socket %d", objName, sockfd);
          return;
        }
    }
  else
    {
      post("%s_send: no socket specified", objName);
      return;
    }
  
  chunk=iemnet__chunk_create_list(argc-1, argv+1);
  tcpserver_send_bytes(x, client, chunk);
  iemnet__chunk_destroy(chunk);
}

static void tcpserver_disconnect(t_tcpserver *x, int client)
{
  int k;
  DEBUG("disconnect %x %d", x, client);
  tcpserver_info_connection(x, x->x_sr[client]);

  tcpserver_socketreceiver_free(x->x_sr[client]);
  x->x_sr[client]=NULL;

  /* rearrange list now: move entries to close the gap */
  for(k = client; k < x->x_nconnections; k++)
    {
      x->x_sr[k] = x->x_sr[k + 1];
    }
  x->x_sr[k + 1]=NULL;
  x->x_nconnections--;


  outlet_float(x->x_connectout, x->x_nconnections);
}


/* disconnect a client by number */
static void tcpserver_disconnect_client(t_tcpserver *x, t_floatarg fclient)
{
  int client = tcpserver_fixindex(x, fclient);

  if(client<0)return;
  tcpserver_disconnect(x, client);
}


/* disconnect a client by socket */
static void tcpserver_disconnect_socket(t_tcpserver *x, t_floatarg fsocket)
{
  int id=tcpserver_socket2index(x, (int)fsocket);
  if(id>=0)
    tcpserver_disconnect_client(x, id+1);
}



/* disconnect a client by socket */
static void tcpserver_disconnect_all(t_tcpserver *x)
{
  int id=x->x_nconnections;
  while(--id>=0) {
    tcpserver_disconnect(x, id);
  }
}

/* ---------------- main tcpserver (receive) stuff --------------------- */
static void tcpserver_receive_callback(void *y0, 
				       t_iemnet_chunk*c) {
  t_tcpserver_socketreceiver *y=(t_tcpserver_socketreceiver*)y0;
  t_tcpserver*x=NULL;
  if(NULL==y || NULL==(x=y->sr_owner))return;
  
  if(c) {
    tcpserver_info_connection(x, y);
	  x->x_floatlist=iemnet__chunk2list(c, x->x_floatlist); // get's destroyed in the dtor
    iemnet__streamout(x->x_msgout, x->x_floatlist->argc, x->x_floatlist->argv, x->x_serialize);
  } else {
    // disconnected
    int sockfd=y->sr_fd;
    verbose(1, "[%s] got disconnection for socket:%d", objName, sockfd);
    tcpserver_disconnect_socket(x, sockfd);
  }

  //  post("tcpserver: %d bytes in %d packets", bytecount, packetcount);
}

static void tcpserver_connectpoll(t_tcpserver *x)
{
  struct sockaddr_in  incomer_address;
  socklen_t           sockaddrl = sizeof( struct sockaddr );
  int                 fd = accept(x->x_connectsocket, (struct sockaddr*)&incomer_address, &sockaddrl);
  int                 i;

  if (fd < 0) post("%s: accept failed", objName);
  else
    {
      t_tcpserver_socketreceiver *y = tcpserver_socketreceiver_new((void *)x, fd, &incomer_address);
      if (!y)
        {
          sys_closesocket(fd);
          return;
        }
      x->x_nconnections++;
      i = x->x_nconnections - 1;
      x->x_sr[i] = y;

      tcpserver_info_connection(x, y);
    }

  outlet_float(x->x_connectout, x->x_nconnections);
}

static void tcpserver_port(t_tcpserver*x, t_floatarg fportno)
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
    sys_rmpollfn(sockfd);
    sys_closesocket(sockfd);
    x->x_connectsocket=-1;
    x->x_port=-1;
  }


  sockfd = socket(AF_INET, SOCK_STREAM, 0);


  server.sin_family = AF_INET;

  /* LATER allow setting of inaddr */
  server.sin_addr.s_addr = INADDR_ANY;

  /* assign server port number */
  server.sin_port = htons((u_short)portno);
  /* name the socket */
  if (bind(sockfd, (struct sockaddr *)&server, serversize) < 0)
    {
      sys_sockerror("tcpserver: bind");
      sys_closesocket(sockfd);
      outlet_anything(x->x_statout, gensym("port"), 1, ap);
      return;
    }

  /* streaming protocol */
  if (listen(sockfd, 5) < 0)
    {
      sys_sockerror("tcpserver: listen");
      sys_closesocket(sockfd);
      sockfd = -1;
      outlet_anything(x->x_statout, gensym("port"), 1, ap);
      return;
    }
  else
    {
      sys_addpollfn(sockfd, (t_fdpollfn)tcpserver_connectpoll, x); // wait for new connections 
    }

  x->x_connectsocket = sockfd;
  x->x_port = portno;


  // find out which port is actually used (useful when assigning "0")
  if(!getsockname(sockfd, (struct sockaddr *)&server, &serversize)) {
    x->x_port=ntohs(server.sin_port);
  }


  SETFLOAT(ap, x->x_port);
  outlet_anything(x->x_statout, gensym("port"), 1, ap);
}

static void tcpserver_serialize(t_tcpserver *x, t_floatarg doit) {
  x->x_serialize=doit;
}


static void *tcpserver_new(t_floatarg fportno)
{
  t_tcpserver         *x;
  int                 i;

  x = (t_tcpserver *)pd_new(tcpserver_class);

  x->x_msgout = outlet_new(&x->x_obj, 0); /* 1st outlet for received data */
  x->x_connectout = outlet_new(&x->x_obj, gensym("float")); /* 2nd outlet for number of connected clients */
  x->x_sockout = outlet_new(&x->x_obj, gensym("float"));
  x->x_addrout = outlet_new(&x->x_obj, gensym("list" ));
  x->x_statout = outlet_new(&x->x_obj, 0);/* 5th outlet for everything else */

  x->x_serialize=1;

  x->x_connectsocket = -1;
  x->x_port = -1;
  x->x_nconnections = 0;

  for(i = 0; i < MAX_CONNECT; i++)
    {
      x->x_sr[i] = NULL;
    }

  x->x_defaulttarget=0;
	
	
  x->x_floatlist=iemnet__floatlist_create(1024);

  tcpserver_port(x, fportno);

  return (x);
}

static void tcpserver_free(t_tcpserver *x)
{
  int     i;

  for(i = 0; i < MAX_CONNECT; i++)
    {
      if (NULL!=x->x_sr[i]) {
        DEBUG("[%s] free %x", objName, x);
        tcpserver_socketreceiver_free(x->x_sr[i]);
        x->x_sr[i]=NULL;
      }
    }
  if (x->x_connectsocket >= 0)
    {
      sys_rmpollfn(x->x_connectsocket);
      sys_closesocket(x->x_connectsocket);
    }
	if(x->x_floatlist)iemnet__floatlist_destroy(x->x_floatlist);x->x_floatlist=NULL;
}

IEMNET_EXTERN void tcpserver_setup(void)
{
  if(!iemnet__register(objName))return;

  tcpserver_class = class_new(gensym(objName),(t_newmethod)tcpserver_new, (t_method)tcpserver_free,
                              sizeof(t_tcpserver), 0, A_DEFFLOAT, 0);
  class_addmethod(tcpserver_class, (t_method)tcpserver_disconnect_client, gensym("disconnectclient"), A_DEFFLOAT, 0);
  class_addmethod(tcpserver_class, (t_method)tcpserver_disconnect_socket, gensym("disconnectsocket"), A_DEFFLOAT, 0);
  class_addmethod(tcpserver_class, (t_method)tcpserver_disconnect_all, gensym("disconnect"), 0);

  class_addmethod(tcpserver_class, (t_method)tcpserver_send_socket, gensym("send"), A_GIMME, 0);
  class_addmethod(tcpserver_class, (t_method)tcpserver_send_client, gensym("client"), A_GIMME, 0);

  class_addmethod(tcpserver_class, (t_method)tcpserver_broadcast, gensym("broadcast"), A_GIMME, 0);

  class_addmethod(tcpserver_class, (t_method)tcpserver_defaulttarget, gensym("target"), A_DEFFLOAT, 0);
  class_addmethod(tcpserver_class, (t_method)tcpserver_targetsocket, gensym("targetsocket"), A_DEFFLOAT, 0);
  class_addlist  (tcpserver_class, (t_method)tcpserver_defaultsend);


  class_addmethod(tcpserver_class, (t_method)tcpserver_serialize, gensym("serialize"), A_FLOAT, 0);


  class_addmethod(tcpserver_class, (t_method)tcpserver_port, gensym("port"), A_DEFFLOAT, 0);
  class_addbang  (tcpserver_class, (t_method)tcpserver_info);

  DEBUGMETHOD(tcpserver_class);
}

IEMNET_INITIALIZER(tcpserver_setup);


/* end of tcpserver.c */
