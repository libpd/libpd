/* ------------------------ mp3streamin~ -------------------------------------- */
/*                                                                              */
/* Tilde object to receive an mp3-stream sent by a peer using mp3streamout~.    */
/* Written by Yves Degoyon (ydegoyon@free.fr).                                  */
/* Tarballs and updates @ http://ydegoyon.free.fr                               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
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
/* Uses the LAME MPEG 1 Layer 3 encoding library (lame_enc.dll) which can       */
/* be found at http://www.cdex.n3.net.                                          */
/*                                                                              */
/* "Repackage sex, your interests."                                              */
/* "Somehow, maintain the interest."                                             */
/* Gang Of Four -- Natural's Not In It                                          */
/* ---------------------------------------------------------------------------- */


#include <m_pd.h>
#include <m_imp.h>
#include <g_canvas.h>

#if PD_MINOR_VERSION >=37
#include "s_stuff.h"
#endif

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#define SOCKET_ERROR -1
#endif /* _WIN32 */

#if defined(__APPLE__) || defined(_WIN32)
#define MSG_NOSIGNAL 0
#define SOL_TCP IPPROTO_TCP
#endif

#include "mpg123.h"      /* mpg123 decoding library from lame 3.92 */
#include "mpglib.h"      /* mpglib decoding library from lame 3.92 */
#include "interface.h"   /* mpglib decoding library from lame 3.92 */

#define MIN_AUDIO_INPUT 8064 // we must a least have 8 chunks to play a correct sound
#define INPUT_BUFFER_SIZE MIN_AUDIO_INPUT
#define OUTPUT_BUFFER_SIZE 131072 /* 128k*/
#define LAME_AUDIO_CHUNK_SIZE 1152
#define BARHEIGHT 10
#define MAX_DECODERS 10

/* useful debugging functions from mpglib */
extern int decode_header( struct frame* fr, unsigned long newhead );
extern void print_header_compact( struct frame* fr );
extern int head_check( unsigned long head, int check_layer );

/* time-out used for select() call */
static struct timeval ztout;

static char   *mp3streamin_version = "mp3streamin~: mp3 peer-to-peer streamer version 0.3, written by ydegoyon@free.fr";

extern void sys_sockerror(char *s);

void mp3streamin_closesocket(int fd)
{
#ifndef _MSC_VER
    if ( close(fd) < 0 )
    {
        perror( "close" );
    }
    else
    {
        post( "mp3streamin~ : closed socket : %d", fd );
    }
#endif
#ifdef _WIN32
    closesocket(fd);
#endif
    sys_rmpollfn(fd);
}

int setsocketoptions(int sockfd)
{
    int sockopt = 1;
    if (setsockopt(sockfd, SOL_TCP, TCP_NODELAY, (const char*) &sockopt, sizeof(int)) < 0)
    {
        post("mp3streamin~ : setsockopt TCP_NODELAY failed");
        perror( "setsockopt" );
        return -1;
    }
    else
    {
        post("mp3streamin~ : TCP_NODELAY set");
    }

#ifndef _MSC_VER
    sockopt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0)
    {
        post("mp3streamin~ : setsockopt SO_REUSEADDR failed");
        perror( "setsockopt" );
        return -1;
    }
    else
    {
        post("mp3streamin~ : setsockopt SO_REUSEADDR done.");
    }
#endif
    return 0;
}


/* ------------------------ mp3streamin~ ----------------------------- */

static t_class *mp3streamin_class;

typedef struct _mp3streamin
{
    t_object x_obj;
    t_int x_instance;
    t_int x_socket;
    t_int x_shutdown;
    t_outlet *x_connectionip;
    t_int x_serversocket;
    t_int x_inpackets;  /* number of packets received */
    t_int x_dpacket;    /* displayed packet in status bar */
    t_int x_packetsize; /* size of the packets */
    t_int x_pblocks;    /* processed blocks */
    t_int x_graphic;    /* indicates if we show a graphic bar */

    void *x_inbuffer;   /* accumulation buffer for incoming mp3 frames */
    t_int x_inwriteposition;
    t_int x_inbuffersize;

    t_float *x_outbuffer;   /* buffer to store audio decoded data */
    t_int x_outwriteposition;
    t_int x_outreadposition;
    t_int x_outunread;
    t_int x_outbuffersize;

    t_canvas *x_canvas;

    t_int x_stream;         /* indicates if a stream is connected ( meaning correct input flow ) */
    t_int x_newstream;      /* at first, the stream must provide enough data to start */
    t_int x_bitrateindex;   /* remember the bitrate index */

} t_mp3streamin;

