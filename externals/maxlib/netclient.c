/* --------------------------  netclient  ------------------------------------- */
/*                                                                              */
/* Extended 'netsend', connects to 'netserver'.                                 */
/* Uses child thread to connect to server. Thus needs pd0.35-test17 or later.   */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at                                                                */
/* http://pure-data.svn.sourceforge.net/viewvc/pure-data/trunk/externals/maxlib/src/netclient.c */
/* March 26 2010                                                                */
/* Additional fixes and improvements by Ivica Ico Bukvic <ico@bukvic.net>       */
/* for the purpose of L2Ork http://l2ork.music.vt.edu                           */
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
/* Changes: use circular message buffer for receive */
/*          using pollfunction instead of clock (suggested by HCS) */

#include "m_pd.h"
#include "s_stuff.h"
#include "m_imp.h"

#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

#define INBUFSIZE 4096	/* maximum numbers of characters to read */

static char *version = "netclient v0.3.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

static t_class *netclient_class;
static t_binbuf *inbinbuf;
static int netclient_instance_count;

typedef struct _netclient
{
    t_object x_obj;
	t_clock *x_clock;
	t_outlet *x_outdata;
	t_outlet *x_outconnect;
    int x_fd;
	char *x_hostname;
	int x_connectstate;
	int x_port;
    int x_protocol;
	char x_inbuf[INBUFSIZE];	/* circular message buffer for received data */
	int x_intail;
	int x_inhead;
		/* multithread stuff */
	pthread_t x_threadid;            /* id of child thread */
	pthread_attr_t x_threadattr;     /* attributes of child thread */
} t_netclient;

	/* one lonely prototype */
static void netclient_rcv(t_netclient *x);


	/* gets called when connection status has changed */
static void netclient_tick(t_netclient *x)
{
    outlet_float(x->x_outconnect, x->x_connectstate);
    /* add pollfunction for checking for input */
	if (x->x_connectstate == 1)
    {
        sys_addpollfn(x->x_fd, (t_fdpollfn)netclient_rcv, x);
    }
}

static void *netclient_child_connect(void *w)
{
	t_netclient *x = (t_netclient*) w;
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno = x->x_port;
    if (x->x_fd >= 0)
    {
    	error("netclient_connect: already connected");
    	return (x);
    }

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
    hp = gethostbyname(x->x_hostname);
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
    x->x_fd = sockfd;
		/* outlet_float is not threadsafe ! */
    // outlet_float(x->x_obj.ob_outlet, 1);
	x->x_connectstate = 1;
		/* use callback instead to set outlet */
	clock_delay(x->x_clock, 0);
	return (x);
}

static void netclient_connect(t_netclient *x, t_symbol *hostname,
    t_floatarg fportno)
{
		/* we get hostname and port and pass them on
		to the child thread that establishes the connection */
	x->x_hostname = hostname->s_name;
	x->x_port = fportno;
	x->x_connectstate = 0;
		/* start child thread */
	if(pthread_create( &x->x_threadid, &x->x_threadattr, netclient_child_connect, x) < 0)
		post("netclient: could not create new thread");
}

static void netclient_disconnect(t_netclient *x)
{
    if (x->x_fd >= 0)
    {
        sys_rmpollfn(x->x_fd);
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
        x->x_connectstate = 0;
        //outlet_float(x->x_outconnect, 0);
        clock_delay(x->x_clock, 0); /* output connect state later */
   }
}

static void netclient_send(t_netclient *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_fd >= 0)
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
	    double timebefore = sys_getrealtime();
    	    int res = send(x->x_fd, buf, length-sent, 0);
    	    double timeafter = sys_getrealtime();
    	    int late = (timeafter - timebefore > 0.005);
    	    if (late || pleasewarn)
    	    {
    	    	if (timeafter > lastwarntime + 2)
    	    	{
    	    	     post("netclient blocked %d msec",
    	    	     	(int)(1000 * ((timeafter - timebefore) + pleasewarn)));
    	    	     pleasewarn = 0;
    	    	     lastwarntime = timeafter;
    	    	}
    	    	else if (late) pleasewarn += timeafter - timebefore;
    	    }
    	    if (res <= 0)
    	    {
    		sys_sockerror("netclient");
    		netclient_disconnect(x);
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
    else error("netclient: not connected");
}

    /* this is in a separately called subroutine so that the buffer isn't
    sitting on the stack while the messages are getting passed. */
static int netclient_doread(t_netclient *x)
{
    char messbuf[INBUFSIZE], *bp = messbuf;
    int indx;
    int inhead = x->x_inhead;
    int intail = x->x_intail;
    char *inbuf = x->x_inbuf;
    if (intail == inhead) return (0);
    for (indx = intail; indx != inhead; indx = (indx+1)&(INBUFSIZE-1))
    {
    	char c = *bp++ = inbuf[indx];
    	if (c == ';' && (!indx || inbuf[indx-1] != '\\'))
    	{
    	    intail = (indx+1)&(INBUFSIZE-1);
    	    binbuf_text(inbinbuf, messbuf, bp - messbuf);
    	    x->x_inhead = inhead;
    	    x->x_intail = intail;
    	    return (1);
    	}
    }
    return (0);
}

