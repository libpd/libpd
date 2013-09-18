/* --------------------------  netserver  ------------------------------------- */
/*                                                                              */
/* A server for bidirectional communication from within Pd.                     */
/* Allows to send back data to specific clients connected to the server.        */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib                */
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
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include "s_stuff.h"
#include "m_imp.h"

#include <sys/types.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#define SOCKET_ERROR -1
#endif

#define MAX_CONNECT  32		/* maximum number of connections */
#define INBUFSIZE    4096   /* size of receiving data buffer */

/* message levels from syslog.h */
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

static char *version = 
   "netserver v0.2.hcs1 :: bidirectional communication for Pd\n"
   "             written by Olaf Matthes <olaf.matthes@gmx.de>\n"
   "             syslogging by Hans-Christoph Steiner <hans@eds.org> ";

/* ----------------------------- netserver ------------------------- */

static t_class *netserver_class;
static t_binbuf *inbinbuf;

typedef void (*t_netserver_socketnotifier)(void *x);
typedef void (*t_netserver_socketreceivefn)(void *x, t_binbuf *b);

typedef struct _netserver
{
	  t_object x_obj;
	  t_outlet *x_msgout;
	  t_outlet *x_connectout;
	  t_outlet *x_clientno;
	  t_outlet *x_connectionip;
	  t_symbol *x_host[MAX_CONNECT];
	  t_int    x_fd[MAX_CONNECT];
	  t_int    x_sock_fd;
	  t_int    x_connectsocket;
	  t_int    x_nconnections;
/* for syslog style logging priorities  */
	  t_int    x_log_pri;
} t_netserver;

typedef struct _netserver_socketreceiver
{
	  char *sr_inbuf;
	  int sr_inhead;
	  int sr_intail;
	  void *sr_owner;
	  t_netserver_socketnotifier sr_notifier;
	  t_netserver_socketreceivefn sr_socketreceivefn;
} t_netserver_socketreceiver;

static t_netserver_socketreceiver *netserver_socketreceiver_new(void *owner, t_netserver_socketnotifier notifier,
																t_netserver_socketreceivefn socketreceivefn)
{
   t_netserver_socketreceiver *x = (t_netserver_socketreceiver *)getbytes(sizeof(*x));
   x->sr_inhead = x->sr_intail = 0;
   x->sr_owner = owner;
   x->sr_notifier = notifier;
   x->sr_socketreceivefn = socketreceivefn;
   if (!(x->sr_inbuf = malloc(INBUFSIZE))) bug("t_netserver_socketreceiver");
   return (x);
}

/* this is in a separately called subroutine so that the buffer isn't
   sitting on the stack while the messages are getting passed. */
static int netserver_socketreceiver_doread(t_netserver_socketreceiver *x)
{
   char messbuf[INBUFSIZE], *bp = messbuf;
   int indx;
   int inhead = x->sr_inhead;
   int intail = x->sr_intail;
   char *inbuf = x->sr_inbuf;
   if (intail == inhead) return (0);
   for (indx = intail; indx != inhead; indx = (indx+1)&(INBUFSIZE-1))
   {
	  char c = *bp++ = inbuf[indx];
	  if (c == ';' && (!indx || inbuf[indx-1] != '\\'))
	  {
		 intail = (indx+1)&(INBUFSIZE-1);
		 binbuf_text(inbinbuf, messbuf, bp - messbuf);
		 x->sr_inhead = inhead;
		 x->sr_intail = intail;
		 return (1);
	  }
   }
   return (0);
}

