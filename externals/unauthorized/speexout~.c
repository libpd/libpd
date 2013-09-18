/* ------------------------ speexout~ ----------------------------------------- */
/*                                                                              */
/* Tilde object to send speex encoded data to a peer using speexin~.            */
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
/* Uses the Speex codec which can                                               */
/* be found at http://speex.sourceforge.net                                     */
/*                                                                              */
/* "Western values mean nothing to us"                                          */
/* "She is beyond good and evil"                                                */
/* Pop Group --                                                                 */
/* ---------------------------------------------------------------------------- */



#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <winsock.h>
#include <windef.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#define SOCKET_ERROR -1
#endif

#if defined(__APPLE__) || defined(_WIN32)
#define MSG_NOSIGNAL 0
#endif

#include <speex/speex.h>        /* speex codec stuff */
#include <speex/speex_bits.h>   /* speex codec stuff */

#include "m_pd.h"            /* standard pd stuff */

#define     IN_BUFFER_SIZE        65536
#define     OUT_BUFFER_SIZE       8192

// #define     DATADEBUG

#define     SPEEX_NB_MODE         0 /* audio data must be 8kHz */
#define     SPEEX_WB_MODE         1 /* audio data must be 16kHz */

#define     SPEEX_DEFAULT_QUALITY 5 /* default quality          */

static char   *speexout_version = "speexout~: speex voice quality streamer version 0.2, written by ydegoyon@free.fr";

static t_class *speexout_class;

typedef struct _speexout
{
    t_object x_obj;
    int x_samplerate;         /* pd sampling rate          */

    /* Speex stuff */
    SpeexBits x_bits;         /* bits packing structure    */
    void *x_encstate;         /* encoder state             */
    t_int x_framesize;        /* frame size                */
    t_int x_mode;             /* Narrow or Wide Band       */
    int x_quality;            /* encoding quality ( 0 to 10 ) */

    /* buffer stuff */
    unsigned short x_inp;     /* in position for buffer */
    unsigned short x_outp;    /* out position for buffer */
    t_int x_encsize;          /* size of encoded data */
    t_float *x_inbuf;         /* data to be coded by Speex */
    char  *x_outbuf;          /* data returned by Speex -> our speex stream */
    int x_bytesbuffered;      /* number of unprocessed bytes in buffer */
    int x_bytesemitted;       /* number of encoded bytes emitted       */
    int x_start;
    t_float *x_encchunk;

    /* connection data        */
    int x_fd;                 /* info about connection status */
    int x_outpackets;         /* speex packets sent           */

    t_float x_f;              /* float needed for signal input */

} t_speexout;


/* encode PCM data to speex frames */
static void speexout_encode(t_speexout *x)
{
    if ( x->x_bytesbuffered > x->x_framesize )
    {
        speex_bits_reset(&x->x_bits);

        {
            t_int sp=0, rp=0;

            while( sp < x->x_framesize )
            {
                rp=(x->x_outp+sp)%IN_BUFFER_SIZE;
                // post( "speexout~ : sp=%d : rp=%d", sp, rp );
                x->x_encchunk[ sp++ ] = *(x->x_inbuf+rp);
            }
            speex_encode(x->x_encstate, x->x_encchunk, &x->x_bits);
        }

        x->x_outp = (x->x_outp+x->x_framesize)%IN_BUFFER_SIZE;
        x->x_bytesbuffered -= x->x_framesize;
        x->x_encsize = speex_bits_write(&x->x_bits, x->x_outbuf+1, OUT_BUFFER_SIZE );
        if ( x->x_encsize < 127 )
        {
            *(x->x_outbuf) = (char)x->x_encsize;
        }
        else
        {
            post( "speexout~ : encoding error : frame is more than 127 bytes" );
            x->x_encsize = -1;
        }
        x->x_bytesemitted += x->x_encsize;
#ifdef DATADEBUG
        {
            t_int si;

            printf( "speexout~ : encoded :  " );
            for ( si=0; si<x->x_encsize; si++ )
            {
                printf( "%d ", *(x->x_outbuf+si) );
            }
            printf( "\n" );
        }
#endif
    }
    else
    {
        x->x_encsize = -1;
    }
}

