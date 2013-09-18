/* ------------------------ mp3write~ ----------------------------------------- */
/*                                                                              */
/* Tilde object to record audio in mp3 format                                   */
/* Note that it is, in no way, related to mp3play~                              */
/* Written by Yves Degoyon (ydegoyon@free.fr).                                  */
/* Tarballs and updates at http://ydegoyon.free.fr                              */
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
/* Uses the LAME MPEG 1 Layer 3 version 3.92                                    */
/* Get it via http://www.freshmeat.net                                          */
/*                                                                              */
/* "All this talk of blood and iron is the cause of all my kicking"             */
/* Gang Of Four - "Guns Before Butter"                                          */
/* ---------------------------------------------------------------------------- */

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
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#define SOCKET_ERROR -1
#endif /* _WIN32 */
#include <sys/time.h>
#include <lame/lame.h>        /* lame encoder stuff */
#include "m_pd.h"            /* standard pd stuff */

#define        MY_MP3_MALLOC_IN_SIZE        65536
/* max size taken from lame readme */
#define        MY_MP3_MALLOC_OUT_SIZE       1.25*MY_MP3_MALLOC_IN_SIZE+7200

#define        MAXDATARATE 320        /* maximum mp3 data rate is 320kbit/s */
#define        STRBUF_SIZE 32

static char   *mp3write_version = "mp3write~: mp3 file recorder version 0.4, written by Yves Degoyon";
static int    sockfd;

static t_class *mp3write_class;

typedef struct _mp3write
{
    t_object x_obj;

    /* LAME stuff */
    int x_lame;               /* info about encoder status */
    int x_lamechunk;          /* chunk size for LAME encoder */

    /* buffer stuff */
    unsigned short x_inp;     /* in position for buffer */
    unsigned short x_outp;    /* out position for buffer*/
    short *x_mp3inbuf;        /* data to be sent to LAME */
    char *x_mp3outbuf;        /* data returned by LAME -> our mp3 stream */
    int x_mp3size;            /* number of returned mp3 samples */
    short *x_buffer;          /* data to be buffered */
    int x_bytesbuffered;      /* number of unprocessed bytes in buffer */
    int x_start;

    /* mp3 format stuff */
    int x_samplerate;
    int x_bitrate;            /* bitrate of mp3 stream */
    int x_mp3mode;            /* mode (mono, joint stereo, stereo, dual mono) */
    int x_mp3quality;         /* quality of encoding */

    /* recording stuff */
    int x_fd;                 /* file descriptor of the mp3 output */
    int x_file_open_mode;     /* file opening mode */
    int x_byteswritten;       /* number of bytes written */
    int x_recflag;            /* recording flag toggled by messages "start" and "stop" */

    t_float x_f;              /* float needed for signal input */
    char *x_title;            /* title of the mp3              */

    lame_global_flags *lgfp;  /* lame encoder configuration */
} t_mp3write;


/* encode PCM data to mp3 stream */
static void mp3write_encode(t_mp3write *x)
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

    /* on start/reconnect set outpoint so that it won't interfere with inpoint */
    if(x->x_start == -1)
    {
        post("mp3write~: reseting buffer positions");
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
    if((unsigned short)(x->x_outp - x->x_inp) < x->x_lamechunk)error("mp3write~: buffers overlap!");

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
    // post( "mp3write~ : encoding returned %d frames", x->x_mp3size );

    /* check result */
    if(x->x_mp3size<0)
    {
        lame_close( x->lgfp );
        error("mp3write~: lame_encode_buffer_interleaved failed (%d)", x->x_mp3size);
        x->x_lame = -1;
    }
}

/* store mp3 frames in the file */
static void mp3write_writeframes(t_mp3write *x)
{
    int err = -1;            /* error return code */

    if ( x->x_fd < 0 )
    {
        post( "mp3write~ : error : trying to write frames but no valid file is opened" );
        return;
    }

#ifdef _WIN32
    err = _write(x->x_fd, x->x_mp3outbuf, x->x_mp3size);
#else
    err = write(x->x_fd, x->x_mp3outbuf, x->x_mp3size);
#endif

    if(err < 0)
    {
        error("mp3write~: could not write encoded data to file (%d)", err);
        lame_close( x->lgfp );
        x->x_lame = -1;
#ifdef _WIN32
        error("mp3write~: writing data");
        _close(x->x_fd);
#else
        perror("mp3write~: writing data");
        close(x->x_fd);
#endif
        x->x_fd = -1;
    }
    else
    {
        x->x_byteswritten += err;
        outlet_float( x->x_obj.ob_outlet, x->x_byteswritten );
    }
    if((err > 0)&&(err != x->x_mp3size))error("mp3write~: %d bytes skipped", x->x_mp3size - err);
}


