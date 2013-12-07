/* -------------------------- oggwrite~ ---------------------------------------- */
/*                                                                              */
/* Tilde object to send ogg/vorbis encoded stream to icecast2 server.           */
/* Written by Olaf Matthes (olaf.matthes@gmx.de).                               */
/* Get source at http://www.akustische-kunst.de/puredata/                       */
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
/* Uses the Ogg/Vorbis encoding library which can be found at                   */
/* http://www.vorbis.org                                                        */
/*                                                                              */
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
#include <time.h>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <winsock.h>
#include <windef.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#define SOCKET_ERROR -1
#endif

#include "m_pd.h"             /* standard pd stuff */
#include <vorbis/vorbisenc.h>        /* vorbis encoder stuff */

#define        READ 1024      /* number of samples send to encoder at each call */
                              /* has to be even multiple of 64 */

static char   *oggwrite_version = "oggwrite~: ogg/vorbis recorder version 0.1c, written by Olaf Matthes";

static t_class *oggwrite_class;

typedef struct _oggwrite
{
    t_object x_obj;
		/* ogg/vorbis related stuff */
	ogg_stream_state x_os;    /* take physical pages, weld into a logical stream of packets */
	ogg_page         x_og;    /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       x_op;    /* one raw packet of data for decode */
	vorbis_info      x_vi;    /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   x_vc;    /* struct that stores all the user comments */
	vorbis_dsp_state x_vd;    /* central working state for the packet->PCM decoder */
	vorbis_block     x_vb;    /* local working space for packet->PCM decode */

	t_int            x_eos;   /* end of stream */
    t_int     x_vorbis;       /* info about encoder status */
	t_float   x_pages;        /* number of pages that have been output to server */
    t_outlet *x_outpages;     /* output to send them to */

        /* ringbuffer stuff */
    t_float *x_buffer;        /* data to be buffered (ringbuffer)*/
    t_int    x_bytesbuffered; /* number of unprocessed bytes in buffer */

        /* ogg/vorbis format stuff */
    t_int    x_samplerate;    /* samplerate of stream (default = getsr() ) */
	t_float  x_quality;       /* desired quality level from 0.0 to 1.0 (lo to hi) */
    t_int    x_br_max;        /* max. bitrate of ogg/vorbis stream */
    t_int    x_br_nom;        /* nom. bitrate of ogg/vorbis stream */
    t_int    x_br_min;        /* min. bitrate of ogg/vorbis stream */
    t_int    x_channels;      /* number of channels (1 or 2) */
	t_int    x_vbr;

        /* IceCast server stuff */
    char*    x_passwd;        /* password for server */
    char*    x_bcname;        /* name of broadcast */
    char*    x_bcurl;         /* url of broadcast */
    char*    x_bcgenre;       /* genre of broadcast */
	char*    x_bcdescription; /* description */
	char*    x_bcartist;      /* artist */
	char*    x_bclocation;
	char*    x_bccopyright;
	char*    x_bcperformer;
	char*    x_bccontact;
	char*    x_bcdate;        /* system date when broadcast started */
	char*    x_mountpoint;    /* mountpoint for IceCast server */
    t_int    x_bcpublic;      /* do(n't) publish broadcast on www.oggwrite.com */

        /* recording stuff */
    t_int    x_fd;            /* file descriptor of the mp3 output */
    t_int    x_file_open_mode;/* file opening mode */
    t_int    x_byteswritten;  /* number of bytes written */
    t_int    x_recflag;       /* recording flag toggled by messages "start" and "stop" */

    t_float x_f;              /* float needed for signal input */
} t_oggwrite;

/* Utility functions */

