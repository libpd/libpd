/* --------------------------  netdist  --------------------------------------- */
/*                                                                              */
/* Distributes incoming data to a changeable list of netreceive objects.        */
/* Uses child thread to connect to clients. Thus needs pd0.35-test17 or later.  */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
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

#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

#define MAX_REC 32

static char *version = "netdist v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

static t_class *netdist_class;

typedef struct _netdist
{
    t_object x_obj;
	t_clock *x_clock;
    int x_fd[MAX_REC];
	char *x_hostname[MAX_REC];
	int x_numconnect;
	int x_port[MAX_REC];
    int x_protocol;
		/* multithread stuff */
	pthread_t x_threadid;            /* id of child thread */
	pthread_attr_t x_threadattr;     /* attributes of child thread */
} t_netdist;

static void sys_sockerror(char *s)
{
#ifdef WIN32
    int err = WSAGetLastError();
    if (err == 10054) return;
#else
    int err = errno;
#endif
    post("%s: %s (%d)\n", s, strerror(err), err);
}

static void sys_closesocket(int fd) {

#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}


static void netdist_tick(t_netdist *x)
{
    outlet_float(x->x_obj.ob_outlet, x->x_numconnect + 1);
}

static void *netdist_new(t_floatarg udpflag)
{
	int i;
    t_netdist *x = (t_netdist *)pd_new(netdist_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_clock = clock_new(x, (t_method)netdist_tick);
    for(i = 0; i < MAX_REC; i++)x->x_fd[i] = -1;
	x->x_numconnect = -1;
    x->x_protocol = (udpflag != 0 ? SOCK_DGRAM : SOCK_STREAM);

	/* prepare child thread */
    if(pthread_attr_init(&x->x_threadattr) < 0)
       post("netdist: warning: could not prepare child thread" );
    if(pthread_attr_setdetachstate(&x->x_threadattr, PTHREAD_CREATE_DETACHED) < 0)
       post("netdist: warning: could not prepare child thread" );
    return (x);
}

static void *netdist_child_connect(void *w)
{
	int i;

	t_netdist *x = (t_netdist*) w;
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno;
	i = x->x_numconnect + 1;
	portno = x->x_port[i];
    	/* create a socket */
    sockfd = socket(AF_INET, x->x_protocol, 0);
#if 0
    fprintf(stderr, "send socket %d\n", sockfd);
#endif
    if (sockfd < 0)
    {
    	sys_sockerror("socket");
    	return (x);
    }
		/* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(x->x_hostname[i]);
    if (hp == 0)
    {
		post("bad host?\n");
		return (x);
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

		/* assign client port number */
    server.sin_port = htons((u_short)portno);

    post("connecting to port %d", portno);
		/* try to connect */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
    	sys_sockerror("connecting stream socket");
    	sys_closesocket(sockfd);
    	return (x);
    }
    x->x_fd[i] = sockfd;
		/* outlet_float is not threadsafe ! */
    // outlet_float(x->x_obj.ob_outlet, 1);
	x->x_numconnect++;	/* count connection */
		/* use callback instead to set outlet */
	clock_delay(x->x_clock, 0);
	return (x);
}

static void netdist_connect(t_netdist *x, t_symbol *hostname,
    t_floatarg fportno)
{
	int i;
		/* we get hostname and port and pass them on
		to the child thread that establishes the connection */
	for(i = 0; i <= x->x_numconnect; i++)
	{		/* check if we are already connected */
		if (hostname->s_name == x->x_hostname[i] && fportno == x->x_port[i])
		{
    		error("netdist_connect: already connected");
    		return;
		}
	}
	x->x_hostname[x->x_numconnect + 1] = hostname->s_name;
	x->x_port[x->x_numconnect + 1] = fportno;

		/* start child thread */
	if(pthread_create( &x->x_threadid, &x->x_threadattr, netdist_child_connect, x) < 0)
		post("netdist: could not create new thread");
}

static void netdist_disconnect(t_netdist *x, t_symbol *hostname, t_floatarg port)
{
	int i, j;
	for(i = 0; i <= x->x_numconnect; i++)
	{
		if((hostname->s_name == x->x_hostname[i]) && ((int)port == x->x_port[i]))
		{
				/* search for connection */
			if (x->x_fd[i] >= 0)
			{
    			sys_closesocket(x->x_fd[i]);
    			x->x_fd[i] = -1;
				x->x_numconnect--;
    			outlet_float(x->x_obj.ob_outlet, x->x_numconnect + 1);
				for(j = i; j <= x->x_numconnect; j++)
				{
					x->x_hostname[j] = x->x_hostname[j + 1];
					x->x_port[j] = x->x_port[j + 1];
					x->x_fd[j] = x->x_fd[j + 1];
				}
			}
		}
	}
}

static void netdist_send(t_netdist *x, t_symbol *s, int argc, t_atom *argv)
{
	int i = 0;

	for(i = 0; i <= x->x_numconnect; i++)
	{
		if (x->x_fd[i] >= 0)
		{
			t_binbuf *b = binbuf_new();
			char *buf, *bp;
			int length, sent;
			t_atom at;
			binbuf_add(b, argc, argv);
			SETSEMI(&at);
			binbuf_add(b, 1, &at);
			binbuf_gettext(b, &buf, &length);
			for (bp = buf, sent = 0; sent < length;)
			{
				static double lastwarntime;
				static double pleasewarn;
				double timebefore = clock_getlogicaltime();
    				int res = send(x->x_fd[i], buf, length-sent, 0);
    				double timeafter = clock_getlogicaltime();
    				int late = (timeafter - timebefore > 0.005);
    				if (late || pleasewarn)
    				{
    	    			if (timeafter > lastwarntime + 2)
    	    			{
    	    				 post("netdist blocked %d msec",
    	    	     			(int)(1000 * ((timeafter - timebefore) + pleasewarn)));
    	    				 pleasewarn = 0;
    	    				 lastwarntime = timeafter;
    	    			}
    	    			else if (late) pleasewarn += timeafter - timebefore;
    				}
    				if (res <= 0)
    				{
    					sys_sockerror("netdist");
    					netdist_disconnect(x, gensym(x->x_hostname[i]), x->x_port[i]);
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
	}
	if(x->x_numconnect == -1) error("netdist: not connected");
}

	/* disconnect all */
static void netdist_clear(t_netdist *x)
{
	int i, j, n;
	n = x->x_numconnect;
	for (i = n; i >= 0; i--)
	{
		netdist_disconnect(x, gensym(x->x_hostname[i]), x->x_port[i]);
	}
}

static void netdist_print(t_netdist *x)
{
	int i;
	post("netdist: %d connection(s) established:", x->x_numconnect + 1);
	for (i = x->x_numconnect; i >= 0; i--)
	{
		post("         \"%s\", port %d",x->x_hostname[i], x->x_port[i]);
	}
}

static void netdist_free(t_netdist *x)
{
    netdist_clear(x);
    clock_free(x->x_clock);
}

#ifndef MAXLIB
void netdist_setup(void)
{
    netdist_class = class_new(gensym("netdist"), (t_newmethod)netdist_new,
    	(t_method)netdist_free, sizeof(t_netdist), 0, A_DEFFLOAT, 0);
    class_addmethod(netdist_class, (t_method)netdist_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netdist_class, (t_method)netdist_disconnect, gensym("disconnect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netdist_class, (t_method)netdist_send, gensym("send"), A_GIMME, 0);
	class_addmethod(netdist_class, (t_method)netdist_clear, gensym("clear"), 0);
	class_addmethod(netdist_class, (t_method)netdist_print, gensym("print"), 0);
	
    logpost(NULL, 4, version);
}
#else
void maxlib_netdist_setup(void)
{
    netdist_class = class_new(gensym("maxlib_netdist"), (t_newmethod)netdist_new,
    	(t_method)netdist_free, sizeof(t_netdist), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)netdist_new, gensym("netdist"), A_DEFFLOAT, 0);
    class_addmethod(netdist_class, (t_method)netdist_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netdist_class, (t_method)netdist_disconnect, gensym("disconnect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netdist_class, (t_method)netdist_send, gensym("send"), A_GIMME, 0);
	class_addmethod(netdist_class, (t_method)netdist_clear, gensym("clear"), 0);
	class_addmethod(netdist_class, (t_method)netdist_print, gensym("print"), 0);
	class_sethelpsymbol(netdist_class, gensym("maxlib/netdist-help.pd"));
}
#endif
