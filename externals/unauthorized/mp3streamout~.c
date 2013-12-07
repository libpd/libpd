/* ------------------------ mp3streamout~ ------------------------------------- */
/*                                                                              */
/* Tilde object to send mp3-stream to a peer using mp3streamin~.                */
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
/* "I'd sell my soul to god"                                                    */
/* "If it could take away the pain."                                            */
/* Theo Hakola --                                                               */
/* ---------------------------------------------------------------------------- */

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

#include <lame/lame.h>        /* lame encoder stuff */

#include "m_pd.h"            /* standard pd stuff */

#include "mpg123.h"           /* sub-library MPGLIB included in lame */
/* useful debugging functions from mpglib */
extern int decode_header( struct frame* fr, unsigned long newhead );
extern void print_header_compact( struct frame* fr );
extern int head_check( unsigned long head, int check_layer );

#define        MY_MP3_MALLOC_IN_SIZE        65536
/* max size taken from lame readme */
#define        MY_MP3_MALLOC_OUT_SIZE       1.25*MY_MP3_MALLOC_IN_SIZE+7200

#define        MAXDATARATE 320        /* maximum mp3 data rate is 320kbit/s */
#define        STRBUF_SIZE 32

static char   *mp3streamout_version = "mp3streamout~: mp3 peer-to-peer streamer version 0.3, written by ydegoyon@free.fr";

static t_class *mp3streamout_class;

typedef struct _mp3streamout
{
    t_object x_obj;

    /* LAME stuff */
    int x_lame;               /* info about encoder status */
    int x_lamechunk;          /* chunk size for LAME encoder */
    int x_mp3size;            /* number of returned mp3 samples */

    /* buffer stuff */
    unsigned short x_inp;     /* in position for buffer */
    unsigned short x_outp;    /* out position for buffer*/
    short *x_mp3inbuf;        /* data to be sent to LAME */
    char *x_mp3outbuf;        /* data returned by LAME -> our mp3 stream */
    short *x_buffer;          /* data to be buffered */
    int x_bytesbuffered;      /* number of unprocessed bytes in buffer */
    int x_start;

    /* mp3 format stuff */
    int x_samplerate;
    int x_bitrate;            /* bitrate of mp3 stream */
    int x_mp3mode;            /* mode (mono, joint stereo, stereo, dual mono) */
    int x_mp3quality;         /* quality of encoding */

    /* connection data        */
    int x_fd;                 /* info about connection status */
    int x_outpackets;         /* mp3 packets sent             */

    t_float x_f;              /* float needed for signal input */

    lame_global_flags* lgfp;
} t_mp3streamout;


/* encode PCM data to mp3 stream */
static void mp3streamout_encode(t_mp3streamout *x)
{
    unsigned short i, wp;
    int err = -1;
    int n = x->x_lamechunk;

#ifdef _WIN32
    if(x->x_lamechunk < sizeof(x->x_mp3inbuf))
#else
    if(x->x_lamechunk < (int)sizeof(x->x_mp3inbuf))
#endif
    {
        error("not enough memory!");
        return;
    }

    /* on start/reconnect set outpoint that it not interferes with inpoint */
    if(x->x_start == -1)
    {
        post("mp3streamout~: initializing buffers");
        /* we try to keep 2.5 times the data the encoder needs in the buffer */
        if(x->x_inp > (2 * x->x_lamechunk))
        {
            x->x_outp = (short) x->x_inp - (2.5 * x->x_lamechunk);
        }
        else if(x->x_inp < (2 * x->x_lamechunk))
        {
            x->x_outp = (short) MY_MP3_MALLOC_IN_SIZE - (2.5 * x->x_lamechunk);
        }
        x->x_start = 1;
    }
    if((unsigned short)(x->x_outp - x->x_inp) < x->x_lamechunk)error("mp3streamout~: buffers overlap!");

    i = MY_MP3_MALLOC_IN_SIZE - x->x_outp;

    /* read from buffer */
    if(x->x_lamechunk <= i)
    {
        /* enough data until end of buffer */
        for(n = 0; n < x->x_lamechunk; n++)                                /* fill encode buffer */
        {
            x->x_mp3inbuf[n] = x->x_buffer[n + x->x_outp];
        }
        x->x_outp += x->x_lamechunk;
    }
    else                                        /* split data */
    {
        for(wp = 0; wp < i; wp++)                /* data at end of buffer */
        {
            x->x_mp3inbuf[wp] = x->x_buffer[wp + x->x_outp];
        }

        for(wp = i; wp < x->x_lamechunk; wp++)    /* write rest of data at beginning of buffer */
        {
            x->x_mp3inbuf[wp] = x->x_buffer[wp - i];
        }
        x->x_outp = x->x_lamechunk - i;
    }

    /* encode mp3 data */

    x->x_mp3size = lame_encode_buffer_interleaved(x->lgfp, x->x_mp3inbuf,
                   x->x_lamechunk/lame_get_num_channels(x->lgfp),
                   x->x_mp3outbuf, MY_MP3_MALLOC_OUT_SIZE);
    x->x_mp3size+=lame_encode_flush( x->lgfp, x->x_mp3outbuf+x->x_mp3size, MY_MP3_MALLOC_OUT_SIZE-x->x_mp3size );
    // post( "mp3streamout~ : encoding returned %d frames", x->x_mp3size );

    /* check result */
    if(x->x_mp3size<0)
    {
        lame_close( x->lgfp );
        error("mp3streamout~: lame_encode_buffer_interleaved failed (%d)", x->x_mp3size);
        x->x_lame = -1;
    }
}

