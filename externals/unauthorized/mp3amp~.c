/* ------------------------- mp3amp~ ------------------------------------------ */
/*                                                                              */
/* Tilde object to receive an mp3-stream from a shoutcast/icecast server.       */
/* Written by Yves Degoyon (ydegoyon@free.fr).                                  */
/* Get source at http://ydegoyon.free.fr                                        */
/*                                                                              */
/* This library is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU Lesser General Public                   */
/* License as published by the Free Software Foundation; either                 */
/* version 2 of the License, or (at your option) any later version.             */
/*                                                                              */
/* This library is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            */
/* Lesser General Public License for more details.                              */
/*                                                                              */
/* You should have received a copy of the GNU Lesser General Public             */
/* License along with this library; if not, write to the                        */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,                 */
/* Boston, MA  02111-1307, USA.                                                 */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/* Uses the LAME MPEG 1 Layer 3 encoding library which can be found at          */
/* http://www.mp3dev.org                                                        */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include <m_pd.h>
#include "m_imp.h"
#include "g_canvas.h"
#include "s_stuff.h"
#include "pthread.h"

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "mpg123.h"      /* mpg123 decoding library from lame 3.92 */
#include "mpglib.h"      /* mpglib decoding library from lame 3.92 */
#include "interface.h"   /* mpglib decoding library from lame 3.92 */
#ifdef _WIN32
#if !defined(__OBJC__) && !defined(__GNU_LIBOBJC__) && !defined(__objc_INCLUDE_GNU)
#define BOOL WINBOOL
#endif
#include <winsock.h>
#include <winbase.h>
#include <io.h>
#define strdup _strdup
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#define SOCKET_ERROR -1
#endif /* _WIN32 */

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#if defined(__APPLE__) || defined(WIN32)
#define MSG_NOSIGNAL 0
#endif

#define     LAME_AUDIO_CHUNK_SIZE     1152
#define     MIN_AUDIO_INPUT           2*LAME_AUDIO_CHUNK_SIZE  /* we must have at least n chunks to play a steady sound */
#define     INPUT_BUFFER_SIZE         131072                   /* data received on the socket : 128k */
#define     OUTPUT_BUFFER_SIZE        131072                   /* audio output buffer : 128k */
#define     DECODE_PACKET_SIZE        131072                   /* size of the data returned by mpglib : 128k */
#define     STRBUF_SIZE               4096                     /* char received from server on startup */
#define     MAX_DECODERS              50
#define     BARHEIGHT                 10

static int guidebug=0;

#define SYS_VGUI2(a,b) if (guidebug) \
                         post(a,b);\
                         sys_vgui(a,b)

#define SYS_VGUI3(a,b,c) if (guidebug) \
                         post(a,b,c);\
                         sys_vgui(a,b,c)

#define SYS_VGUI4(a,b,c,d) if (guidebug) \
                         post(a,b,c,d);\
                         sys_vgui(a,b,c,d)

#define SYS_VGUI5(a,b,c,d,e) if (guidebug) \
                         post(a,b,c,d,e);\
                         sys_vgui(a,b,c,d,e)

#define SYS_VGUI6(a,b,c,d,e,f) if (guidebug) \
                         post(a,b,c,d,e,f);\
                         sys_vgui(a,b,c,d,e,f)

#define SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g );\
                         sys_vgui(a,b,c,d,e,f,g)

#define SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h );\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)


/* useful debugging functions from mpglib */
extern int decode_header( struct frame* fr, unsigned long newhead );
extern void print_header_compact( struct frame* fr );
extern int head_check( unsigned long head, int check_layer );

static char   *mp3amp_version = "mp3amp~: mp3 streaming client v0.12, written by Yves Degoyon";

/* ------------------------ mp3amp~ ----------------------------- */

static t_class *mp3amp_class;

/* too bad, this needs to be static,
   handling an array to enable several decoders in pd */
static  MPSTR   mps[MAX_DECODERS];  /* decoder buffer */
static int nbinstances = 0;

extern const long  freqs[9];

/* time-out used for select() call */
static struct timeval ztout;

typedef struct _mp3amp
{
    t_object x_obj;
    t_int    x_instance;       /* instance of the object */
    t_outlet *x_connection;
    t_int    x_fd;             /* the socket number */
    t_int    x_inframes;       /* number of waiting frames */
    t_int    x_dframes;        /* displayed frames in status bar */
    t_int    x_packetsize;     /* size of the packets */
    t_int    x_pblocks;        /* processed blocks */
    t_int    x_graphic;        /* indicates if we show a graphic bar */
    t_canvas *x_canvas;        /* remember canvas */
    t_int    x_nbwaitloops;    /* number of loops to wait */
    t_int    x_nbloops;        /* number of loops processed */
    t_int    x_blocksize;      /* size of a dsp block */
    t_int    x_resample;       /* resampling factor (pd's sr / stream sr) */
    t_int    x_dsp;            /* number of dsp calls, used to measure time */
    t_int    x_standby;        /* flag to freeze decoding */
    t_int    x_nooutput;       /* flag to avoid output of connection state */

#ifdef _WIN32
    char    *x_inbuffer;       /* accumulation buffer for incoming mp3 frames */
#else
    unsigned char  *x_inbuffer;       /* accumulation buffer for incoming mp3 frames */
#endif

    t_int    x_inwriteposition;
    t_int    x_inbuffersize;
    t_int    x_offset;         /* offset used for start of decoding */

    t_float *x_outbuffer;      /* buffer to store audio decoded data */
    t_int    x_outwriteposition;
    t_int    x_outreadposition;
    t_int    x_outunread;
    t_int    x_outbuffersize;
    char     x_out[DECODE_PACKET_SIZE];

    /* mp3 stuff */
    t_int    x_samplerate;
    t_int    x_bitrate;        /* bitrate of mp3 stream read at connection time */
    t_int    x_bitrateindex;   /* bitrate index for each frame, might change dynamically */
    t_int    x_mp3mode;        /* mode (mono, joint stereo, stereo, dual mono) */
    char*    x_bcname;         /* name of broadcast */
    char*    x_bcurl;          /* url of broadcast */
    char*    x_bcgenre;        /* genre of broadcast */
    char*    x_bcaim;          /* aim of broadcast */
    char*    x_mountpoint;     /* mountpoint for IceCast server */
    char*    x_hostname;       /* hostname to connect to */
    t_int    x_port;           /* port number to connect to */

    t_int    x_stream;         /* indicates if a stream is connected ( meaning correct input flow ) */
} t_mp3amp;

