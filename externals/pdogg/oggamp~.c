/* ------------------------- oggamp~ ------------------------------------------ */
/*                                                                              */
/* Tilde object to receive an Ogg Vorbis stream from an IceCast2 server.        */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Get source at http://www.akustische-kunst.de/puredata/                       */
/*                                                                              */
/* Graphical buffer status display written by Yves Degoyon.                     */
/*                                                                              */
/* Thanks for hours (maybe days?) of beta testing to Oliver Thuns.              */
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

	/* Pd includes */
#include "m_pd.h"
#include "g_canvas.h"
	/* Vorbis includes */
#include <vorbis/codec.h>

#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>	/* for 'write' in pute-function only */
#include <winsock.h>
#include <winbase.h>
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

#ifdef WIN32
#define     sys_closesocket closesocket
#else
#define     sys_closesocket close
#endif


/************************* oggamp~ object ******************************/

/*
	Each instance of oggamp~ owns a "child" thread for doing the data
	transfer.  The parent thread signals the child each time:
		(1) a connection wants opening or closing;
		(2) we've eaten another 1/16 of the shared buffer (so that the
    		child thread should check if it's time to receive some more.)
	The child signals the parent whenever a receive has completed.  Signalling
	is done by setting "conditions" and putting data in mutex-controlled common
	areas.
*/


#define     REQUEST_NOTHING 0
#define     REQUEST_CONNECT 1
#define     REQUEST_CLOSE 2
#define     REQUEST_QUIT 3
#define     REQUEST_BUSY 4
#define     REQUEST_DATA 5
#define     REQUEST_RECONNECT 6

#define     STATE_IDLE 0
#define     STATE_STARTUP 1                     /* connecting and filling the buffer */
#define     STATE_STREAM 2                      /* streaming aund audio output */

#define     READSIZE                65536       /* _encoded_ data we request from buffer */
#define     READ                    1024        /* amount of data we pass on to decoder */
#define     DEFBUFPERCHAN           262144      /* audio output buffer default: 256k */
#define     MINBUFSIZE              (4 * READSIZE)
#define     MAXBUFSIZE              16777216 	/* arbitrary; just don't want to hang malloc */
#define     STRBUF_SIZE             1024        /* char received from server on startup */
#define     OBJWIDTH 			    68			/* width of buffer statis display */
#define     OBJHEIGHT 			    10 			/* height of buffer statis display */
#define		MAXSTREAMCHANS          250         /* maximum number of channels */

static char   *oggamp_version = "oggamp~: ogg/vorbis streaming client version 0.3, written by Olaf Matthes";

static t_class *oggamp_class;

typedef struct _oggamp
{
    t_object x_obj;
    t_canvas *x_canvas;        /* remember canvas */
    t_outlet *x_connection;
	t_clock  *x_clock;

    t_float *x_buf;    	    	    	    /* audio data buffer */
    t_int x_bufsize;  	    	    	    /* buffer size in bytes */
    t_int x_noutlets; 	    	    	    /* number of audio outlets */
    t_sample **x_outvec;					/* audio vectors */
    t_int x_vecsize;  	    	    	    /* vector size for transfers */
    t_int x_state;    	    	    	    /* opened, running, or idle */

    	/* parameters to communicate with subthread */
    t_int x_requestcode;	   /* pending request from parent to I/O thread */
    t_int x_connecterror;	   /* slot for "errno" return */
    t_int x_streamchannels;	   /* number of channels in Ogg Vorbis bitstream */
	t_int x_streamrate;        /* sample rate of stream */

		/* buffer stuff */
    t_int x_fifosize; 	       /* buffer size appropriately rounded down */	    
    t_int x_fifohead; 	       /* index of next byte to get from file */
    t_int x_fifotail; 	       /* index of next byte the ugen will read */
	t_int x_fifobytes;         /* number of bytes available in buffer */
    t_int x_eof;   	           /* true if ogg stream has ended */
    t_int x_sigcountdown;      /* counter for signalling child for more data */
    t_int x_sigperiod;	       /* number of ticks per signal */
	t_int x_siginterval;       /* number of times per buffer (depends on data rate) */

		/* ogg/vorbis related stuff */
	ogg_stream_state x_os;     /* take physical pages, weld into a logical stream of packets */
    ogg_sync_state   x_oy;     /* sync and verify incoming physical bitstream */
	ogg_page         x_og;     /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       x_op;     /* one raw packet of data for decode */
	vorbis_info      x_vi;     /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   x_vc;     /* struct that stores all the user comments */
	vorbis_dsp_state x_vd;     /* central working state for the packet->PCM decoder */
	vorbis_block     x_vb;     /* local working space for packet->PCM decode */
	t_int            x_eos;    /* end of stream */
    char            *x_buffer; /* buffer used to pass on to ogg/vorbis */

    t_int     x_vorbis;        /* info about encoder status */
	t_int     x_sync;          /* indicates whether the decoder has been synced */
	t_float   x_pages;         /* number of pages that have been output to server */
    t_outlet *x_outpages;      /* output to send them to */


	t_int    x_connectstate;   /* indicates the state of socket connection */
    t_int    x_fd;             /* the socket number */
    t_int    x_graphic;        /* indicates if we show a graphic bar */ 
	t_float  x_resample;       /* indicates if we need to resample signal (1 = no resampling) */
	t_int    x_recover;        /* indicate how to behave on buffer underruns */
	t_int    x_disconnect;     /* indicates that user want's to disconnect */
    t_int    x_samplerate;     /* Pd's sample rate */

		/* server stuff */
	char    *x_hostname;       /* name or IP of host to connect to */
	char    *x_mountpoint;     /* mountpoint of ogg-bitstream */
	t_int    x_port;           /* port number on which the connection is made */

		/* tread stuff */
    pthread_mutex_t   x_mutex;
    pthread_cond_t    x_requestcondition;
    pthread_cond_t    x_answercondition;
    pthread_t         x_childthread;
} t_oggamp;

	/* check if we can read from socket */
