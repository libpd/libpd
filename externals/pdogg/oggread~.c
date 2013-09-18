/* ------------------------- oggread~ ------------------------------------------ */
/*                                                                              */
/* Tilde object to read and play back Ogg Vorbis files.                         */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
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
/* Uses the Ogg Vorbis decoding library which can be found at                   */
/* http://www.vorbis.com/                                                       */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include "s_stuff.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#include <stdlib.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define     READ                    4096        /* amount of data we pass on to decoder */
#define     MIN_AUDIO_INPUT 		READ*8      /* this is completely guessed! we just fill half the buffer */
#define     OUTPUT_BUFFER_SIZE 		65536   	/* audio output buffer: 64k */

static char   *oggread_version = "oggread~: ogg/vorbis file reader version 0.2c, written by Olaf Matthes";

/* ------------------------ oggread~ ----------------------------- */

static t_class *oggread_class;

typedef struct _oggread
{
    t_object x_obj;
	t_clock  *x_clock;
		/* ogg/vorbis related stuff */
	OggVorbis_File   x_ov;
	ogg_stream_state x_os;    /* take physical pages, weld into a logical stream of packets */
    ogg_sync_state   x_oy;    /* sync and verify incoming physical bitstream */
	ogg_page         x_og;    /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       x_op;    /* one raw packet of data for decode */
	vorbis_info     *x_vi;    /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   x_vc;    /* struct that stores all the user comments */
	vorbis_dsp_state x_vd;    /* central working state for the packet->PCM decoder */
	vorbis_block     x_vb;    /* local working space for packet->PCM decode */
	t_int            x_eos;   /* end of stream */
    char            *x_buffer;/* buffer used to pass on data to ogg/vorbis */

	t_float   x_position;     /* current playing position */
    t_outlet *x_out_position; /* output to send them to */
	t_outlet *x_out_end;      /* signal end of file */

    t_outlet *x_connection;
    t_int    x_fd;            /* the file handle */
	FILE     *x_file;
	int      x_current_section;
    t_int    x_blocksize;     /* size of a dsp block */    
	t_int    x_decoded;       /* number of samples we got from decoder on last call */

    t_float *x_outbuffer;     /* buffer to store audio decoded data */
    t_int    x_outwriteposition;
    t_int    x_outreadposition;
    t_int    x_outunread;
    t_int    x_outbuffersize;

    t_int    x_samplerate;    /* pd's samplerate, might differ from stream */
    t_int    x_stream;        /* indicates if a stream gets output */
} t_oggread;

	/* output playing position */
static void oggread_tick(t_oggread *x)
{
	outlet_float(x->x_out_position, x->x_position);
	clock_delay(x->x_clock, 250);
}

static int oggread_decode_input(t_oggread *x)
{
	long ret;		/* bytes per channel returned by decoder */
	int i;
	float **pcm;

	x->x_vi = ov_info(&x->x_ov, x->x_current_section);

	while(!x->x_eos)
	{
		ret = ov_read_float(&x->x_ov, &pcm, READ, &x->x_current_section); 
		if (ret == 0)
		{
			/* EOF */
			x->x_eos = 1;
			x->x_stream = 0;
			clock_unset(x->x_clock);
			// post("oggread~: end of file detected, stopping");
			outlet_bang(x->x_out_end);
		}
		else if (ret < 0)
		{
			/* error in the stream.  Not a problem, just reporting it in
			case we (the app) cares.  In this case, we don't. */
		}
		else
		{
			/* we don't bother dealing with sample rate changes, etc, but
			you'll have to */
			long j;
			for(j = 0; j < ret; j++)
			{
				for(i = 0; i < x->x_vi->channels; i++)
				{
					x->x_outbuffer[x->x_outwriteposition] = pcm[i][j];
					x->x_outwriteposition = (x->x_outwriteposition + 1)%x->x_outbuffersize;
				}
			}
			x->x_outunread += (t_int)ret * x->x_vi->channels;

		}
		break;
	}
	x->x_decoded = (t_int)ret * x->x_vi->channels;		/* num. of samples we got from decoder */

	x->x_position = (t_float)ov_time_tell(&x->x_ov);
  
		/* exit decoding 'loop' here, we'll get called again by perform() */
	return 1;
}
  

