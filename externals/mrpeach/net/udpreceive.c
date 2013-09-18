/* x_net_udpreceive.c 20060424. Martin Peach did it based on x_net.c. x_net.c header follows: */
/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "s_stuff.h"
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#endif


/* ----------------------------- udpreceive ------------------------- */

static t_class *udpreceive_class;

#define MAX_UDP_RECEIVE 65536L // longer than data in maximum UDP packet

typedef struct _udpreceive
{
    t_object        x_obj;
    t_outlet        *x_msgout;
    t_outlet        *x_addrout;
    int             x_connectsocket;
    int             x_multicast_joined;
    long            x_total_received;
    t_atom          x_addrbytes[5];
    t_atom          x_msgoutbuf[MAX_UDP_RECEIVE];
    char            x_msginbuf[MAX_UDP_RECEIVE];
    char            x_addr_name[256]; // a multicast address or 0
} t_udpreceive;

void udpreceive_setup(void);
static void udpreceive_free(t_udpreceive *x);
static void *udpreceive_new(t_symbol *s, int argc, t_atom *argv);
static int udpreceive_new_socket(t_udpreceive *x, char *address, int port);
static void udpreceive_sock_err(t_udpreceive *x, char *err_string);
static void udpreceive_status(t_udpreceive *x);
static void udpreceive_read(t_udpreceive *x, int sockfd);
static void udpreceive_port(t_udpreceive *x, t_float portno);

static void udpreceive_read(t_udpreceive *x, int sockfd)
{
    int                 i, read = 0;
    struct sockaddr_in  from;
    socklen_t           fromlen = sizeof(from);
    t_atom              output_atom;
    long                addr;
    unsigned short      port;

    read = recvfrom(sockfd, x->x_msginbuf, MAX_UDP_RECEIVE, 0, (struct sockaddr *)&from, &fromlen);
#ifdef DEBUG
    post("udpreceive_read: read %lu x->x_connectsocket = %d",
        read, x->x_connectsocket);
#endif
    /* get the sender's ip */
    addr = ntohl(from.sin_addr.s_addr);
    port = ntohs(from.sin_port);

    x->x_addrbytes[0].a_w.w_float = (addr & 0xFF000000)>>24;
    x->x_addrbytes[1].a_w.w_float = (addr & 0x0FF0000)>>16;
    x->x_addrbytes[2].a_w.w_float = (addr & 0x0FF00)>>8;
    x->x_addrbytes[3].a_w.w_float = (addr & 0x0FF);
    x->x_addrbytes[4].a_w.w_float = port;
    outlet_anything(x->x_addrout, gensym("from"), 5L, x->x_addrbytes);

    if (read < 0)
    {
        udpreceive_sock_err(x, "udpreceive_read");
        sys_closesocket(x->x_connectsocket);
        return;
    }
    if (read > 0)
    {
        for (i = 0; i < read; ++i)
        {
            /* convert the bytes in the buffer to floats in a list */
            x->x_msgoutbuf[i].a_w.w_float = (float)(unsigned char)x->x_msginbuf[i];
        }
        x->x_total_received += read;
        SETFLOAT(&output_atom, read);
        outlet_anything(x->x_addrout, gensym("received"), 1, &output_atom);
        /* send the list out the outlet */
        if (read > 1) outlet_list(x->x_msgout, &s_list, read, x->x_msgoutbuf);
        else outlet_float(x->x_msgout, x->x_msgoutbuf[0].a_w.w_float);
    }
}

static void *udpreceive_new(t_symbol *s, int argc, t_atom *argv)
{
    t_udpreceive        *x;
    int                 result = 0, portno = 0;
    int                 i;

    x = (t_udpreceive *)pd_new(udpreceive_class); /* if something fails we return 0 instead of x. Is this OK? */
    if (NULL == x) return x;
    x->x_addr_name[0] = '\0';
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
#ifdef DEBUG
    post("udpreceive_new:argc is %d s is %s", argc, s->s_name);
#endif
    for (i = 0; i < argc ;++i)
    {
        if (argv[i].a_type == A_FLOAT)
        { // float is taken to be a port number
#ifdef DEBUG
            post ("argv[%d] is a float: %f", i, argv[i].a_w.w_float);
#endif
            portno = (int)argv[i].a_w.w_float;
        }
        else if (argv[i].a_type == A_SYMBOL)
        { // symbol is taken to be an ip address (for multicast)
#ifdef DEBUG
            post ("argv[%d] is a symbol: %s", i, argv[i].a_w.w_symbol->s_name);
#endif
            atom_string(&argv[i], x->x_addr_name, 256);
        }
    }
#ifdef DEBUG
    post("Setting port %d, address %s", portno, x->addr);
#endif

    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    x->x_addrout = outlet_new(&x->x_obj, &s_anything);

    x->x_connectsocket = -1; // no socket
    result = udpreceive_new_socket(x, x->x_addr_name, portno);
    return (x);
}