static int oggamp_check_for_data(t_int sock)
{

	fd_set set;
	struct timeval tv;
	t_int ret;

	tv.tv_sec = 0;
	tv.tv_usec = 20000;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	ret = select(sock + 1, &set, NULL, NULL, &tv);
	if (ret > 0)
		return 1;
	return 0;
}

	/* receive 'size' bytes from server */
static int oggamp_child_receive(int fd, char *buffer, int size)
{
	int   ret = -1;
	int   i;
 
	ret = recv(fd, buffer, size, 0);
	if(ret < 0)
	{
		post("oggamp~: receive error" );
	}
	return ret;
}

	/* ogg/vorbis decoder sync init */
static void oggamp_vorbis_sync_init(t_oggamp *x)
{
	ogg_sync_init(&(x->x_oy)); /* Now we can read pages */
	x->x_sync = 1;
}

	/* ogg/vorbis decoder setup */
static int oggamp_vorbis_init(t_oggamp *x, int fd)
{
    int i;
	int result; /* error return code */
	int bytes;	/* number of bytes receive returned */
	char *buffer; /* buffer for undecoded ogg vorbis data */

	if(!x->x_sync)oggamp_vorbis_sync_init(x);	/* init the sync */

	x->x_eos = 0;	/* indicate beginning of new stream */

		/* submit a 4k block to libvorbis' ogg layer */
    buffer = ogg_sync_buffer(&(x->x_oy),4096);
	post("oggamp~: prebuffering...");
	bytes = oggamp_child_receive(fd, buffer, 4096);
	ogg_sync_wrote(&(x->x_oy),bytes);
	result = ogg_sync_pageout(&(x->x_oy),&(x->x_og));
			
		/* we might need more data */
	if(result == -1)
	{
		post("reading more...");
		buffer = ogg_sync_buffer(&(x->x_oy),4096);
		bytes = oggamp_child_receive(fd, buffer, 4096);
		ogg_sync_wrote(&(x->x_oy),bytes);
		result = ogg_sync_pageout(&(x->x_oy),&(x->x_og));
	}
		/* Get the first page. */
    if(result != 1)
	{
			/* error case.  Must not be Vorbis data */
		error("oggamp~: input does not appear to be an ogg bitstream (error %d)", result);
		return -1;
    }
  
    ogg_stream_init(&(x->x_os),ogg_page_serialno(&(x->x_og)));
    
    vorbis_info_init(&(x->x_vi));
    vorbis_comment_init(&(x->x_vc));
    if(ogg_stream_pagein(&(x->x_os),&(x->x_og))<0){ 
			/* error; stream version mismatch perhaps */
		error("oggamp~: error reading first page of ogg bitstream data");
		return -1;
    }
    
    if(ogg_stream_packetout(&(x->x_os),&(x->x_op))!=1){ 
			/* no page? must not be vorbis */
		error("oggamp~: error reading initial header packet");
		return -1;
    }
    
    if(vorbis_synthesis_headerin(&(x->x_vi),&(x->x_vc),&(x->x_op))<0){ 
			/* error case; not a vorbis header */
		error("oggamp~: this ogg bitstream does not contain Vorbis audio data");
		return -1;
    }
    
    i=0;
    while(i<2){
		while(i<2){
			result = ogg_sync_pageout(&(x->x_oy),&(x->x_og));
			if(result == 0)break; /* Need more data */
			/* Don't complain about missing or corrupt data yet.  We'll
			catch it at the packet output phase */
			if(result == 1)
			{
				ogg_stream_pagein(&(x->x_os),&(x->x_og)); /* we can ignore any errors here
				as they'll also become apparent
				at packetout */
				while(i<2){
					result = ogg_stream_packetout(&(x->x_os),&(x->x_op));
					if(result==0)break;
					if(result<0){
						/* Uh oh; data at some point was corrupted or missing!
						We can't tolerate that in a header.  Die. */
						error("oggamp~: corrupt secondary header, exiting");
						return -1;
					}
					vorbis_synthesis_headerin(&(x->x_vi),&(x->x_vc),&(x->x_op));
					i++;
				}
			}
		}
			/* no harm in not checking before adding more */
		buffer = ogg_sync_buffer(&(x->x_oy),READ);
		    /* read in next 4k of data */
		bytes = oggamp_child_receive(fd, buffer, READ);
		if(bytes==0 && i<2){
			error("oggamp~: end of stream before finding all Vorbis headers");
			return -1;
		}
		ogg_sync_wrote(&(x->x_oy),bytes);
    }
    
    /* Throw the comments plus a few lines about the bitstream we're decoding */
	post("oggamp~: reading Ogg Vorbis header...");
    {
		char **ptr = x->x_vc.user_comments;
		while(*ptr){
			post("         %s",*ptr);
			++ptr;
		}
		post("oggamp~: bitstream is %d channels @ %ld Hz with %ldkbps", x->x_vi.channels, x->x_vi.rate, x->x_vi.bitrate_nominal / 1000);
		x->x_streamchannels = x->x_vi.channels;
		x->x_streamrate = x->x_vi.rate;
		if(x->x_samplerate != x->x_streamrate)	/* upsampling */
		{		/* we would need to use upsampling */
			post("oggamp~: resampling from %ld Hz to %ld Hz not supported !", x->x_vi.rate, x->x_samplerate);
			return (-1);
		}
		post("oggamp~: encoded by: %s", x->x_vc.vendor);
    }
    
    /* OK, got and parsed all three headers. Initialize the Vorbis packet->PCM decoder. */
    vorbis_synthesis_init(&(x->x_vd),&(x->x_vi));   /* central decode state */
    vorbis_block_init(&(x->x_vd),&(x->x_vb));       /* local state */
	x->x_vorbis = 1;
	return (1);
}

	/* clear the ogg/vorbis decoder */
