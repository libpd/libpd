/* --------------------------  netrec  ---------------------------------------- */
/*                                                                              */
/* A 'netreceive' that tells the IP of the connecting netsend.                  */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
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
#include "s_stuff.h"
#include "m_imp.h"

#include <sys/types.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
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

static char *version = "netrec v0.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

/* ----------------------------- netrec ------------------------- */

static t_class *netrec_class;
static t_binbuf *inbinbuf;

typedef void (*t_netrec_socketnotifier)(void *x);
typedef void (*t_netrec_socketreceivefn)(void *x, t_binbuf *b);

typedef struct _netrec
{
    t_object x_obj;
    t_outlet *x_msgout;
    t_outlet *x_connectout;
	t_outlet *x_clientno;
	t_outlet *x_connectionip;
	t_symbol *x_host[MAX_CONNECT];
	t_int    x_fd[MAX_CONNECT];
	t_int    x_sock_fd;
    int x_connectsocket;
    int x_nconnections;
    int x_udp;
} t_netrec;

typedef struct _netrec_socketreceiver
{
    char *sr_inbuf;
    int sr_inhead;
    int sr_intail;
    void *sr_owner;
    int sr_udp;
    t_netrec_socketnotifier sr_notifier;
    t_netrec_socketreceivefn sr_socketreceivefn;
} t_netrec_socketreceiver;

static t_netrec_socketreceiver *netrec_socketreceiver_new(void *owner, t_netrec_socketnotifier notifier,
    t_netrec_socketreceivefn socketreceivefn, int udp)
{
    t_netrec_socketreceiver *x = (t_netrec_socketreceiver *)getbytes(sizeof(*x));
    x->sr_inhead = x->sr_intail = 0;
    x->sr_owner = owner;
    x->sr_notifier = notifier;
    x->sr_socketreceivefn = socketreceivefn;
    x->sr_udp = udp;
    if (!(x->sr_inbuf = malloc(INBUFSIZE))) bug("t_netrec_socketreceiver");
    return (x);
}

    /* this is in a separately called subroutine so that the buffer isn't
    sitting on the stack while the messages are getting passed. */
static int netrec_socketreceiver_doread(t_netrec_socketreceiver *x)
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

