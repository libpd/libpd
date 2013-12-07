/* tcpsend.c
 * copyright (c) 2010 IOhannes m zmölnig, IEM
 * copyright (c) 2006-2010 Martin Peach
 * copyright (c) Miller Puckette
 */

/*                                                                              */
/* A client for unidirectional communication from within Pd.                     */
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

#define DEBUGLEVEL 1

static const char objName[] = "tcpsend";

#include "iemnet.h"
#include <string.h>

#ifndef _WIN32
# include <netinet/tcp.h>
#endif


static t_class *tcpsend_class;

typedef struct _tcpsend
{
  t_object x_obj;
  int      x_fd;
  t_iemnet_sender*x_sender;
} t_tcpsend;

static void tcpsend_disconnect(t_tcpsend *x)
{
  if (x->x_fd >= 0)
    {
      if(x->x_sender)iemnet__sender_destroy(x->x_sender); x->x_sender=NULL;
      sys_closesocket(x->x_fd);
      x->x_fd = -1;
      outlet_float(x->x_obj.ob_outlet, 0);
      post("tcpsend: disconnected");
    }
}



static void tcpsend_connect(t_tcpsend *x, t_symbol *hostname,
			    t_floatarg fportno)
{
  struct sockaddr_in  server;
  struct hostent      *hp;
  int                 sockfd;
  int                 portno = fportno;
  int                 intarg;

  if (x->x_fd >= 0)
    {
      error("tcpsend: already connected");
      return;
    }

  /* create a socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DEBUG("send socket %d\n", sockfd);
  if (sockfd < 0)
    {
      sys_sockerror("tcpsend: socket");
      return;
    }
  /* connect socket using hostname provided in command line */
  server.sin_family = AF_INET;
  hp = gethostbyname(hostname->s_name);
  if (hp == 0)
    {
      post("tcpsend: bad host?\n");
      return;
    }
  /* for stream (TCP) sockets, specify "nodelay" */
  intarg = 1;
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
		 (char *)&intarg, sizeof(intarg)) < 0)
    post("tcpsend: setsockopt (TCP_NODELAY) failed\n");

  memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

  /* assign client port number */
  server.sin_port = htons((u_short)portno);

  post("tcpsend: connecting to port %d", portno);
  /* try to connect. */
  if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
      sys_sockerror("tcpsend: connecting stream socket");
      sys_closesocket(sockfd);
      return;
    }
  x->x_fd = sockfd;

  x->x_sender=iemnet__sender_create(sockfd);

  outlet_float(x->x_obj.ob_outlet, 1);
}

static void tcpsend_send(t_tcpsend *x, t_symbol *s, int argc, t_atom *argv)
{
  int size=0;
  t_iemnet_sender*sender=x->x_sender;
  t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);
  if(sender && chunk) {
    size=iemnet__sender_send(sender, chunk);
  }
  iemnet__chunk_destroy(chunk);
}

static void tcpsend_free(t_tcpsend *x)
{
  tcpsend_disconnect(x);
}

static void *tcpsend_new(void)
{
  t_tcpsend *x = (t_tcpsend *)pd_new(tcpsend_class);
  outlet_new(&x->x_obj, gensym("float"));
  x->x_fd = -1;
  return (x);
}

IEMNET_EXTERN void tcpsend_setup(void)
{
  if(!iemnet__register(objName))return;
  tcpsend_class = class_new(gensym(objName), 
			    (t_newmethod)tcpsend_new, (t_method)tcpsend_free,
			    sizeof(t_tcpsend), 
			    0, 0);

  class_addmethod(tcpsend_class, (t_method)tcpsend_connect,
		  gensym("connect"), A_SYMBOL, A_FLOAT, 0);
  class_addmethod(tcpsend_class, (t_method)tcpsend_disconnect,
		  gensym("disconnect"), 0);
  class_addmethod(tcpsend_class, (t_method)tcpsend_send, gensym("send"),
		  A_GIMME, 0);
  class_addlist(tcpsend_class, (t_method)tcpsend_send);

  DEBUGMETHOD(tcpsend_class);
}

IEMNET_INITIALIZER(tcpsend_setup);


/* end tcpsend.c */
