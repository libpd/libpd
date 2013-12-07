/* -------------------------- mp3cast~ ---------------------------------------- */
/*                                                                              */
/* Tilde object to send mp3-stream to shoutcast/icecast server.                 */
/* Written by Olaf Matthes (olaf.matthes@gmx.de).                               */
/* Get source at http://www.akustische-kunst.de/puredata/                       */
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
/* ---------------------------------------------------------------------------- */



#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include "m_pd.h"            /* standard pd stuff */

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

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

#include <lame/lame.h>        /* lame encoder stuff */
#include "mpg123.h"


#define        MY_MP3_MALLOC_IN_SIZE        65536
/* max size taken from lame readme */
#define        MY_MP3_MALLOC_OUT_SIZE       1.25*MY_MP3_MALLOC_IN_SIZE+7200

#define        MAXDATARATE 320        /* maximum mp3 data rate is 320kbit/s */
#define        STRBUF_SIZE 1024

static char   *mp3cast_version = "mp3cast~: mp3 streamer version 0.5, written by Yves Degoyon";

static char base64table[65] =
{
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
};

static t_class *mp3cast_class;

typedef struct _mp3cast
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

    /* SHOUTcast server stuff */
    int x_fd;                 /* info about connection status */
    char* x_passwd;           /* password for server */
    int x_icecast;            /* tells if we use a IceCast server or SHOUTcast */
    /* special IceCast server stuff */
    char* x_mountpoint;
    char* x_name;
    char* x_url;
    char* x_genre;
    char* x_description;
    int x_isPublic;

    t_float x_f;              /* float needed for signal input */

    lame_global_flags *lgfp;  /* lame encoder configuration */

} t_mp3cast;


/* encode PCM data to mp3 stream */
static void mp3cast_encode(t_mp3cast *x)
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
        post("mp3cast~: initialising buffers");
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
    if((unsigned short)(x->x_outp - x->x_inp) < x->x_lamechunk)error("mp3cast~: buffers overlap!");

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
    // post( "mp3cast~ : encoding returned %d frames", x->x_mp3size );

    /* check result */
    if(x->x_mp3size<0)
    {
        lame_close( x->lgfp );
        error("mp3cast~: lame_encode_buffer_interleaved failed (%d)", x->x_mp3size);
        x->x_lame = -1;
    }
}


/* stream mp3 to SHOUTcast server */
static void mp3cast_stream(t_mp3cast *x)
{
    int err = -1, i;            /* error return code */
    struct frame hframe;

    err = send(x->x_fd, x->x_mp3outbuf, x->x_mp3size, 0);
    if(err < 0)
    {
        error("mp3cast~: could not send encoded data to server (%d)", err);
        lame_close( x->lgfp );
        x->x_lame = -1;
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif /* _WIN32 */
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
    }
    if((err > 0)&&(err != x->x_mp3size))error("mp3cast~: %d bytes skipped", x->x_mp3size - err);
}


/* buffer data as channel interleaved PCM */
static t_int *mp3cast_perform(t_int *w)
{
    t_float *in1   = (t_float *)(w[1]);       /* left audio inlet */
    t_float *in2   = (t_float *)(w[2]);       /* right audio inlet */
    t_mp3cast *x = (t_mp3cast *)(w[3]);
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

        /* encode and send to server */
        if(x->x_bytesbuffered > x->x_lamechunk)
        {
            mp3cast_encode(x);        /* encode to mp3 */
            mp3cast_stream(x);        /* stream mp3 to server */
            x->x_bytesbuffered -= x->x_lamechunk;
        }
    }
    else
    {
        x->x_start = -1;
    }
    return (w+5);
}

