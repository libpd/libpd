/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* network */

#include "m_pd.h"
#include "s_stuff.h"

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#define SOCKET_ERROR -1
#endif

static t_class *netsend_class;

typedef struct _netsend
{
    t_object x_obj;
    int x_fd;
    int x_protocol;
} t_netsend;

static void *netsend_new(t_floatarg udpflag)
{
    t_netsend *x = (t_netsend *)pd_new(netsend_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_fd = -1;
    x->x_protocol = (udpflag != 0 ? SOCK_DGRAM : SOCK_STREAM);
    return (x);
}

static void netsend_connect(t_netsend *x, t_symbol *hostname,
    t_floatarg fportno)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno = fportno;
    int intarg;
    if (x->x_fd >= 0)
    {
        error("netsend_connect: already connected");
        return;
    }

        /* create a socket */
    sockfd = socket(AF_INET, x->x_protocol, 0);
#if 0
    fprintf(stderr, "send socket %d\n", sockfd);
#endif
    if (sockfd < 0)
    {
        sys_sockerror("socket");
        return;
    }
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
        post("bad host?\n");
        return;
    }
#if 0
    intarg = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,
        &intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_RCVBUF) failed\n");
#endif
    intarg = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, 
                  (const void *)&intarg, sizeof(intarg)) < 0)
        post("setting SO_BROADCAST");
        /* for stream (TCP) sockets, specify "nodelay" */
    if (x->x_protocol == SOCK_STREAM)
    {
        intarg = 1;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&intarg, sizeof(intarg)) < 0)
                post("setsockopt (TCP_NODELAY) failed\n");
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((u_short)portno);

    post("connecting to port %d", portno);
        /* try to connect.  LATER make a separate thread to do this
        because it might block */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        sys_sockerror("connecting stream socket");
        sys_closesocket(sockfd);
        return;
    }
    x->x_fd = sockfd;
    outlet_float(x->x_obj.ob_outlet, 1);
}

static void netsend_disconnect(t_netsend *x)
{
    if (x->x_fd >= 0)
    {
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
    }
}

static void netsend_send(t_netsend *x, t_symbol *s, int argc, t_atom *argv)
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
            int res = send(x->x_fd, bp, length-sent, 0);
            double timeafter = sys_getrealtime();
            int late = (timeafter - timebefore > 0.005);
            if (late || pleasewarn)
            {
                if (timeafter > lastwarntime + 2)
                {
                     post("netsend blocked %d msec",
                        (int)(1000 * ((timeafter - timebefore) + pleasewarn)));
                     pleasewarn = 0;
                     lastwarntime = timeafter;
                }
                else if (late) pleasewarn += timeafter - timebefore;
            }
            if (res <= 0)
            {
                sys_sockerror("netsend");
                netsend_disconnect(x);
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
    else error("netsend: not connected");
}

static void netsend_free(t_netsend *x)
{
    netsend_disconnect(x);
}

void netsend_setup(void)
{
    netsend_class = class_new(gensym("netsend"), (t_newmethod)netsend_new,
        (t_method)netsend_free,
        sizeof(t_netsend), 0, A_DEFFLOAT, 0);
    class_addmethod(netsend_class, (t_method)netsend_connect,
        gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netsend_class, (t_method)netsend_disconnect,
        gensym("disconnect"), 0);
    class_addmethod(netsend_class, (t_method)netsend_send, gensym("send"),
        A_GIMME, 0);
}