static void mp3amp_recv(t_mp3amp *x);
static void mp3amp_disconnect(t_mp3amp *x);
static void mp3amp_connect_url(t_mp3amp *x, t_symbol *url);

static int strip_shout_header(char *head, int n)
{
    int i;
    for (i = 0; i < (n - 2); i++)
    {
        if (head[i] == 10 && head[i + 1] == 13)
            break;
        if (head[i] == '\n' && head[i + 1] == '\n')
            break;
    }
    head[i + 1] = '\0';
    return n - (i + 1);
}

static int strip_ice_header(char *head, int n)
{
    int i;
    for (i = 0; i < (n - 2); i++)
    {
        if ((head[i] == '\n') && (head[i + 1] == '\n'))
            break;
    }
    head[i + 1] = '\0';
    return n - (i + 1);
}

static void mp3amp_tilde_mpglib_init(t_mp3amp *x)
{
    int ret;

    InitMP3(&mps[x->x_instance]);
}

static int mp3amp_decode_input(t_mp3amp *x)
{
    t_int i;
    t_int alength = 0;
    float resample = 0;
    struct frame hframe;
    unsigned int a,b,c,d;
    unsigned long cheader;
    signed short int *p = (signed short int *) x->x_out;
    t_int     pbytes;
    t_int     ret, totlength=0;
    t_int     pframes = 0;

    x->x_offset=0;
    // search for an header to check dynamic bitrate
    while ( x->x_offset < x->x_inwriteposition )
    {
        /* decode first 4 bytes as the header */
        a = *((unsigned char*)x->x_inbuffer+x->x_offset);
        b = *((unsigned char*)x->x_inbuffer+x->x_offset+1);
        c = *((unsigned char*)x->x_inbuffer+x->x_offset+2);
        d = *((unsigned char*)x->x_inbuffer+x->x_offset+3);

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
            decode_header( &hframe, cheader );
            if ( hframe.framesize == 0 )
            {
                post( "mp3amp~: weird header ( frame size = 0 ) .... ignored" );
                x->x_offset++;
                continue;
            }
            x->x_packetsize = hframe.framesize;
            // print_header_compact( &hframe );
            // when the bitrate change, reinit decoder
            if ( x->x_bitrateindex != hframe.bitrate_index )
            {
                post( "mp3amp~: bitrate has changed, reinitialize decoder" );
                ExitMP3(&mps[x->x_instance]);
                InitMP3(&mps[x->x_instance]);
                x->x_bitrateindex = hframe.bitrate_index;
            }
            break;
        }
        x->x_offset++;
    }

    if ( x->x_inframes > 0 )
    {

        ret = decodeMP3(&mps[x->x_instance], (unsigned char*)(x->x_inbuffer+x->x_offset),
                        hframe.framesize+sizeof( unsigned long), (char *) p, sizeof(x->x_out), &pbytes);

        switch (ret)
        {
        case MP3_OK:
            switch (mps[x->x_instance].fr.stereo)
            {
            case 1:
            case 2:
                alength = ((mps[x->x_instance].fr.stereo==1)?pbytes >> 1 : pbytes >> 2);
                // post( "mp3amp~: stereo : %d", mps[x->x_instance].fr.stereo );
                // update outbuffer contents
                for ( i=0; i<alength; i++ )
                {
                    if ( x->x_outunread >= x->x_outbuffersize-2 )
                    {
                        // post( "mp3amp~: decode : too much input ... ignored" );
                        continue;
                    }
                    *(x->x_outbuffer+x->x_outwriteposition) = ((t_float)(*p++))/32767.0;
                    x->x_outwriteposition = (x->x_outwriteposition + 1)%x->x_outbuffersize;
                    *(x->x_outbuffer+x->x_outwriteposition) =
                        ((mps[x->x_instance].fr.stereo==2)?((t_float)(*p++))/32767.0 : 0.0);
                    x->x_outwriteposition = (x->x_outwriteposition + 1)%x->x_outbuffersize;
                    x->x_outunread+=2;

                    if ( x->x_outunread >= MIN_AUDIO_INPUT && !x->x_stream )
                    {
                        post("mp3amp~: stream connected" );
                        x->x_resample = x->x_samplerate / freqs[hframe.sampling_frequency];
                        if ( x->x_resample == 0 ) x->x_resample=1;
                        post("mp3amp~: resampling stream from %d to %d Hz (r=%d)",
                             freqs[hframe.sampling_frequency], x->x_samplerate, x->x_resample );
                        x->x_stream = 1;
                    }
                }
                break;
            default:
                alength = -1;
                break;
            }
            // roll buffer
            if (  x->x_inwriteposition > hframe.framesize+ (int) sizeof( unsigned long ) + x->x_offset )
                // ^----- maybe the frame is not complete
            {
                x->x_inwriteposition -= hframe.framesize+sizeof( unsigned long )+x->x_offset;
                memcpy( (void *)(x->x_inbuffer),
                        (void *)(x->x_inbuffer+x->x_offset+hframe.framesize+sizeof( unsigned long )),
                        x->x_inwriteposition);
                x->x_offset = 0;
                // post ( "mp3amp~: decoded frame %d", x->x_inframes );
                x->x_inframes--;
                pframes++;
            }
            else // sorry, it will be ignored
            {
                // error( "mp3amp~: incomplete frame...ignored");
                // x->x_offset = 0;
                // x->x_inwriteposition = 0;
                // x->x_inframes = 0;
            }

            totlength += alength;
            break;

        case MP3_NEED_MORE:
            if ( mps[x->x_instance].framesize == 0 && mps[x->x_instance].fsizeold != 0 )
            {
                post( "mp3amp~: decoding done (totlength=%d).", totlength );
            }
            else
            {
                post( "mp3amp~: retry lame decoding (more data needed)." );
                return -1;
            }
            break;

        case MP3_ERR:
            post( "mp3amp~: lame decoding failed." );
            return ret;
            break;

        }

    }

    if ( x->x_graphic && glist_isvisible( x->x_canvas ) )
    {
        /* update graphical read status */
        if ( x->x_inframes != x->x_dframes )
        {
            char color[32];
            t_int width;

            width = rtext_width( glist_findrtext( (t_glist*)x->x_canvas, (t_text *)x ) );
            SYS_VGUI3(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x );
            if ( x->x_inframes < (MIN_AUDIO_INPUT/LAME_AUDIO_CHUNK_SIZE) )
            {
                strcpy( color, "red" );
            }
            else
            {
                strcpy( color, "lightgreen" );
            }
            SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %xSTATUS\n",
                      x->x_canvas, x->x_obj.te_xpix, x->x_obj.te_ypix-BARHEIGHT-1,
                      x->x_obj.te_xpix+(x->x_inwriteposition*width)/INPUT_BUFFER_SIZE,
                      x->x_obj.te_ypix - 1, color, x );
            x->x_dframes = x->x_inframes;
        }
    }
    return totlength;
}