static void netserver_socketreceiver_read(t_netserver_socketreceiver *x, int fd)
{
   char *semi;
   int readto = (x->sr_inhead >= x->sr_intail ? INBUFSIZE : x->sr_intail-1);
   int ret;

   t_netserver *y = x->sr_owner;

   y->x_sock_fd = fd;

   /* the input buffer might be full.  If so, drop the whole thing */
   if (readto == x->sr_inhead)
   {
	  if (y->x_log_pri >= LOG_ERR)
		  post("netserver: dropped message");
	  x->sr_inhead = x->sr_intail = 0;
	  readto = INBUFSIZE;
   }
   else
   {
	  ret = recv(fd, x->sr_inbuf + x->sr_inhead,
				 readto - x->sr_inhead, 0);
	  if (ret < 0)
	  {
		 sys_sockerror("recv");
		 if (x->sr_notifier) (*x->sr_notifier)(x->sr_owner);
		 sys_rmpollfn(fd);
		 sys_closesocket(fd);
	  }
	  else if (ret == 0)
	  {
		 if (y->x_log_pri >= LOG_NOTICE) 
			post("netserver: << connection closed on socket %d", fd);
		 if (x->sr_notifier) (*x->sr_notifier)(x->sr_owner);
		 sys_rmpollfn(fd);
		 sys_closesocket(fd);
	  }
	  else
	  {
		 x->sr_inhead += ret;
		 if (x->sr_inhead >= INBUFSIZE) x->sr_inhead = 0;
		 while (netserver_socketreceiver_doread(x))
		 {
			outlet_setstacklim();
			if (x->sr_socketreceivefn)
			   (*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
			else binbuf_eval(inbinbuf, 0, 0, 0);
		 }
	  }
   }
}

static void netserver_socketreceiver_free(t_netserver_socketreceiver *x)
{
   free(x->sr_inbuf);
   freebytes(x, sizeof(*x));
}

/* ---------------- main netserver (send) stuff --------------------- */

/* send message to client using socket number */
static void netserver_send(t_netserver *x, t_symbol *s, int argc, t_atom *argv)
{
   int sockfd, client = -1, i;
   if(x->x_nconnections < 0)
   {
	  if (x->x_log_pri >= LOG_WARNING)
		  post("netserver: no clients connected");
	  return;
   }
   if(argc < 2)
   {
	  if (x->x_log_pri >= LOG_WARNING)
		 post("netserver: nothing to send");
	  return;
   }
   /* get socket number of connection (first element in list) */
   if(argv[0].a_type == A_FLOAT)
   {
	  sockfd = atom_getfloatarg(0, argc, argv);
	  for(i = 0; i < x->x_nconnections; i++)	/* check if connection exists */
	  {
		 if(x->x_fd[i] == sockfd)
		 {
			client = i;	/* the client we're sending to */
			break;
		 }
	  }
	  if(client == -1)
	  {
		 if (x->x_log_pri >= LOG_CRIT)
			post("netserver: no connection on socket %d", sockfd);
		 return;
	  }
   }
   else
   {
	  if (x->x_log_pri >= LOG_CRIT)
		 post("netserver: no socket specified");
	  return;
   }
   /* process & send data */
   if(sockfd > 0)
   {
	  t_binbuf *b = binbuf_new();
	  char *buf, *bp;
	  int length, sent;
	  t_atom at;
	  binbuf_add(b, argc - 1, argv + 1);	/* skip first element */
	  SETSEMI(&at);
	  binbuf_add(b, 1, &at);
	  binbuf_gettext(b, &buf, &length);

	  if (x->x_log_pri >= LOG_DEBUG)  
	  {
		 post("netserver: sending data to client %d on socket %d", client + 1, sockfd);
		 post("netserver: sending \"%s\"", buf);
	  }

	  for (bp = buf, sent = 0; sent < length;)
	  {
		 static double lastwarntime;
		 static double pleasewarn;
		 double timebefore = clock_getlogicaltime();
		 int res = send(sockfd, buf, length-sent, 0);
		 double timeafter = clock_getlogicaltime();
		 int late = (timeafter - timebefore > 0.005);
		 if (late || pleasewarn)
		 {
			if (timeafter > lastwarntime + 2)
			{
			   if (x->x_log_pri >= LOG_WARNING)
				  post("netserver blocked %d msec",
					   (int)(1000 * ((timeafter - timebefore) + pleasewarn)));
			   pleasewarn = 0;
			   lastwarntime = timeafter;
			}
			else if (late) pleasewarn += timeafter - timebefore;
		 }
		 if (res <= 0)
		 {
			sys_sockerror("netserver");
			if (x->x_log_pri >= LOG_ERR) 
			   post("netserver: could not send data to client");
			break;
		 }
		 else
		 {
			sent += res;
			bp += res;
		 }
	  }
	  t_freebytes(buf, length);
	  binbuf_free(b);
   }
   else if (x->x_log_pri >= LOG_CRIT)
	  post("netserver: not a valid socket number (%d)", sockfd);
}

/* send message to client using client number 
   note that the client numbers might change in case a client disconnects! */
static void netserver_client_send(t_netserver *x, t_symbol *s, int argc, t_atom *argv)
{
   int sockfd, client;
   if(x->x_nconnections < 0)
   {
	  if (x->x_log_pri >= LOG_WARNING)
		 post("netserver: no clients connected");
	  return;
   }
   if(argc < 2)
   {
	  if (x->x_log_pri >= LOG_WARNING)
		 post("netserver: nothing to send");
	  return;
   }
   /* get number of client (first element in list) */
   if(argv[0].a_type == A_FLOAT)
	  client = atom_getfloatarg(0, argc, argv);
   else
   {
	  if (x->x_log_pri >= LOG_WARNING)
		 post("netserver: no client specified");
	  return;
   }
   sockfd = x->x_fd[client - 1];	/* get socket number for that client */

   /* process & send data */
   if(sockfd > 0)
   {
	  t_binbuf *b = binbuf_new();
	  char *buf, *bp;
	  int length, sent;
	  t_atom at;
	  binbuf_add(b, argc - 1, argv + 1);	/* skip first element */
	  SETSEMI(&at);
	  binbuf_add(b, 1, &at);
	  binbuf_gettext(b, &buf, &length);

	  if (x->x_log_pri >= LOG_DEBUG)
	  {
		 post("netserver: sending data to client %d on socket %d", client, sockfd);
		 post("netserver: >> sending \"%s\"", buf);
	  }

	  for (bp = buf, sent = 0; sent < length;)
	  {
		 static double lastwarntime;
		 static double pleasewarn;
		 double timebefore = clock_getlogicaltime();
		 int res = send(sockfd, buf, length-sent, 0);
		 double timeafter = clock_getlogicaltime();
		 int late = (timeafter - timebefore > 0.005);
		 if (late || pleasewarn)
		 {
			if (timeafter > lastwarntime + 2)
			{
			   if (x->x_log_pri >= LOG_WARNING)
				  post("netserver blocked %d msec",
					   (int)(1000 * ((timeafter - timebefore) + pleasewarn)));
			   pleasewarn = 0;
			   lastwarntime = timeafter;
			}
			else if (late) pleasewarn += timeafter - timebefore;
		 }
		 if (res <= 0)
		 {
			sys_sockerror("netserver");
			if (x->x_log_pri >= LOG_ERR)
			   post("netserver: could not send data to cient");
			break;
		 }
		 else
		 {
			sent += res;
			bp += res;
		 }
	  }
	  t_freebytes(buf, length);
	  binbuf_free(b);
   }
   else if (x->x_log_pri >= LOG_CRIT)
	  post("netserver: not a valid socket number (%d)", sockfd);
}

/* broadcasts a message to all connected clients */
static void netserver_broadcast(t_netserver *x, t_symbol *s, int argc, t_atom *argv)
{
   int i, client = x->x_nconnections;	/* number of clients to send to */
   t_atom at[256];
   for(i = 0; i < argc; i++)
   {
	  at[i + 1] = argv[i];
   }
   argc++;
   /* enumerate through the clients and send each the message */
   while(client--)
   {
	  SETFLOAT(at, client + 1);	/* prepend number of client */
	  netserver_client_send(x, s, argc, at);
   }
}


/* ---------------- main netserver (receive) stuff --------------------- */

static void netserver_notify(t_netserver *x)
{
   int i, k;
   /* remove connection from list */
   for(i = 0; i < x->x_nconnections; i++)
   {
	  if(x->x_fd[i] == x->x_sock_fd)
	  {
		 x->x_nconnections--;
		 if (x->x_log_pri >= LOG_NOTICE)
			post("netserver: \"%s\" removed from list of clients", x->x_host[i]->s_name);
		 x->x_host[i] = NULL;	/* delete entry */
		 x->x_fd[i] = -1;
		 /* rearrange list now: move entries to close the gap */
		 for(k = i; k < x->x_nconnections; k++)
		 {
			x->x_host[k] = x->x_host[k + 1];
			x->x_fd[k] = x->x_fd[k + 1];
		 }
	  }
   }
   outlet_float(x->x_connectout, x->x_nconnections);
}

static void netserver_doit(void *z, t_binbuf *b)
{
   t_atom messbuf[1024];
   t_netserver *x = (t_netserver *)z;
   int msg, natom = binbuf_getnatom(b);
   t_atom *at = binbuf_getvec(b);
   int i;
   /* output clients IP and socket no. */
   for(i = 0; i < x->x_nconnections; i++)	/* search for corresponding IP */
   {
	  if(x->x_fd[i] == x->x_sock_fd)
	  {
		 outlet_symbol(x->x_connectionip, x->x_host[i]);
		 break;
	  }
   }
   outlet_float(x->x_clientno, x->x_sock_fd);	/* the socket number */
   /* process data */
   for (msg = 0; msg < natom;)
   {
	  int emsg;
	  for (emsg = msg; emsg < natom && at[emsg].a_type != A_COMMA
			  && at[emsg].a_type != A_SEMI; emsg++);

	  if (emsg > msg)
	  {
		 int ii;
		 for (ii = msg; ii < emsg; ii++)
			if (at[ii].a_type == A_DOLLAR || at[ii].a_type == A_DOLLSYM)
			{
			   pd_error(x, "netserver: got dollar sign in message");
			   goto nodice;
			}
		 if (at[msg].a_type == A_FLOAT)
		 {
			if (emsg > msg + 1)
			   outlet_list(x->x_msgout, 0, emsg-msg, at + msg);
			else outlet_float(x->x_msgout, at[msg].a_w.w_float);
		 }
		 else if (at[msg].a_type == A_SYMBOL)
			outlet_anything(x->x_msgout, at[msg].a_w.w_symbol,
							emsg-msg-1, at + msg + 1);
	  }
	 nodice:
	  msg = emsg + 1;
   }
}

static void netserver_connectpoll(t_netserver *x)
{
   struct sockaddr_in incomer_address;
   int sockaddrl = (int) sizeof( struct sockaddr );
   int fd = accept(x->x_connectsocket, (struct sockaddr*)&incomer_address, &sockaddrl);
   if (fd < 0) post("netserver: accept failed");
   else
   {
	  t_netserver_socketreceiver *y = netserver_socketreceiver_new((void *)x, 
																   (t_netserver_socketnotifier)netserver_notify,
																   (x->x_msgout ? netserver_doit : 0));
	  sys_addpollfn(fd, (t_fdpollfn)netserver_socketreceiver_read, y);
	  x->x_nconnections++;
	  x->x_host[x->x_nconnections - 1] = gensym(inet_ntoa(incomer_address.sin_addr));
	  x->x_fd[x->x_nconnections - 1] = fd;

	  if (x->x_log_pri >= LOG_NOTICE) 
		 post("netserver: ** accepted connection from %s on socket %d", 
			  x->x_host[x->x_nconnections - 1]->s_name, x->x_fd[x->x_nconnections - 1]);
	  outlet_float(x->x_connectout, x->x_nconnections);
   }
}

static void netserver_print(t_netserver *x)
{
   int i;
   if(x->x_nconnections > 0)
   {
	  post("netserver: %d open connections:", x->x_nconnections);

	  for(i = 0; i < x->x_nconnections; i++)
	  {
		 post("        \"%s\" on socket %d", 
			  x->x_host[i]->s_name, x->x_fd[i]);
	  }
   } else post("netserver: no open connections");
}

static void netserver_emerg(t_netserver *x)
{
   x->x_log_pri = LOG_EMERG;
}
static void netserver_alert(t_netserver *x)
{
   x->x_log_pri = LOG_ALERT;
}
static void netserver_crit(t_netserver *x)
{
   x->x_log_pri = LOG_CRIT;
}
static void netserver_err(t_netserver *x)
{
   x->x_log_pri = LOG_ERR;
}
static void netserver_warning(t_netserver *x)
{
   x->x_log_pri = LOG_WARNING;
}
static void netserver_notice(t_netserver *x)
{
   x->x_log_pri = LOG_NOTICE;
}
static void netserver_info(t_netserver *x)
{
   x->x_log_pri = LOG_INFO;
}
static void netserver_debug(t_netserver *x)
{
   x->x_log_pri = LOG_DEBUG;
}

static void *netserver_new(t_floatarg fportno)
{
   t_netserver *x;
   int i;
   struct sockaddr_in server;
   int sockfd, portno = fportno;

   x = (t_netserver *)pd_new(netserver_class);
/* set default debug message level */
   x->x_log_pri = LOG_ERR;
/* create a socket */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (x->x_log_pri >= LOG_NOTICE)
	  post("netserver: receive socket %d", sockfd);
   if (sockfd < 0)
   {
	  sys_sockerror("socket");
	  return (0);
   }
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;

#ifdef IRIX
   /* this seems to work only in IRIX but is unnecessary in
	  Linux.  Not sure what NT needs in place of this. */
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 0, 0) < 0)
	  post("setsockopt failed\n");