static void mp3cast_dsp(t_mp3cast *x, t_signal **sp)
{
    dsp_add(mp3cast_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

/* initialize the lame library */
static void mp3cast_tilde_lame_init(t_mp3cast *x)
{
    int    ret;
    x->lgfp = lame_init(); /* set default parameters for now */

#ifdef _WIN32
    /* load lame_enc.dll library */
    HINSTANCE dll;
    dll=LoadLibrary("lame_enc.dll");
    if(!dll)
    {
        error("mp3cast~: error loading lame_enc.dll");
        closesocket(x->x_fd);
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
        post("mp3cast~: connection closed");
        return;
    }
#endif  /* _WIN32 */
    {
        const char *lameVersion = get_lame_version();
        logpost(NULL, 4,  "mp3cast~ : using lame version : %s", lameVersion );
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
    //lame_set_padding_type( x->lgfp, PAD_NO ); /* deprecated function */
    ret = lame_init_params( x->lgfp );
    if ( ret<0 )
    {
        post( "mp3cast~ : error : lame params initialization returned : %d", ret );
    }
    else
    {
        x->x_lame=1;
        /* magic formula copied from windows dll for MPEG-I */
        x->x_lamechunk = 2*1152;

        post( "mp3cast~ : lame initialization done. (%d)", x->x_lame );
    }
    lame_init_bitstream( x->lgfp );
}

char *mp3cast_base64_encode(char *data)
{
    int len = strlen(data);
    char *out = t_getbytes(len*4/3 + 4);
    char *result = out;
    int chunk;

    while(len > 0)
    {
        chunk = (len >3)?3:len;
        *out++ = base64table[(*data & 0xFC)>>2];
        *out++ = base64table[((*data & 0x03)<<4) | ((*(data+1) & 0xF0) >> 4)];

        switch(chunk)
        {
        case 3:
            *out++ = base64table[((*(data+1) & 0x0F)<<2) | ((*(data+2) & 0xC0)>>6)];
            *out++ = base64table[(*(data+2)) & 0x3F];
            break;
        case 2:
            *out++ = base64table[((*(data+1) & 0x0F)<<2)];
            *out++ = '=';
            break;
        case 1:
            *out++ = '=';
            *out++ = '=';
            break;
        }
        data += chunk;
        len -= chunk;
    }
    *out = 0;

    return result;
}

/* connect to server */
static void mp3cast_connect(t_mp3cast *x, t_symbol *hostname, t_floatarg fportno)
{
    struct          sockaddr_in server;
    struct          hostent *hp;
    int             portno            = fportno;    /* get port from message box */

    /* information about this broadcast to be send to the server */
    const char     *name            = x->x_name;                     /* name of broadcast */
    const char     *url             = x->x_url;                      /* url of broadcast */
    const char     *genre           = x->x_genre;                    /* genre of broadcast */
    const char     *description     = x->x_description;              /* description of broadcast */
    const char     *aim             = "N/A";                         /* aim of broadcast */
    const char     *irc             = "#mp3cast";                    /* ??? what's this ??? */
    const char     *icq             = "";                            /* icq id of broadcaster */
    const char     *mountpoint      = x->x_mountpoint;               /* mountpoint for IceCast server */
    int            isPublic        = x->x_isPublic;                /* don't publish broadcast on www.shoutcast.com */

    /* variables used for communication with server */
    const char      * buf = 0;
    char            resp[STRBUF_SIZE];
    unsigned int    len;
    fd_set          fdset;
    struct timeval  tv;
    int    sockfd;
    int    ret;

    if(x->x_icecast == 0)portno++;    /* use SHOUTcast, portno is one higher */

    if (x->x_fd >= 0)
    {
        error("mp3cast~: already connected");
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("mp3cast~: internal error while attempting to open socket");
        return;
    }

    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
        post("mp3cast~: bad host?");
        close(sockfd);
        return;
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((unsigned short)portno);

    /* try to connect.  */
    post("mp3cast~: connecting to port %d", portno);
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        error("mp3cast~: connection failed!\n");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif /* _WIN32 */
        return;
    }

    /* sheck if we can read/write from/to the socket */
    FD_ZERO( &fdset);
    FD_SET( sockfd, &fdset);
    tv.tv_sec  = 0;            /* seconds */
    tv.tv_usec = 500;        /* microseconds */

    ret = select(sockfd + 1, &fdset, NULL, NULL, &tv);
    if(ret < 0)
    {
        error("mp3cast~: can not read from socket");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif /* _WIN32 */
        return;
    }
#ifndef _WIN32
    ret = select(sockfd + 1, NULL, &fdset, NULL, &tv);
    if(ret < 0)
    {
        error("mp3cast~: can not write to socket");
        close(sockfd);
        return;
    }
#endif /* not _WIN32 */

    if(x->x_icecast == 0) /* SHOUTCAST */
    {
        /* now try to log in at SHOUTcast server */
        post("mp3cast~: logging in to SHOUTcast server...");

        /* first line is the passwd */
        buf = x->x_passwd;
        send(sockfd, buf, strlen(buf), 0);
        buf = "\n";
        send(sockfd, buf, strlen(buf), 0);

        /* header for SHOUTcast server */
        buf = "icy-name:";                        /* name of broadcast */
        send(sockfd, buf, strlen(buf), 0);
        buf = name;
        send(sockfd, buf, strlen(buf), 0);
        buf = "\nicy-url:";                        /* URL of broadcast */
        send(sockfd, buf, strlen(buf), 0);
        buf = url;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nicy-genre:";                    /* genre of broadcast */
        send(sockfd, buf, strlen(buf), 0);
        buf = genre;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nicy-description:";                    /* description of broadcast */
        send(sockfd, buf, strlen(buf), 0);
        buf = description;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nicy-irc:";
        send(sockfd, buf, strlen(buf), 0);
        buf = irc;
        send(sockfd, buf, strlen(buf), 0);
        buf = "\nicy-aim:";
        send(sockfd, buf, strlen(buf), 0);
        buf = aim;
        send(sockfd, buf, strlen(buf), 0);
        buf = "\nicy-icq:";
        send(sockfd, buf, strlen(buf), 0);
        buf = icq;
        send(sockfd, buf, strlen(buf), 0);
        buf = "\nicy-br:";
        send(sockfd, buf, strlen(buf), 0);
        if(sprintf(resp, "%d", x->x_bitrate) == -1)    /* convert int to a string */
        {
            error("mp3cast~: wrong bitrate");
        }
        send(sockfd, resp, strlen(resp), 0);
        buf = "\nicy-pub:";
        send(sockfd, buf, strlen(buf), 0);
        if(isPublic==0)                            /* set the public flag for broadcast */
        {
            buf = "no";
        }
        else
        {
            buf ="yes";
        }
        send(sockfd, buf, strlen(buf), 0);
        buf = "\n\n";
        send(sockfd, buf, strlen(buf), 0);
    }
    else if ( x->x_icecast == 1 )  /* IceCast */
    {
        /* now try to log in at IceCast server */
        post("mp3cast~: logging in to IceCast server...");

        /* send the request, a string like:
        * "SOURCE <password> /<mountpoint>\n" */
        buf = "SOURCE ";
        send(sockfd, buf, strlen(buf), 0);
        buf = x->x_passwd;
        send(sockfd, buf, strlen(buf), 0);
        buf = " /";
        send(sockfd, buf, strlen(buf), 0);
        buf = mountpoint;
        send(sockfd, buf, strlen(buf), 0);

        /* send the x-audiocast headers */
        buf = "\nx-audiocast-bitrate: ";
        send(sockfd, buf, strlen(buf), 0);
        if(sprintf(resp, "%d", x->x_bitrate) == -1)    /* convert int to a string */
        {
            error("mp3cast~: wrong bitrate");
        }
        send(sockfd, resp, strlen(resp), 0);

        buf = "\nx-audiocast-public: ";
        send(sockfd, buf, strlen(buf), 0);
        if(isPublic==0)                            /* set the public flag for broadcast */
        {
            buf = "no";
        }
        else
        {
            buf ="yes";
        }
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nx-audiocast-name: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = name;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nx-audiocast-url: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = url;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nx-audiocast-genre: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = genre;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\nx-audiocast-description: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = description;
        send(sockfd, buf, strlen(buf), 0);

        buf = "\n\n";
        send(sockfd, buf, strlen(buf), 0);
        /* end login for IceCast */
    }
    else if ( x->x_icecast == 2 ) /* Icecast 2 */
    {
        char *base64;    /* buffer to hold 64bit encoded strings */
        /* send the request, a string like: "SOURCE /<mountpoint> HTTP/1.0\r\n" */
        buf = "SOURCE /";
        send(sockfd, buf, strlen(buf), 0);
        buf = x->x_mountpoint;
        send(sockfd, buf, strlen(buf), 0);
        buf = " HTTP/1.0\r\n";
        send(sockfd, buf, strlen(buf), 0);
        /* send basic authorization as base64 encoded string */
        sprintf(resp, "source:%s", x->x_passwd);
        len = strlen(resp);
        base64 = mp3cast_base64_encode(resp);
        sprintf(resp, "Authorization: Basic %s\r\n", base64);
        send(sockfd, resp, strlen(resp), 0);
        t_freebytes(base64, len*4/3 + 4);
        /* send application name */
        buf = "User-Agent: mp3cast~";
        send(sockfd, buf, strlen(buf), 0);
        /* send content type: mpeg */
        buf = "\r\nContent-Type: audio/mpeg";
        send(sockfd, buf, strlen(buf), 0);
        /* send the ice headers */
        /* name */
        buf = "\r\nice-name: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = x->x_name;
        send(sockfd, buf, strlen(buf), 0);
        /* url */
        buf = "\r\nice-url: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = x->x_url;
        send(sockfd, buf, strlen(buf), 0);
        /* genre */
        buf = "\r\nice-genre: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = genre;
        send(sockfd, buf, strlen(buf), 0);
        /* public */
        buf = "\r\nice-public: ";
        send(sockfd, buf, strlen(buf), 0);
        if(isPublic==0)
        {
            buf = "0";
        }
        else
        {
            buf = "1";
        }
        send(sockfd, buf, strlen(buf), 0);
        /* bitrate */
        if(sprintf(resp, "\r\nice-audio-info: bitrate=%d", x->x_bitrate) == -1)
        {
            error("shoutcast~: could not create audio-info");
        }
        send(sockfd, resp, strlen(resp), 0);
        /* description */
        buf = "\r\nice-description: ";
        send(sockfd, buf, strlen(buf), 0);
        buf = description;
        send(sockfd, buf, strlen(buf), 0);
        /* end of header: write an empty line */
        buf = "\r\n\r\n";
        send(sockfd, buf, strlen(buf), 0);
    }

    /* read the anticipated response: "OK" */
    len = recv(sockfd, resp, STRBUF_SIZE, 0);
    if ( strstr( resp, "OK" ) == NULL )
    {
        post("mp3cast~: login failed!");
        if ( len>0 ) post("mp3cast~: server answered : %s", resp);
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif /* _WIN32 */
        return;
    }

    /* suck anything that the other side has to say */
    // while (len = recv(sockfd, resp, STRBUF_SIZE,0))
    // {
    //     post("mp3cast~: server answered : %s", resp);
    // }

    x->x_fd = sockfd;
    outlet_float(x->x_obj.ob_outlet, 1);
    post("mp3cast~: logged in to %s", hp->h_name);

    mp3cast_tilde_lame_init(x);

}

/* close connection to SHOUTcast server */
static void mp3cast_disconnect(t_mp3cast *x)
{
    int err = -1;
    if(x->x_lame >= 0)
    {
        /* ignore remaining bytes */
        if ( x->x_mp3size = lame_encode_flush( x->lgfp, x->x_mp3outbuf, 0) < 0 )
        {
            post( "mp3cast~ : warning : remaining encoded bytes" );
        }
        lame_close( x->lgfp );

        x->x_lame = -1;
        post("mp3cast~: encoder stream closed");
    }

    if(x->x_fd >= 0)            /* close socket */
    {
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif /* _WIN32 */
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
        post("mp3cast~: connection closed");
    }
}

/* set password for SHOUTcast server */
static void mp3cast_password(t_mp3cast *x, t_symbol *password)
{
    post("mp3cast~ : setting password to %s", password->s_name );
    x->x_passwd = password->s_name;
}

/* settings for mp3 encoding */
static void mp3cast_mpeg(t_mp3cast *x, t_floatarg fsamplerate, t_floatarg fbitrate,
                         t_floatarg fmode, t_floatarg fquality)
{
    x->x_samplerate = fsamplerate;
    if(fbitrate > MAXDATARATE)
    {
        fbitrate = MAXDATARATE;
    }
    x->x_bitrate = fbitrate;
    x->x_mp3mode = fmode;
    x->x_mp3quality = fquality;
    post("mp3cast~: setting mp3 stream to %dHz, %dkbit/s, mode %d, quality %d",
         x->x_samplerate, x->x_bitrate, x->x_mp3mode, x->x_mp3quality);
    if(x->x_fd>=0)post("mp3cast~ : reconnect to make changes take effect! ");
}

/* print settings */
static void mp3cast_print(t_mp3cast *x)
{
    const char        * buf = 0;
    logpost(NULL, 4, mp3cast_version);
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
    post("    mp3 chunk size: %d", x->x_lamechunk);
    if(x->x_samplerate!=sys_getsr())
    {
        post("    resampling from %d to %d Hz!", (int)sys_getsr(), x->x_samplerate);
    }

    if ( x->x_icecast == 0)
    {
        post("  server type is SHOUTcast");
    }
    else if ( x->x_icecast == 1 )
    {
        post("  server type is IceCast");
    }
    else if ( x->x_icecast == 2 )
    {
        post("  server type is IceCast 2");
    }
}

static void mp3cast_icecast(t_mp3cast *x)
{
    x->x_icecast = 1;
    post("mp3cast~: set server type to IceCast");
}

static void mp3cast_icecast2(t_mp3cast *x)
{
    x->x_icecast = 2;
    post("mp3cast~: set server type to IceCast 2");
}

static void mp3cast_shoutcast(t_mp3cast *x)
{
    x->x_icecast = 0;
    post("mp3cast~: set server type to SHOUTcast");
}

/* set mountpoint for IceCast server */
static void mp3cast_mountpoint(t_mp3cast *x, t_symbol *mount)
{
    x->x_mountpoint = mount->s_name;
    post("mp3cast~: mountpoint set to %s", x->x_mountpoint);
}

/* set namle for IceCast server */
static void mp3cast_name(t_mp3cast *x, t_symbol *name)
{
    x->x_name = name->s_name;
    post("mp3cast~: name set to %s", x->x_name);
}

/* set url for IceCast server */
static void mp3cast_url(t_mp3cast *x, t_symbol *url)
{
    x->x_url = url->s_name;
    post("mp3cast~: url set to %s", x->x_url);
}

/* set genre for IceCast server */
static void mp3cast_genre(t_mp3cast *x, t_symbol *genre)
{
    x->x_genre = genre->s_name;
    post("mp3cast~: genre set to %s", x->x_genre);
}

/* set isPublic for IceCast server */
static void mp3cast_isPublic(t_mp3cast *x, t_floatarg isPublic)
{
    x->x_isPublic = isPublic;
    char* isPublicStr;
    if(isPublic==0)
    {
        isPublicStr = "no";
    }
    else
    {
        isPublicStr = "yes";
    }
    post("mp3cast~: isPublic set to %s", isPublicStr);
}

/* set description for IceCast server */
static void mp3cast_description(t_mp3cast *x, t_symbol *description)
{
    x->x_description = description->s_name;
    post("mp3cast~: description set to %s", x->x_description);
}

/* clean up */
static void mp3cast_free(t_mp3cast *x)
{
    if(x->x_lame >= 0)
        lame_close( x->lgfp );
    if(x->x_fd >= 0)
#ifdef _WIN32
        closesocket(x->x_fd);
#else
        close(x->x_fd);
#endif /* _WIN32 */
    freebytes(x->x_mp3inbuf, MY_MP3_MALLOC_IN_SIZE*sizeof(short));
    freebytes(x->x_mp3outbuf, MY_MP3_MALLOC_OUT_SIZE);
    freebytes(x->x_buffer, MY_MP3_MALLOC_IN_SIZE*sizeof(short));
}

static void *mp3cast_new(void)
{
    t_mp3cast *x = (t_mp3cast *)pd_new(mp3cast_class);
    inlet_new (&x->x_obj, &x->x_obj.ob_pd, gensym ("signal"), gensym ("signal"));
    outlet_new(&x->x_obj, gensym("float"));
    x->x_fd = -1;
    x->x_lame = -1;
    x->x_passwd = "pd";
    x->x_samplerate = sys_getsr();
    x->x_bitrate = 224;
    x->x_mp3mode = 1;
    x->x_mp3quality = 5;
    x->x_mp3inbuf = getbytes(MY_MP3_MALLOC_IN_SIZE*sizeof(short));  /* buffer for encoder input */
    x->x_mp3outbuf = getbytes(MY_MP3_MALLOC_OUT_SIZE*sizeof(char)); /* our mp3 stream */
    x->x_buffer = getbytes(MY_MP3_MALLOC_IN_SIZE*sizeof(short));    /* what we get from pd, converted to PCM */
    if ((!x->x_buffer)||(!x->x_mp3inbuf)||(!x->x_mp3outbuf))        /* check buffers... */
    {
        error("out of memory!");
    }
    x->x_bytesbuffered = 0;
    x->x_inp = 0;
    x->x_outp = 0;
    x->lgfp = NULL;
    x->x_start = -1;
    x->x_icecast = 1;
    x->x_mountpoint = "puredata";
    x->x_name = "puredata";
    x->x_url = "http://puredata.info";
    x->x_genre = "experimental sound";
    x->x_isPublic = 1;
    x->x_description = "playing with my patches";
    return(x);
}

void mp3cast_tilde_setup(void)
{
    logpost(NULL, 4, mp3cast_version);
    mp3cast_class = class_new(gensym("mp3cast~"), (t_newmethod)mp3cast_new, (t_method)mp3cast_free,
                              sizeof(t_mp3cast), 0, 0);
    CLASS_MAINSIGNALIN(mp3cast_class, t_mp3cast, x_f );
    class_addmethod(mp3cast_class, (t_method)mp3cast_dsp, gensym("dsp"), 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_disconnect, gensym("disconnect"), 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_password, gensym("passwd"), A_SYMBOL, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_mpeg, gensym("mpeg"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_print, gensym("print"), 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_icecast, gensym("icecast"), 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_icecast2, gensym("icecast2"), 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_shoutcast, gensym("shoutcast"), 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_mountpoint, gensym("mountpoint"), A_SYMBOL, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_name, gensym("name"), A_SYMBOL, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_url, gensym("url"), A_SYMBOL, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_genre, gensym("genre"), A_SYMBOL, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_isPublic, gensym("isPublic"), A_FLOAT, 0);
    class_addmethod(mp3cast_class, (t_method)mp3cast_description, gensym("description"), A_SYMBOL, 0);
}