/* too bad, this needs to be static,
   handling an array to enable several decoders in pd */
static  MPSTR   mp[MAX_DECODERS];  /* decoder buffer */
int nbinstances = 0;

void mp3streamin_tilde_mpglib_init(t_mp3streamin *x)
{
    int ret;

    InitMP3(&mp[x->x_instance]);
}

static int mp3streamin_decode_input(t_mp3streamin *x)
{
    int i;
    int alength = 0;
    struct frame hframe;
    unsigned int a,b,c,d;
    unsigned long cheader;
    static char out[8192];
    signed short int *p = (signed short int *) out;
    int     pbytes;
    int     ret;

    if ( !x->x_shutdown )
    {
        /* decode first 4 bytes as the header */
        a = *((unsigned char*)x->x_inbuffer);
        b = *((unsigned char*)x->x_inbuffer+1);
        c = *((unsigned char*)x->x_inbuffer+2);
        d = *((unsigned char*)x->x_inbuffer+3);

        cheader = 0;
        cheader = a;
        cheader <<= 8;
        cheader |= b;
        cheader <<= 8;
        cheader |= c;
        cheader <<= 8;
        cheader |= d;
        if ( head_check( cheader, 0 ) )
        {
            // post( "mp3streamin~ : valid header ( packet=%d)", x->x_inpackets );
            decode_header( &hframe, cheader );
            // print_header_compact( &hframe );
            // when the bitrate change, reinit decoder
            if ( x->x_bitrateindex != hframe.bitrate_index )
            {
                ExitMP3(&mp[x->x_instance]);
                InitMP3(&mp[x->x_instance]);
                x->x_bitrateindex = hframe.bitrate_index;
            }
        }
        else
        {
            post( "mp3streamin~ : error : mp3 packet received without header" );
            // ignore data
            x->x_inwriteposition = 0;
            x->x_inpackets--;
            return( -1 );
        }

        // post( "mp3streamin~ : decoding %d bytes framesize=%d", x->x_inwriteposition, hframe.framesize );
        ret =
            decodeMP3(&mp[x->x_instance], (unsigned char*)x->x_inbuffer,
                      x->x_inwriteposition, (char *) p, sizeof(out), &pbytes);

        switch (ret)
        {
        case MP3_OK:
            switch (mp[x->x_instance].fr.stereo)
            {
            case 1:
            case 2:
                alength = ((mp[x->x_instance].fr.stereo==1)?pbytes >> 1 : pbytes >> 2);
                // post( "mp3streamin~ : processed %d samples", alength );
                // update outbuffer contents
                for ( i=0; i<alength; i++ )
                {
                    if ( x->x_outunread >= x->x_outbuffersize-2 )
                    {
                        post( "mp3streamin~ : too much input ... ignored" );
                        continue;
                    }
                    *(x->x_outbuffer+x->x_outwriteposition) = ((t_float)(*p++))/32767.0;
                    x->x_outwriteposition = (x->x_outwriteposition + 1)%x->x_outbuffersize;
                    *(x->x_outbuffer+x->x_outwriteposition) =
                        ((mp[x->x_instance].fr.stereo==2)?((t_float)(*p++))/32767.0 : 0.0);
                    x->x_outwriteposition = (x->x_outwriteposition + 1)%x->x_outbuffersize;
                    x->x_outunread+=2;

                    if ( x->x_outunread > MIN_AUDIO_INPUT && !x->x_stream )
                    {
                        post( "mp3streamin~ : stream connected." );
                        x->x_stream = 1;
                    }
                }

                break;
            default:
                alength = -1;
                break;
            }
            break;

        case MP3_NEED_MORE:
            post( "mp3streamin~ : retry lame decoding (more data needed)." );
            alength = 0;
            break;

        case MP3_ERR:
            post( "mp3streamin~ : lame decoding failed." );
            alength = -1;
            break;

        }

        x->x_inwriteposition = 0;
    }
    else
    {
        if ( x->x_outunread == 0 )
        {
            post( "mp3streamin~ : connection closed" );
            mp3streamin_closesocket(x->x_socket);
            x->x_stream = 0;
            x->x_newstream = 0;
            x->x_inpackets = 0;
            x->x_inwriteposition = 0;
            x->x_bitrateindex = -1;
            x->x_socket=-1;
            outlet_symbol( x->x_connectionip, gensym("") );
        }
    }

    if ( x->x_graphic && glist_isvisible( x->x_canvas ) )
    {
        /* update graphical read status */
        if ( x->x_inpackets != x->x_dpacket )
        {
            char color[32];
            int minpackets = ( MIN_AUDIO_INPUT/LAME_AUDIO_CHUNK_SIZE )-2; // audio loop has eaten some already


            sys_vgui(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x );
            sys_vgui(".x%lx.c delete line %xTHRESHOLD\n", x->x_canvas, x );
            if ( x->x_outunread > 0 )
            {
                t_int width;

                if ( x->x_inpackets < (MIN_AUDIO_INPUT/LAME_AUDIO_CHUNK_SIZE)/2 )
                {
                    strcpy( color, "red" );
                }
                else
                {
                    strcpy( color, "lightgreen" );
                }
                width = rtext_width( glist_findrtext( (t_glist*)x->x_canvas, (t_text *)x ) );
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %xSTATUS\n",
                         x->x_canvas, x->x_obj.te_xpix, x->x_obj.te_ypix-BARHEIGHT-1,
                         x->x_obj.te_xpix+(x->x_inpackets*x->x_packetsize*width)/INPUT_BUFFER_SIZE,
                         x->x_obj.te_ypix - 1, color, x );
                sys_vgui(".x%lx.c create line %d %d %d %d -fill red -tags %xTHRESHOLD\n",
                         x->x_canvas, x->x_obj.te_xpix+(minpackets*x->x_packetsize*width)/INPUT_BUFFER_SIZE,
                         x->x_obj.te_ypix-BARHEIGHT-1,
                         x->x_obj.te_xpix+(minpackets*x->x_packetsize*width)/INPUT_BUFFER_SIZE,
                         x->x_obj.te_ypix-1, x );
                x->x_dpacket = x->x_inpackets;
            }
            else
            {
                if ( x->x_shutdown )
                {
                    x->x_shutdown=0;
                    sys_vgui(".x%lx.c delete rectangle %xPBAR\n", x->x_canvas, x );
                    sys_vgui(".x%lx.c delete line %xTHRESHOLD\n", x->x_canvas, x );
                }
            }
        }
    }
    return alength;
}