/* stream mp3 to the peer */
static void mp3streamout_stream(t_mp3streamout *x)
{
    int count = -1, i;
    struct frame hframe;

    /* header needs to be included in each packet */

    for( i=0; i<x->x_mp3size; i++ )
    {
        // track valid data
        if ( head_check( *((unsigned long*)x->x_mp3outbuf+i), 3 ) )
        {
            // post( "valid header emitted @ %d (byte %d)", i, i*sizeof(unsigned long) );
        }
    }

    count = send(x->x_fd, x->x_mp3outbuf, x->x_mp3size, MSG_NOSIGNAL);
    if(count < 0)
    {
        error("mp3streamout~: could not send encoded data to the peer (%d)", count);
        lame_close( x->lgfp );
        x->x_lame = -1;
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
        if ( x->x_outpackets%100 == 0 )
        {
            // post( "mp3streamout~ : emitted %d bytes (packets = %d)", count, x->x_outpackets );
        }
    }
    if((count > 0)&&(count != x->x_mp3size))
    {
        error("mp3streamout~: %d bytes skipped", x->x_mp3size - count);
    }
}


/* buffer data as channel interleaved PCM */
static t_int *mp3streamout_perform(t_int *w)
{
    t_float *in1   = (t_float *)(w[1]);       /* left audio inlet */
    t_float *in2   = (t_float *)(w[2]);       /* right audio inlet */
    t_mp3streamout *x = (t_mp3streamout *)(w[3]);
    int n = (int)(w[4]);                      /* number of samples */
    unsigned short i,wp;
    float in;

    /* copy the data into the buffer */
    i = MY_MP3_MALLOC_IN_SIZE - x->x_inp;     /* space left at the end of buffer */

    n *= 2;                                  /* two channels go into one buffer */

    if( n <= i )
    {
        /* the place between inp and MY_MP3_MALLOC_IN_SIZE */
        /* is big enough to hold the data                  */

        for(wp = 0; wp < n; wp++)
        {
            if(wp%2)
            {
                in = *(in2++);    /* right channel / inlet */
            }
            else
            {
                in = *(in1++);    /* left channel / inlet */
            }
            if (in > 1.0)
            {
                in = 1.0;
            }
            if (in < -1.0)
            {
                in = -1.0;
            }
            x->x_buffer[wp + x->x_inp] = (short) (32767.0 * in);
        }
        x->x_inp += n;    /* n more samples written to buffer */
    }
    else
    {
        /* the place between inp and MY_MP3_MALLOC_IN_SIZE is not */
        /* big enough to hold the data                              */
        /* writing will take place in two turns, one from         */
        /* x->x_inp -> MY_MP3_MALLOC_IN_SIZE, then from 0 on      */

        for(wp = 0; wp < i; wp++)            /* fill up to end of buffer */
        {
            if(wp%2)
            {
                in = *(in2++);
            }
            else
            {
                in = *(in1++);
            }
            if (in > 1.0)
            {
                in = 1.0;
            }
            if (in < -1.0)
            {
                in = -1.0;
            }
            x->x_buffer[wp + x->x_inp] = (short) (32767.0 * in);
        }
        for(wp = i; wp < n; wp++)        /* write rest at start of buffer */
        {
            if(wp%2)
            {
                in = *(in2++);
            }
            else
            {
                in = *(in1++);
            }
            if (in > 1.0)
            {
                in = 1.0;
            }
            if (in < -1.0)
            {
                in = -1.0;
            }
            x->x_buffer[wp - i] = (short) (32767.0 * in);
        }
        x->x_inp = n - i;                /* new writeposition in buffer */
    }

    if((x->x_fd >= 0)&&(x->x_lame >= 0))
    {
        /* count buffered samples when things are running */
        x->x_bytesbuffered += n;

        /* encode and send to the peer */
        if(x->x_bytesbuffered > x->x_lamechunk)
        {
            mp3streamout_encode(x);        /* encode to mp3 */
            mp3streamout_stream(x);        /* stream mp3 to the peer */
            x->x_bytesbuffered -= x->x_lamechunk;
        }
    }
    else
    {
        x->x_start = -1;
    }
    return (w+5);
}

