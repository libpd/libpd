/* ------------------------ speexin~ ------------------------------------------ */
/*                                                                              */
/* Object to receive a speex encoded stream sent by a peer using speexin~.     */
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
/* Uses the Speex voice quality encoding library which can                      */
/* be found at http://speex.sourceforge.net.                                    */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


#include <m_pd.h>
#include <g_canvas.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
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
#endif

#if defined(__APPLE__) || defined(_WIN32)
#define MSG_NOSIGNAL 0
#define SOL_TCP IPPROTO_TCP
#endif

#include <speex/speex.h>        /* speex decoder stuff */
#include <speex/speex_bits.h>   /* speex decoder stuff */

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define MIN_AUDIO_INPUT 1024 // we must a least have 8 chunks to play a correct sound
#define INPUT_BUFFER_SIZE 32768
#define OUTPUT_BUFFER_SIZE 32768 /* 32k */
#define BARHEIGHT 10

#define     SPEEX_NB_MODE         0 /* audio data must be 8kHz */
#define     SPEEX_WB_MODE         1 /* audio data must be 16kHz */

//#define DATADEBUG

typedef void (*t_fdpollfn)(void *ptr, int fd);
extern void sys_rmpollfn(int fd);
extern void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);

/* time-out used for select() call */
static struct timeval ztout;

static char   *speexin_version = "speexin~: speex voice quality streamer version 0.2, written by ydegoyon@free.fr";

extern void sys_sockerror(char *s);

void speexin_closesocket(int fd)
{
#ifndef _WIN32
    if ( close(fd) < 0 )
    {
        perror( "close" );
    }
    else
    {
        post( "speexin~ : closed socket : %d", fd );
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
        post("speexin~ : setsockopt TCP_NODELAY failed");
        perror( "setsockopt" );
        return -1;
    }
    else
    {
        post("speexin~ : TCP_NODELAY set");
    }

#ifdef _WIN32
    sockopt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0)
    {
        post("speexin~ : setsockopt SO_REUSEADDR failed");
        perror( "setsockopt" );
        return -1;
    }
    else
    {
        post("speexin~ : setsockopt SO_REUSEADDR done.");
    }
#endif
    return 0;
}


/* ------------------------ speexin~ ----------------------------- */

static t_class *speexin_class;

typedef struct _speexin
{
    t_object x_obj;
    t_int x_socket;
    t_outlet *x_connectionip;
    t_int x_serversocket;
    t_int x_samplerate;

    /* Speex stuff */
    SpeexBits x_bits;         /* bits packing structure    */
    void *x_decstate;         /* decoder state             */
    t_int x_framesize;        /* frame size                */
    t_int x_mode;             /* Narrow or Wide Band       */
    int x_quality;            /* encoding quality ( 0 to 10 ) */

    t_int x_inpackets;  /* number of packets received */
    t_int x_dpacket;    /* displayed packet in status bar */
    t_int x_packetsize; /* size of the packets */
    t_int x_pblocks;    /* processed blocks */
    t_int x_graphic;    /* indicates if we show a graphic bar */

    void *x_inbuffer;   /* accumulation buffer for incoming speex frames */
    t_int x_inwritepos;   /* accumulation buffer for incoming speex frames */
    t_int x_encsize;
    t_int x_inbuffersize;

    t_float *x_outbuffer;   /* buffer to store audio decoded data */
    t_int x_oinp;
    t_int x_ooutp;
    t_int x_outunread;
    t_int x_outbuffersize;
    t_float *x_decchunk;

    t_canvas *x_canvas;

    t_int x_stream;         /* indicates if a stream is connected ( meaning correct input flow ) */
    t_int x_newstream;      /* at first, the stream must provide enough data to start */

} t_speexin;