static void oggamp_vorbis_sync_clear(t_oggamp *x)
{
		/* OK, clean up the framer */
	ogg_sync_clear(&(x->x_oy));
	x->x_sync = 0;
	post("oggamp~: decoder cleared");
}
	/* deinit the ogg/vorbis decoder */
static void oggamp_vorbis_deinit(t_oggamp *x)
{
	if(x->x_vorbis)
	{
		x->x_vorbis = 0;

		/* clean up this logical bitstream; before exit we see if we're
		   followed by another [chained] */
		ogg_stream_clear(&(x->x_os));

		/* ogg_page and ogg_packet structs always point to storage in
		   libvorbis.  They're never freed or manipulated directly */
    
		vorbis_block_clear(&(x->x_vb));
		vorbis_dsp_clear(&(x->x_vd));
		vorbis_comment_clear(&(x->x_vc));
		vorbis_info_clear(&(x->x_vi));  /* must be called last */

		post("oggamp~: decoder deinitialised");
			/* only clear completely in case we're going to disconnect */
			/* !! must not be called when receiving chained streams !! */
		if(x->x_disconnect)oggamp_vorbis_sync_clear(x);
	}
}

	/* decode ogg/vorbis and receive new data */
static int oggamp_decode_input(t_oggamp *x, float *buf, int fifohead, int fifosize, int fd)
{
	int i, result;

	float **pcm;	/* pointer to decoded float samples */
	char *buffer;   /* buffer for ogg vorbis */
	int samples;	/* number of samples returned by decoder at each block! */
	int n = 0;      /* total number of samples returned by decoder at this call */
	int bytes;		/* number of bytes submitted to decoder */
	int position = fifohead;
	int streamchannels = x->x_streamchannels;
	int channels = x->x_noutlets;


		/* the rest is just a straight decode loop until end of stream */
	while(!x->x_eos)
	{
		result = ogg_sync_pageout(&(x->x_oy),&(x->x_og));
		if(result == 0)break; /* need more data */
		if(result < 0)
		{		/* missing or corrupt data at this page position */
			error("oggamp~: corrupt or missing data in bitstream, continuing...");
		}
		else{
			ogg_stream_pagein(&(x->x_os),&(x->x_og)); /* can safely ignore errors at this point */
			while(1)
			{
				result = ogg_stream_packetout(&(x->x_os),&(x->x_op));

				if(result==0)break; /* need more data */
				if(result<0)
				{ /* missing or corrupt data at this page position */
				/* no reason to complain; already complained above */
				}else
				{
						/* we have a packet.  Decode it */
					if(vorbis_synthesis(&(x->x_vb),&(x->x_op))==0) /* test for success! */
						vorbis_synthesis_blockin(&(x->x_vd),&(x->x_vb));
						/* 

						**pcm is a multichannel float vector.  In stereo, for
						example, pcm[0] is left, and pcm[1] is right.  samples is
						the size of each channel.  Convert the float values
						(-1.<=range<=1.) to whatever PCM format and write it out */

					while((samples = vorbis_synthesis_pcmout(&(x->x_vd),&pcm))>0)
					{
						int     j;

							/* copy into our output buffer */
						if(streamchannels >= channels)
						{
							for(j = 0; j < samples; j++)
							{
								for(i = 0; i < channels; i++)
								{
									buf[position + i] = pcm[i][j];
								}
								position = (position + channels) % fifosize;
							}
							vorbis_synthesis_read(&(x->x_vd),samples);  /* tell libvorbis how
																		many samples we
																		actually consumed */
							n += samples;	/* sum up the samples we got from decoder */
						}
						else
						{
							for(j = 0; j < samples; j++)
							{
									/* copy the channels we have */
								for(i = 0; i < streamchannels; i++)
								{
									buf[position + i] = pcm[i][j];
								}
									/* fill rest of buffer with silence */
								for(i = streamchannels; i < channels; i++)
								{
									buf[position + i] = 0.;
								}
								position = (position + channels) % fifosize;
							}
							vorbis_synthesis_read(&(x->x_vd),samples);  /* tell libvorbis how
																		many samples we
																		actually consumed */
							n += samples;	/* sum up the samples we got from decoder */
						}
					}	    
				}
			}
			if(ogg_page_eos(&(x->x_og)))x->x_eos=1;
		}
 	}

		/* read data from socket */
	if(!x->x_eos)	/* read from input until end of stream */
	{
		buffer = ogg_sync_buffer(&(x->x_oy), READ);
			/* read next 4k of data out of buffer */
		bytes = oggamp_child_receive(fd, buffer, READ);
		if(bytes < 0)
		{
			x->x_eos = 1;	/* indicate end of stream */
			return (bytes);
		}
		ogg_sync_wrote(&(x->x_oy),bytes);
	}
	else			/* we read through all the file... */
	{				/* will have to reinit decoder for new file */
		post("oggamp~: end of stream detected");
		return (0);
	}
	return (n*x->x_vi.channels);
}
  
    /* connect to shoutcast server */