static void mp3amp_recv(t_mp3amp *x)
{
    int ret, i;
    float resample = 0;
    struct frame hframe;
    unsigned int a,b,c,d;
    unsigned long cheader;

#ifdef _WIN32
    if(( ret = recv(x->x_fd, (char*)x->x_inbuffer + x->x_inwriteposition,
                    (x->x_inbuffersize-x->x_inwriteposition), 0)) < 0)
#else
    if ( ( ret = recv(x->x_fd, (void*) (x->x_inbuffer + x->x_inwriteposition),
                      (size_t)((x->x_inbuffersize-x->x_inwriteposition)),
                      MSG_NOSIGNAL) ) < 0 )
#endif
    {
        post( "mp3amp~: receive error" );
#ifndef _MSC_VER
        perror( "recv" );
#endif
        mp3amp_disconnect(x);
        return;
    }
    else
    {

        // post( "mp3amp~: received %d bytes at %d on %d ( up to %d)",
        //        ret, x->x_inwriteposition, x->x_fd,
        //        x->x_inbuffersize-x->x_inwriteposition );

        if ( ret == 0 && ( x->x_inbuffersize-x->x_inwriteposition != 0 ) )
        {
            error("mp3amp~: stream lost...");
            mp3amp_disconnect(x);
        }
        else
        {
            // check if we should decode those packets
            if ( x->x_standby )
            {
                return;
            }
            // check we don't overflow input buffer
            if ( ( x->x_inwriteposition + ret ) > x->x_inbuffersize*0.9 )
            {
                post( "mp3streamin~ : too much input...resetting" );
                x->x_inwriteposition=0;
                x->x_offset = 0;
                x->x_inframes = 0;
                return;
            }
            x->x_inwriteposition += ret;
            x->x_offset = 0;
            // check some parameters in the stream
            while ( x->x_offset < x->x_inwriteposition )
            {
                /* decode first 4 bytes as the header */
                a = *((unsigned char*)x->x_inbuffer+x->x_offset);
                b = *((unsigned char*)x->x_inbuffer+x->x_offset+1);
                c = *((unsigned char*)x->x_inbuffer+x->x_offset+2);
                d = *((unsigned char*)x->x_inbuffer+x->x_offset+3);

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
                    decode_header( &hframe, cheader );
                    // print_header_compact( &hframe );
                    x->x_packetsize = hframe.framesize;
                    if ( hframe.framesize == 0 )
                    {
                        post( "mp3amp~: weird header ( frame size = 0 ) .... ignored" );
                        x->x_inwriteposition -= ret;
                        return;
                    }
                    x->x_inframes += ret/x->x_packetsize;
                    // post( "mp3amp~: nb frames %d", x->x_inframes );
                    break;
                }
                x->x_offset++;
            }
        }
    }
}