static void mp3streamin_recv(t_mp3streamin *x)
{
    int ret;

    if ( ( ret = recv(x->x_socket, (void*) (x->x_inbuffer + x->x_inwriteposition),
                      (size_t)((x->x_inbuffersize-x->x_inwriteposition)),
                      MSG_NOSIGNAL) ) < 0 )
    {
        post( "mp3_streamin~ : receive error" );
        perror( "recv" );
        return;
    }
    else
    {
        // post( "streamin~ : received %d bytes at %d on %d ( up to %d)",
        //        ret, x->x_inwriteposition, x->x_socket,
        //        x->x_inbuffersize-x->x_inwriteposition*sizeof( unsigned long) );

        if ( ret == 0 )
        {
            /* initiate the shutdown phase */
            x->x_shutdown=1;
        }
        else
        {
            // check we don't overflow input buffer
            if ( (x->x_inpackets+1)*x->x_packetsize > x->x_inbuffersize )
            {
                post( "mp3streamin~ : too much input...resetting" );
                x->x_inpackets=0;
                x->x_inwriteposition=0;
                return;
            }
            x->x_inpackets++;
            x->x_packetsize=ret;
            if ( x->x_inpackets % 100 == 0 )
            {
                // post( "mp3streamin~ : received %d packets", x->x_inpackets );
            }
            x->x_inwriteposition += ret;
        }

        mp3streamin_decode_input(x);
    }
}

