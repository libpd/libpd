/* udpsend.c
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

static const char objName[] = "udpsend";

#include "iemnet.h"
#include <string.h>

static t_class *udpsend_class;

typedef struct _udpsend
{
  t_object x_obj;
  t_iemnet_sender*x_sender;
} t_udpsend;

static void udpsend_connect(t_udpsend *x, t_symbol *hostname,
			    t_floatarg fportno)
{
  struct sockaddr_in  server;
  int                 sockfd;
  int                 portno = fportno;
  int                 broadcast = 1;/* nonzero is true */

  if (x->x_sender)
    {
      error("[%s] already connected", objName);
      return;
    }

  /* create a socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  DEBUG("send socket %d\n", sockfd);
  if (sockfd < 0)
    {
      sys_sockerror("[udpsend] socket");
      return;
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

  do {
#if 0
    struct addrinfo * addr=NULL;
    if(getaddrinfo(hostname->s_name, NULL, NULL, &addr)) {
      error("[%s] bad host '%s'?", objName, hostname->s_name);
      return;
    } else {
      struct addrinfo * res;
      for (res = addr; res != NULL; res = res->ai_next) {
	struct sockaddr_in *sa = (struct sockaddr_in *) res->ai_addr;
	int len = res->ai_addrlen;
	//      memcpy((char *)&server.sin_addr, (char *)res->ai_addr, hp->h_length);
	// LATER check how to do that...
      }
    }
    freeaddrinfo(addr);
#else
    struct hostent      *hp = gethostbyname(hostname->s_name);
    if (hp == 0)
      {
	error("[%s] bad host '%s'?", objName, hostname->s_name);
	return;
      }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
#endif
  } while(0);

  /* assign client port number */
  server.sin_port = htons((u_short)portno);

  DEBUG("connecting to port %d", portno);
  /* try to connect. */
  if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
      sys_sockerror("[udpsend] connecting stream socket");
      sys_closesocket(sockfd);
      return;
    }
  x->x_sender=iemnet__sender_create(sockfd);
  outlet_float(x->x_obj.ob_outlet, 1);
}

static void udpsend_disconnect(t_udpsend *x)
{
  if(x->x_sender) {
    iemnet__sender_destroy(x->x_sender);
    x->x_sender=NULL;
    outlet_float(x->x_obj.ob_outlet, 0);
  }
}

static void udpsend_send(t_udpsend *x, t_symbol *s, int argc, t_atom *argv)
{
  if(x->x_sender) {
    t_iemnet_chunk*chunk=iemnet__chunk_create_list(argc, argv);
    iemnet__sender_send(x->x_sender, chunk);
    iemnet__chunk_destroy(chunk);
  } else {
    error("[%s]: not connected", objName);
  }
}

static void udpsend_free(t_udpsend *x)
{
  udpsend_disconnect(x);
}

static void *udpsend_new(void)
{
  t_udpsend *x = (t_udpsend *)pd_new(udpsend_class);
  outlet_new(&x->x_obj, gensym("float"));
  x->x_sender=NULL;
  return (x);
}

IEMNET_EXTERN void udpsend_setup(void)
{
  if(!iemnet__register(objName))return;
  udpsend_class = class_new(gensym(objName), (t_newmethod)udpsend_new,
			    (t_method)udpsend_free,
			    sizeof(t_udpsend), 0, 0);

  class_addmethod(udpsend_class, (t_method)udpsend_connect,
		  gensym("connect"), A_SYMBOL, A_FLOAT, 0);
  class_addmethod(udpsend_class, (t_method)udpsend_disconnect,
		  gensym("disconnect"), 0);

  class_addmethod(udpsend_class, (t_method)udpsend_send, gensym("send"),
		  A_GIMME, 0);
  class_addlist(udpsend_class, (t_method)udpsend_send);
  DEBUGMETHOD(udpsend_class);
}

IEMNET_INITIALIZER(udpsend_setup);

/* end udpsend.c*/