/* stream data to the peer */
static void speexout_stream(t_speexout *x)
{
    int count = -1, i;

    if ( x->x_encsize > 0 )
    {
        count = send(x->x_fd, x->x_outbuf, x->x_encsize+1, MSG_NOSIGNAL);
        if(count < 0)
        {
            error("speexout~: could not send encoded data to the peer (%d)", count);
#ifdef _WIN32
            closesocket(x->x_fd);
#else
            close(x->x_fd);
#endif
            x->x_fd = -1;
            outlet_float(x->x_obj.ob_outlet, 0);
        }
        else
        {
            x->x_outpackets++;
            // post( "speexout~ : emitted %d bytes (packets = %d)", count, x->x_outpackets );
            if ( x->x_outpackets%100 == 0 )
            {
                // post( "speexout~ : emitted %d bytes (packets = %d)", x->x_bytesemitted, x->x_outpackets );
            }
            if(count != x->x_encsize+1)
            {
                error("speexout~: %d bytes skipped", x->x_encsize - count);
            }
        }
        x->x_encsize = -1;
    }
}


/* buffer and downsample the data */
static t_int *speexout_perform(t_int *w)
{
    t_float *in   = (t_float *)(w[1]);       /* audio inlet */
    t_speexout *x = (t_speexout *)(w[2]);
    int n = (int)(w[3]);                      /* number of samples */
    unsigned short i,wp;
    t_float accum = 0.;
    int sratio;

    /* samplerate is supposed to be > 16kHz, thus sratio > 1 */
    if ( x->x_mode == SPEEX_NB_MODE )
    {
        sratio = x->x_samplerate / 8000;
    }
    else
    {
        sratio = x->x_samplerate / 16000;
    }

    /* copy the data into the buffer and resample audio data */

    accum=0;
    for(wp = 0; wp < n; wp++)
    {
        accum += *(in+wp);
        if ( wp % sratio == sratio - 1 )
        {
            x->x_inbuf[x->x_inp] = ( accum / sratio ) * 8000; // scale the input for speex best efficiency
            // post( "x->x_inp : %d", x->x_inp );
            x->x_inp = (x->x_inp+1)%IN_BUFFER_SIZE;
            x->x_bytesbuffered ++;
            accum = 0;
        }
    }

    if( ( x->x_fd >= 0 ) && ( x->x_bytesbuffered > x->x_framesize ) )
    {
        /* encode and send to the peer */
        speexout_encode(x);        /* speex encoding         */
        speexout_stream(x);        /* stream mp3 to the peer */
    }
    else
    {
        x->x_start = -1;
    }
    return (w+4);
}