static int oggamp_child_connect(char *hostname, char *mountpoint, t_int portno)
{
    struct          sockaddr_in server;
    struct          hostent *hp;

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
	t_int           eof = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("oggamp~: internal error while attempting to open socket");
        return (-1);
    }

        /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname);
    if (hp == 0)
    {
        post("oggamp~: bad host?");
        sys_closesocket(sockfd);
        return (-1);
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

        /* assign client port number */
    server.sin_port = htons((unsigned short)portno);

        /* try to connect.  */
    post("oggamp~: connecting to http://%s:%d/%s", hostname, portno, mountpoint);
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        error("oggamp~: connection failed!\n");
        sys_closesocket(sockfd);
        return (-1);
    }

        /* sheck if we can read/write from/to the socket */
    FD_ZERO( &fdset);
    FD_SET( sockfd, &fdset);
    tv.tv_sec  = 0;            /* seconds */
    tv.tv_usec = 500;        /* microseconds */

    ret = select(sockfd + 1, &fdset, NULL, NULL, &tv);
    if(ret != 0)
    {
        error("oggamp~: can not read from socket");
        sys_closesocket(sockfd);
        return (-1);
    }

       /* build up stuff we need to send to server */
    sprintf(request, "GET /%s HTTP/1.0 \r\nHost: %s\r\nUser-Agent: oggamp~ 0.2\r\nAccept: audio/x-ogg\r\n\r\n", 
            mountpoint, hostname);

    if ( send(sockfd, request, strlen(request), 0) < 0 )    /* say hello to server */
    {
        post("oggamp~: could not contact server...");
        return (-1);
    }

		/* read first line of response */
	i = 0;
	while(i < STRBUF_SIZE - 1)
	{
		if(oggamp_check_for_data(sockfd))
		{
			if (recv(sockfd, request + i, 1, 0) <= 0)
			{
				error("oggamp~: could not read from socket, quitting");
				sys_closesocket(sockfd);
				return (-1);
			}
			if (request[i] == '\n')
				break;
			if (request[i] != '\r')
				i++;
		}
	}
	request[i] = '\0';

        /* time to parse content of the response... */
    if(strstr(request, "HTTP/1.0 200 OK"))    /* server is ready */
    {
        post("oggamp~: IceCast2 server detected");
		while(!eof)		/* read everything we can get */
		{
			i = 0;
			while(i < STRBUF_SIZE - 1)
			{
				if(oggamp_check_for_data(sockfd))
				{
					if (recv(sockfd, request + i, 1, 0) <= 0)
					{
						error("oggamp~: could not read from socket, quitting");
						sys_closesocket(sockfd);
						return (-1);
					}
					if(request[i] == '\n')  /* leave at end of line */
						break;
					if(request[i] == 0x0A)  /* leave at end of line */
						break;
					if(request[i] != '\r')	/* go on until 'return' */
						i++;
				}
			}
			request[i] = '\0';	/* make it a null terminated string */

			if(sptr = strstr(request, "application/x-ogg")) 
			{		/* check for content type */
				post("oggamp~: Ogg Vorbis stream found");
			}
			if(sptr = strstr(request, "ice-name:")) 
			{		/* display ice-name */
				post("oggamp~: \"%s\"", sptr + 10);
			}
			if(i == 0)eof = 1;	/* we got last '\r\n' from server */
		}
    }
    else    /* wrong server or wrong answer */
    {
        post("oggamp~: unknown response from server");
		sys_closesocket(sockfd);
        return (-1);
    }
    post("oggamp~: connected to http://%s:%d/%s", hp->h_name, portno, mountpoint);

	return (sockfd);
}


static void oggamp_child_dographics(t_oggamp *x)
{
		/* do graphics stuff :: create rectangle */
    if ( x->x_graphic && glist_isvisible( x->x_canvas ) )
    {
		sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill lightblue -tags %xPBAR\n",
                  x->x_canvas, x->x_obj.te_xpix, x->x_obj.te_ypix-OBJHEIGHT-1,
                  x->x_obj.te_xpix + OBJWIDTH, x->x_obj.te_ypix - 1, x );
    } 
}

static void oggamp_child_updategraphics(t_oggamp *x)
{
		/* update buffer status display */
	if(x->x_graphic && glist_isvisible(x->x_canvas))
	{
			/* update graphical read status */
		char color[32];

		sys_vgui(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x); 
		if(x->x_fifobytes < (x->x_fifosize / 8))
		{
			strcpy(color, "red");
		}
		else
		{
			strcpy(color, "lightgreen");
		}
		sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %xSTATUS\n",
		x->x_canvas, x->x_obj.te_xpix, x->x_obj.te_ypix-OBJHEIGHT-1,
		x->x_obj.te_xpix+((x->x_fifobytes*OBJWIDTH)/x->x_fifosize),
		x->x_obj.te_ypix - 1, color, x);
	}
}
static void oggamp_child_delgraphics(t_oggamp *x)
{
    if(x->x_graphic)			/* delete graphics */
    {
       sys_vgui(".x%lx.c delete rectangle %xPBAR\n", x->x_canvas, x );
       sys_vgui(".x%lx.c delete rectangle %xSTATUS\n", x->x_canvas, x ); 
    }
}

static void oggamp_child_disconnect(t_int fd)
{
    sys_closesocket(fd);
	post("oggamp~: connection closed");
}
/************** the child thread which performs data I/O ***********/

#if 0			/* set this to get debugging output */
static void pute(char *s)   /* debug routine */
{
    write(2, s, strlen(s));
}
#else
#define pute(x)
#endif

#if 1
#define oggamp_cond_wait pthread_cond_wait
#define oggamp_cond_signal pthread_cond_signal
#else
#include <sys/time.h>    /* debugging version... */
#include <sys/types.h>
static void oggamp_fakewait(pthread_mutex_t *b)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 1000000;
    pthread_mutex_unlock(b);
    select(0, 0, 0, 0, &timout);
    pthread_mutex_lock(b);
}

void oggamp_banana( void)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 200000;
    pute("banana1\n");
    select(0, 0, 0, 0, &timout);
    pute("banana2\n");
}


#define oggamp_cond_wait(a,b) oggamp_fakewait(b)
#define oggamp_cond_signal(a) 
#endif