void speexin_tilde_speex_init(t_speexin *x)
{
    int ret;
    int pf=1;

    speex_bits_init(&x->x_bits);

    switch ( x->x_mode )
    {
    case SPEEX_NB_MODE :
        x->x_decstate = speex_decoder_init(&speex_nb_mode);
        break;

    case SPEEX_WB_MODE :
        x->x_decstate = speex_decoder_init(&speex_wb_mode);
        break;

    default :
        error( "speexin~ : severe error : decoding scheme is unknown" );
        break;
    }

    speex_decoder_ctl(x->x_decstate, SPEEX_GET_FRAME_SIZE, (void*)&x->x_framesize);

    speex_decoder_ctl(x->x_decstate, SPEEX_SET_PF, &pf);

    post( "speexin~ : frame size : %d", x->x_framesize );

}

static void speexin_decode_input(t_speexin *x)
{
    int i;
    int alength = 0;
    static char out[8192];
    signed short int *p = (signed short int *) out;
    int     pbytes;
    int     ret;
    int     flength = 0;

    if ( x->x_encsize > 0 )
    {

        while ( x->x_encsize > *(char *)(x->x_inbuffer) )
        {

            flength = *(char *)(x->x_inbuffer );

            // post( "speexin~ : reading bits from 1 to : %d", flength+1 );
            speex_bits_read_from(&x->x_bits, x->x_inbuffer+1, flength);

#ifdef DATADEBUG
            {
                t_int si;

                printf( "speexin~ : decoding :  " );
                for ( si=0; si<flength; si++ )
                {
                    printf( "%d ", *(char *)(x->x_inbuffer+1+si) );
                }
                printf( "\n" );
            }
#endif

            {
                t_int sp=0, rp=0;

                speex_decode(x->x_decstate, &x->x_bits, x->x_decchunk);

                while( sp < x->x_framesize )
                {
                    rp=(x->x_oinp+sp)%x->x_outbuffersize;
                    // if ( rp == x->x_outbuffersize - 1 ) post( "speexin~ : write at the end of audio buffer" );
                    // post( "speexin~ : sp=%d : rp=%d", sp, rp );
                    x->x_outbuffer[ rp ] = x->x_decchunk[sp++];
                }
                x->x_oinp = rp+1;
            }
            x->x_outunread += x->x_framesize;
            memcpy( x->x_inbuffer, x->x_inbuffer+flength+1, x->x_inbuffersize-flength-1 );
            x->x_encsize -= flength+1;
            x->x_inwritepos -= flength+1;

        }

    }

    if ( x->x_graphic && glist_isvisible( x->x_canvas ) )
    {
        /* update graphical read status */
        if ( x->x_inpackets != x->x_dpacket )
        {
            char color[32];
            int minpackets = ( MIN_AUDIO_INPUT/x->x_framesize)-2; // audio loop has eaten some already


            sys_vgui(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x );
            sys_vgui(".x%lx.c delete line %xTHRESHOLD\n", x->x_canvas, x );
            if ( x->x_outunread > 0 )
            {
                t_int width;

                if ( x->x_inpackets < (MIN_AUDIO_INPUT/x->x_framesize)/2 )
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
        }

    }

}

static void speexin_recv(t_speexin *x)
{
    int ret;

    if ( x->x_inwritepos > x->x_inbuffersize - 1024 )
    {
        post( "speexin~ : input buffer is full" );
        return;
    }
    if ( ( ret = recv(x->x_socket, (void*) x->x_inbuffer + x->x_inwritepos,
                      (size_t)x->x_inbuffersize,
                      MSG_NOSIGNAL) ) < 0 )
    {
        post( "speexin~ : receive error" );
        perror( "recv" );
        return;
    }
    else
    {
        // post( "speexin~ : received %d bytes at %d on %d ( up to %d)",
        //        ret, x->x_inwritepos, x->x_socket,
        //        x->x_inbuffersize );

        if ( ret == 0 )
        {
            post( "speexin~ : closing connection ( s=%d )", x->x_socket );
            speexin_closesocket(x->x_socket);
            x->x_socket = -1;
            sys_vgui(".x%lx.c delete rectangle %xPBAR\n", x->x_canvas, x );
            sys_vgui(".x%lx.c delete line %xTHRESHOLD\n", x->x_canvas, x );
            sys_vgui(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x );
            outlet_symbol( x->x_connectionip, gensym("") );
        }
        else
        {
            x->x_inpackets++;
        }

        x->x_encsize += ret;
        x->x_inwritepos += ret;

        speexin_decode_input(x);
    }
}

static void speexin_acceptconnection(t_speexin *x)
{
    struct sockaddr_in incomer_address;
    int sockaddrl = (int) sizeof( struct sockaddr );

    int fd = accept(x->x_serversocket, (struct sockaddr*)&incomer_address, &sockaddrl );
    post("speexin~: accepted incomer : %d.", fd );

    if (fd < 0)
    {
        post("speexin~: accept failed");
        return;
    }

    if (x->x_socket > 0)
    {
        post("speexin~: the source has changed to %s ( new socket = %d ).",
             inet_ntoa( incomer_address.sin_addr ), fd );
        speexin_closesocket(x->x_socket);
    }

    x->x_socket = fd;
    sys_addpollfn(x->x_socket, (t_fdpollfn)speexin_recv, x);
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


static int speexin_startservice(t_speexin* x, int portno)
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
        speexin_closesocket(sockfd);
        return (0);
    }

    if (listen(sockfd, 5) < 0)
    {
        sys_sockerror("listen");
        speexin_closesocket(sockfd);
    }
    else
    {
        x->x_serversocket = sockfd;
        sys_addpollfn(x->x_serversocket, (t_fdpollfn)speexin_acceptconnection, x);
    }

    return 1;
}