static t_int *mp3amp_perform(t_int *w)
{
    t_mp3amp *x = (t_mp3amp*) (w[1]);
    t_float *out1 = (t_float *)(w[2]);
    t_float *out2 = (t_float *)(w[3]);
    int n = (int)(w[4]);
    int ret;
    int i = 0;

    x->x_blocksize = n;
    x->x_nbwaitloops = LAME_AUDIO_CHUNK_SIZE/x->x_blocksize;
    // post( "mp3mp3amp~  : will wait %d loops", x->x_nbwaitloops );
    x->x_dsp++;

    while( n-- )
    {
        if(x->x_stream  && !x->x_standby ) // check that the stream provides enough data
        {
            if(x->x_resample == 1)    /* don't need to resample */
            {
                *out1++=*(x->x_outbuffer+x->x_outreadposition);
                x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
                *out2++=*(x->x_outbuffer+x->x_outreadposition);
                x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
                x->x_outunread-=2;
            }
            else
            {
                /* we just use the same sample x->x_resample times */
                *out1++=*(x->x_outbuffer+x->x_outreadposition);
                *out2++=*(x->x_outbuffer+((x->x_outreadposition + 1)%x->x_outbuffersize));
                if((n%x->x_resample)== 0)
                {
                    x->x_outreadposition = (x->x_outreadposition + 2)%x->x_outbuffersize;
                    x->x_outunread-=2;
                }
            }
            if ( n == 1 ) x->x_pblocks++;
        }
        else
        {
            *out1++=0.0;
            *out2++=0.0;
        }
    }

    if ( x->x_pblocks == LAME_AUDIO_CHUNK_SIZE/x->x_blocksize )
    {
        x->x_pblocks = 0;
    }

    /* check for readability, then fill the input buffer */
    if(( x->x_fd > 0 )&&(x->x_dsp >= 16))    /* determine how often we try to read */
    {
        fd_set readset;
        fd_set exceptset;

        FD_ZERO(&readset);
        FD_ZERO(&exceptset);
        FD_SET(x->x_fd, &readset );
        FD_SET(x->x_fd, &exceptset );

        x->x_dsp = 0;

        if ( select( x->x_fd+1, &readset, NULL, &exceptset, &ztout ) >0 )
        {
            if ( FD_ISSET( x->x_fd, &readset) || FD_ISSET( x->x_fd, &exceptset ) )
            {
                /* receive data or error */
                mp3amp_recv(x);
            }
        }
    }

    // check new incoming data
    if ( x->x_fd > 0 && ( x->x_nbloops == 0 ) && ( x->x_inframes > 0 ) && ( !x->x_standby ) )
    {
        mp3amp_decode_input(x);
    }
    if ( x->x_nbwaitloops != 0 )
    {
        x->x_nbloops = (x->x_nbloops+1 ) % x->x_nbwaitloops;
    }
    return (w+5);
}

static void mp3amp_dsp(t_mp3amp *x, t_signal **sp)
{
    dsp_add(mp3amp_perform, 4, x, sp[1]->s_vec, sp[2]->s_vec, sp[1]->s_n);
}


/* freeze decoding */
static void mp3amp_standby(t_mp3amp *x, t_floatarg fstandby )
{
    if ( fstandby == 0. )
    {
        x->x_standby = 0;
    }
    else
    {
        x->x_standby = 1;
    }
}