static void mp3streamin_acceptconnection(t_mp3streamin *x)
{
    struct sockaddr_in incomer_address;
    int sockaddrl = (int) sizeof( struct sockaddr );

    int fd = accept(x->x_serversocket, (struct sockaddr*)&incomer_address, &sockaddrl );

    if (fd < 0)
    {
        post("mp3streamin~: accept failed");
        return;
    }

    if (x->x_socket > 0)
    {
        sys_addpollfn(fd, (t_fdpollfn)mp3streamin_recv, x);
        if ( x->x_outunread != 0 )
        {
            post("mp3streamin~: still have some data to decode, retry later you %s.",
                 inet_ntoa( incomer_address.sin_addr ));
            mp3streamin_closesocket( fd );
            return;
        }
        post("mp3streamin~: the source has changed to %s.",
             inet_ntoa( incomer_address.sin_addr ));
        mp3streamin_closesocket(x->x_socket);
    }

    x->x_socket = fd;
    sys_addpollfn(x->x_socket, (t_fdpollfn)mp3streamin_recv, x);
    outlet_symbol( x->x_connectionip, gensym( inet_ntoa( incomer_address.sin_addr) ) );

    if ( x->x_graphic && glist_isvisible( x->x_canvas ) )
    {
        t_int width;

        width = rtext_width( glist_findrtext( (t_glist*)x->x_canvas, (t_text *)x ) );
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill lightblue -tags %xPBAR\n",
                 x->x_canvas, x->x_obj.te_xpix, x->x_obj.te_ypix-BARHEIGHT-1,
                 x->x_obj.te_xpix + width, x->x_obj.te_ypix - 1, x );
    }
    x->x_stream = 0;
    x->x_newstream = 1;

}


static int mp3streamin_startservice(t_mp3streamin* x, int portno)
{
    struct sockaddr_in server;
    int sockfd;

    /* create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        sys_sockerror("socket");
        return (0);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    /* assign server port number */
    server.sin_port = htons((u_short)portno);
    post("listening to port number %d", portno);

    setsocketoptions(sockfd);

    /* name the socket */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        sys_sockerror("bind");
        mp3streamin_closesocket(sockfd);
        return (0);
    }

    if (listen(sockfd, 5) < 0)
    {
        sys_sockerror("listen");
        mp3streamin_closesocket(sockfd);
    }
    else
    {
        x->x_serversocket = sockfd;
        sys_addpollfn(x->x_serversocket, (t_fdpollfn)mp3streamin_acceptconnection, x);
    }

    return 1;
}

static void mp3streamin_free(t_mp3streamin *x)
{
    post( "mp3streamin~ : free %x", x );
    if (x->x_serversocket > 0)
    {
        post( "mp3streamin~ : closing server socket" );
        mp3streamin_closesocket(x->x_serversocket);
        x->x_serversocket = -1;
    }
    if (x->x_socket > 0)
    {
        post( "mp3streamin~ : closing socket" );
        mp3streamin_closesocket(x->x_socket);
        x->x_socket = -1;
    }
    if ( x->x_inbuffer ) freebytes( x->x_inbuffer, x->x_inbuffersize );
    if ( x->x_outbuffer ) freebytes( x->x_outbuffer, x->x_outbuffersize*sizeof(t_float) );
    if ( x->x_instance == nbinstances-1 )
    {
        nbinstances--;
    }
}

static t_int *mp3streamin_perform(t_int *w)
{
    t_mp3streamin *x = (t_mp3streamin*) (w[1]);
    t_float *out1 = (t_float *)(w[2]);
    t_float *out2 = (t_float *)(w[3]);
    int n = (int)(w[4]);
    int bsize = n;
    int ret;
    int i = 0;

    while( n-- )
    {
        if ( ( ( x->x_outunread > MIN_AUDIO_INPUT ) && x->x_newstream ) || // wait the buffer to load
                ( ( x->x_shutdown ) && ( x->x_outunread >= 2 ) ) ||  // clean disconnection
                ( x->x_stream ) // check that the stream provides enough data
           )
        {
            if ( x->x_newstream && !x->x_shutdown )
            {
                x->x_newstream = 0;
                x->x_stream = 1;
            }
            *out1++=*(x->x_outbuffer+x->x_outreadposition);
            x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
            *out2++=*(x->x_outbuffer+x->x_outreadposition);
            x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
            x->x_outunread-=2;
            if ( n == 1 ) x->x_pblocks++;
        }
        else
        {
            *out1++=0.0;
            *out2++=0.0;
        }
    }

    if ( ( x->x_outunread <= MIN_AUDIO_INPUT/10  ) && ( x->x_stream ) )
    {
        post( "mp3streamin~ : stream lost (too little input)" );
        x->x_stream = 0;
        x->x_newstream = 1; // waiting for a new stream
    }

    if ( x->x_pblocks == LAME_AUDIO_CHUNK_SIZE/bsize )
    {
        x->x_inpackets--;
        x->x_pblocks = 0;
    }

#ifdef DO_MY_OWN_SELECT
    // check new incoming data
    if ( x->x_socket > 0 )
    {
        fd_set readset;
        fd_set exceptset;

        FD_ZERO(&readset);
        FD_ZERO(&exceptset);
        FD_SET(x->x_socket, &readset );
        FD_SET(x->x_socket, &exceptset );

        if ( select( maxfd+1, &readset, NULL, &exceptset, &ztout ) >0 )
        {
            if ( FD_ISSET( x->x_socket, &readset) || FD_ISSET( x->x_socket, &exceptset ) )
            {
                /* receive data or error and decode it */
                mp3streamin_recv(x);
            }
        }
    }
#endif
    return (w+5);
}

