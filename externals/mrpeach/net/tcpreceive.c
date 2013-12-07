/* x_net_tcpreceive.c 20060424. Martin Peach did it based on x_net.c. x_net.c header follows: */
/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "s_stuff.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h> /* for socklen_t */
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#endif


/* ----------------------------- tcpreceive ------------------------- */

static t_class *tcpreceive_class;

#define MAX_UDP_RECEIVE 65536L // longer than data in maximum UDP packet
#define MAX_CONNECTIONS 128 // this is going to cause trouble down the line...:(

typedef struct _tcpconnection
{
    long            addr;
    unsigned short  port;
    int             socket;
} t_tcpconnection;

typedef struct _tcpreceive
{
    t_object        x_obj;
    t_outlet        *x_msgout;
	t_outlet        *x_addrout;
    t_outlet        *x_connectout;
    int             x_connectsocket;
    int             x_nconnections;
	t_tcpconnection x_connection[MAX_CONNECTIONS];
    t_atom          x_addrbytes[5];
    t_atom          x_msgoutbuf[MAX_UDP_RECEIVE];
    unsigned char   x_msginbuf[MAX_UDP_RECEIVE];
} t_tcpreceive;

void tcpreceive_setup(void);
static void tcpreceive_free(t_tcpreceive *x);
static void *tcpreceive_new(t_floatarg fportno);
static void tcpreceive_read(t_tcpreceive *x, int sockfd);
static void tcpreceive_connectpoll(t_tcpreceive *x);
static int tcpreceive_addconnection(t_tcpreceive * x, int fd, long addr, unsigned short port);
static int tcpreceive_removeconnection(t_tcpreceive * x, int fd);
static void tcpreceive_closeall(t_tcpreceive *x);
static long tcpreceive_getconnection(t_tcpreceive * x, int fd);
static unsigned short tcpreceive_getconnectionport(t_tcpreceive * x, int fd);

static void tcpreceive_read(t_tcpreceive *x, int sockfd)
{
    int             i, read = 0;
    long            addr;
    unsigned short  port;

//	read = recvfrom(sockfd, x->x_msginbuf, MAX_UDP_RECEIVE, 0, (struct sockaddr *)&from, &fromlen);
	read = recv(sockfd, x->x_msginbuf, MAX_UDP_RECEIVE, 0);
#ifdef DEBUG
    post("tcpreceive_read: read %lu x->x_connectsocket = %d",
        read, x->x_connectsocket);
#endif
    if (read < 0)
    {
		sys_sockerror("tcpreceive_read: recv");
        sys_rmpollfn(sockfd);
        sys_closesocket(sockfd);
        tcpreceive_removeconnection(x, sockfd);
        outlet_float(x->x_connectout, --x->x_nconnections);
    }
    else if (read == 0)
    {
        post("tcpreceive: EOF on socket %d\n", sockfd);
        sys_rmpollfn(sockfd);
        sys_closesocket(sockfd);
        tcpreceive_removeconnection(x, sockfd);
        outlet_float(x->x_connectout, --x->x_nconnections);
    }
    else if (read > 0)
    {
        for (i = 0; i < read; ++i)
        {
            /* convert the bytes in the buffer to floats in a list */
            x->x_msgoutbuf[i].a_w.w_float = (float)x->x_msginbuf[i];
        }
        /* find sender's ip address and output it */
		addr = tcpreceive_getconnection(x, sockfd);
        port = tcpreceive_getconnectionport(x, sockfd);
        x->x_addrbytes[0].a_w.w_float = (addr & 0xFF000000)>>24;
        x->x_addrbytes[1].a_w.w_float = (addr & 0x0FF0000)>>16;
        x->x_addrbytes[2].a_w.w_float = (addr & 0x0FF00)>>8;
        x->x_addrbytes[3].a_w.w_float = (addr & 0x0FF);
        x->x_addrbytes[4].a_w.w_float = port;
        outlet_list(x->x_addrout, &s_list, 5L, x->x_addrbytes);
        /* send the list out the outlet */
        if (read > 1) outlet_list(x->x_msgout, &s_list, read, x->x_msgoutbuf);
        else outlet_float(x->x_msgout, x->x_msgoutbuf[0].a_w.w_float);
    }
}

static void *tcpreceive_new(t_floatarg fportno)
{
    t_tcpreceive       *x;
    struct sockaddr_in server;
    int                sockfd, portno = fportno;
    int                intarg, i;

	/* create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef DEBUG
    post("tcpreceive_new: socket %d port %d", sockfd, portno);
#endif
    if (sockfd < 0)
    {
        sys_sockerror("tcpreceive: socket");
        return (0);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    /* ask OS to allow another Pd to repoen this port after we close it. */
    intarg = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&intarg, sizeof(intarg)) < 0)
        post("tcpreceive: setsockopt (SO_REUSEADDR) failed");
    /* Stream (TCP) sockets are set NODELAY */
    intarg = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
        (char *)&intarg, sizeof(intarg)) < 0)
            post("setsockopt (TCP_NODELAY) failed\n");

    /* assign server port number */
    server.sin_port = htons((u_short)portno);

    /* name the socket */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        sys_sockerror("tcpreceive: bind");
    	sys_closesocket(sockfd);
        return (0);
    }
    x = (t_tcpreceive *)pd_new(tcpreceive_class);
    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    x->x_addrout = outlet_new(&x->x_obj, &s_list);
    x->x_connectout = outlet_new(&x->x_obj, &s_float);
    /* clear the connection list */
    for (i = 0; i < MAX_CONNECTIONS; ++i)
	{
		x->x_connection[i].socket = -1;
		x->x_connection[i].addr = 0L;
		x->x_connection[i].port = 0;
    }
	/* convert the bytes in the buffer to floats in a list */
    for (i = 0; i < MAX_UDP_RECEIVE; ++i)
	{
		x->x_msgoutbuf[i].a_type = A_FLOAT;
		x->x_msgoutbuf[i].a_w.w_float = 0;
	}
    for (i = 0; i < 5; ++i)
    {
        x->x_addrbytes[i].a_type = A_FLOAT;
        x->x_addrbytes[i].a_w.w_float = 0;
    }

    /* streaming protocol */
    if (listen(sockfd, 5) < 0)
    {
        sys_sockerror("tcpreceive: listen");
        sys_closesocket(sockfd);
        sockfd = -1;
    }
    else
    {
        sys_addpollfn(sockfd, (t_fdpollfn)tcpreceive_connectpoll, x);
    }
    x->x_connectsocket = sockfd;
    x->x_nconnections = 0;