static void speexin_free(t_speexin *x)
{
    post( "speexin~ : free %x", x );
    if (x->x_serversocket > 0)
    {
        post( "speexin~ : closing server socket" );
        speexin_closesocket(x->x_serversocket);
        x->x_serversocket = -1;
    }
    if (x->x_socket > 0)
    {
        post( "speexin~ : closing socket" );
        speexin_closesocket(x->x_socket);
        x->x_socket = -1;
    }
    if ( x->x_inbuffer ) freebytes( x->x_inbuffer, x->x_inbuffersize );
    if ( x->x_outbuffer ) freebytes( x->x_outbuffer, x->x_outbuffersize*sizeof(t_float) );
    if ( x->x_decchunk ) freebytes(x->x_decchunk, x->x_framesize*sizeof(t_float));
}

static t_int *speexin_perform(t_int *w)
{
    t_speexin *x = (t_speexin*) (w[1]);
    t_float *out = (t_float *)(w[2]);
    t_int n = (int)(w[3]);
    t_int bsize = n;
    t_int ret;
    t_int i = 0;
    t_int j = 0;
    t_int sp = 0;
    t_int sratio;

    // samplerate is supposed to be above 16kHz, thus sratio>1
    if ( x->x_mode == SPEEX_NB_MODE )
    {
        sratio = x->x_samplerate / 8000;
    }
    else
    {
        sratio = x->x_samplerate / 16000;
    }
    // post( "speexin~ : ratio : %d", sratio );

    memset( out, 0x0, n*sizeof(t_float ) );

    sp = 0;
    while( sp < n )
    {
        if ( ( ( x->x_outunread > MIN_AUDIO_INPUT ) && x->x_newstream ) || // wait the buffer to load
                ( x->x_stream ) // check that the stream provides enough data
           )
        {
            if ( x->x_newstream )
            {
                x->x_newstream = 0;
                x->x_stream = 1;
            }
            /* resampling */
            for ( j=0; j<sratio; j++ )
            {
                *(out+sp)=*(x->x_outbuffer+x->x_ooutp)/8000;  // input has been scaled
                //*(x->x_outbuffer+x->x_ooutp)=0.0; // data read, now zeroed
                sp++;
                if ( sp >= n ) break;
            }
            x->x_ooutp = (x->x_ooutp + 1)%x->x_outbuffersize;
            // if (  x->x_ooutp == x->x_outbuffersize - 1 ) post( "speexin~ : end of audio buffer" );
            x->x_outunread-=1;
        }
        else
        {
            for ( j=0; j<sratio; j++ )
            {
                *(out+sp)=0.0;
                sp++;
                if ( sp >= n ) break;
            }
        }
    }
    x->x_pblocks++;

    if ( ( x->x_outunread <= MIN_AUDIO_INPUT/10  ) && ( x->x_stream ) )
    {
        // post( "speexin~ : stream lost (too little input)" );
        x->x_stream = 0;
        x->x_newstream = 1; // waiting for a new stream
    }

    if ( x->x_pblocks == x->x_framesize/bsize )
    {
        x->x_inpackets--;
        x->x_pblocks = 0;
    }

    return (w+4);
}