static void netrec_socketreceiver_getudp(t_netrec_socketreceiver *x, int fd)
{
    char buf[INBUFSIZE+1];
    int ret = recv(fd, buf, INBUFSIZE, 0);
    if (ret < 0)
    {
		sys_sockerror("recv");
		sys_rmpollfn(fd);
		sys_closesocket(fd);
    }
    else if (ret > 0)
    {
	buf[ret] = 0;
#if 0
	post("%s", buf);
#endif
    	if (buf[ret-1] != '\n')
	{
#if 0
	    buf[ret] = 0;
	    error("dropped bad buffer %s\n", buf);
#endif
	}
	else
	{
	    char *semi = strchr(buf, ';');
	    if (semi) 
	    	*semi = 0;
    	    binbuf_text(inbinbuf, buf, strlen(buf));
	    outlet_setstacklim();
	    if (x->sr_socketreceivefn)
		(*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
    	    else bug("netrec_socketreceiver_getudp");
    	}
    }
}

static void netrec_socketreceiver_read(t_netrec_socketreceiver *x, int fd)
{
    if (x->sr_udp)   /* UDP ("datagram") socket protocol */
    	netrec_socketreceiver_getudp(x, fd);
    else  /* TCP ("streaming") socket protocol */
    {
		char *semi;
		int readto =
			(x->sr_inhead >= x->sr_intail ? INBUFSIZE : x->sr_intail-1);
		int ret;

		t_netrec *y = x->sr_owner;

		y->x_sock_fd = fd;

    			/* the input buffer might be full.  If so, drop the whole thing */
		if (readto == x->sr_inhead)
		{
    			fprintf(stderr, "netrec: dropped message");
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
	    		post("netrec: connection closed on socket %d", fd);
				if (x->sr_notifier) (*x->sr_notifier)(x->sr_owner);
	    		sys_rmpollfn(fd);
	    		sys_closesocket(fd);
			}
			else
			{
    			x->sr_inhead += ret;
    			if (x->sr_inhead >= INBUFSIZE) x->sr_inhead = 0;
    			while (netrec_socketreceiver_doread(x))
				{
					outlet_setstacklim();
					if (x->sr_socketreceivefn)
		    			(*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
    				else binbuf_eval(inbinbuf, 0, 0, 0);
 	    		}
			}
		}
    }
}

static void netrec_socketreceiver_free(t_netrec_socketreceiver *x)
{
    free(x->sr_inbuf);
    freebytes(x, sizeof(*x));
}

/* ---------------- main netrec stuff --------------------- */

static void netrec_notify(t_netrec *x)
{
	int i, k;
		/* remove connection from list */
	for(i = 0; i < x->x_nconnections; i++)
	{
			if(x->x_fd[i] == x->x_sock_fd)
			{
				x->x_nconnections--;
				post("netrec: \"%s\" removed from list of clients", x->x_host[i]->s_name);
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

static void netrec_doit(void *z, t_binbuf *b)
{
    t_atom messbuf[1024];
    t_netrec *x = (t_netrec *)z;
    int msg, natom = binbuf_getnatom(b);
    t_atom *at = binbuf_getvec(b);
	int i;
		/* output clients IP and socket no */
	for(i = 0; i < x->x_nconnections; i++)
	{
		if(x->x_fd[i] == x->x_sock_fd)
		{
			outlet_symbol(x->x_connectionip, x->x_host[i]);
			break;
		}
	}
	outlet_float(x->x_clientno, x->x_sock_fd);
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
	    			pd_error(x, "netrec: got dollar sign in message");
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

static void netrec_connectpoll(t_netrec *x)
{
    struct sockaddr_in incomer_address;
    int sockaddrl = (int) sizeof( struct sockaddr );
    int fd = accept(x->x_connectsocket, (struct sockaddr*)&incomer_address, &sockaddrl);
    if (fd < 0) post("netrec: accept failed");
    else
    {
    	t_netrec_socketreceiver *y = netrec_socketreceiver_new((void *)x, 
    	    (t_netrec_socketnotifier)netrec_notify,
	    	(x->x_msgout ? netrec_doit : 0), 0);
    	sys_addpollfn(fd, (t_fdpollfn)netrec_socketreceiver_read, y);
		x->x_nconnections++;
		x->x_host[x->x_nconnections - 1] = gensym(inet_ntoa(incomer_address.sin_addr));
		x->x_fd[x->x_nconnections - 1] = fd;

		// outlet_symbol( x->x_connectionip, x->x_host[x->x_nconnections - 1]);
		post("netrec: accepted connection from %s on socket %d", 
			x->x_host[x->x_nconnections - 1]->s_name, x->x_fd[x->x_nconnections - 1]);
    	outlet_float(x->x_connectout, x->x_nconnections);
    }
}

static void netrec_print(t_netrec *x)
{
	int i;
	if(x->x_nconnections > 0)
	{
		post("netrec: %d open connections:", x->x_nconnections);

		for(i = 0; i < x->x_nconnections; i++)
		{
			post("        \"%s\" on socket %d", 
				x->x_host[i]->s_name, x->x_fd[i]);
		}
	} else post("netrec: no open connections");
}

static void *netrec_new(t_symbol *compatflag,
    t_floatarg fportno, t_floatarg udpflag)
{
    t_netrec *x;
	int i;
    struct sockaddr_in server;
    int sockfd, portno = fportno, udp = (udpflag != 0);
    int old = !strcmp(compatflag->s_name , "old");
    	/* create a socket */
    sockfd = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), 0);
#if 1
    post("netrec: receive socket %d\n", sockfd);
#endif
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
    x = (t_netrec *)pd_new(netrec_class);
    if (old)
    {
    	/* old style, nonsecure version */
		x->x_msgout = 0;
    }
    else x->x_msgout = outlet_new(&x->x_obj, &s_anything);

    if (udp)	    /* datagram protocol */
    {
    	t_netrec_socketreceiver *y = netrec_socketreceiver_new((void *)x, 
    	    (t_netrec_socketnotifier)netrec_notify,
	    	(x->x_msgout ? netrec_doit : 0), 1);
		sys_addpollfn(sockfd, (t_fdpollfn)netrec_socketreceiver_read, y);
		x->x_connectout = 0;
    }
    else    	/* streaming protocol */
    {
		if (listen(sockfd, 5) < 0)
		{
    		sys_sockerror("listen");
    		sys_closesocket(sockfd);
			sockfd = -1;
		}
    	else
		{
			sys_addpollfn(sockfd, (t_fdpollfn)netrec_connectpoll, x);
    		x->x_connectout = outlet_new(&x->x_obj, &s_float);
			x->x_clientno = outlet_new(&x->x_obj, &s_float);
			x->x_connectionip = outlet_new(&x->x_obj, &s_symbol);
			inbinbuf = binbuf_new();
		}
    }
    x->x_connectsocket = sockfd;
    x->x_nconnections = 0;
    x->x_udp = udp;
	for(i = 0; i < MAX_CONNECT; i++)x->x_fd[i] = -1;

    return (x);
}

static void netrec_free(t_netrec *x)
{
    	/* LATER make me clean up open connections */
    if (x->x_connectsocket >= 0)
    {
    	sys_rmpollfn(x->x_connectsocket);
    	sys_closesocket(x->x_connectsocket);
    }
	binbuf_free(inbinbuf);
}

#ifndef MAXLIB
void netrec_setup(void)
{
    netrec_class = class_new(gensym("netrec"),(t_newmethod)netrec_new, (t_method)netrec_free,
    	sizeof(t_netrec), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFSYM, 0);
	class_addmethod(netrec_class, (t_method)netrec_print, gensym("print"), 0);
	
    logpost(NULL, 4, version);
}
#else
void maxlib_netrec_setup(void)
{
    netrec_class = class_new(gensym("maxlib_netrec"),(t_newmethod)netrec_new, (t_method)netrec_free,
    	sizeof(t_netrec), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFSYM, 0);
	class_addcreator((t_newmethod)netrec_new, gensym("netrec"), A_DEFFLOAT, A_DEFFLOAT, A_DEFSYM, 0);
	class_addmethod(netrec_class, (t_method)netrec_print, gensym("print"), 0);
	class_sethelpsymbol(netrec_class, gensym("maxlib/netrec-help.pd"));
}
#endif
