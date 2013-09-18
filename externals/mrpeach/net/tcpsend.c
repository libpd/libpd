/* tcpsend.c 20060424 Martin Peach did it based on x_net.c. x_net.c header follows: */
/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* network */

#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

static t_class *tcpsend_class;

typedef struct _tcpsend
{
    t_object x_obj;
    int      x_fd;
} t_tcpsend;

void tcpsend_setup(void);
static void tcpsend_free(t_tcpsend *x);
static void tcpsend_send(t_tcpsend *x, t_symbol *s, int argc, t_atom *argv);
static void tcpsend_disconnect(t_tcpsend *x);
static void tcpsend_connect(t_tcpsend *x, t_symbol *hostname, t_floatarg fportno);
static void *tcpsend_new(void);

static void *tcpsend_new(void)
{
    t_tcpsend *x = (t_tcpsend *)pd_new(tcpsend_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_fd = -1;
    return (x);
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
#ifdef DEBUG
    fprintf(stderr, "tcpsend_connect: send socket %d\n", sockfd);
#endif
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
    outlet_float(x->x_obj.ob_outlet, 1);
}

static void tcpsend_disconnect(t_tcpsend *x)
{
    if (x->x_fd >= 0)
    {
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
        post("tcpsend: disconnected");
    }
}

static void tcpsend_send(t_tcpsend *x, t_symbol *s, int argc, t_atom *argv)
{
#define BYTE_BUF_LEN 65536 // arbitrary maximum similar to max IP packet size
    static char     byte_buf[BYTE_BUF_LEN];
    int             i, j;
    unsigned int    d;
    unsigned char   c;
    float           f, e;
    char            *bp;
    int             length, sent;
    int             result;
    static double   lastwarntime;
    static double   pleasewarn;
    double          timebefore;
    double          timeafter;
    int             late;
    char            fpath[FILENAME_MAX];
    FILE            *fptr;

#ifdef DEBUG
    post("s: %s", s->s_name);
    post("argc: %d", argc);
#endif
    for (i = j = 0; i < argc; ++i)
    {
        if (argv[i].a_type == A_FLOAT)
        {
            f = argv[i].a_w.w_float;
            d = (unsigned int)f;
            e = f - d;
            if (e != 0)
            {
                pd_error(x, "tcpsend_send: item %d (%f) is not an integer", i, f);
                return;
            }
	        c = (unsigned char)d;
	        if (c != d)
            {
                pd_error(x, "tcpsend_send: item %d (%f) is not between 0 and 255", i, f);
                return;
            }
#ifdef DEBUG
	        post("tcpsend_send: argv[%d]: %d", i, c);
#endif
	        byte_buf[j++] = c;
        }
        else if (argv[i].a_type == A_SYMBOL)
        {

            atom_string(&argv[i], fpath, FILENAME_MAX);
#ifdef DEBUG
            post ("tcpsend fname: %s", fpath);
#endif
            fptr = sys_fopen(fpath, "rb");
            if (fptr == NULL)
            {
                post("tcpsend: unable to open \"%s\"", fpath);
                return;
            }
            rewind(fptr);
#ifdef DEBUG
            post("tcpsend: d is %d", d);
#endif
            while ((d = fgetc(fptr)) != EOF)
            {
                byte_buf[j++] = (char)(d & 0x0FF);
#ifdef DEBUG
                post("tcpsend: byte_buf[%d] = %d", j-1, byte_buf[j-1]);
#endif
                if (j >= BYTE_BUF_LEN)
                {
                    post ("tcpsend: file too long, truncating at %lu", BYTE_BUF_LEN);
                    break;
                }
            }
            fclose(fptr);
            fptr = NULL;
            post("tcpsend: read \"%s\" length %d byte%s", fpath, j, ((d==1)?"":"s"));
        }
        else
	    {
            pd_error(x, "tcpsend_send: item %d is not a float or a file name", i);
            return;
        }
    }

    length = j;
    if ((x->x_fd >= 0) && (length > 0))
    {
        for (bp = byte_buf, sent = 0; sent < length;)
        {
            timebefore = sys_getrealtime();
            result = send(x->x_fd, byte_buf, length-sent, 0);
            timeafter = sys_getrealtime();
            late = (timeafter - timebefore > 0.005);
            if (late || pleasewarn)
            {
                if (timeafter > lastwarntime + 2)
                {
                    post("tcpsend blocked %d msec",
                        (int)(1000 * ((timeafter - timebefore) + pleasewarn)));
                    pleasewarn = 0;
                    lastwarntime = timeafter;
                }
                else if (late) pleasewarn += timeafter - timebefore;
            }
            if (result <= 0)
            {
                sys_sockerror("tcpsend");
                tcpsend_disconnect(x);
                break;
            }
            else
            {
                sent += result;
                bp += result;
	        }
        }
    }
    else pd_error(x, "tcpsend: not connected");
}

static void tcpsend_free(t_tcpsend *x)
{
    tcpsend_disconnect(x);
}

void tcpsend_setup(void)
{
    tcpsend_class = class_new(gensym("tcpsend"), (t_newmethod)tcpsend_new,
        (t_method)tcpsend_free,
        sizeof(t_tcpsend), 0, 0);
    class_addmethod(tcpsend_class, (t_method)tcpsend_connect,
        gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(tcpsend_class, (t_method)tcpsend_disconnect,
        gensym("disconnect"), 0);
    class_addmethod(tcpsend_class, (t_method)tcpsend_send, gensym("send"),
        A_GIMME, 0);
    class_addlist(tcpsend_class, (t_method)tcpsend_send);
}

/* end tcpsend.c */