/* connection main procedure executed by a thread */
static void *mp3amp_do_connect(void *tdata )
{
    t_mp3amp *x = (t_mp3amp*) tdata;
    struct          sockaddr_in server;
    struct          hostent *hp;
    t_int           portno            = x->x_port;    /* get port from message box */

    /* variables used for communication with server */
    char            *sptr = NULL;
    char            request[STRBUF_SIZE];           /* string to be send to server */
    char            *url;               /* used for relocation */
    fd_set          fdset;
    struct timeval  tv;
    t_int           sockfd;                         /* socket to server */
    t_int           relocate, numrelocs = 0;
    t_int           i, ret, rest, nanswers=0;
    char            *cpoint = NULL;
    t_int           offset = 0, endofheaders = 0;

    if (x->x_fd >= 0)
    {
        error("mp3amp~: already connected");
        return NULL;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("mp3amp~: internal error while attempting to open socket");
        return NULL;
    }

    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(x->x_hostname);
    if (hp == 0)
    {
        post("mp3amp~: bad host?");
        sys_closesocket(sockfd);
        return NULL;
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((unsigned short)portno);

    /* try to connect.  */
    post("mp3amp~: connecting to http:/%s:%d/%s", x->x_hostname, x->x_port, x->x_mountpoint );
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        error("mp3amp~: connection failed!\n");
        sys_closesocket(sockfd);
        return NULL;
    }
    post("mp3amp~: connected  : socket opened" );

    /* sheck if we can read/write from/to the socket */
    FD_ZERO( &fdset);
    FD_SET( sockfd, &fdset);
    tv.tv_sec  = 0;            /* seconds */
    tv.tv_usec = 500;        /* microseconds */

    ret = select(sockfd + 1, &fdset, NULL, NULL, &tv);
    if(ret != 0)
    {
        error("mp3amp~: can not read from socket");
        sys_closesocket(sockfd);
        return NULL;
    }
    post("mp3amp~: select done" );

    /* check mountpoint */
    if( strstr(x->x_mountpoint, "listen.pls") )
    {
        /* SHOUTcast playlist -> get / */
        x->x_mountpoint = "";
    }

    /* build up stuff we need to send to server */
    sprintf(request, "GET /%s HTTP/1.0 \r\nHost: %s\r\nUser-Agent: mp3amp~ 0.11\r\nAccept: */*\r\n\r\n",
            x->x_mountpoint, x->x_hostname);

    if ( send(sockfd, request, strlen(request), 0) < 0 )    /* say hello to server */
    {
        post( "mp3amp~: could not contact server... " );
#ifndef _MSC_VER
        perror( "send" );
#endif
        return NULL;
    }
    post("mp3amp~: send done" );

    relocate = FALSE;
    memset( request, 0x00, STRBUF_SIZE );

    // read all the answer
    endofheaders=0;
    while ( !endofheaders )
    {
        if( ( ret = recv(sockfd, request+offset, STRBUF_SIZE, MSG_NOSIGNAL) ) <0)
        {
            error("mp3amp~: no response from server");
#ifndef _MSC_VER
            perror( "recv" );
#endif
            return NULL;
        }
        post ( "mp3amp~ : received %d bytes at %d", ret, offset );
        for ( i=offset; i<offset+ret-1; i++ )
        {
            if ( ( request[i] == '\n' && request[i+1] == '\n' ) ||
                    ( request[i] == 10 && request[i+1] == 13 ) )
            {
                endofheaders=1;
            }
        }
        offset+=ret;
    }

    // time to parse content of the response...
    if ( strstr(request, "audio/x-scpls") )    /* SHOUTcast playlist */
    {
        /* playlist playing not supported */
        post("mp3amp~: SHOUTcast server returned a playlist, quitting");
        sys_closesocket(sockfd);
        return NULL;
    }
    if ( strstr(request, "HTTP") )    /* seems to be IceCast server */
    {
        strip_ice_header(request, STRBUF_SIZE);
        if(sptr = strstr(request, "302"))
        {
            cpoint = NULL;
            cpoint = strstr(request, "Location:");
            if ( cpoint == NULL )
            {
                post( "mp3amp~ : stream has moved but couldn't find new location out of this :" );
                post("mp3amp~: %s", request );
                return NULL;
            }
            url = strdup(cpoint + 10);
            post("mp3amp~: relocating to %s", url);
            sys_closesocket(sockfd);
            x->x_nooutput = 1;
            mp3amp_connect_url(x, gensym(url));
            x->x_nooutput = 0;
            return NULL;
            // relocate = TRUE;
        }
        if( !(sptr = strstr(request, "200")) && !relocate )
        {
            error("mp3amp~: cannot connect to the (default) stream");
            return NULL;
        }

        post("mp3amp~: IceCast server detected");

        // post("mp3amp~: server's header : %s", request );

        // check what we got
        if( cpoint = strstr(request, "x-audiocast-mount:"))
        {
            x->x_mountpoint = strdup(cpoint + 18);
            for ( i=0; i<(int)strlen(x->x_mountpoint); i++ )
            {
                if ( x->x_mountpoint[i] == '\n' )
                {
                    x->x_mountpoint[i] = '\0';
                    break;
                }
            }
            post("           mountpoint: %s", x->x_mountpoint);
        }
        if( cpoint = strstr(request, "x-audiocast-server-url:"))
        {
            sptr = strdup( cpoint + 24);
            for ( i=0; i<(int)strlen(sptr); i++ )
            {
                if ( sptr[i] == '\n' )
                {
                    sptr[i] = '\0';
                    break;
                }
            }
            post("           server-url: %s", sptr);
        }
        if( cpoint = strstr(request, "x-audiocast-location:"))
        {
            sptr = strdup( cpoint + 22);
            for ( i=0; i<(int)strlen(sptr); i++ )
            {
                if ( sptr[i] == '\n' )
                {
                    sptr[i] = '\0';
                    break;
                }
            }
            post("           location: %s", sptr);
        }
        if( cpoint = strstr(request, "x-audiocast-admin:"))
        {
            sptr = strdup( cpoint + 19);
            for ( i=0; i<(int)strlen(sptr); i++ )
            {
                if ( sptr[i] == '\n' )
                {
                    sptr[i] = '\0';
                    break;
                }
            }
            post("           admin: %s", sptr);
        }
        if( cpoint = strstr(request, "x-audiocast-name:"))
        {
            x->x_bcname = strdup( cpoint + 17);
            for ( i=0; i<(int)strlen(x->x_bcname); i++ )
            {
                if ( x->x_bcname[i] == '\n' )
                {
                    x->x_bcname[i] = '\0';
                    break;
                }
            }
            post("           name: %s", x->x_bcname);
        }
        if( cpoint = strstr(request, "x-audiocast-genre:"))
        {
            x->x_bcgenre = strdup( cpoint + 18);
            for ( i=0; i<(int)strlen(x->x_bcgenre); i++ )
            {
                if ( x->x_bcgenre[i] == '\n' )
                {
                    x->x_bcgenre[i] = '\0';
                    break;
                }
            }
            post("           genre: %s", x->x_bcgenre);
        }
        if( cpoint = strstr(request, "x-audiocast-url:"))
        {
            x->x_bcurl = strdup( cpoint + 16);
            for ( i=0; i<(int)strlen(x->x_bcurl); i++ )
            {
                if ( x->x_bcurl[i] == '\n' )
                {
                    x->x_bcurl[i] = '\0';
                    break;
                }
            }
            post("           url: %s", x->x_bcurl);
        }
        if( cpoint = strstr(request, "x-audiocast-public:1"))
        {
            post("           broadcast is public");
        }
        else if( cpoint = strstr(request, "x-audiocast-public:0"))
        {
            post("           broadcast is NOT public");
        }
        if( cpoint = strstr(request, "x-audiocast-bitrate:"))
        {
            sptr = strdup( cpoint + 20);
            for ( i=0; i<(int)strlen(sptr); i++ )
            {
                if ( sptr[i] == '\n' )
                {
                    sptr[i] = '\0';
                    break;
                }
            }
            if(!strncmp(sptr, "320", 3))x->x_bitrate = 320;
            else if(!strncmp(sptr, "256", 3))x->x_bitrate = 256;
            else if(!strncmp(sptr, "224", 3))x->x_bitrate = 224;
            else if(!strncmp(sptr, "192", 3))x->x_bitrate = 192;
            else if(!strncmp(sptr, "160", 3))x->x_bitrate = 160;
            else if(!strncmp(sptr, "144", 3))x->x_bitrate = 144;
            else if(!strncmp(sptr, "128", 3))x->x_bitrate = 128;
            else if(!strncmp(sptr, "112", 3))x->x_bitrate = 112;
            else if(!strncmp(sptr, "96", 2))x->x_bitrate = 96;
            else if(!strncmp(sptr, "80", 2))x->x_bitrate = 80;
            else if(!strncmp(sptr, "64", 2))x->x_bitrate = 64;
            else if(!strncmp(sptr, "56", 2))x->x_bitrate = 56;
            else if(!strncmp(sptr, "48", 2))x->x_bitrate = 48;
            else if(!strncmp(sptr, "40", 2))x->x_bitrate = 40;
            else if(!strncmp(sptr, "32", 2))x->x_bitrate = 32;
            else if(!strncmp(sptr, "24", 2))x->x_bitrate = 24;
            else if(!strncmp(sptr, "16", 2))x->x_bitrate = 16;
            else if(!strncmp(sptr, "8", 1))x->x_bitrate = 8;
            else
            {
                post("mp3amp~: unsupported bitrate! : %s", sptr);
                return NULL;
            }
            post("           bitrate: %d", x->x_bitrate);
        }
        if( cpoint = strstr(request, "x-audiocast-udpport:"))
        {
            post("mp3amp~: sorry, server wants UDP connection!");
            return NULL;
        }
    }
    else    /* it is a SHOUTcast server */
    {
        strip_shout_header (request, STRBUF_SIZE);
        post("mp3amp~: SHOUTcast server detected");
        if(strstr(request, "ICY 401") != 0)
        {
            post("mp3amp~: ICY 401 Service Unavailable");
            return NULL;
        }
        if (strstr(request, "ICY 200 OK"))
        {
            /* recv and decode info about broadcast line by line */
            post("mp3amp~: connecting to stream...");
            i = ret;
            /* check what we got */

            if( cpoint = strstr(request, "icy-name:"))
            {
                x->x_bcname = strdup( cpoint + 10);
                for ( i=0; i<(int)strlen(x->x_bcname); i++ )
                {
                    if ( x->x_bcname[i] == '\n' )
                    {
                        x->x_bcname[i] = '\0';
                        break;
                    }
                }
                post("           name: %s", x->x_bcname);
            }
            if( cpoint = strstr(request, "x-audiocast-name:"))
            {
                x->x_bcname = strdup( cpoint + 18);
                for ( i=0; i<(int)strlen(x->x_bcname); i++ )
                {
                    if ( x->x_bcname[i] == '\n' )
                    {
                        x->x_bcname[i] = '\0';
                        break;
                    }
                }
                post("           name: %s", x->x_bcname);
            }
            if( cpoint = strstr(request, "icy-genre:"))
            {
                x->x_bcgenre = strdup( cpoint + 10);
                for ( i=0; i<(int)strlen(x->x_bcgenre); i++ )
                {
                    if ( x->x_bcgenre[i] == '\n' )
                    {
                        x->x_bcgenre[i] = '\0';
                        break;
                    }
                }
                post("           name: %s", x->x_bcname);
            }
            if( cpoint = strstr(request, "icy-aim:"))
            {
                x->x_bcaim = strdup( cpoint + 8);
                for ( i=0; i<(int)strlen(x->x_bcaim); i++ )
                {
                    if ( x->x_bcaim[i] == '\n' )
                    {
                        x->x_bcaim[i] = '\0';
                        break;
                    }
                }
                post("           name: %s", x->x_bcname);
            }
            if( cpoint = strstr(request, "icy-url:"))
            {
                x->x_bcurl = strdup( cpoint + 8);
                for ( i=0; i<(int)strlen(x->x_bcurl); i++ )
                {
                    if ( x->x_bcurl[i] == '\n' )
                    {
                        x->x_bcurl[i] = '\0';
                        break;
                    }
                }
                post("           name: %s", x->x_bcname);
            }
            if(strstr(request, "icy-pub:1"))
            {
                post("           broadcast is public");
            }
            else if(strstr(request, "icy-pub:0"))
            {
                post("           broadcast is NOT public");
            }
            if( cpoint = strstr(request, "icy-br:"))
            {
                sptr = strdup( cpoint + 7);
                if(!strncmp(sptr, "320", 3))x->x_bitrate = 320;
                else if(!strncmp(sptr, "256", 3))x->x_bitrate = 256;
                else if(!strncmp(sptr, "224", 3))x->x_bitrate = 224;
                else if(!strncmp(sptr, "192", 3))x->x_bitrate = 192;
                else if(!strncmp(sptr, "160", 3))x->x_bitrate = 160;
                else if(!strncmp(sptr, "144", 3))x->x_bitrate = 144;
                else if(!strncmp(sptr, "128", 3))x->x_bitrate = 128;
                else if(!strncmp(sptr, "112", 3))x->x_bitrate = 112;
                else if(!strncmp(sptr, "96", 2))x->x_bitrate = 96;
                else if(!strncmp(sptr, "80", 2))x->x_bitrate = 80;
                else if(!strncmp(sptr, "64", 2))x->x_bitrate = 64;
                else if(!strncmp(sptr, "56", 2))x->x_bitrate = 56;
                else if(!strncmp(sptr, "48", 2))x->x_bitrate = 48;
                else if(!strncmp(sptr, "40", 2))x->x_bitrate = 40;
                else if(!strncmp(sptr, "32", 2))x->x_bitrate = 32;
                else if(!strncmp(sptr, "24", 2))x->x_bitrate = 24;
                else if(!strncmp(sptr, "16", 2))x->x_bitrate = 16;
                else if(!strncmp(sptr, "8", 2))x->x_bitrate = 8;
                else
                {
                    post("mp3amp~: unsupported bitrate! (%s)", sptr);
                    return NULL;
                }
                post("           bitrate: %d", x->x_bitrate);
            }
            if(strstr(request, "x-audiocast-udpport:"))
            {
                post("mp3amp~: sorry, server wants UDP connection!");
                return NULL;
            }
        }
        else
        {
            post("mp3amp~: unknown response from server");
            return NULL;
        }
        relocate = FALSE;
    }
    if (relocate)
    {
        error("mp3amp~: too many HTTP relocations");
        return NULL;
    }
    post("mp3amp~: connected to http://%s:%d/%s", hp->h_name, portno, x->x_mountpoint);
    x->x_fd = sockfd;
    if ( x->x_graphic && glist_isvisible( x->x_canvas ) )
    {
        t_int width;

        width = rtext_width( glist_findrtext( (t_glist*)x->x_canvas, (t_text *)x ) );
        SYS_VGUI3(".x%lx.c delete rectangle %xPBAR\n", x->x_canvas, x );
        SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill lightblue -tags %xPBAR\n",
                  x->x_canvas, x->x_obj.te_xpix, x->x_obj.te_ypix-BARHEIGHT-1,
                  x->x_obj.te_xpix + width, x->x_obj.te_ypix - 1, x );
    }

    return NULL;
}