static void *oggamp_child_main(void *zz)
{
    t_oggamp *x = zz;
    pute("1\n");
    pthread_mutex_lock(&x->x_mutex);
    while (1)
    {
    	int fd, fifohead;
		char *buffer;	/* Ogg Vorbis data */
		float *buf;		/* encoded PCM floats */
		pute("0\n");
		if (x->x_requestcode == REQUEST_NOTHING)
		{
    			pute("wait 2\n");
			oggamp_cond_signal(&x->x_answercondition);
			oggamp_cond_wait(&x->x_requestcondition, &x->x_mutex);
    			pute("3\n");
		}
			// connect to Icecast2 server
		else if (x->x_requestcode == REQUEST_CONNECT)
		{
    			char boo[80];
			int sysrtn, wantbytes;
			
	    		/* copy connect stuff out of the data structure so we can
			relinquish the mutex while we're in oggcast_child_connect(). */
			char *hostname = x->x_hostname;
			char *mountpoint = x->x_mountpoint;
			t_int portno = x->x_port;
			x->x_disconnect = 0;
	    		/* alter the request code so that an ensuing "open" will get
			noticed. */
    			pute("4\n");
			x->x_requestcode = REQUEST_BUSY;
			x->x_connecterror = 0;

	    		/* if there's already a connection open, close it */
			if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
	    		pthread_mutex_unlock(&x->x_mutex);
    	    	oggamp_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
				x->x_connectstate = 0;
				clock_delay(x->x_clock, 0);
	    		x->x_fd = -1;
				if (x->x_requestcode != REQUEST_BUSY)
					goto lost;
			}
    	    		/* open the socket with the mutex unlocked */
			pthread_mutex_unlock(&x->x_mutex);
			fd = oggamp_child_connect(hostname, mountpoint, portno);
			pthread_mutex_lock(&x->x_mutex);
    		pute("5\n");
    	    		/* copy back into the instance structure. */
			x->x_connectstate = 1;
			clock_delay(x->x_clock, 0);
			x->x_fd = fd;
			if (fd < 0)
			{
    	    	x->x_connecterror = fd;
				x->x_eof = 1;
				x->x_connectstate = 0;
				clock_delay(x->x_clock, 0);
    	    	pute("connect failed\n");
				goto lost;
			}
			else
			{
					/* initialise the decoder */
				if(oggamp_vorbis_init(x, fd) != -1)
				{
					post("oggamp~: decoder initialised");
					oggamp_child_dographics(x);
				}
				else
				{
					post("oggamp~: could not init decoder");
					oggamp_child_disconnect(fd);
					post("oggamp~: connection closed due to bitstream error");
					x->x_disconnect = 1;
					x->x_fd = -1;
					x->x_eof = 1;
					x->x_connectstate = 0;
					clock_delay(x->x_clock, 0);
    	    		pute("initialisation failed\n");
					goto lost;
				} 
			}
	    		/* check if another request has been made; if so, field it */
			if (x->x_requestcode != REQUEST_BUSY)
	    		goto lost;
    		pute("6\n");
    		x->x_fifohead = fifohead = 0;
	    		/* set fifosize from bufsize.  fifosize must be a
			multiple of the number of bytes eaten for each DSP
			tick.  We pessimistically assume MAXVECSIZE samples
			per tick since that could change.  There could be a
			problem here if the vector size increases while a
			stream is being played...  */
			x->x_fifosize = x->x_bufsize - (x->x_bufsize %
	    		(x->x_streamchannels * 2));
				/* arrange for the "request" condition to be signalled x->x_siginterval
				times per buffer */
    		sprintf(boo, "fifosize %d\n", 
    	    	x->x_fifosize);
    		pute(boo);
			x->x_sigcountdown = x->x_sigperiod = (x->x_fifosize / (x->x_siginterval * x->x_noutlets * x->x_vecsize));

    	    	/* in a loop, wait for the fifo to get hungry and feed it */
			while (x->x_requestcode == REQUEST_BUSY)
			{
	    		int fifosize = x->x_fifosize;
				buf = x->x_buf;
    	    		pute("77\n");
				if (x->x_eof)
					break;
		    		/* try to get new data from decoder whenever
				there is some space at end of buffer */
				if(x->x_fifobytes < fifosize - READSIZE)
				{
		    		sprintf(boo, "head %d, tail %d\n", x->x_fifohead, x->x_fifotail);
					pute(boo);

					/* we pass x on to the routine since we need the ogg vorbis stuff
					   to be presend. all other values should not be changed because
					   mutex is unlocked ! */
					pute("decode... ");
				    pthread_mutex_unlock(&x->x_mutex);
					sysrtn = oggamp_decode_input(x, buf, fifohead, fifosize, fd);
					oggamp_child_updategraphics(x);
	    			pthread_mutex_lock(&x->x_mutex);
					if (x->x_requestcode != REQUEST_BUSY)
						break;
					if (sysrtn == 0)
					{
						if (x->x_eos && !x->x_disconnect)	/* got end of stream */
						{
							pute("end of stream\n");
							oggamp_vorbis_deinit(x);
							if(oggamp_vorbis_init(x, fd) == -1)	/* reinit stream */
							{
								x->x_state = STATE_IDLE;
								x->x_disconnect = 1;
								goto quit;
							}
						}
						else if (x->x_eos && x->x_disconnect)	/* we're disconnecting */
						{
							pute("end of stream: disconnecting\n");
							break;		/* go to disconnect */
						}
					}
					else if (sysrtn < 0)		/* got any other error from decoder */
					{
						pute("connecterror\n");
	    				x->x_connecterror = sysrtn;
						break;
					}
					x->x_fifohead = (fifohead + sysrtn) % fifosize;
					x->x_fifobytes += sysrtn;
    	    		sprintf(boo, "after: head %d, tail %d\n", 
    	    			x->x_fifohead, x->x_fifotail);
    	    		pute(boo);
						/* check wether the buffer is filled enough to start streaming */
				}
				else	/* there is enough data in the buffer :: do nothing */
				{
					x->x_state = STATE_STREAM;
    	    	    pute("wait 7...\n");
	    	    	oggamp_cond_signal(&x->x_answercondition);
		    		oggamp_cond_wait(&x->x_requestcondition, &x->x_mutex);
    	    	    pute("7 done\n");
					continue;
				}
    	    	pute("8\n");
				fd = x->x_fd;
				buf = x->x_buf;
				fifohead = x->x_fifohead;

					/* signal parent in case it's waiting for data */
				oggamp_cond_signal(&x->x_answercondition);
			}

lost:

    		if (x->x_requestcode == REQUEST_BUSY)
	    	x->x_requestcode = REQUEST_NOTHING;
    	    		/* fell out of read loop: close connection if necessary,
			set EOF and signal once more */
			if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
    	    	pthread_mutex_unlock(&x->x_mutex);
				oggamp_vorbis_deinit(x);
    	    	oggamp_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
	    		x->x_fd = -1;
				oggamp_child_delgraphics(x);
    		}
			oggamp_cond_signal(&x->x_answercondition);

		}
			/* reconnect to server */
		else if (x->x_requestcode == REQUEST_RECONNECT)
		{
			if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
	    		pthread_mutex_unlock(&x->x_mutex);
				oggamp_vorbis_deinit(x);
    	    	oggamp_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
				oggamp_child_delgraphics(x);
	    		x->x_fd = -1;
			}
				/* connect again */
				x->x_requestcode = REQUEST_CONNECT;
			
			x->x_connectstate = 0;
			clock_delay(x->x_clock, 0);
			oggamp_cond_signal(&x->x_answercondition);
		}
			/* close connection to server (disconnect) */
		else if (x->x_requestcode == REQUEST_CLOSE)
		{
quit:
			if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
	    		pthread_mutex_unlock(&x->x_mutex);
				oggamp_vorbis_deinit(x);
    	    	oggamp_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
				oggamp_child_delgraphics(x);
	    		x->x_fd = -1;
			}
			if (x->x_requestcode == REQUEST_CLOSE)
	    		x->x_requestcode = REQUEST_NOTHING;
			else if (x->x_requestcode == REQUEST_BUSY)
	    		x->x_requestcode = REQUEST_NOTHING;
			else if (x->x_requestcode == REQUEST_CONNECT)
	    		x->x_requestcode = REQUEST_NOTHING;
			x->x_connectstate = 0;
			clock_delay(x->x_clock, 0);
			oggamp_cond_signal(&x->x_answercondition);
		}
			// quit everything
		else if (x->x_requestcode == REQUEST_QUIT)
		{
			if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
	    		pthread_mutex_unlock(&x->x_mutex);
				oggamp_vorbis_deinit(x);
    	    	oggamp_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
				x->x_fd = -1;
			}
			x->x_connectstate = 0;
			clock_delay(x->x_clock, 0);
			x->x_requestcode = REQUEST_NOTHING;
			oggamp_cond_signal(&x->x_answercondition);
			break;
		}
		else
		{
			pute("13\n");
		}
    }
    pute("thread exit\n");
    pthread_mutex_unlock(&x->x_mutex);
    return (0);
}