static void mp3streamin_dsp(t_mp3streamin *x, t_signal **sp)
{
    dsp_add(mp3streamin_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}


static void *mp3streamin_new(t_floatarg fportno, t_floatarg fdographics)
{
    t_mp3streamin *x;
    int i;

    if ( fportno < 0 || fportno > 65535 )
    {
        post( "mp3streamin~ : error : wrong portnumber : %d", (int)fportno );
        return NULL;
    }
    if ( ((int)fdographics != 0) && ((int)fdographics != 1.) )
    {
        post( "mp3streamin~ : error : constructor : mp3streamin~ <portnumber> [graphic flag = 0 | 1 ] ( got = %f)", fdographics );
        return NULL;
    }

    x = (t_mp3streamin *)pd_new(mp3streamin_class);
    post( "mp3streamin~ : new %x (instance = %d) %d", x, nbinstances, sizeof( MPSTR ) );
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_connectionip = outlet_new(&x->x_obj, &s_symbol);

    x->x_serversocket = -1;
    x->x_socket = -1;
    x->x_shutdown = 0;
    x->x_inpackets = 0;
    x->x_dpacket = -1;

    x->x_canvas = canvas_getcurrent();

    x->x_inbuffersize = INPUT_BUFFER_SIZE;
    x->x_outbuffersize = OUTPUT_BUFFER_SIZE;
    x->x_inbuffer = (char*) getbytes( x->x_inbuffersize );
    memset( x->x_inbuffer, 0x0, INPUT_BUFFER_SIZE );
    x->x_outbuffer = (t_float*) getbytes( x->x_outbuffersize*sizeof(t_float) );
    memset( x->x_outbuffer, 0x0, OUTPUT_BUFFER_SIZE );

    if ( !x->x_inbuffer || !x->x_outbuffer )
    {
        post( "mp3streamin~ : could not allocate buffers." );
        return NULL;
    }

    if ( nbinstances < MAX_DECODERS )
    {
        x->x_instance = nbinstances++;
    }
    else
    {
        post( "mp3streamin~ : cannot create more decoders (memory issues), sorry" );
        return NULL;
    }

    x->x_inwriteposition = 0;
    x->x_outreadposition = 0;
    x->x_outwriteposition = 0;
    x->x_outunread = 0;

    ztout.tv_sec = 0;
    ztout.tv_usec = 0;

    x->x_graphic = (int)fdographics;

    post( "mp3streamin~ : starting service on port %d", (int)fportno );
    mp3streamin_startservice(x, (int)fportno);

    // init lame decoder
    mp3streamin_tilde_mpglib_init(x);

    return (x);
}


void mp3streamin_tilde_setup(void)
{
    logpost(NULL, 4,  mp3streamin_version );
    mp3streamin_class = class_new(gensym("mp3streamin~"),
                                  (t_newmethod) mp3streamin_new, (t_method) mp3streamin_free,
                                  sizeof(t_mp3streamin),  CLASS_NOINLET, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    class_addmethod(mp3streamin_class, nullfn, gensym("signal"), 0);
    class_addmethod(mp3streamin_class, (t_method) mp3streamin_dsp, gensym("dsp"), 0);
}