/* launch the connection thread */
static void mp3amp_connect(t_mp3amp *x, t_symbol *hostname, t_symbol *mountpoint, t_floatarg fportno )
{
    pthread_attr_t update_child_attr;
    pthread_t connectchild;

    // store data
    x->x_hostname = (char*) getbytes( strlen( hostname->s_name ) + 1 ); // there's a memory leak here
    strcpy( x->x_hostname, hostname->s_name );

    x->x_mountpoint = (char*) getbytes( strlen( mountpoint->s_name ) + 1 ); // there's a memory leak here
    strcpy( x->x_mountpoint, mountpoint->s_name );

    x->x_port = fportno;

    // launch connection thread
    if ( pthread_attr_init( &update_child_attr ) < 0 )
    {
        post( "mp3amp~ : could not launch connection thread" );
        perror( "pthread_attr_init" );
        return;
    }
    if ( pthread_attr_setdetachstate( &update_child_attr, PTHREAD_CREATE_DETACHED ) < 0 )
    {
        post( "mp3amp~ : could not launch connection thread" );
        perror( "pthread_attr_setdetachstate" );
        return;
    }
    if ( pthread_create( &connectchild, &update_child_attr, mp3amp_do_connect, x ) < 0 )
    {
        post( "mp3amp~ : could not launch connection thread" );
        perror( "pthread_create" );
        return;
    }
    else
    {
        // post( "cooled~ : drawing thread %d launched", (int)x->x_updatechild );
    }

    if ( !x->x_nooutput ) outlet_float(x->x_connection, 1);
}