/******** the object proper runs in the calling (parent) thread ****/

static void oggamp_tick(t_oggamp *x)
{
	outlet_float(x->x_connection, x->x_connectstate);
}

static void *oggamp_new(t_floatarg fdographics, t_floatarg fnchannels, t_floatarg fbufsize)
{
    t_oggamp *x;
    int nchannels = fnchannels, bufsize = fbufsize * 1024, i;
    float *buf;
    
    if (nchannels < 1)
    	nchannels = 2;		/* two channels as default */
    else if (nchannels > MAXSTREAMCHANS)
    	nchannels = MAXSTREAMCHANS;
		/* check / set buffer size */
    if (!bufsize) bufsize = DEFBUFPERCHAN * nchannels;
    else if (bufsize < MINBUFSIZE)
    	bufsize = MINBUFSIZE;
    else if (bufsize > MAXBUFSIZE)
    	bufsize = MAXBUFSIZE;
    buf = getbytes(bufsize*sizeof(t_float));
    if (!buf) return (0);
    
    x = (t_oggamp *)pd_new(oggamp_class);
    
    for (i = 0; i < nchannels; i++)
    	outlet_new(&x->x_obj, gensym("signal"));
    x->x_noutlets = nchannels;
    x->x_connection = outlet_new(&x->x_obj, gensym("float"));
	x->x_clock = clock_new(x, (t_method)oggamp_tick);
	x->x_outvec = (t_sample **)getbytes(nchannels * sizeof(t_sample *));

    pthread_mutex_init(&x->x_mutex, 0);
    pthread_cond_init(&x->x_requestcondition, 0);
    pthread_cond_init(&x->x_answercondition, 0);

    x->x_vecsize = 2;
	x->x_disconnect = 0;
    x->x_state = STATE_IDLE;
    x->x_canvas = canvas_getcurrent();
    x->x_streamchannels = 2;
    x->x_fd = -1;
    x->x_buf = buf;
    x->x_bufsize = bufsize;
	x->x_siginterval = 16;	/* signal 16 times per buffer */
    x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_fifobytes = x->x_requestcode = 0;

	x->x_connectstate = 0;  /* indicating state of connection */
    
    x->x_samplerate = x->x_streamrate = sys_getsr();
	x->x_resample = 0;
	x->x_vorbis = 0;
	x->x_sync = 0;
	x->x_recover = -1;   /* just ignore buffer underruns */

		/* graphical buffer status display */
    x->x_graphic = (int)fdographics;
    x->x_canvas = canvas_getcurrent(); 
    
    logpost(NULL, 4, oggamp_version);
	post("oggamp~: set buffer to %dk bytes", bufsize/1024);

		/* start child thread */
    pthread_create(&x->x_childthread, 0, oggamp_child_main, x);
    return (x);
}