static void speexout_dsp(t_speexout *x, t_signal **sp)
{
    dsp_add(speexout_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

/* initialize the speex library */
static void speexout_tilde_speex_init(t_speexout *x)
{

    speex_bits_init(&x->x_bits);

    switch ( x->x_mode )
    {
    case SPEEX_NB_MODE :
        x->x_encstate = speex_encoder_init(&speex_nb_mode);
        break;

    case SPEEX_WB_MODE :
        x->x_encstate = speex_encoder_init(&speex_wb_mode);
        break;

    default :
        error( "speexout~ : severe error : encoding scheme is unknown" );
        break;
    }

    speex_encoder_ctl(x->x_encstate, SPEEX_GET_FRAME_SIZE, (void*)&x->x_framesize);
    post( "speexout~ : frame size : %d", x->x_framesize );

}

/* connect to the peer         */
static void speexout_connect(t_speexout *x, t_symbol *hostname, t_floatarg fportno)
{
    struct          sockaddr_in csocket;
    struct          hostent *hp;
    int             portno            = fportno;    /* get port from message box */

    /* variables used for communication with the peer */
    const char      *buf = 0;
    unsigned int    len;
    int    sockfd;

#ifdef _WIN32
    unsigned int    ret;
#else
    int    ret;
#endif

    if (x->x_fd >= 0)
    {
        error("speexout~: already connected");
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("speexout~: internal error while attempting to open socket");
        return;
    }

    /* connect socket using hostname provided in command line */
    csocket.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
        post("speexout~: bad host?");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        return;
    }
    memcpy((char *)&csocket.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    csocket.sin_port = htons((unsigned short)portno);

    /* try to connect.  */
    post("speexout~: connecting to port %d", portno);
    if (connect(sockfd, (struct sockaddr *) &csocket, sizeof (csocket)) < 0)
    {
        error("speexout~: connection failed!\n");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        return;
    }

    x->x_fd = sockfd;
    x->x_outpackets = 0;
    outlet_float( x->x_obj.ob_outlet, 1 );
    post( "speexout~ : connected to peer" );


}

/* close connection to the peer         */
static void speexout_disconnect(t_speexout *x)
{

    int err = -1;

    if(x->x_fd >= 0)            /* close socket */
    {
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif
        x->x_fd = -1;
        outlet_float( x->x_obj.ob_outlet, 0 );
        post("speexout~: connection closed");
    }
}

/* settings for encoding quality */
static void speexout_quality(t_speexout *x, t_floatarg fquality )
{
    if ( fquality < 0 || fquality > 10 )
    {
        post( "speexout~ : wrong quality." );
        return;
    }
    x->x_quality = fquality;
    post("speexout~: setting quality to : %d", x->x_quality);
    speex_encoder_ctl(x->x_encstate, SPEEX_SET_QUALITY, &x->x_quality);
}

/* clean up */
static void speexout_free(t_speexout *x)
{

    speex_bits_destroy(&x->x_bits);

    speex_encoder_destroy(x->x_encstate);

    post("speexout~: encoder destroyed");

    if(x->x_fd >= 0)
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif
    freebytes(x->x_inbuf, IN_BUFFER_SIZE*sizeof(t_float));
    freebytes(x->x_outbuf, OUT_BUFFER_SIZE);
    freebytes(x->x_encchunk, x->x_framesize*sizeof(t_float));
}

static void *speexout_new(t_symbol *s, int argc, t_atom *argv)
{
    t_speexout *x = (t_speexout *)pd_new(speexout_class);
    outlet_new( &x->x_obj, &s_float );

    x->x_mode = SPEEX_NB_MODE;
    x->x_quality = SPEEX_DEFAULT_QUALITY;
    x->x_fd = -1;
    x->x_outpackets = 0;
    x->x_samplerate = sys_getsr();
    x->x_inbuf = getbytes(IN_BUFFER_SIZE*sizeof(t_float));  /* buffer for encoder input */
    x->x_outbuf = getbytes(OUT_BUFFER_SIZE);              /* our mp3 stream */
    if ((!x->x_inbuf)||(!x->x_outbuf)) /* check buffers... */
    {
        error("speexout~ : cannot allocate buffers");
        return NULL;
    }
    x->x_bytesbuffered = 0;
    x->x_bytesemitted = 0;
    x->x_inp = 0;
    x->x_outp = 0;
    x->x_encsize = 0;
    x->x_start = -1;
    speexout_tilde_speex_init(x);

    x->x_encchunk = (t_float*)getbytes(x->x_framesize*sizeof(t_float));
    if (!x->x_encchunk) /* check allocation... */
    {
        error("speexout~ : cannot allocate chunk");
        return NULL;
    }
    return(x);
}

void speexout_tilde_setup(void)
{
    logpost(NULL, 4, speexout_version);
    speexout_class = class_new(gensym("speexout~"), (t_newmethod)speexout_new, (t_method)speexout_free,
                               sizeof(t_speexout), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(speexout_class, t_speexout, x_f );
    class_addmethod(speexout_class, (t_method)speexout_dsp, gensym("dsp"), 0);
    class_addmethod(speexout_class, (t_method)speexout_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(speexout_class, (t_method)speexout_disconnect, gensym("disconnect"), 0);
    class_addmethod(speexout_class, (t_method)speexout_quality, gensym("quality"), A_FLOAT, 0);
}