static t_int *oggread_perform(t_int *w)
{
	t_oggread *x = (t_oggread*) (w[1]);
	t_float *out1 = (t_float *)(w[2]);
	t_float *out2 = (t_float *)(w[3]);
	int n = (int)(w[4]);
	int ret;
	int i = 0;

	x->x_blocksize = n;

	while( n-- )
	{		 /* check that the stream provides enough data */
		if((x->x_stream == 1) && (x->x_outunread > (x->x_blocksize * x->x_vi->channels)))
		{
			if(x->x_vi->channels != 1)	/* play stereo */
			{
				*out1++=*(x->x_outbuffer+x->x_outreadposition); 
				x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
				*out2++=*(x->x_outbuffer+x->x_outreadposition);
				x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
				x->x_outunread-=2;
			}
			else	/* play mono on both sides */
			{
				*out1++=*(x->x_outbuffer+x->x_outreadposition); 
				*out2++=*(x->x_outbuffer+x->x_outreadposition);
				x->x_outreadposition = (x->x_outreadposition + 1)%x->x_outbuffersize;
				x->x_outunread--;
			}
		}
		else		/* silence in case of buffer underrun */
		{
            *out1++=0.0;
            *out2++=0.0;
		}
	}
 
		/* decode data whenever we used up some samples from outbuffer */
	if((x->x_fd > 0) && (x->x_stream)						/* only go when file is open and ready */
		&& (x->x_outunread < (MIN_AUDIO_INPUT - x->x_decoded)))	/* we used up data from last decode */
	{
		// post("oggread~: decoding...");
		if(oggread_decode_input(x) != 1)
		{
			post("oggread~: decoder error");
		}
		// else post("oggread~: decoder returned %d samples", x->x_decoded);
	}
	return (w+5);
} 

static void oggread_dsp(t_oggread *x, t_signal **sp)
{
    dsp_add(oggread_perform, 4, x, sp[1]->s_vec, sp[2]->s_vec, sp[1]->s_n);
}


    /* start playing */               
static void oggread_start(t_oggread *x)
{
    if(x->x_fd > 0)
    {
		if(ov_time_seek(&x->x_ov, 0) < 0)
		{
			post("oggread~: could not rewind file to beginning");
		}
		post("oggread~: START");
		x->x_eos = 0;
		x->x_outreadposition = 0;
		x->x_outwriteposition = 0;
		x->x_outunread = 0;
		x->x_position = 0;
		clock_delay(x->x_clock, 0);
		x->x_stream = 1;
    }
	else post("oggread~: no file open (ignored)");
}

    /* resume file reading */               
static void oggread_resume(t_oggread *x)
{
    if(x->x_fd > 0)
	{
		x->x_stream = 1;
		clock_delay(x->x_clock, 0);
		post("oggread~: RESUME");
	}
	else post("oggread~: encoder not initialised");
}

	/* seek in  file */
static void oggread_seek(t_oggread *x, t_floatarg f)
{
	if(x->x_fd > 0)
		if(ov_time_seek(&x->x_ov, f) < 0)
		{
			post("oggread~: could not set playing position to %g seconds", f);
		}
		else post("oggread~: playing position set to %g seconds", f);
}

    /* stop playing */               
static void oggread_stop(t_oggread *x)
{
	if(x->x_stream)post("oggread~: STOP");
    x->x_stream = 0;
	clock_unset(x->x_clock);
}

static void oggread_float(t_oggread *x, t_floatarg f)
{
	if(f == 0) oggread_stop(x);
	else oggread_start(x);
}

    /* open ogg/vorbis file */                 