static t_int *oggamp_perform(t_int *w)
{
    t_oggamp *x = (t_oggamp *)(w[1]);
    t_int vecsize = x->x_vecsize, noutlets = x->x_noutlets, i, j, r;
    t_float *fp;
	t_float *buffer = x->x_buf;

    if (x->x_state == STATE_STREAM)
    {
    	t_int wantbytes, getbytes, havebytes, nchannels, streamchannels = x->x_streamchannels;

		pthread_mutex_lock(&x->x_mutex);

			/* get 'getbytes' bytes from input buffer, convert them to 
		       'wantbytes' which is the number of bytes after resampling */
		wantbytes = noutlets * vecsize;		/* number of bytes we get after resampling */
		havebytes = x->x_fifobytes;

			/* check for error */
		if(havebytes < wantbytes)
		{
			if(x->x_connecterror)
			{		/* report error and close connection */
	    		pd_error(x, "dsp: error %d", x->x_connecterror);
				x->x_state = STATE_IDLE;
				x->x_requestcode = REQUEST_CLOSE;
				x->x_disconnect = 1;
				oggamp_cond_signal(&x->x_requestcondition);
				pthread_mutex_unlock(&x->x_mutex);
			}
			if(!x->x_disconnect)	/* it's not due to disconnect */
			{
				if(x->x_recover == 0)			/* disconnect */
				{
					x->x_state = STATE_IDLE;
					x->x_requestcode = REQUEST_CLOSE;
					x->x_disconnect = 1;
					oggamp_cond_signal(&x->x_requestcondition);
					pthread_mutex_unlock(&x->x_mutex);
				}
				else if(x->x_recover == 1)		/* reconnect */
				{
					x->x_state = STATE_IDLE;
					x->x_requestcode = REQUEST_RECONNECT;
					x->x_disconnect = 1;
					oggamp_cond_signal(&x->x_requestcondition);
					pthread_mutex_unlock(&x->x_mutex);
				}
				else		/* resume */
				{
					x->x_state = STATE_IDLE;
					x->x_disconnect = 0;
					oggamp_cond_signal(&x->x_requestcondition);
					pthread_mutex_unlock(&x->x_mutex);
				}
			}
			goto idle;
		}

			/* output audio */
		buffer += x->x_fifotail;	/* go to actual audio position */

		for(j = 0; j < vecsize; j++)
		{
			for(i = 0; i < noutlets; i++)
			{
				x->x_outvec[i][j] = *buffer++;
			}
		}

		
		x->x_fifotail += wantbytes;
		x->x_fifobytes -= wantbytes;
		if (x->x_fifotail >= x->x_fifosize)
			x->x_fifotail = 0;
			/* signal the child thread */
		if ((--x->x_sigcountdown) <= 0)
		{
    		oggamp_cond_signal(&x->x_requestcondition);
			x->x_sigcountdown = x->x_sigperiod;
		}
		pthread_mutex_unlock(&x->x_mutex);
    }
    else
    {
    idle:
    	for (i = 0; i < noutlets; i++)
	    for (j = vecsize, fp = x->x_outvec[i]; j--; )
	    	*fp++ = 0;
    }

    return (w+2);
}


static void oggamp_disconnect(t_oggamp *x)
{
    	/* LATER rethink whether you need the mutex just to set a variable? */
    pthread_mutex_lock(&x->x_mutex);
	x->x_disconnect = 1;
    x->x_state = STATE_IDLE;
    x->x_requestcode = REQUEST_CLOSE;
    oggamp_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}


    /* connect method.  Called as:
    connect <hostname or IP> <mountpoint> <portnumber>
    */

static void oggamp_connect(t_oggamp *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *hostsym = atom_getsymbolarg(0, argc, argv);
    t_symbol *mountsym = atom_getsymbolarg(1, argc, argv);
    t_float portno = atom_getfloatarg(2, argc, argv);
    if (!*hostsym->s_name)	/* check for hostname */
    	return;
    if (!portno)			/* check wether the portnumber is specified */
    	portno = 8000;		/* ...assume port 8000 as standard */
    pthread_mutex_lock(&x->x_mutex);
	if(x->x_fd == -1)
	{
		x->x_hostname = hostsym->s_name;
		x->x_mountpoint = mountsym->s_name;
		x->x_port = portno;
		x->x_requestcode = REQUEST_CONNECT;
			/* empty buffer */
		x->x_fifotail = 0;
		x->x_fifohead = 0;
		x->x_fifobytes = 0;
		x->x_streamchannels = 2;
		x->x_eof = 0;
		x->x_connecterror = 0;
		x->x_state = STATE_STARTUP;
		x->x_disconnect = 0;
		oggamp_cond_signal(&x->x_requestcondition);
	}
	else post("oggamp~: already connected");
    pthread_mutex_unlock(&x->x_mutex);
}

	/* connect using url like "http://localhost:8000/mountpoint" */
static void oggamp_connect_url(t_oggamp *x, t_symbol *url)
{
	char *hname, *port;
	char *h, *p;
	char *hostptr;
	char *r_hostptr;
	char *pathptr;
	char *portptr;
	char *p0;
	char *defaultportstr = "8000";
	t_int stringlength;
	t_int portno;

		/* strip http:// or ftp:// */
	p = url->s_name;
	if (strncmp(p, "http://", 7) == 0)
		p += 7;

        if (strncmp(p, "ftp://", 6) == 0)
               p += 6;

	hostptr = p;
	while (*p && *p != '/')	/* look for end of hostname:port */
		p++;
	p++;					/* also skip '/' */
	pathptr = p;

	r_hostptr = --p;
	while (*p && hostptr < p && *p != ':' && *p != ']')	/* split at ':' */
		p--;

	if (!*p || p < hostptr || *p != ':') {
		portptr = NULL;
	}
	else{
		portptr = p + 1;
		r_hostptr = p - 1;
	}
	if (*hostptr == '[' && *r_hostptr == ']') {
		hostptr++;
		r_hostptr--;
	}

	stringlength = r_hostptr - hostptr + 1;
	h = getbytes(stringlength + 1);
	if (h == NULL) {
		hname = NULL;
		port = NULL;
		pathptr = NULL; 
	}
	strncpy(h, hostptr, stringlength);
	*(h+stringlength) = '\0';
	hname = h;	/* the hostname */

	if (portptr) {
		stringlength = (pathptr - portptr);
		if(!stringlength) portptr = NULL;
	}
	if (portptr == NULL) {
		portptr = defaultportstr;
		stringlength = strlen(defaultportstr);
	}
	p0 = getbytes(stringlength + 1);
	if (p0 == NULL) {
		freebytes(h, stringlength + 1);
		hname = NULL;
		port = NULL;
		pathptr = NULL;
	}
	strncpy(p0, portptr, stringlength);
	*(p0 + stringlength) = '\0';

	for (p = p0; *p && isdigit((unsigned char) *p); p++) ;

	*p = '\0';
	port = (unsigned char *) p0;
		/* convert port from string to int */
	portno = (int)strtol(port, NULL, 10);
	freebytes(p0, stringlength + 1);
		/* set values and signal child to connect */
	pthread_mutex_lock(&x->x_mutex);
	if(x->x_fd == -1)
	{
		x->x_hostname = hname;
		x->x_mountpoint = pathptr;
		x->x_port = portno;
		x->x_requestcode = REQUEST_CONNECT;
		x->x_fifotail = 0;
		x->x_fifohead = 0;
		x->x_fifobytes = 0;
		x->x_streamchannels = 2;
		x->x_eof = 0;
		x->x_connecterror = 0;
		x->x_state = STATE_STARTUP;
		oggamp_cond_signal(&x->x_requestcondition);
	}
	else post("oggamp~: already connected");
	pthread_mutex_unlock(&x->x_mutex);
}

