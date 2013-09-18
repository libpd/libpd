/* ------------------------ mp3fileout~ --------------------------------------- */
/*                                                                              */
/* Tilde object to send an mp3 file to a peer using mp3streamin~                */
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
/* "See mass murder on a scale you've never seen"                                */
/* "And all the one who tried hard to succeed"                                   */
/* You know who, don't you ???                                                  */
/* ---------------------------------------------------------------------------- */


#include <m_pd.h>
#include <m_imp.h>
#include <s_stuff.h>
#include <g_canvas.h>

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#include <io.h>
#include <fcntl.h>
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
#endif


#include "mpg123.h"      /* mpg123 decoding library from lame 3.92 */
#include "mpglib.h"      /* mpglib decoding library from lame 3.92 */
#include "interface.h"   /* mpglib decoding library from lame 3.92 */

#define INPUT_BUFFER_SIZE 32768
#define OUTPUT_BUFFER_SIZE 32768
#define MAX_FRAME_SIZE 1152

/* useful debugging functions from mpglib */
extern int decode_header( struct frame* fr, unsigned long newhead );
extern void print_header_compact( struct frame* fr );
extern int head_check( unsigned long head, int check_layer );

/* time-out used for select() call */
static struct timeval ztout;

static char   *mp3fileout_version = "mp3fileout~: mp3 file streamer version 0.2, written by ydegoyon@free.fr";

extern void sys_sockerror(char *s);

void mp3fileout_closesocket(int fd)
{
#ifndef _MSC_VER
    if ( close(fd) < 0 )
    {
        perror( "close" );
    }
    else
    {
        post( "mp3fileout~ : closed socket : %d", fd );
    }
#endif
#ifdef _WIN32
    closesocket(fd);
#else
    sys_rmpollfn(fd);
#endif
}

/* ------------------------ mp3fileout~ ----------------------------- */

static t_class *mp3fileout_class;

typedef struct _mp3fileout
{
    t_object x_obj;
    t_int x_socket;
    t_int x_fd;         /* file descriptor for the mp3 file */
    t_int x_eof;        /* end of file is reached           */
    t_int x_emit;       /* indicates the ability to emit    */
    t_int x_nbwaitloops;/* synchronization cycles count     */
    t_int x_blocksize;  /* actual blocksize                 */

    void *x_inbuffer;   /* accumulation buffer for read mp3 frames */
    t_int x_inwriteposition;
    t_int x_inbuffersize;
    t_int x_framesize;
    t_int x_offset;     /* offset used for decoding                    */
    t_int x_nbloops;    /* number of perform loops                     */

    void *x_outbuffer;   /* buffer to be emitted */
    t_int x_outframes;   /* number of frames emitted */
    t_int x_outbuffersize;
    t_int x_outavable;   /* number of available bytes to emit */

    t_canvas *x_canvas;

    t_outlet *x_connected; /* indicates state of the connection */
    t_outlet *x_endreached;/* indicates the end of file         */
    t_outlet *x_frames;    /* indicates the number of frames emitted */

} t_mp3fileout;

static int mp3fileout_search_header(t_mp3fileout *x)
{
    t_int i;
    t_int length = 0;
    struct frame hframe;
    unsigned long cheader;
    t_int ret = sizeof( unsigned long);
    t_int foffset = 0;
    unsigned int a,b,c,d;
    unsigned char buf[sizeof(unsigned long)];
    t_float nbsamplesframe = 0;

    while( ret>0 )
    {
        ret = read( x->x_fd, (void *)buf, sizeof( unsigned long ) );

        foffset+=ret;

        if ( ret>0 )
        {
            /* check for a valid header */
            a = buf[0];
            b = buf[1];
            c = buf[2];
            d = buf[3];

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
                x->x_framesize = hframe.framesize;
                nbsamplesframe = ( hframe.mpeg25 ? 576 : 1152 );
                x->x_nbwaitloops = (int)(nbsamplesframe/x->x_blocksize);
                if ( x->x_nbwaitloops == 0 ) x->x_nbwaitloops = 1;
                // post ( "mp3fileout~ : will wait %d loops", x->x_nbwaitloops );

                // rewind file to the start of the frame
                if ( lseek( x->x_fd, -sizeof(unsigned long), SEEK_CUR ) < 0 )
                {
                    post( "mp3fileout~ : could not rewind file." );
                }
                if ( x->x_outframes == 0 )
                {
                    post( "mp3fileout~ : found firstframe @ %d", foffset );
                }
                break;
            }
            // post( "mp3fileout~ : read %d bytes.", ret );
        }
        else
        {
            if ( ret < 0 )
            {
                post( "mp3fileout~ : error encountered ( ret=%d )...file reading done.", ret );
                perror( "read" );
                x->x_eof = 1;
            }
            else
            {
                post( "mp3fileout~ : file reading done.", ret );
                x->x_eof = 1;
                outlet_bang( x->x_endreached );
            }
            return -1;
        }

    }

    return x->x_framesize;
}