#endif

   /* assign server port number */
   server.sin_port = htons((u_short)portno);

   /* name the socket */
   if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
	  sys_sockerror("bind");
	  sys_closesocket(sockfd);
	  return (0);
   }
   x->x_msgout = outlet_new(&x->x_obj, &s_anything);

   /* streaming protocol */
   if (listen(sockfd, 5) < 0)
   {
	  sys_sockerror("listen");
	  sys_closesocket(sockfd);
	  sockfd = -1;
   }
   else
   {
	  sys_addpollfn(sockfd, (t_fdpollfn)netserver_connectpoll, x);
	  x->x_connectout = outlet_new(&x->x_obj, &s_float);
	  x->x_clientno = outlet_new(&x->x_obj, &s_float);
	  x->x_connectionip = outlet_new(&x->x_obj, &s_symbol);
	  inbinbuf = binbuf_new();
   }
   x->x_connectsocket = sockfd;
   x->x_nconnections = 0;
   for(i = 0; i < MAX_CONNECT; i++)x->x_fd[i] = -1;

   return (x);
}

static void netserver_free(t_netserver *x)
{
   int i;
   for(i = 0; i < x->x_nconnections; i++)
   {
	  sys_rmpollfn(x->x_fd[i]);
	  sys_closesocket(x->x_fd[i]);
   }
   if (x->x_connectsocket >= 0)
   {
	  sys_rmpollfn(x->x_connectsocket);
	  sys_closesocket(x->x_connectsocket);
   }
   binbuf_free(inbinbuf);
}