static void oggread_open(t_oggread *x, t_symbol *filename)
{
	int i;

	x->x_stream = 0;
		/* first close previous file */
	if(x->x_fd > 0)
	{
		ov_clear(&x->x_ov);
		post("oggread~: previous file closed");
	}
		/* open file for reading */
    if((x->x_file = sys_fopen(filename->s_name, "r")) < 0)
    {
		post("oggread~: could not open file \"%s\"", filename->s_name);
		x->x_eos = 1;
		x->x_fd = -1;
    }
    else
    {
		x->x_stream = 0;
		x->x_eos = 0;
		x->x_fd = 1;
		x->x_outreadposition = 0;
		x->x_outwriteposition = 0;
		x->x_outunread = 0;
		post("oggread~: file \"%s\" opened", filename->s_name);
		outlet_float( x->x_out_position, 0);

			/* try to open as ogg vorbis file */
		if(ov_open(x->x_file, &x->x_ov, NULL, -1) < 0)
		{		/* an error occured (no ogg vorbis file ?) */
			post("oggread~: error: could not open \"%s\" as an OggVorbis file", filename->s_name);
			ov_clear(&x->x_ov);
			post("oggread~: file closed due to error");
      x->x_fd=-1;
      x->x_eos=1;
      return;
		}

			/* print details about each logical bitstream in the input */
		if(ov_seekable(&x->x_ov))
		{
			post("oggread~: input bitstream contained %ld logical bitstream section(s)", ov_streams(&x->x_ov));
			post("oggread~: total bitstream playing time: %ld seconds", (long)ov_time_total(&x->x_ov,-1));
			post("oggread~: encoded by: %s\n",ov_comment(&x->x_ov,-1)->vendor);
		}
		else
		{
			post("oggread~: file \"%s\" was not seekable\n"
			"oggread~: first logical bitstream information:", filename->s_name);
		}

		for(i = 0; i < ov_streams(&x->x_ov); i++)
		{
			x->x_vi = ov_info(&x->x_ov,i);
			post("\tlogical bitstream section %d information:",i+1);
			post("\t\t%ldHz %d channels bitrate %ldkbps serial number=%ld",
				x->x_vi->rate,x->x_vi->channels,ov_bitrate(&x->x_ov,i)/1000, ov_serialnumber(&x->x_ov,i));
			post("\t\theader length: %ld bytes",(long)
			(x->x_ov.dataoffsets[i] - x->x_ov.offsets[i]));
			post("\t\tcompressed length: %ld bytes",(long)(ov_raw_total(&x->x_ov,i)));
			post("\t\tplay time: %ld seconds\n",(long)ov_time_total(&x->x_ov,i));
		}

    } 
}


static void oggread_free(t_oggread *x)
{
    if (x->x_fd > 0) {
        post( "oggread~: closing file" );
		ov_clear(&x->x_ov);
        x->x_fd = -1;
    }
    freebytes(x->x_outbuffer, OUTPUT_BUFFER_SIZE*sizeof(t_float));
	clock_free(x->x_clock);
}

static void *oggread_new(t_floatarg fdographics)
{
    t_oggread *x = NULL;
    
    x = (t_oggread *)pd_new(oggread_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_out_position = outlet_new(&x->x_obj, gensym("float"));
    x->x_out_end      = outlet_new(&x->x_obj, gensym("bang"));
	x->x_clock = clock_new(x, (t_method)oggread_tick);
    
    x->x_fd = -1;
	x->x_eos = 1;
    x->x_stream = 0;
	x->x_position = 0;
    x->x_samplerate = sys_getsr();

    x->x_outbuffersize = OUTPUT_BUFFER_SIZE; 
    x->x_outbuffer = (t_float*) getbytes(OUTPUT_BUFFER_SIZE*sizeof(t_float));

    if(!x->x_outbuffer)
    {
		post( "oggread~: could not allocate buffer" );
		return NULL;
    }
    memset(x->x_outbuffer, 0x0, OUTPUT_BUFFER_SIZE);

    x->x_outreadposition = 0;
    x->x_outwriteposition = 0;
    x->x_outunread = 0;
	x->x_decoded = 0;

    logpost(NULL, 4, oggread_version);

    return (x);
}


void oggread_tilde_setup(void)
{
    oggread_class = class_new(gensym("oggread~"), 
        (t_newmethod) oggread_new, (t_method) oggread_free,
        sizeof(t_oggread), 0, A_DEFFLOAT, A_NULL);
	class_addfloat(oggread_class, (t_method)oggread_float);
    class_addmethod(oggread_class, nullfn, gensym("signal"), 0);
    class_addmethod(oggread_class, (t_method)oggread_dsp, gensym("dsp"), 0);
    class_addmethod(oggread_class, (t_method)oggread_open, gensym("open"), A_SYMBOL, 0);
    class_addmethod(oggread_class, (t_method)oggread_start, gensym("start"), 0);
    class_addmethod(oggread_class, (t_method)oggread_resume, gensym("resume"), 0);
    class_addmethod(oggread_class, (t_method)oggread_seek, gensym("seek"), A_DEFFLOAT, 0);
    class_addmethod(oggread_class, (t_method)oggread_stop, gensym("stop"), 0);
}