static void netclient_rcv(t_netclient *x)
{
	int fd = x->x_fd;
	int ret;
	fd_set readset;
	fd_set exceptset;
    struct timeval ztout;
		/* output data */
    int msg, natom;
    t_atom *at;
	
	if(x->x_connectstate)
	{
			/* check if we can read/write from/to the socket */
		FD_ZERO(&readset);
		FD_ZERO(&exceptset);
		FD_SET(fd, &readset );
		FD_SET(fd, &exceptset );

		ztout.tv_sec = 0;
		ztout.tv_usec = 0;
		ret = select(fd+1, &readset, NULL, &exceptset, &ztout);
		if(ret < 0)
		{
			error("netclient: can not read from socket");
            //sys_closesocket(fd);
            netclient_disconnect(x);
			return;
		}
		if(FD_ISSET(fd, &readset) || FD_ISSET(fd, &exceptset))
		{
			int readto = (x->x_inhead >= x->x_intail ? INBUFSIZE : x->x_intail-1);

			ret = recv(fd, x->x_inbuf + x->x_inhead, readto - x->x_inhead, 0);
			if (ret < 0)
			{
				sys_sockerror("recv");
                //sys_rmpollfn(fd);
                //sys_closesocket(fd);
                netclient_disconnect(x);
			}
			else if (ret == 0)
			{
	    		post("netclient: connection closed on socket %d", fd);
				//sys_rmpollfn(fd);
	    		//sys_closesocket(fd);
                netclient_disconnect(x);
			}
			else
			{
    			x->x_inhead += ret;
    			if (x->x_inhead >= INBUFSIZE) x->x_inhead = 0;
    			while (netclient_doread(x))
				{
					/* output binbuf */
					natom = binbuf_getnatom(inbinbuf);	/* get number of atoms */
					at = binbuf_getvec(inbinbuf);	/* get the atoms */
						/* now split it into several parts at every A_SEMI because
						   we probably received more than one message at a time     */
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
	    							pd_error(x, "netserver: -- got dollar sign in message");
									goto nodice;
								}
							if (at[msg].a_type == A_FLOAT)
							{
	    						if (emsg > msg + 1)
									outlet_list(x->x_outdata, 0, emsg-msg, at + msg);
								else outlet_float(x->x_outdata, at[msg].a_w.w_float);
							}
							else if (at[msg].a_type == A_SYMBOL)
	    						outlet_anything(x->x_outdata, at[msg].a_w.w_symbol,
								emsg-msg-1, at + msg + 1);
						}
						nodice:
    						msg = emsg + 1;
					}
 	    		}
			}
		}
	}
	else post("netclient: not connected");
}

static void *netclient_new(t_floatarg udpflag)
{
    t_netclient *x = (t_netclient *)pd_new(netclient_class);
    x->x_outdata = outlet_new(&x->x_obj, &s_anything);	/* received data */
    x->x_outconnect = outlet_new(&x->x_obj, &s_float);	/* connection state */
    x->x_clock = clock_new(x, (t_method)netclient_tick);
    if(netclient_instance_count == 0)
        inbinbuf = binbuf_new();
    x->x_fd = -1;
    x->x_protocol = (udpflag != 0 ? SOCK_DGRAM : SOCK_STREAM);
		/* prepare child thread */
    if(pthread_attr_init(&x->x_threadattr) < 0)
       post("netclient: warning: could not prepare child thread" );
    if(pthread_attr_setdetachstate(&x->x_threadattr, PTHREAD_CREATE_DETACHED) < 0)
       post("netclient: warning: could not prepare child thread" );
	netclient_instance_count++;
    return (x);
}

static void netclient_free(t_netclient *x)
{
    netclient_disconnect(x);
    clock_free(x->x_clock);
	netclient_instance_count--;
    if(netclient_instance_count == 0)
        binbuf_free(inbinbuf);
}

#ifndef MAXLIB
void netclient_setup(void)
{
    netclient_class = class_new(gensym("netclient"), (t_newmethod)netclient_new,
    	(t_method)netclient_free,
    	sizeof(t_netclient), 0, A_DEFFLOAT, 0);
    class_addmethod(netclient_class, (t_method)netclient_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netclient_class, (t_method)netclient_disconnect, gensym("disconnect"), 0);
    class_addmethod(netclient_class, (t_method)netclient_send, gensym("send"), A_GIMME, 0);
	class_addmethod(netclient_class, (t_method)netclient_rcv, gensym("receive"), 0);
	class_addmethod(netclient_class, (t_method)netclient_rcv, gensym("rcv"), 0);
    
	logpost(NULL, 4, version);
}
#else
void maxlib_netclient_setup(void)
{
    netclient_class = class_new(gensym("maxlib_netclient"), (t_newmethod)netclient_new,
    	(t_method)netclient_free,
    	sizeof(t_netclient), 0, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)netclient_new, gensym("netclient"), A_DEFFLOAT, 0);
    class_addmethod(netclient_class, (t_method)netclient_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netclient_class, (t_method)netclient_disconnect, gensym("disconnect"), 0);
    class_addmethod(netclient_class, (t_method)netclient_send, gensym("send"), A_GIMME, 0);
	class_addmethod(netclient_class, (t_method)netclient_rcv, gensym("receive"), 0);
	class_addmethod(netclient_class, (t_method)netclient_rcv, gensym("rcv"), 0);
    class_sethelpsymbol(netclient_class, gensym("maxlib/netclient-help.pd"));
}
#endif