static void sys_closesocket(int fd)
{
#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}
	/* prototypes */
static void oggwrite_vorbis_deinit(t_oggwrite *x);

/* ----------------------------------------- oggwrite~ ---------------------------------- */
    /* write ogg/vorbis to file */
static int oggwrite_write(t_oggwrite *x)
{
    int err = -1;            /* error return code */

		/* write out pages (if any) */
	while(!x->x_eos)
	{
		int result=ogg_stream_pageout(&(x->x_os),&(x->x_og));
		if(result==0)break;
#ifdef WIN32
		err = _write(x->x_fd, x->x_og.header, x->x_og.header_len);
#else
		err = write(x->x_fd, x->x_og.header, x->x_og.header_len);
#endif 
		if(err < 0)
		{
			error("oggwrite~: could not send ogg header to server (%d)", err);
			x->x_eos = 1;	/* indicate (artificial) end of stream */
			return err;
		} 
#ifdef WIN32
		err = _write(x->x_fd, x->x_og.body, x->x_og.body_len);
#else
		err = write(x->x_fd, x->x_og.body, x->x_og.body_len);
#endif 
		if(err < 0)
		{
			error("oggwrite~: could not send ogg body to server (%d)", err);
			x->x_eos = 1;	/* indicate (artificial) end of stream */
			return err;
		} 
		x->x_pages++;	/* count number of pages */
			/* there might be more than one pages we have to send */
		if(ogg_page_eos(&(x->x_og)))x->x_eos=1;
	}
    outlet_float(x->x_outpages, x->x_pages);	/* update info */
	return 1;
}


    /* encode data to ogg/vorbis stream */
static void oggwrite_encode(t_oggwrite *x)
{
    unsigned short i, ch;
    int err = 1;
    int n;
	int channel = x->x_channels;	/* make lokal copy of num. of channels */

		/* expose the buffer to submit data */
	float **inbuffer=vorbis_analysis_buffer(&(x->x_vd),READ);

		/* read from buffer */
	for(n = 0; n < READ / channel; n++)		             /* fill encode buffer */
	{
		for(ch = 0; ch < channel; ch++)
		{
			inbuffer[ch][n] = (float)x->x_buffer[n * channel + ch];
		}
	}
		/* tell the library how much we actually submitted */
	vorbis_analysis_wrote(&(x->x_vd),n);

		/* vorbis does some data preanalysis, then divvies up blocks for
		   more involved (potentially parallel) processing.  Get a single
		   block for encoding now */
	while(vorbis_analysis_blockout(&(x->x_vd),&(x->x_vb))==1)
	{
			/* analysis, assume we want to use bitrate management */
		vorbis_analysis(&(x->x_vb),NULL);
		vorbis_bitrate_addblock(&(x->x_vb));

		while(vorbis_bitrate_flushpacket(&(x->x_vd),&(x->x_op)))
		{
				/* weld the packet into the bitstream */
			ogg_stream_packetin(&(x->x_os),&(x->x_op));
			err = oggwrite_write(x);	/* stream packet to server */
		}
	}
		/* check for errors */
	if(err < 0)
	{
		if(x->x_fd > 0)
		{
#ifdef WIN32
			if(_close(x->x_fd) < 0)
#else
			if(close(x->x_fd) < 0)
#endif
			{
				post( "oggwrite~: file closed due to an error" );
				outlet_float(x->x_obj.ob_outlet, 0);
			}
		}
	}
}
    
    /* buffer data as channel interleaved floats */
static t_int *oggwrite_perform(t_int *w)
{
    t_float *in1   = (t_float *)(w[1]);       /* left audio inlet */
    t_float *in2   = (t_float *)(w[2]);       /* right audio inlet */
    t_oggwrite *x = (t_oggwrite *)(w[3]);
    int n = (int)(w[4]);                      /* number of samples */
    int i;
    t_float in;

        /* copy the data into the buffer */
	if(x->x_channels != 1)					  /* everything but mono */
	{
		n *= 2;								  /* two channels go into one buffer */

		for(i = 0; i < n; i++)
		{
			if(i%2)
			{
				in = *(in2++);	/* right inlet */
			}
			else
			{
				in = *(in1++);	/* left inlet */
			}
			if (in > 1.0) { in = 1.0; }
			if (in < -1.0) { in = -1.0; }
			x->x_buffer[i + x->x_bytesbuffered] = in;
		}
	}
	else	/* mono encoding -> just take left signal inlet 'in1' */
	{
		for(i = 0; i < n; i++)
		{
			in = *(in1++);
			if (in > 1.0) { in = 1.0; }
			if (in < -1.0) { in = -1.0; }
			x->x_buffer[i + x->x_bytesbuffered] = in;
		}
	}
		/* count, encode and send ogg/vorbis */
    if((x->x_fd >= 0)&&(x->x_recflag))
    { 
            /* count buffered samples when connected */
        x->x_bytesbuffered += n;

            /* encode and send to server */
        if((x->x_bytesbuffered >= READ)&&(x->x_vorbis >= 0))
        {
            oggwrite_encode(x);       /* encode data */
            x->x_bytesbuffered = 0;  /* assume we got rid of all of them */
        }
    }
    return (w+5);
}

static void oggwrite_dsp(t_oggwrite *x, t_signal **sp)
{
    dsp_add(oggwrite_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

    /* initialize the vorbisenc library */
static void oggwrite_vorbis_init(t_oggwrite *x)
{
	int err = -1;

	x->x_vorbis = -1;	/* indicate that encoder is not available right now */

		/* choose an encoding mode */
	vorbis_info_init(&(x->x_vi));

	if(x->x_samplerate != sys_getsr())post("oggwrite~: warning: resampling from %d to %.0f not supported", x->x_samplerate, sys_getsr());
	if(x->x_vbr == 1)
	{		/* quality based setting */
		if(vorbis_encode_init_vbr(&(x->x_vi), x->x_channels, x->x_samplerate, x->x_quality))
		{
			  post("oggwrite~: ogg/vorbis mode initialisation failed: invalid parameters for quality");
			  vorbis_info_clear(&(x->x_vi));
			  return;
		}
	}
	else
	{		/* bitrate based setting */
		if(vorbis_encode_init(&(x->x_vi), x->x_channels, x->x_samplerate, x->x_br_max*1024, x->x_br_nom*1024, x->x_br_min*1024))
		{
			  post("oggwrite~: ogg/vorbis mode initialisation failed: invalid parameters for quality");
			  vorbis_info_clear(&(x->x_vi));
			  return;
		}
	}

		/* add a comment */
	vorbis_comment_init(&(x->x_vc));
	vorbis_comment_add_tag(&(x->x_vc),"TITLE", x->x_bcname);
	vorbis_comment_add_tag(&(x->x_vc),"ARTIST", x->x_bcartist);
	vorbis_comment_add_tag(&(x->x_vc),"GENRE",x->x_bcgenre);
	vorbis_comment_add_tag(&(x->x_vc),"DESCRIPTION", x->x_bcdescription);
	vorbis_comment_add_tag(&(x->x_vc),"LOCATION",x->x_bclocation);
	vorbis_comment_add_tag(&(x->x_vc),"PERFORMER",x->x_bcperformer);
	vorbis_comment_add_tag(&(x->x_vc),"COPYRIGHT",x->x_bccopyright);
	vorbis_comment_add_tag(&(x->x_vc),"CONTACT",x->x_bccontact);
	vorbis_comment_add_tag(&(x->x_vc),"DATE",x->x_bcdate);
	vorbis_comment_add_tag(&(x->x_vc),"ENCODER","oggwrite~ v0.1b for pure-data");

		/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init(&(x->x_vd),&(x->x_vi));
	vorbis_block_init(&(x->x_vd),&(x->x_vb));

		/* set up our packet->stream encoder */
		/* pick a random serial number; that way we can more likely build
		   chained streams just by concatenation */
	srand(time(NULL));
	ogg_stream_init(&(x->x_os),rand());

	/* Vorbis streams begin with three headers; the initial header (with
	 most of the codec setup parameters) which is mandated by the Ogg
	 bitstream spec.  The second header holds any comment fields.  The
	 third header holds the bitstream codebook.  We merely need to
	 make the headers, then pass them to libvorbis one at a time;
	 libvorbis handles the additional Ogg bitstream constraints */

	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;

		vorbis_analysis_headerout(&(x->x_vd),&(x->x_vc),&header,&header_comm,&header_code);
		ogg_stream_packetin(&(x->x_os),&header); /* automatically placed in its own page */
		ogg_stream_packetin(&(x->x_os),&header_comm);
		ogg_stream_packetin(&(x->x_os),&header_code);

		/* We don't have to write out here, but doing so makes streaming 
		 * much easier, so we do, flushing ALL pages. This ensures the actual
		 * audio data will start on a new page
		 *
		 * IceCast2 server will take this as a first info about our stream
		 */
		while(!x->x_eos)
		{
			int result=ogg_stream_flush(&(x->x_os),&(x->x_og));
			if(result==0)break;
#ifdef WIN32
			err = _write(x->x_fd, x->x_og.header, x->x_og.header_len);
#else
			err = write(x->x_fd, x->x_og.header, x->x_og.header_len);
#endif 
			if(err < 0)
			{
				error("oggwrite~: could not write ogg header to file (%d)", err);
				x->x_eos = 1;	/* indicate end of stream */
				x->x_vorbis = -1; /* stop encoding instantly */
				if(x->x_fd > 0)
				{
#ifdef WIN32
					if(_close(x->x_fd) < 0)
#else
					if(close(x->x_fd) < 0)
#endif
					{
						post( "oggwrite~: file closed due to an error" );
						outlet_float(x->x_obj.ob_outlet, 0);
					}
				}
				return;
			} 
#ifdef WIN32
			err = _write(x->x_fd, x->x_og.body, x->x_og.body_len);
#else
			err = write(x->x_fd, x->x_og.body, x->x_og.body_len);
#endif 
			if(err < 0)
			{
				error("oggwrite~: could not write ogg body to file (%d)", err);
				x->x_eos = 1;	/* indicate end of stream */
				x->x_vorbis = -1; /* stop encoding instantly */
				if(x->x_fd > 0)
				{
#ifdef WIN32
					if(_close(x->x_fd) < 0)
#else
					if(close(x->x_fd) < 0)
#endif
					{
						post( "oggwrite~: file closed due to an error" );
						outlet_float(x->x_obj.ob_outlet, 0);
					}
				}
				return;
			} 
		}
	}
	x->x_vorbis = 1;	/* vorbis encoder initialised */
	post("oggwrite~: ogg/vorbis encoder (re)initialised");
}

    /* initialize the vorbisenc library */
static void oggwrite_vorbis_deinit(t_oggwrite *x)
{
	x->x_vorbis = -1;
	vorbis_analysis_wrote(&(x->x_vd),0);
		/* clean up and exit.  vorbis_info_clear() must be called last */
	ogg_stream_clear(&(x->x_os));
	vorbis_block_clear(&(x->x_vb));
	vorbis_dsp_clear(&(x->x_vd));
	vorbis_comment_clear(&(x->x_vc));
	vorbis_info_clear(&(x->x_vi));
	post("oggwrite~: ogg/vorbis encoder closed");
}

    /* connect to oggwrite server */
static void oggwrite_open(t_oggwrite *x, t_symbol *sfile)
{
	time_t          now;							/* to get the time */

		/* closing previous file descriptor */
    if(x->x_fd > 0)
	{
#ifdef WIN32
		if(_close(x->x_fd) < 0)
#else
		if(close(x->x_fd) < 0)
#endif
		{
			error( "oggwrite~: file closed" );
			outlet_float(x->x_obj.ob_outlet, 0);
		}
    }

    if(x->x_recflag)
	{
		x->x_recflag = 0;
    }

    if((x->x_fd = sys_open( sfile->s_name, x->x_file_open_mode, 0666 )) < 0)
    {
       error( "oggwrite~: can not open \"%s\"", sfile->s_name); 
       x->x_fd=-1;
       return;
    }
    x->x_byteswritten = 0;
    post( "oggwrite~: \"%s \" opened", sfile->s_name); 
	outlet_float(x->x_obj.ob_outlet, 1);

		/* get the time for the DATE comment, then init encoder */
	now=time(NULL);
	x->x_bcdate = ctime(&now);

	x->x_eos = 0;
    oggwrite_vorbis_init(x);
}

    /* setting file write mode to append */
static void oggwrite_append(t_oggwrite *x)
{
#ifdef WIN32
    x->x_file_open_mode = _O_CREAT|_O_WRONLY|_O_APPEND|_O_BINARY;
#else
	x->x_file_open_mode = O_CREAT|O_WRONLY|O_APPEND|O_NONBLOCK;
#endif
    if(x->x_fd>=0)post("oggwrite~: mode set to append: open a new file to make changes take effect");
}

    /* setting file write mode to truncate */
static void oggwrite_truncate(t_oggwrite *x)
{
#ifdef WIN32
    x->x_file_open_mode = _O_CREAT|_O_WRONLY|_O_TRUNC|_O_BINARY;
#else
    x->x_file_open_mode = O_CREAT|O_WRONLY|O_TRUNC|O_NONBLOCK;
#endif
    if(x->x_fd>=0)post("oggwrite~: mode set to truncate: open a new file to make changes take effect");
}

    /* start recording */
static void oggwrite_start(t_oggwrite *x)
{
    if ( x->x_fd < 0 ) {
       post("oggwrite~: no file selected");
	   return;
    }
 
    if ( x->x_recflag == 1 ) {
       post("oggwrite~: already recording");
       return;
    }
    if(x->x_vorbis < 0)
	{
		oggwrite_vorbis_init(x);
    }
 
    x->x_recflag = 1;
    post("oggwrite~: start recording");
}

    /* stop recording */
static void oggwrite_stop(t_oggwrite *x)
{
	int err = -1;

		/* first stop recording / buffering and so on, than do the rest */
    x->x_recflag = 0;
	post("oggwrite~: recording stoped");
    if(x->x_vorbis >= 0)
	{
		oggwrite_vorbis_deinit(x);
    }
}

    /* set comment fields for header (reads in just anything) */
static void oggwrite_comment(t_oggwrite *x, t_symbol *s, t_int argc, t_atom* argv)
{
	int i = argc;
	char *comment = NULL;
	int len = strlen(atom_gensym(argv)->s_name);

	comment = atom_gensym(argv)->s_name;

	while (len--)
	{
		if(*(comment + len) == '=')*(comment + len) = ' ';
	}

    if(strstr(s->s_name, "ARTIST"))
	{
		x->x_bcartist = comment;
		post("oggwrite~: ARTIST = %s", x->x_bcartist);
	}
	else if(strstr(s->s_name, "GENRE"))
	{
		x->x_bcgenre = comment;
		post("oggwrite~: GENRE = %s", x->x_bcgenre);
	}
	else if(strstr(s->s_name, "TITLE"))
	{
		x->x_bcname = comment;
		post("oggwrite~: TITLE = %s", x->x_bcname);
	}
	else if(strstr(s->s_name, "PERFORMER"))
	{
		x->x_bcperformer = comment;
		post("oggwrite~: PERFORMER = %s", x->x_bcperformer);
	}
	else if(strstr(s->s_name, "LOCATION"))
	{
		x->x_bclocation = comment;
		post("oggwrite~: LOCATION = %s", x->x_bclocation);
	}
	else if(strstr(s->s_name, "COPYRIGHT"))
	{
		x->x_bccopyright = comment;
		post("oggwrite~: COPYRIGHT = %s", x->x_bccopyright);
	}
	else if(strstr(s->s_name, "CONTACT"))
	{
		x->x_bccontact = comment;
		post("oggwrite~: CONTACT = %s", x->x_bccontact);
	}
	else if(strstr(s->s_name, "DESCRIPTION"))
	{
		x->x_bcdescription = comment;
		post("oggwrite~: DESCRIPTION = %s", x->x_bcdescription);
	}
	else if(strstr(s->s_name, "DATE"))
	{
		x->x_bcdate = comment;
		post("oggwrite~: DATE=%s", x->x_bcdate);
	}
	else post("oggwrite~: no method for %s", s->s_name);
	if(x->x_vorbis >=0)
	{
		oggwrite_vorbis_deinit(x);
		oggwrite_vorbis_init(x);
	}
}

    /* settings for variable bitrate encoding */
static void oggwrite_vbr(t_oggwrite *x, t_floatarg fsr, t_floatarg fchannels,
                           t_floatarg fquality)
{
    x->x_vbr = 1;
	x->x_samplerate = (t_int)fsr;
	x->x_quality = fquality;
	x->x_channels = (t_int)fchannels;
	post("oggwrite~: %d channels @ %d Hz, quality %.2f", x->x_channels, x->x_samplerate, x->x_quality);
	if(x->x_vorbis >=0)
	{
		oggwrite_vorbis_deinit(x);
		oggwrite_vorbis_init(x);
	}
}

    /* settings for bitrate-based vbr encoding */
static void oggwrite_vorbis(t_oggwrite *x, t_floatarg fsr, t_floatarg fchannels,
                           t_floatarg fmax, t_floatarg fnom, t_floatarg fmin)
{
    x->x_vbr = 0;
	x->x_samplerate = (t_int)fsr;
	x->x_channels = (t_int)fchannels;
	x->x_br_max = (t_int)fmax;
	x->x_br_nom = (t_int)fnom;
	x->x_br_min = (t_int)fmin;
	post("oggwrite~: %d channels @ %d Hz, bitrates: max. %d / nom. %d / min. %d", 
		  x->x_channels, x->x_samplerate, x->x_br_max, x->x_br_nom, x->x_br_min);
	if(x->x_vorbis >=0)
	{
		oggwrite_vorbis_deinit(x);
		oggwrite_vorbis_init(x);
	}
}

    /* print settings to pd's console window */
static void oggwrite_print(t_oggwrite *x)
{
	if(x->x_vbr == 1)
	{
		post("oggwrite~: Ogg Vorbis encoder: %d channels @ %d Hz, quality %.2f", x->x_channels, x->x_samplerate, x->x_quality);
	}
	else
	{
		post("oggwrite~: Ogg Vorbis encoder: %d channels @ %d Hz, bitrates: max. %d, nom. %d, min. %d", 
			  x->x_channels, x->x_samplerate, x->x_br_max, x->x_br_nom, x->x_br_min);
	}
	post("oggwrite~: Ogg Vorbis comments:");
	post("          TITLE = %s", x->x_bcname);
	post("          ARTIST = %s", x->x_bcartist);
	post("          PERFORMER = %s", x->x_bcperformer);
	post("          GENRE = %s", x->x_bcgenre);
	post("          LOCATION = %s", x->x_bclocation);
	post("          COPYRIGHT = %s", x->x_bccopyright);
	post("          CONTACT = %s", x->x_bccontact);
	post("          DESCRIPTION = %s", x->x_bcdescription);
	post("          DATE = %s", x->x_bcdate);
}


	/* clean up */
static void oggwrite_free(t_oggwrite *x)    
{
    if(x->x_vorbis >= 0)	/* close encoder */
	{
		oggwrite_vorbis_deinit(x);
	}
    if(x->x_fd >= 0)
	{		/* close file */
#ifdef WIN32
        _close(x->x_fd);
#else
        close(x->x_fd);
#endif
		outlet_float(x->x_obj.ob_outlet, 0);
	}
    freebytes(x->x_buffer, READ*sizeof(t_float));
}

static void *oggwrite_new(void)
{
    t_oggwrite *x = (t_oggwrite *)pd_new(oggwrite_class);
    inlet_new (&x->x_obj, &x->x_obj.ob_pd, gensym ("signal"), gensym ("signal"));
    outlet_new(&x->x_obj, gensym("float"));
    x->x_outpages = outlet_new(&x->x_obj, gensym("float"));
    x->x_fd = -1;
#ifdef WIN32
    x->x_file_open_mode = _O_CREAT|_O_WRONLY|_O_APPEND|_O_BINARY;
#else
	x->x_file_open_mode = O_CREAT|O_WRONLY|O_APPEND|O_NONBLOCK;
#endif
    x->x_vorbis = -1;
	x->x_eos = 0;
	x->x_vbr = 1;                   /* use the vbr setting by default */
    x->x_samplerate = sys_getsr();	/* no resampling supported, sorry */
    x->x_quality = 0.4;             /* quality 0.4 gives roughly 128kbps VBR stream */
	x->x_channels = 2;              /* stereo */
    x->x_br_max = 144;
    x->x_br_nom = 128;
    x->x_br_min = 96;
    x->x_buffer = getbytes(READ*sizeof(t_float));
    if (!x->x_buffer)        /* check buffer */
    {
        error("out of memory!");
    }
    x->x_bytesbuffered = 0;
	x->x_pages = 0;
	x->x_bcname = "ogg/vorbis stream";
	x->x_bcurl = "http://www.pure-data.info/";
	x->x_bcgenre = "experimental";
	x->x_bcdescription = "ogg/vorbis stream recorded with pure-data using oggwrite~";
	x->x_bcartist = "pure-data";
	x->x_bclocation = x->x_bcurl;
	x->x_bccopyright = "";
	x->x_bcperformer = "";
	x->x_bccontact = "";
	x->x_bcdate = "";
    logpost(NULL, 4, oggwrite_version);
	return(x);
}

void oggwrite_tilde_setup(void)
{
    oggwrite_class = class_new(gensym("oggwrite~"), (t_newmethod)oggwrite_new, (t_method)oggwrite_free,
        sizeof(t_oggwrite), 0, 0);
    CLASS_MAINSIGNALIN(oggwrite_class, t_oggwrite, x_f );
    class_addmethod(oggwrite_class, (t_method)oggwrite_dsp, gensym("dsp"), 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_open, gensym("open"), A_SYMBOL, 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_start, gensym("start"), 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_stop, gensym("stop"), 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_append, gensym("append"), 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_truncate, gensym("truncate"), 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_vorbis, gensym("vorbis"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_vbr, gensym("vbr"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(oggwrite_class, (t_method)oggwrite_print, gensym("print"), 0);
    class_addanything(oggwrite_class, oggwrite_comment);
}