#ifndef MAXLIB
void netserver_setup(void)
{
   netserver_class = class_new(gensym("netserver"),(t_newmethod)netserver_new, (t_method)netserver_free,
							   sizeof(t_netserver), 0, A_DEFFLOAT, 0);
   class_addmethod(netserver_class, (t_method)netserver_print, gensym("print"), 0);
   class_addmethod(netserver_class, (t_method)netserver_send, gensym("send"), A_GIMME, 0);
   class_addmethod(netserver_class, (t_method)netserver_client_send, gensym("client"), A_GIMME, 0);
   class_addmethod(netserver_class, (t_method)netserver_broadcast, gensym("broadcast"), A_GIMME, 0);
/* syslog log level messages */
   class_addmethod(netserver_class, (t_method)netserver_emerg, gensym("emerg"), 0);
   class_addmethod(netserver_class, (t_method)netserver_emerg, gensym("emergency"), 0);
   class_addmethod(netserver_class, (t_method)netserver_alert, gensym("alert"), 0);
   class_addmethod(netserver_class, (t_method)netserver_crit, gensym("crit"), 0);
   class_addmethod(netserver_class, (t_method)netserver_crit, gensym("critical"), 0);
   class_addmethod(netserver_class, (t_method)netserver_err, gensym("err"), 0);
   class_addmethod(netserver_class, (t_method)netserver_err, gensym("error"), 0);
   class_addmethod(netserver_class, (t_method)netserver_err, gensym("quiet"), 0);
   class_addmethod(netserver_class, (t_method)netserver_warning, gensym("warning"), 0);
   class_addmethod(netserver_class, (t_method)netserver_notice, gensym("notice"), 0);
   class_addmethod(netserver_class, (t_method)netserver_info, gensym("info"), 0);
   class_addmethod(netserver_class, (t_method)netserver_info, gensym("verbose"), 0);
   class_addmethod(netserver_class, (t_method)netserver_debug, gensym("debug"), 0);
   
   
   logpost(NULL, 4, version);
}
#else
void maxlib_netserver_setup(void)
{
   netserver_class = class_new(gensym("maxlib_netserver"),(t_newmethod)netserver_new, (t_method)netserver_free,
							   sizeof(t_netserver), 0, A_DEFFLOAT, 0);
   class_addcreator((t_newmethod)netserver_new, gensym("netserver"), A_DEFFLOAT, 0);
   class_addmethod(netserver_class, (t_method)netserver_print, gensym("print"), 0);
   class_addmethod(netserver_class, (t_method)netserver_send, gensym("send"), A_GIMME, 0);
   class_addmethod(netserver_class, (t_method)netserver_client_send, gensym("client"), A_GIMME, 0);
   class_addmethod(netserver_class, (t_method)netserver_broadcast, gensym("broadcast"), A_GIMME, 0);
   class_sethelpsymbol(netserver_class, gensym("maxlib/netserver-help.pd"));
}
#endif