static int mp3fileout_read_frame(t_mp3fileout *x)
{
    int size, ret;

    if ( x->x_fd > 0 && !x->x_eof)
    {
        if ( ( size = mp3fileout_search_header( x ) ) > 0 )
        {
            if ( size+sizeof(unsigned long) >  INPUT_BUFFER_SIZE )
            {
                post( "mp3fileout~ : cannot read frame : size too big : %d", size );
                return -1;
            }
            // post( "mp3fileout~ : reading a frame : size : %d", size );
            ret = read( x->x_fd, x->x_inbuffer, size+sizeof(unsigned long) );

            if ( ret>0 )
            {
                memcpy( x->x_outbuffer, x->x_inbuffer, ret );
                x->x_outavable += ret;
                return ret;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

static int mp3fileout_send_frame(t_mp3fileout *x)
{
    int ret=0;

    if ( x->x_socket > 0 && x->x_emit )
    {
        if ( ( ret = send( x->x_socket, x->x_outbuffer, x->x_outavable, MSG_NOSIGNAL ) ) < 0 )
        {
            post( "mp3fileout~ : connection lost." );
            perror( "send" );
            x->x_socket = -1;
            x->x_emit = 0;
            return -1;
        }
        else
        {
            memcpy( x->x_outbuffer, x->x_outbuffer+ret, x->x_outbuffersize-ret );
            x->x_outavable -= ret;
            x->x_outframes++;
            outlet_float( x->x_frames, x->x_outframes );
            // post( "mp3fileout~ : sent %d bytes, x->x_outavable : %d", ret, x->x_outavable );
        }

    }
    else
    {
        // artificially empty buffer
        x->x_outavable = 0;
    }
    return ret;
}

static void mp3fileout_free(t_mp3fileout *x)
{
    if (x->x_socket > 0)
    {
        post( "mp3fileout~ : closing socket" );
        mp3fileout_closesocket(x->x_socket);
        x->x_socket = -1;
    }
    if ( x->x_fd > 0 )
    {
        if ( close( x->x_fd ) < 0 )
        {
            post( "mp3fileout~ : could not close file." );
            perror( "close" );
        }
    }
    if ( x->x_inbuffer ) freebytes( x->x_inbuffer, x->x_inbuffersize );
}

static t_int *mp3fileout_perform(t_int *w)
{
    t_mp3fileout *x = (t_mp3fileout*) (w[1]);
    int ret;
    int i = 0;

    x->x_blocksize = (t_int)(w[2]);
    // check new incoming data
    if ( x->x_socket > 0 )
    {
        if ( x->x_nbloops % x->x_nbwaitloops == 0 )
        {
            /* read a frame in the file */
            if ( mp3fileout_read_frame(x) > 0 )
            {
                /* send the frame to the peer */
                mp3fileout_send_frame(x);
            }
        }
        x->x_nbloops = ( x->x_nbloops+1 ) % x->x_nbwaitloops;
    }
    return (w+3);
}

static void mp3fileout_dsp(t_mp3fileout *x, t_signal **sp)
{
    dsp_add(mp3fileout_perform, 2, x, sp[0]->s_n);
}

/* start streaming */
static void mp3fileout_start(t_mp3fileout *x)
{
    x->x_emit = 1;
    if ( x->x_fd > 0 )
    {
        // reset file pointer
        if ( lseek( x->x_fd, 0, SEEK_SET ) < 0 )
        {
            post ( "mp3fileout~ : could not reset file pointer.");
            x->x_eof = 1;
            return;
        }
        x->x_eof = 0;
        x->x_outframes = 0;
        outlet_float( x->x_frames, x->x_outframes );
    }
}

/* resume file reading */
static void mp3fileout_resume(t_mp3fileout *x)
{
    x->x_emit = 1;
}

/* seek in  file */
static void mp3fileout_seek(t_mp3fileout *x, t_floatarg foffset)
{
    if ( foffset < 0 )
    {
        post( "mp3fileout~ : wrong offset.");
        return;
    }
    if ( x->x_fd > 0 )
    {
        // reset file pointer
        if ( lseek( x->x_fd, (int)foffset, SEEK_SET ) < 0 )
        {
            post ( "mp3fileout~ : could not reset file pointer.");
            x->x_eof = 1;
            return;
        }
        x->x_eof = 0;
    }
}

/* stop streaming */
static void mp3fileout_stop(t_mp3fileout *x)
{
    x->x_emit = 0;
}

/* open mp3 file */
static void mp3fileout_open(t_mp3fileout *x, t_symbol *filename)
{
    // first close previous file
    if ( x->x_fd > 0 )
    {
        if ( close( x->x_fd ) < 0 )
        {
            post( "mp3fileout~ : could not close file." );
            perror( "close" );
        }
        x->x_outframes = 0;
        outlet_float( x->x_frames, x->x_outframes );
    }

    if ( ( x->x_fd = sys_open( filename->s_name, O_RDONLY ) ) < 0 )
    {
        post( "mp3fileout~ : could not open file : %s", filename->s_name );
        perror( "open" );
        x->x_eof = 1;
    }
    else
    {
        x->x_eof = 0;
        post( "mp3fileout~ : opened file : %s ( fd = %d )", filename->s_name, x->x_fd );
    }
}

/* connect to the peer         */
static void mp3fileout_connect(t_mp3fileout *x, t_symbol *hostname, t_floatarg fportno)
{
    struct          sockaddr_in csocket;
    struct          hostent *hp;
    int             portno            = fportno;    /* get port from message box */

    /* variables used for communication with the peer */
    unsigned int    len;
    int    sockfd;

#ifdef _WIN32
    unsigned int    ret;
#else
    int    ret;
#endif

    if (x->x_socket >= 0)
    {
        error("mp3fileout~: already connected");
        return;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("mp3fileout~: internal error while attempting to open socket");
        return;
    }

    /* connect socket using hostname provided in command line */
    csocket.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
        post("mp3fileout~: bad host?");
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
    post("mp3fileout~: connecting to port %d", portno);
    if (connect(sockfd, (struct sockaddr *) &csocket, sizeof (csocket)) < 0)
    {
        error("mp3fileout~: connection failed!\n");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        return;
    }

    x->x_socket = sockfd;
    x->x_outframes = 0;
    outlet_float( x->x_frames, x->x_outframes );
    outlet_float( x->x_connected, 1 );
    post( "mp3fileout~ : connected to peer" );

}

/* close connection to the peer         */
static void mp3fileout_disconnect(t_mp3fileout *x)
{

    int err = -1;
    if(x->x_socket >= 0)            /* close socket */
    {
#ifdef _WIN32
        closesocket(x->x_socket);
#else
        close(x->x_socket);
#endif
        x->x_socket = -1;
        outlet_float( x->x_connected, 0 );
        x->x_outframes = 0;
        outlet_float( x->x_frames, x->x_outframes );
        post("mp3fileout~: connection closed");
    }
}

static void *mp3fileout_new(void)
{
    t_mp3fileout *x;
    int i;

    x = (t_mp3fileout *)pd_new(mp3fileout_class);
    x->x_connected = outlet_new( &x->x_obj, &s_float );
    x->x_frames = outlet_new( &x->x_obj, &s_float );
    x->x_endreached = outlet_new( &x->x_obj, &s_bang );

    x->x_socket = -1;
    x->x_fd = -1;
    x->x_eof = 0;
    x->x_canvas = canvas_getcurrent();

    x->x_offset = 0;
    x->x_inbuffersize = INPUT_BUFFER_SIZE;
    x->x_inbuffer = (char*) getbytes( x->x_inbuffersize );
    if ( !x->x_inbuffer )
    {
        post( "mp3fileout~ : could not allocate buffers." );
        return NULL;
    }
    memset( x->x_inbuffer, 0x0, INPUT_BUFFER_SIZE );

    x->x_outbuffersize = OUTPUT_BUFFER_SIZE;
    x->x_outbuffer = (char*) getbytes( x->x_outbuffersize );
    if ( !x->x_outbuffer )
    {
        post( "mp3fileout~ : could not allocate buffers." );
        return NULL;
    }
    memset( x->x_outbuffer, 0x0, OUTPUT_BUFFER_SIZE );
    x->x_outavable = 0;

    x->x_inwriteposition = 0;
    x->x_nbloops = 0;
    x->x_nbwaitloops = 1;

    return (x);
}

void mp3fileout_tilde_setup(void)
{
    logpost(NULL, 4,  mp3fileout_version );
    mp3fileout_class = class_new(gensym("mp3fileout~"),
                                 (t_newmethod) mp3fileout_new, (t_method) mp3fileout_free,
                                 sizeof(t_mp3fileout),  0, A_NULL);

    class_addmethod(mp3fileout_class, nullfn, gensym("signal"), 0);
    class_addmethod(mp3fileout_class, (t_method) mp3fileout_dsp, gensym("dsp"), 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_open, gensym("open"), A_SYMBOL, 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_disconnect, gensym("disconnect"), 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_start, gensym("start"), 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_resume, gensym("resume"), 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_seek, gensym("seek"), A_DEFFLOAT, 0);
    class_addmethod(mp3fileout_class, (t_method)mp3fileout_stop, gensym("stop"), 0);
}