//udp version...    sys_addpollfn(x->x_connectsocket, (t_fdpollfn)tcpreceive_read, x);
    return (x);
}

/* tcpreceive_connectpoll checks for incoming connection requests on the original socket */
/* a new socket is assigned  */
static void tcpreceive_connectpoll(t_tcpreceive *x)
{
    struct sockaddr_in  from;
    socklen_t           fromlen = sizeof(from);
	long                addr;
    unsigned short      port;
    int                 fd;

    fd = accept(x->x_connectsocket, (struct sockaddr *)&from, &fromlen);
    if (fd < 0) post("tcpreceive: accept failed");
    else
    {
 //       t_socketreceiver *y = socketreceiver_new((void *)x,
   //         (t_socketnotifier)tcpreceive_notify,
     //           0, 0);

        /* get the sender's ip */
        addr = ntohl(from.sin_addr.s_addr);
        port = ntohs(from.sin_port);
		if (tcpreceive_addconnection(x, fd, addr, port))
		{
            sys_addpollfn(fd, (t_fdpollfn)tcpreceive_read, x);
            outlet_float(x->x_connectout, ++x->x_nconnections);
            x->x_addrbytes[0].a_w.w_float = (addr & 0xFF000000)>>24;
            x->x_addrbytes[1].a_w.w_float = (addr & 0x0FF0000)>>16;
            x->x_addrbytes[2].a_w.w_float = (addr & 0x0FF00)>>8;
            x->x_addrbytes[3].a_w.w_float = (addr & 0x0FF);
            x->x_addrbytes[4].a_w.w_float = port;
            outlet_list(x->x_addrout, &s_list, 5L, x->x_addrbytes);
        }
        else
        {
            error ("tcpreceive: Too many connections");
            sys_closesocket(fd);
        }
    }
}

/* tcpreceive_addconnection tries to add the socket fd to the list */
/* returns 1 on success, else 0 */
static int tcpreceive_addconnection(t_tcpreceive *x, int fd, long addr, unsigned short port)
{
	int i;
	for (i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (x->x_connection[i].socket == -1)
        {
            x->x_connection[i].socket = fd;
            x->x_connection[i].addr = addr;
            x->x_connection[i].port = port;
            return 1;
        }
    }
    return 0;
}

/* tcpreceive_closeall closes all open sockets and deletes them from the list */
static void tcpreceive_closeall(t_tcpreceive *x)
{
    int i;

    for (i = 0; ((i < MAX_CONNECTIONS) && (x->x_nconnections > 0)); ++i)
    {
        if (x->x_connection[i].socket != -1)
        {
			post ("tcpreceive: closing socket %d", x->x_connection[i].socket);
            sys_rmpollfn(x->x_connection[i].socket);
            sys_closesocket(x->x_connection[i].socket);
            x->x_connection[i].socket = -1;
            x->x_connection[i].addr = 0L;
            x->x_connection[i].port = 0;
            outlet_float(x->x_connectout, --x->x_nconnections);
        }
    }
}

/* tcpreceive_removeconnection tries to delete the socket fd from the list */
/* returns 1 on success, else 0 */
static int tcpreceive_removeconnection(t_tcpreceive *x, int fd)
{
    int i;
    for (i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (x->x_connection[i].socket == fd)
        {
            x->x_connection[i].socket = -1;
            x->x_connection[i].addr = 0L;
            x->x_connection[i].port = 0;
            return 1;
        }
    }
    return 0;
}

/* tcpreceive_getconnectionport tries to find the socket fd in the list */
/* returns port on success, else 0 */
static u_short tcpreceive_getconnectionport(t_tcpreceive *x, int fd)
{
    int i;
    for (i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (x->x_connection[i].socket == fd)
            return x->x_connection[i].port;
    }
    return 0;
}

/* tcpreceive_getconnection tries to find the socket fd in the list */
/* returns addr on success, else 0 */
static long tcpreceive_getconnection(t_tcpreceive *x, int fd)
{
    int i;
    for (i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (x->x_connection[i].socket == fd)
            return x->x_connection[i].addr;
    }
    return 0;
}

static void tcpreceive_free(t_tcpreceive *x)
{ /* is this ever called? */
    if (x->x_connectsocket >= 0)
    {
        sys_rmpollfn(x->x_connectsocket);
        sys_closesocket(x->x_connectsocket);
    }
    tcpreceive_closeall(x);
}

void tcpreceive_setup(void)
{
    tcpreceive_class = class_new(gensym("tcpreceive"),
        (t_newmethod)tcpreceive_new, (t_method)tcpreceive_free,
        sizeof(t_tcpreceive), CLASS_NOINLET, A_DEFFLOAT, 0);
}

/* end x_net_tcpreceive.c */