/* buffer data as channel interleaved PCM */
static t_int *mp3write_perform(t_int *w)
{
    t_float *in1   = (t_float *)(w[1]);       /* left audio inlet */
    t_float *in2   = (t_float *)(w[2]);       /* right audio inlet */
    t_mp3write *x = (t_mp3write *)(w[3]);
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

    if((x->x_fd >= 0)&&(x->x_lame >= 0)&&(x->x_recflag))
    {
        /* count buffered samples when things are running */
        x->x_bytesbuffered += n;

        /* encode and send to server */
        if(x->x_bytesbuffered > x->x_lamechunk)
        {
            mp3write_encode(x);        /* encode to mp3 */
            mp3write_writeframes(x);   /* write mp3 to file */
            x->x_bytesbuffered -= x->x_lamechunk;
        }
    }
    else
    {
        x->x_start = -1;
    }
    return (w+5);
}

static void mp3write_dsp(t_mp3write *x, t_signal **sp)
{
    dsp_add(mp3write_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

/* initialize the lame library */
static int mp3write_tilde_lame_init(t_mp3write *x)
{
    time_t now;

    int    ret;
    x->lgfp = lame_init(); /* set default parameters for now */

#ifdef _WIN32
    /* load lame_enc.dll library */
    HINSTANCE dll;
    dll=LoadLibrary("lame_enc.dll");
    if(dll==NULL)
    {
        error("mp3write~: error loading lame_enc.dll");
        closesocket(x->x_fd);
        x->x_fd = -1;
        post("mp3write~: connection closed");
        return -1;
    }
#endif
    {
        const char *lameVersion = get_lame_version();
        logpost(NULL, 4,  "mp3write~ : using lame version : %s", lameVersion );
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
        post( "mp3write~ : error : lame params initialization returned : %d", ret );
        return -1;
    }
    else
    {
        x->x_lame=1;
        /* magic formula copied from windows dll for MPEG-I */
        x->x_lamechunk = 2*1152;

        post( "mp3write~ : lame initialization done. (%d)", x->x_lame );
    }
    lame_init_bitstream( x->lgfp );

    /* setting tag information */
    id3tag_init(x->lgfp);
    id3tag_v1_only(x->lgfp);
    id3tag_space_v1(x->lgfp);
    id3tag_set_artist(x->lgfp, "Pd Session");
    now=time(NULL);
    sprintf( x->x_title, "Started at %s", ctime(&now) );
    id3tag_set_title(x->lgfp, x->x_title );

    return 0;

}

/* open file and initialize lame */
static void mp3write_open(t_mp3write *x, t_symbol *sfile)
{
    if ( mp3write_tilde_lame_init(x) < 0 )
    {
        error( "mp3write~ : lame initialization failed ... check parameters.");
        return;
    }

    /* closing previous file descriptor */
    if ( x->x_fd > 0 )
    {
        if(sys_close(x->x_fd) < 0)
        {
            perror( "mp3write~ : closing file" );
        }
    }

    if ( x->x_recflag )
    {
        x->x_recflag = 0;
    }


#ifdef _WIN32
    int mode = _S_IREAD|_S_IWRITE;
#else
    int mode = S_IRWXU|S_IRWXG|S_IRWXO;
#endif
    if ( ( x->x_fd = sys_open( sfile->s_name, x->x_file_open_mode, mode) ) < 0 )
    {
        error( "mp3write~ : cannot open >%s<", sfile->s_name);
        x->x_fd=-1;
        return;
    }
    x->x_byteswritten = 0;
    post( "mp3write~ : opened >%s< fd=%d", sfile->s_name, x->x_fd);
}

/* setting file write mode to append */
static void mp3write_append(t_mp3write *x)
{
#ifdef _WIN32
    x->x_file_open_mode = _O_CREAT|_O_WRONLY|_O_APPEND|_O_BINARY;
#else
    x->x_file_open_mode = O_CREAT|O_WRONLY|O_APPEND|O_NONBLOCK;
#endif
    if(x->x_fd>=0)post("mp3write~ : mode set to append : open a new file to make changes take effect! ");
}

/* setting file write mode to truncate */
static void mp3write_truncate(t_mp3write *x)
{
#ifdef _WIN32
    x->x_file_open_mode = _O_CREAT|_O_WRONLY|_O_TRUNC|_O_BINARY;
#else
    x->x_file_open_mode = O_CREAT|O_WRONLY|O_TRUNC|O_NONBLOCK;
#endif
    if(x->x_fd>=0)post("mp3write~ : mode set to truncate : open a new file to make changes take effect! ");
}

/* settings for mp3 encoding */
static void mp3write_mpeg(t_mp3write *x, t_floatarg fsamplerate, t_floatarg fbitrate,
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
    post("mp3write~: setting mp3 stream to %dHz, %dkbit/s, mode %d, quality %d",
         x->x_samplerate, x->x_bitrate, x->x_mp3mode, x->x_mp3quality);
    if(x->x_fd>=0)post("mp3write~ : restart recording to make changes take effect! ");
}

/* print settings */
static void mp3write_print(t_mp3write *x)
{
    const char        * buf = 0;
    logpost(NULL, 4, mp3write_version);
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

/* start recording */
static void mp3write_start(t_mp3write *x)
{
    if ( x->x_fd < 0 )
    {
        post("mp3write~: start received but no file has been set ... ignored.");
        return;
    }

    if ( x->x_recflag == 1 )
    {
        post("mp3write~: start received but recording is started ... ignored.");
        return;
    }

    x->x_recflag = 1;
    post("mp3write~: start recording");
}

/* stop recording */
static void mp3write_stop(t_mp3write *x)
{
    int err = -1;

    if ( x->x_fd < 0 )
    {
        post("mp3write~: stop received but no file has been set ... ignored.");
        return;
    }

    if ( x->x_recflag == 0 )
    {
        post("mp3write~: stop received but recording is stopped ... ignored.");
        return;
    }
    /* first stop recording / buffering and so on, than do the rest */
    x->x_recflag = 0;

    /* flushing remaining frames and tag */
    x->x_mp3size = lame_encode_flush( x->lgfp, x->x_mp3outbuf, MY_MP3_MALLOC_OUT_SIZE );

    mp3write_writeframes(x);   /* write mp3 to file */

    x->x_recflag = 0;
    post("mp3write~: stop recording, flushed %d bytes", x->x_mp3size);
}

/* clean up */
static void mp3write_free(t_mp3write *x)
{
    if(x->x_lame >= 0)
        lame_close( x->lgfp );
    if(x->x_fd >= 0)
#ifdef _WIN32
        _close(x->x_fd);
#else
        close(x->x_fd);
#endif
    freebytes(x->x_mp3inbuf, MY_MP3_MALLOC_IN_SIZE*sizeof(short));
    freebytes(x->x_mp3outbuf, MY_MP3_MALLOC_OUT_SIZE);
    freebytes(x->x_buffer, MY_MP3_MALLOC_IN_SIZE*sizeof(short));
    freebytes( x->x_title, 255 );
}

static void *mp3write_new(void)
{
    t_mp3write *x = (t_mp3write *)pd_new(mp3write_class);
    inlet_new (&x->x_obj, &x->x_obj.ob_pd, gensym ("signal"), gensym ("signal"));
    outlet_new (&x->x_obj, &s_float);
    x->x_fd = -1;
#ifdef _WIN32
    x->x_file_open_mode = _O_CREAT|_O_WRONLY|_O_APPEND|_O_BINARY;
#else
    x->x_file_open_mode = O_CREAT|O_WRONLY|O_APPEND|O_NONBLOCK;
#endif
    x->x_lame = -1;
    x->x_samplerate = sys_getsr();
    x->x_bitrate = 128;
    x->x_byteswritten = 0;
    x->x_mp3mode = 1;
    x->lgfp = NULL;
    x->x_title = getbytes( 255 );
    x->x_mp3quality = 5;
    x->x_mp3inbuf = getbytes(MY_MP3_MALLOC_IN_SIZE*sizeof(short));  /* buffer for encoder input */
    x->x_mp3outbuf = getbytes(MY_MP3_MALLOC_OUT_SIZE*sizeof(char)); /* our mp3 stream */
    x->x_buffer = getbytes(MY_MP3_MALLOC_IN_SIZE*sizeof(short));    /* what we get from pd, converted to PCM */
    if ((!x->x_buffer)||(!x->x_mp3inbuf)||(!x->x_mp3outbuf))        /* check buffers... */
    {
        error("mp3write~ : cannot allocate internal buffers");
        return NULL;
    }
    x->x_bytesbuffered = 0;
    x->x_inp = 0;
    x->x_outp = 0;
    x->x_start = -1;
    return(x);
}

void mp3write_tilde_setup(void)
{
    logpost(NULL, 4, mp3write_version);
    mp3write_class = class_new(gensym("mp3write~"), (t_newmethod)mp3write_new, (t_method)mp3write_free,
                               sizeof(t_mp3write), 0, 0);
    CLASS_MAINSIGNALIN(mp3write_class, t_mp3write, x_f );
    class_addmethod(mp3write_class, (t_method)mp3write_dsp, gensym("dsp"), 0);
    class_addmethod(mp3write_class, (t_method)mp3write_open, gensym("open"), A_SYMBOL, 0);
    class_addmethod(mp3write_class, (t_method)mp3write_mpeg, gensym("mpeg"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(mp3write_class, (t_method)mp3write_start, gensym("start"), 0);
    class_addmethod(mp3write_class, (t_method)mp3write_stop, gensym("stop"), 0);
    class_addmethod(mp3write_class, (t_method)mp3write_print, gensym("print"), 0);
    class_addmethod(mp3write_class, (t_method)mp3write_append, gensym("append"), 0);
    class_addmethod(mp3write_class, (t_method)mp3write_truncate, gensym("truncate"), 0);
}