static int udpreceive_new_socket(t_udpreceive *x, char *address, int port)
{
// return nonzero if successful in creating and binding a socket
    int                 sockfd;
    int                 intarg;
    int                 multicast_joined = 0;
    struct sockaddr_in  server;
    struct hostent      *hp;
#if defined __APPLE__ || defined _WIN32
    struct ip_mreq      mreq;
#else
    struct ip_mreqn     mreq;
#endif

    if (x->x_connectsocket >= 0)
    {
        // close the existing socket first
        sys_rmpollfn(x->x_connectsocket);
        sys_closesocket(x->x_connectsocket);
    }
    /* create a socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef DEBUG
    post("udpreceive_new: socket %d port %d", sockfd, portno);
#endif
    if (sockfd < 0)
    {
        udpreceive_sock_err(x, "udpreceive: socket");
        return 0;
    }
    server.sin_family = AF_INET;
    if (address[0] == 0) server.sin_addr.s_addr = INADDR_ANY;
    else 
    {
        hp = gethostbyname(address);
        if (hp == 0)
        {
            pd_error(x, "udpreceive: bad host?\n");
            return 0;
        }
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    }
    /* enable delivery of all multicast or broadcast (but not unicast)
    * UDP datagrams to all sockets bound to the same port */
    intarg = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&intarg, sizeof(intarg)) < 0)
        udpreceive_sock_err(x, "udpreceive: setsockopt (SO_REUSEADDR) failed");

    /* assign server port number */
    server.sin_port = htons((u_short)port);

    /* if a multicast address was specified, join the multicast group */
    /* hop count defaults to 1 so we won't leave the subnet*/
    if (0xE0000000 == (ntohl(server.sin_addr.s_addr) & 0xF0000000))
    {
        server.sin_addr.s_addr = INADDR_ANY;
        /* first bind the socket to INADDR_ANY */
        if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            udpreceive_sock_err(x, "udpreceive: bind");
            sys_closesocket(sockfd);
            return 0;
        }
        /* second join the multicast group */
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

#if defined __APPLE__ || defined _WIN32
        mreq.imr_multiaddr.s_addr = server.sin_addr.s_addr;
        mreq.imr_interface.s_addr = INADDR_ANY;/* can put a specific local IP address here if host is multihomed */
#else
        mreq.imr_multiaddr.s_addr = server.sin_addr.s_addr;
        mreq.imr_address.s_addr = INADDR_ANY;
        mreq.imr_ifindex = 0;
#endif //__APPLE__ || _WIN32
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (char *)&mreq, sizeof(mreq)) < 0)
            udpreceive_sock_err(x, "udpreceive: setsockopt IP_ADD_MEMBERSHIP");
        else
        {
            multicast_joined = 1;
            post ("udpreceive: added to multicast group");
        }
    }
    else
    {
        /* name the socket */
        if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            udpreceive_sock_err(x, "udpreceive: bind");
            sys_closesocket(sockfd);
            return 0;
        }
    }
    x->x_multicast_joined = multicast_joined;
    x->x_connectsocket = sockfd;
    x->x_total_received = 0L;
    sys_addpollfn(x->x_connectsocket, (t_fdpollfn)udpreceive_read, x);
    return 1;
}

static void udpreceive_sock_err(t_udpreceive *x, char *err_string)
{
/* prints the last error from errno or WSAGetLastError() */
#ifdef _WIN32
    LPVOID  lpMsgBuf;
    DWORD   dwRetVal = WSAGetLastError();
    int     len = 0, i;
    char    *cp;

    if (len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
        , NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL))
    {
        cp = (char *)lpMsgBuf;
        for(i = 0; i < len; ++i)
        {
            if (cp[i] < 0x20)
            { /* end string at first weird character */
                cp[i] = 0;
                break;
            }
        }
        pd_error(x, "%s: %s (%d)", err_string, lpMsgBuf, dwRetVal);
        LocalFree(lpMsgBuf);
    }
#else
    pd_error(x, "%s: %s (%d)", err_string, strerror(errno), errno);
#endif
}

static void udpreceive_status(t_udpreceive *x)
{
    t_atom output_atom;

    SETFLOAT(&output_atom, x->x_multicast_joined);
    outlet_anything( x->x_addrout, gensym("multicast"), 1, &output_atom);
    SETFLOAT(&output_atom, x->x_total_received);
    outlet_anything( x->x_addrout, gensym("total"), 1, &output_atom);
}

static void udpreceive_port(t_udpreceive *x, t_float portno)
{
    int result = udpreceive_new_socket(x, x->x_addr_name, (int)portno);
}

static void udpreceive_free(t_udpreceive *x)
{
    if (x->x_connectsocket >= 0)
    {
        sys_rmpollfn(x->x_connectsocket);
        sys_closesocket(x->x_connectsocket);
    }
}

void udpreceive_setup(void)
{
    udpreceive_class = class_new(gensym("udpreceive"),
        (t_newmethod)udpreceive_new, (t_method)udpreceive_free,
        sizeof(t_udpreceive), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(udpreceive_class, (t_method)udpreceive_status,
        gensym("status"), 0);
    class_addmethod(udpreceive_class, (t_method)udpreceive_port,
        gensym("port"), A_DEFFLOAT, 0);
}

/* end udpreceive.c */