/* connect using url like "http://localhost:8000/mountpoint" */
static void mp3amp_connect_url(t_mp3amp *x, t_symbol *url)
{
    char *hostptr = NULL, *p, *endhost = NULL, *hostname = NULL;
    char *pathptr = NULL;
    t_int portno = 8000;

    post( "mp3amp~ : connect url : %s", url->s_name );

    /* strip http:// or ftp:// */
    p = url->s_name;
    if (strncmp(p, "http://", 7) == 0)
        p += 7;

    if (strncmp(p, "ftp://", 6) == 0)
        p += 6;

    hostptr = p;
    while (*p && *p != '/' && *p != ':')    /* look for end of hostname: */
        p++;

    endhost = p;
    switch ( *p )
    {
    case ':' :
        portno = atoi( p+1 );
        while (*p && *p != '/') p++;
        pathptr = p+1;
        break;
    case '/' :
        portno = 8000;
        pathptr = p+1;
        break;
    default :
        if ( ( p - url->s_name ) != (int)strlen( url->s_name ) )
        {
            post( "mp3amp~ : wrong url : %s", hostptr );
            return;
        }
        pathptr = "";
        break;
    }

    hostname=(char*)getbytes( (int)(endhost - hostptr) + 1);
    strncpy( hostname, hostptr, (int)(endhost - hostptr) );
    hostname[ endhost - hostptr ] = '\0';

    post ("mp3amp~ : connecting to host=%s port=%d path=%s", hostname, portno, pathptr );

    /* call the 'normal' connection routine */
    mp3amp_connect(x, gensym(hostname), gensym(pathptr), portno);
}