static void mp3streamout_dsp(t_mp3streamout *x, t_signal **sp)
{
    dsp_add(mp3streamout_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

/* initialize the lame library */
static void mp3streamout_tilde_lame_init(t_mp3streamout *x)
{
    int    ret;
    x->lgfp = lame_init(); /* set default parameters for now */

#ifdef _WIN32
    /* load lame_enc.dll library */
    HINSTANCE dll;
    dll=LoadLibrary("lame_enc.dll");
    if(dll==NULL)
    {
        error("mp3streamout~: error loading lame_enc.dll");
        closesocket(x->x_fd);
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
        post("mp3streamout~: connection closed");
        return;
    }
#endif
    {
        const char *lameVersion = get_lame_version();
        logpost(NULL, 4,  "mp3streamout~ : using lame version : %s", lameVersion );
    }

    /* setting lame parameters */
    lame_set_num_channels( x->lgfp, 2);
    lame_set_in_samplerate( x->lgfp, sys_getsr() );
    lame_set_out_samplerate( x->lgfp, x->x_samplerate );
    lame_set_brate( x->lgfp, x->x_bitrate );
    lame_set_mode( x->lgfp, x->x_mp3mode );
    lame_set_quality( x->lgfp, x->x_mp3quality );
    lame_set_emphasis( x->lgfp, 1 );
    lame_set_original( x->lgfp, 1 );
    lame_set_copyright( x->lgfp, 1 ); /* viva free music societies !!! */
    lame_set_disable_reservoir( x->lgfp, 0 );
    //lame_set_padding_type( x->lgfp, PAD_NO ); /* deprecated in LAME */
    ret = lame_init_params( x->lgfp );
    if ( ret<0 )
    {
        post( "mp3streamout~ : error : lame params initialization returned : %d", ret );
    }
    else
    {
        x->x_lame=1;
        /* magic formula copied from windows dll for MPEG-I */
        x->x_lamechunk = 2*1152;

        post( "mp3streamout~ : lame initialization done. (%d)", x->x_lame );
    }
    lame_init_bitstream( x->lgfp );
}

/* connect to the peer         */
static void mp3streamout_connect(t_mp3streamout *x, t_symbol *hostname, t_floatarg fportno)
{
    struct          sockaddr_in csocket;
    struct          hostent *hp;
    int             portno            = fportno;    /* get port from message box */

    /* variables used for communication with the peer */
    const char      *buf = 0;
    char            resp[STRBUF_SIZE];
    unsigned int    len;
    int    sockfd;

#ifdef _WIN32
    unsigned int    ret;
#else
    int    ret;
#endif

    if (x->x_fd >= 0)
    {
        error("mp3streamout~: already connected");
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("mp3streamout~: internal error while attempting to open socket");
        return;
    }

    /* connect socket using hostname provided in command line */
    csocket.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
        post("mp3streamout~: bad host?");
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
    post("mp3streamout~: connecting to port %d", portno);
    if (connect(sockfd, (struct sockaddr *) &csocket, sizeof (csocket)) < 0)
    {
        error("mp3streamout~: connection failed!\n");
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
    post( "mp3streamout~ : connected to peer" );

    mp3streamout_tilde_lame_init(x);

}

/* close connection to the peer         */
static void mp3streamout_disconnect(t_mp3streamout *x)
{

    int err = -1;
    if(x->x_lame >= 0)
    {
        /* ignore remaining bytes */
        if ( x->x_mp3size = lame_encode_flush( x->lgfp, x->x_mp3outbuf, 0) < 0 )
        {
            post( "mp3streamout~ : warning : remaining encoded bytes" );
        }
        lame_close( x->lgfp );
        x->x_lame = -1;
        post("mp3streamout~: encoder stream closed");
    }

    if(x->x_fd >= 0)            /* close socket */
    {
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif
        x->x_fd = -1;
        outlet_float( x->x_obj.ob_outlet, 0 );
        post("mp3streamout~: connection closed");
    }
}

/* settings for mp3 encoding */
static void mp3streamout_mpeg(t_mp3streamout *x, t_floatarg fbitrate,
                              t_floatarg fmode, t_floatarg fquality)
{
    if ( fbitrate != 32 && fbitrate != 40 && fbitrate != 48 && fbitrate != 56 &&
            fbitrate != 64 && fbitrate != 80 && fbitrate != 96 && fbitrate != 112 &&
            fbitrate != 128 && fbitrate != 160 && fbitrate != 192 && fbitrate != 224 &&
            fbitrate != 256 && fbitrate != 320 )
    {
        post( "mp3streamout~ : wrong bitrate." );
        return;
    }
    if ( fmode <0 || fmode>2 )
    {
        post( "mp3streamout~ : wrong mp3 mode." );
        if ( fmode == 3 )
        {
            post( "mp3streamout~ : mone is not supported by streamout~ for now." );
        }
        return;
    }
    /* there is a bug in lame 3.92 and quality below 5 will not work */
    /* WAIT FOR A FIX */
    if ( fquality <5 || fquality>9 )
    {
        post( "mp3streamout~ : wrong quality." );
        return;
    }
    x->x_bitrate = fbitrate;
    x->x_mp3mode = fmode;
    x->x_mp3quality = (int)fquality;
    post("mp3streamout~: setting mp3 stream to %dHz, %dkbit/s, mode %d, quality %d",
         x->x_samplerate, x->x_bitrate, x->x_mp3mode, x->x_mp3quality);
    mp3streamout_tilde_lame_init(x);
}

/* print settings */
static void mp3streamout_print(t_mp3streamout *x)
{
    const char        * buf = 0;

    logpost(NULL, 4, mp3streamout_version);
    post("  LAME mp3 settings:\n"
         "    output sample rate: %d Hz\n"
         "    bitrate: %d kbit/s", x->x_samplerate, x->x_bitrate);
    switch(x->x_mp3mode)
    {
    case 0 :
        buf = "stereo";
        break;
    case 1 :
        buf = "joint stereo";
        break;
    case 2 :
        buf = "dual channel";
        break;
    case 3 :
        buf = "mono";
        break;
    }
    post("    mode: %s\n"
         "    quality: %d", buf, x->x_mp3quality);
#ifdef _WIN32
    if(x->x_lamechunk!=0)post("    calculated mp3 chunk size: %d", x->x_lamechunk);
#else
    post("    mp3 chunk size: %d", x->x_lamechunk);
#endif
    if(x->x_samplerate!=sys_getsr())
    {
        post("    resampling from %d to %d Hz!", (int)sys_getsr(), x->x_samplerate);
    }
}

/* clean up */
static void mp3streamout_free(t_mp3streamout *x)
{

    if(x->x_lame >= 0)
        lame_close( x->lgfp );

    if(x->x_fd >= 0)
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif
    freebytes(x->x_mp3inbuf, MY_MP3_MALLOC_IN_SIZE*sizeof(short));
    freebytes(x->x_mp3outbuf, MY_MP3_MALLOC_OUT_SIZE);
    freebytes(x->x_buffer, MY_MP3_MALLOC_IN_SIZE*sizeof(short));
}

static void *mp3streamout_new(void)
{
    t_mp3streamout *x = (t_mp3streamout *)pd_new(mp3streamout_class);
    inlet_new (&x->x_obj, &x->x_obj.ob_pd, gensym ("signal"), gensym ("signal"));
    outlet_new( &x->x_obj, &s_float );
    x->lgfp = NULL;
    x->x_fd = -1;
    x->x_outpackets = 0;
    x->x_lame = -1;
    x->x_samplerate = sys_getsr();
    x->x_bitrate = 128;
    x->x_mp3mode = 1;
    x->x_mp3quality = 5;
    x->x_mp3inbuf = getbytes(MY_MP3_MALLOC_IN_SIZE*sizeof(short));  /* buffer for encoder input */
    x->x_mp3outbuf = getbytes(MY_MP3_MALLOC_OUT_SIZE);              /* our mp3 stream */
    x->x_buffer = getbytes(MY_MP3_MALLOC_IN_SIZE*sizeof(short));    /* what we get from pd, converted to PCM */
    if ((!x->x_buffer)||(!x->x_mp3inbuf)||(!x->x_mp3outbuf))        /* check buffers... */
    {
        error("out of memory!");
    }
    x->x_bytesbuffered = 0;
    x->x_inp = 0;
    x->x_outp = 0;
    x->x_start = -1;
    return(x);
}

void mp3streamout_tilde_setup(void)
{
    logpost(NULL, 4, mp3streamout_version);
    mp3streamout_class = class_new(gensym("mp3streamout~"), (t_newmethod)mp3streamout_new, (t_method)mp3streamout_free,
                                   sizeof(t_mp3streamout), 0, 0);
    CLASS_MAINSIGNALIN(mp3streamout_class, t_mp3streamout, x_f );
    class_addmethod(mp3streamout_class, (t_method)mp3streamout_dsp, gensym("dsp"), 0);
    class_addmethod(mp3streamout_class, (t_method)mp3streamout_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(mp3streamout_class, (t_method)mp3streamout_disconnect, gensym("disconnect"), 0);
    class_addmethod(mp3streamout_class, (t_method)mp3streamout_mpeg, gensym("mpeg"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(mp3streamout_class, (t_method)mp3streamout_print, gensym("print"), 0);
}

