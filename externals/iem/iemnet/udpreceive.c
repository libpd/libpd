/* udpreceive.c
 * copyright (c) 2010 IOhannes m zmölnig, IEM
 * copyright (c) 2006-2010 Martin Peach
 * copyright (c) Miller Puckette
 */

/*                                                                              */
/* A server for unidirectional communication from within Pd.                    */
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

#define DEBUGLEVEL 1

static const char objName[] = "udpreceive";

#include "iemnet.h"

/* ----------------------------- udpreceive ------------------------- */

static t_class *udpreceive_class;

typedef struct _udpreceive
{
  t_object  x_obj;
  t_outlet  *x_msgout;
  t_outlet  *x_addrout;
  t_outlet  *x_statout;

  int       x_connectsocket;
  int       x_port;
  t_iemnet_receiver*x_receiver;
	t_iemnet_floatlist         *x_floatlist;
} t_udpreceive;


static void udpreceive_read_callback(void*y, t_iemnet_chunk*c) {
  t_udpreceive*x=(t_udpreceive*)y;
  if(c) {
    iemnet__addrout(x->x_statout, x->x_addrout, c->addr, c->port);
    x->x_floatlist=iemnet__chunk2list(c, x->x_floatlist); // gets destroyed in the dtor
    outlet_list(x->x_msgout, gensym("list"), x->x_floatlist->argc, x->x_floatlist->argv);
  } else {
    post("[%s] nothing received", objName);
  }
}

static void udpreceive_port(t_udpreceive*x, t_floatarg fportno)
{
  static t_atom ap[1];
  int                 portno = fportno;
  struct sockaddr_in  server;
  socklen_t           serversize=sizeof(server);
  int sockfd = x->x_connectsocket;
  int intarg;

  SETFLOAT(ap, -1);
  if(x->x_port == portno) {
    return;
  }

  /* cleanup any open ports */
  if(sockfd>=0) {
    iemnet__receiver_destroy(x->x_receiver);
    x->x_connectsocket=-1;
    x->x_port=-1;
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd<0) {
    error("[%s]: unable to create socket", objName);
    return;
  }

  /* ask OS to allow another Pd to reopen this port after we close it. */
#ifdef SO_REUSEADDR
  intarg = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&intarg, sizeof(intarg)) 
      < 0) {
    error("[%s]: setsockopt (SO_REUSEADDR) failed", objName);
  }
#endif /* SO_REUSEADDR */
#ifdef SO_REUSEPORT
  intarg = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                 (char *)&intarg, sizeof(intarg)) 
      < 0) {
    error("[%s]: setsockopt (SO_REUSEPORT) failed", objName);
  }
#endif /* SO_REUSEPORT */


  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons((u_short)portno);

  /* name the socket */
  if (bind(sockfd, (struct sockaddr *)&server, serversize) < 0)
    {
      sys_sockerror("[udpreceive] bind failed");
      sys_closesocket(sockfd);
      sockfd = -1;
      outlet_anything(x->x_statout, gensym("port"), 1, ap);
      return;
    }

  x->x_connectsocket = sockfd;
  x->x_port = portno;

  // find out which port is actually used (useful when assigning "0")
  if(!getsockname(sockfd, (struct sockaddr *)&server, &serversize)) {
    x->x_port=ntohs(server.sin_port);
  }

  x->x_receiver=iemnet__receiver_create(sockfd,
					x, 
					udpreceive_read_callback);

  SETFLOAT(ap, x->x_port);
  outlet_anything(x->x_statout, gensym("port"), 1, ap);
}


static void *udpreceive_new(t_floatarg fportno)
{
    t_udpreceive*x = (t_udpreceive *)pd_new(udpreceive_class);

    x->x_msgout = outlet_new(&x->x_obj, 0);
    x->x_addrout = outlet_new(&x->x_obj, gensym("list"));
    x->x_statout = outlet_new(&x->x_obj, 0);

    x->x_connectsocket = -1;
    x->x_port = -1;
    x->x_receiver = NULL;

    x->x_floatlist=iemnet__floatlist_create(1024);

    udpreceive_port(x, fportno);

    return (x);
}

static void udpreceive_free(t_udpreceive *x)
{
  iemnet__receiver_destroy(x->x_receiver);
  x->x_connectsocket=0;

  outlet_free(x->x_msgout);
  outlet_free(x->x_addrout);
  outlet_free(x->x_statout);

	if(x->x_floatlist)iemnet__floatlist_destroy(x->x_floatlist);x->x_floatlist=NULL;
}

IEMNET_EXTERN void udpreceive_setup(void)
{
  if(!iemnet__register(objName))return;
    udpreceive_class = class_new(gensym(objName),
        (t_newmethod)udpreceive_new, (t_method)udpreceive_free,
        sizeof(t_udpreceive), 0, A_DEFFLOAT, 0);

    class_addmethod(udpreceive_class, (t_method)udpreceive_port, 
		    gensym("port"), A_DEFFLOAT, 0);

  DEBUGMETHOD(udpreceive_class);
}

IEMNET_INITIALIZER(udpreceive_setup);

/* end udpreceive.c */