/* close connection to SHOUTcast server */
static void mp3amp_disconnect(t_mp3amp *x)
{
    x->x_stream = 0;
    x->x_inframes = 0;
    x->x_dframes = 0;
    x->x_inwriteposition = 0;
    x->x_offset = 0;
    if(x->x_fd >= 0)            /* close socket */
    {
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
    }
    ExitMP3(&mps[x->x_instance]);
    InitMP3(&mps[x->x_instance]);
    if ( x->x_graphic )
    {
        SYS_VGUI3(".x%lx.c delete rectangle %xPBAR\n", x->x_canvas, x );
        SYS_VGUI3(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x );
    }
    post("mp3amp~: connection closed");
    outlet_float(x->x_connection, 0);
}

static void mp3amp_free(t_mp3amp *x)
{
    if (x->x_fd > 0)
    {
        post( "mp3amp~: closing socket" );
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
    }
    freebytes(x->x_inbuffer, INPUT_BUFFER_SIZE);
    freebytes(x->x_outbuffer, OUTPUT_BUFFER_SIZE*sizeof(t_float));
}

static void *mp3amp_new(t_floatarg fdographics)
{
    t_mp3amp *x = NULL;

    if ( ((int)fdographics != 0) && ((int)fdographics != 1.) )
    {
        post( "mp3amp~: error : constructor : mp3amp~ [graphic flag = 0 | 1 ] ( got = %f)", fdographics );
        return NULL;
    }

    x = (t_mp3amp *)pd_new(mp3amp_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_connection = outlet_new(&x->x_obj, gensym("float"));

    if ( nbinstances < MAX_DECODERS )
    {
        x->x_instance = nbinstances++;
    }
    else
    {
        post( "mp3amp~: cannot create more decoders (memory issues), sorry" );
        return NULL;
    }

    x->x_fd = -1;
    x->x_stream = 0;
    x->x_inframes = 0;
    x->x_samplerate = sys_getsr();
    x->x_nbwaitloops = 1;
    x->x_nbloops = 0;
    x->x_dsp = 0;

    x->x_inbuffersize = INPUT_BUFFER_SIZE;
    x->x_outbuffersize = OUTPUT_BUFFER_SIZE;
    x->x_inbuffer = (unsigned char*) getbytes(INPUT_BUFFER_SIZE);
    x->x_offset = 0;
    x->x_outbuffer = (t_float*) getbytes(OUTPUT_BUFFER_SIZE*sizeof(t_float));

    if ( !x->x_inbuffer || !x->x_outbuffer )
    {
        post( "mp3amp~: could not allocate buffers" );
        return NULL;
    }
    memset( x->x_inbuffer, 0x0, INPUT_BUFFER_SIZE );
    memset( x->x_outbuffer, 0x0, OUTPUT_BUFFER_SIZE );

    x->x_inwriteposition = 0;
    x->x_outreadposition = 0;
    x->x_outwriteposition = 0;
    x->x_outunread = 0;
    x->x_standby = 0;
    x->x_resample = -1;
    x->x_nooutput = 0;

    ztout.tv_sec = 0;
    ztout.tv_usec = 0;

    x->x_graphic = (int)fdographics;
    post( "mp3amp~: getting canvas" );
    x->x_canvas = canvas_getcurrent();

    post( "mp3amp~: initializing decoder..." );
    /* init mpg123 decoder */
    mp3amp_tilde_mpglib_init(x);

    logpost(NULL, 4, mp3amp_version);

    return (x);
}


void mp3amp_tilde_setup(void)
{
    mp3amp_class = class_new(gensym("mp3amp~"),
                             (t_newmethod) mp3amp_new, (t_method) mp3amp_free,
                             sizeof(t_mp3amp), 0, A_DEFFLOAT, A_NULL);

    class_addmethod(mp3amp_class, nullfn, gensym("signal"), 0);
    class_addmethod(mp3amp_class, (t_method)mp3amp_dsp, gensym("dsp"), 0);
    class_addmethod(mp3amp_class, (t_method)mp3amp_connect, gensym("connect"), A_SYMBOL, A_SYMBOL, A_FLOAT, 0);
    class_addmethod(mp3amp_class, (t_method)mp3amp_connect_url, gensym("connecturl"), A_SYMBOL, 0);
    class_addmethod(mp3amp_class, (t_method)mp3amp_standby, gensym("standby"), A_DEFFLOAT, 0);
    class_addmethod(mp3amp_class, (t_method)mp3amp_disconnect, gensym("disconnect"), 0);
}