static void oggamp_float(t_oggamp *x, t_floatarg f)
{
    if (f != 0)
	{
		pthread_mutex_lock(&x->x_mutex);
		if(x->x_fd == -1)
		{
			x->x_requestcode = REQUEST_CONNECT;

			x->x_fifotail = 0;
			x->x_fifohead = 0;
			x->x_fifobytes = 0;
			x->x_streamchannels = 2;
			x->x_eof = 0;
			x->x_connecterror = 0;
			x->x_state = STATE_STARTUP;
			oggamp_cond_signal(&x->x_requestcondition);
		}
		else post("oggamp~: already connected");
		pthread_mutex_unlock(&x->x_mutex);
	}
    else oggamp_disconnect(x);
}

static void oggamp_dsp(t_oggamp *x, t_signal **sp)
{
    int i, noutlets = x->x_noutlets;
    pthread_mutex_lock(&x->x_mutex);
    x->x_vecsize = sp[0]->s_n;
    
    x->x_sigperiod = (x->x_fifosize / (x->x_siginterval * x->x_streamchannels * x->x_vecsize));
    for (i = 0; i < noutlets; i++)
    	x->x_outvec[i] = sp[i]->s_vec;
    pthread_mutex_unlock(&x->x_mutex);
    dsp_add(oggamp_perform, 1, x);
}

static void oggamp_print(t_oggamp *x)
{
    pthread_mutex_lock(&x->x_mutex);
	if(x->x_fd >= 0)
	{
		post("oggamp~: connected to http://%s:%d/%s", x->x_hostname, x->x_port, x->x_mountpoint);
		post("oggamp~: bitstream is %d channels @ %ld Hz with %ldkbps nominal bitrate",
			x->x_streamchannels, x->x_streamrate, x->x_vi.bitrate_nominal / 1000);
	} else post("oggamp~: not connected");
	if(x->x_recover == 0)
	post("oggamp~: recover mode set to \"disconnect\" (0)");
	else if(x->x_recover == 1)
	post("oggamp~: recover mode set to \"reconnect\" (1)");
	else if(x->x_recover == -1)
	post("oggamp~: recover mode set to \"resume\" (-1)");
    pthread_mutex_unlock(&x->x_mutex);
}

	/* set behavior for buffer underruns */
static void oggamp_recover(t_oggamp *x, t_floatarg f)
{
    pthread_mutex_lock(&x->x_mutex);
	if(f <= -1)
	{		/* mute audio and try to fill buffer again: the default */
		post("oggamp~: set recover mode to \"resume\" (-1)");
		f = -1;
	}
	else if(f >= 1)
	{		/* reconnect to server */
		post("oggamp~: set recover mode to \"reconnect\" (1)");
		f = 1;
	}
	else
	{		/* disconnect from server */
		post("oggamp~: set recover mode to \"disconnect\" (0)");
		f = 0;
	}
	x->x_recover = f;
    pthread_mutex_unlock(&x->x_mutex);
}

static void oggamp_free(t_oggamp *x)
{
    	/* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_QUIT;
    x->x_disconnect = 1;
    post("stopping oggamp thread...");
    oggamp_cond_signal(&x->x_requestcondition);
    while (x->x_requestcode != REQUEST_NOTHING)
    {
    	post("signalling...");
		oggamp_cond_signal(&x->x_requestcondition);
    	oggamp_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    pthread_mutex_unlock(&x->x_mutex);
    if (pthread_join(x->x_childthread, &threadrtn))
    	error("oggamp_free: join failed");
    post("... done.");
    
    pthread_cond_destroy(&x->x_requestcondition);
    pthread_cond_destroy(&x->x_answercondition);
    pthread_mutex_destroy(&x->x_mutex);
    freebytes(x->x_buf, x->x_bufsize*sizeof(t_float));
	freebytes(x->x_outvec, x->x_noutlets * sizeof(t_sample *));
	clock_free(x->x_clock);
}

void oggamp_tilde_setup(void)
{
    oggamp_class = class_new(gensym("oggamp~"), (t_newmethod)oggamp_new, 
    	(t_method)oggamp_free, sizeof(t_oggamp), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(oggamp_class, (t_method)oggamp_float);
    class_addmethod(oggamp_class, (t_method)oggamp_disconnect, gensym("disconnect"), 0);
    class_addmethod(oggamp_class, (t_method)oggamp_dsp, gensym("dsp"), 0);
    class_addmethod(oggamp_class, (t_method)oggamp_connect, gensym("connect"), A_GIMME, 0);
    class_addmethod(oggamp_class, (t_method)oggamp_connect_url, gensym("connecturl"), A_SYMBOL, 0);
    class_addmethod(oggamp_class, (t_method)oggamp_recover, gensym("recover"), A_FLOAT, 0);
    class_addmethod(oggamp_class, (t_method)oggamp_print, gensym("print"), 0);
}