static void speexin_dsp(t_speexin *x, t_signal **sp)
{
    dsp_add(speexin_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void *speexin_new(t_floatarg fportno, t_floatarg fdographics)
{
    t_speexin *x;
    int i;

    if ( fportno < 0 || fportno > 65535 )
    {
        post( "speexin~ : error : wrong portnumber : %d", (int)fportno );
        return NULL;
    }
    if ( ((int)fdographics != 0) && ((int)fdographics != 1.) )
    {
        post( "speexin~ : error : constructor : speexin~ <portnumber> [graphic flag = 0 | 1 ] ( got = %f)", fdographics );
        return NULL;
    }

    x = (t_speexin *)pd_new(speexin_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_connectionip = outlet_new(&x->x_obj, &s_symbol);

    x->x_serversocket = -1;
    x->x_socket = -1;
    x->x_inpackets = 0;
    x->x_inwritepos = 0;
    x->x_dpacket = -1;
    x->x_mode = SPEEX_NB_MODE;
    x->x_samplerate = sys_getsr();

    x->x_canvas = canvas_getcurrent();

    x->x_inbuffersize = INPUT_BUFFER_SIZE;
    x->x_outbuffersize = OUTPUT_BUFFER_SIZE;
    x->x_inbuffer = (char*) getbytes( x->x_inbuffersize );
    memset( x->x_inbuffer, 0x0, INPUT_BUFFER_SIZE );
    x->x_outbuffer = (t_float*) getbytes( x->x_outbuffersize*sizeof(t_float) );
    memset( x->x_outbuffer, 0x0, OUTPUT_BUFFER_SIZE*sizeof(t_float) );

    if ( !x->x_inbuffer || !x->x_outbuffer )
    {
        post( "speexin~ : could not allocate buffers." );
        return NULL;
    }

    x->x_encsize = 0;
    x->x_oinp = 0;
    x->x_ooutp = 0;

    ztout.tv_sec = 0;
    ztout.tv_usec = 0;

    x->x_graphic = (int)fdographics;

    post( "speexin~ : starting service on port %d", (int)fportno );
    speexin_startservice(x, (int)fportno);

    // init speex decoder
    speexin_tilde_speex_init(x);

    x->x_decchunk = (t_float*)getbytes(x->x_framesize*sizeof(t_float));
    if (!x->x_decchunk) /* check allocation... */
    {
        error("speexin~ : cannot allocate chunk");
        return NULL;
    }

    return (x);
}


void speexin_tilde_setup(void)
{
    logpost(NULL, 4,  speexin_version );
    speexin_class = class_new(gensym("speexin~"),
                              (t_newmethod) speexin_new, (t_method) speexin_free,
                              sizeof(t_speexin),  CLASS_NOINLET, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    class_addmethod(speexin_class, nullfn, gensym("signal"), 0);
    class_addmethod(speexin_class, (t_method) speexin_dsp, gensym("dsp"), 0);
}
