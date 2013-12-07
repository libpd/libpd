/* ------------------------- oggcast~ ----------------------------------------- */
/*                                                                              */
/* Tilde object to send an Ogg Vorbis stream from to IceCast2 server.           */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.org/puredata/pdogg/                */
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
#include <vorbis/vorbisenc.h>

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#ifdef WIN32
# include <io.h>	/* for 'write' in pute-function only */
# include <winsock.h>
# include <winbase.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/time.h>
# include <unistd.h>
# define SOCKET_ERROR -1
#endif

#ifdef _MSC_VER
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
#endif

#ifdef WIN32
# define     sys_closesocket closesocket
# define     pdogg_strdup(s) _strdup(s)
#else
# define     sys_closesocket close
# define     pdogg_strdup(s) strdup(s)
#endif

/************************* oggcast~ object ******************************/

/* Each instance of oggcast~ owns a "child" thread for doing the data
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
#define     REQUEST_REINIT 6

#define     STATE_IDLE 0
#define     STATE_STARTUP 1                     /* connecting and filling the buffer */
#define     STATE_STREAM 2                      /* streaming aund audio output */

#define     READ                    4096        /* amount of data we pass on to encoder */
#define     DEFBUFPERCHAN           262144      /* audio output buffer by default: 256k */
#define     MINBUFSIZE              65536
#define     MAXBUFSIZE              16777216 	/* arbitrary; just don't want to hang malloc */
#define     STRBUF_SIZE             1024        /* char received from server on startup */
#define		MAXSTREAMCHANS          256         /* maximum number of channels: restricted by Pd? */
#define     UPDATE_INTERVAL         250         /* time in milliseconds between updates of output values */

#ifdef __linux__	// 'real' linux only, not for OS X !
#define SEND_OPT MSG_DONTWAIT|MSG_NOSIGNAL
#else
#define SEND_OPT 0
#endif


static char   *oggcast_version = "oggcast~: ogg/vorbis streaming client version 0.2k, written by Olaf Matthes";

static t_class *oggcast_class;

typedef struct _oggcast
{
    t_object x_obj;
    t_float *x_f;
    t_clock *x_clock_connect;
    t_clock *x_clock_pages;
    t_outlet *x_connection;    /* outlet for connection state */
    t_outlet *x_outpages;	   /* outlet for no. of ogg pages */

    t_float *x_buf;    	    	    	    /* audio data buffer */
    t_int x_bufsize;  	    	    	    /* buffer size in bytes */
    t_int x_ninlets; 	    	    	    /* number of audio outlets */
    t_sample **x_outvec;	/* audio vectors */
    t_int x_vecsize;  	    	    	    /* vector size for transfers */
    t_int x_state;    	    	    	    /* opened, running, or idle */

    	/* parameters to communicate with subthread */
    t_int x_requestcode;	   /* pending request from parent to I/O thread */
    t_int x_connecterror;	   /* slot for "errno" return */

		/* buffer stuff */
    t_int x_fifosize; 	       /* buffer size appropriately rounded down */	    
    t_int x_fifohead; 	       /* index of next byte to get from file */
    t_int x_fifotail; 	       /* index of next byte the ugen will read */
    t_int x_sigcountdown;      /* counter for signalling child for more data */
    t_int x_sigperiod;	       /* number of ticks per signal */
	t_int x_siginterval;       /* number of times per buffer (depends on data rate) */

		/* ogg/vorbis related stuff */
	ogg_stream_state x_os;    /* take physical pages, weld into a logical stream of packets */
	ogg_page         x_og;    /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       x_op;    /* one raw packet of data for decode */
	vorbis_info      x_vi;    /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   x_vc;    /* struct that stores all the user comments */
	vorbis_dsp_state x_vd;    /* central working state for the packet->PCM decoder */
	vorbis_block     x_vb;    /* local working space for packet->PCM decode */

	t_int            x_eos;   /* end of stream */
	t_float   x_pages;        /* number of pages that have been output to server */
	t_float   x_lastpages;

        /* ringbuffer stuff */
    t_float *x_buffer;        /* data to be buffered (ringbuffer)*/
    t_int    x_bytesbuffered; /* number of unprocessed bytes in buffer */

        /* ogg/vorbis format stuff */
    t_int    x_samplerate;    /* samplerate of stream (default = getsr() ) */
	t_int    x_skip;          /* samples from input to skip (for resampling) */
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
	char*    x_hostname;      /* name or IP of host to connect to */
	char*    x_mountpoint;    /* mountpoint for IceCast server */
	t_float  x_port;          /* port number on which the connection is made */
    t_int    x_bcpublic;      /* do(n't) publish broadcast on www.oggcast.com */
	t_int    x_servertype;    /* type of server: 0 = JRoar or old Icecast2; 1 = new Icecast2 */

	
	

	t_int    x_connectstate;   /* indicates to state of socket connection */
	t_int    x_outvalue;       /* value that has last been output via outlet */
    t_int    x_fd;             /* the socket number */
	t_resample    x_resample;  /* resampling unit */
	t_int    x_recover;        /* indicate how to behave on buffer underruns */

		/* thread stuff */
    pthread_mutex_t   x_mutex;
    pthread_cond_t    x_requestcondition;
    pthread_cond_t    x_answercondition;
    pthread_t         x_childthread;
} t_oggcast;


static char base64table[65] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
};

	/* This isn't efficient, but it doesn't need to be */
char *oggcast_util_base64_encode(char *data)
{
	int len = strlen(data);
	char *out = (char *)getbytes(len*4/3 + 4);
	char *result = out;
	int chunk;

	while(len > 0) {
		chunk = (len >3)?3:len;
		*out++ = base64table[(*data & 0xFC)>>2];
		*out++ = base64table[((*data & 0x03)<<4) | ((*(data+1) & 0xF0) >> 4)];

		switch(chunk) {
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

	/* check server for writeability */
static int oggcast_checkserver(t_int sock)
{
    fd_set          fdset;
    struct timeval  ztout;
	fd_set writeset;
	fd_set exceptset;

	FD_ZERO(&writeset);
	FD_ZERO(&exceptset);
	FD_SET(sock, &writeset );
	FD_SET(sock, &exceptset );

	if(select(sock+1, NULL, &writeset, &exceptset, &ztout) > 0)
	{
		if(!FD_ISSET(sock, &writeset))
		{
			post("oggcast~: can not write data to the server, quitting");
			return -1;
		}
        if(FD_ISSET(sock, &exceptset))
		{
			post("oggcast~: socket returned an error, quitting");
			return -1;
		}   
	}
	return 0;
}

    /* stream ogg/vorbis to IceCast2 server */
static int oggcast_stream(t_oggcast *x, t_int fd)
{
    int err = -1;            /* error return code */
	int pages = 0;

		/* write out pages (if any) */
	while(!x->x_eos)
	{
		int result=ogg_stream_pageout(&(x->x_os),&(x->x_og));
		if(result==0)break;
		err = send(fd, x->x_og.header, x->x_og.header_len, SEND_OPT);
		if(err < 0)
		{
			error("oggcast~: could not send ogg header to server (%d)", err);
			x->x_eos = 1;	/* indicate (artificial) end of stream */
			return err;
		} 
		err = send(fd, x->x_og.body, x->x_og.body_len, SEND_OPT);
		if(err < 0)
		{
			error("oggcast~: could not send ogg body to server (%d)", err);
			x->x_eos = 1;	/* indicate (artificial) end of stream */
			return err;
		} 
		pages++;	/* count number of pages */
			/* there might be more than one pages we have to send */
		if(ogg_page_eos(&(x->x_og)))x->x_eos=1;
	}
	return (pages);
}

	/* ogg/vorbis decoder setup */
static int oggcast_vorbis_init(t_oggcast *x)
{
	int err = -1;

	x->x_eos = 0;
	x->x_skip = 1;	/* assume no resampling */
		/* choose an encoding mode */
	vorbis_info_init(&(x->x_vi));

	if(x->x_samplerate != sys_getsr())	/* downsampling for Oliver (http://radiostudio.org) */
	{
		if(sys_getsr() / x->x_samplerate == 2.0)
		{
			post("oggcast~: downsampling from %.0f to %d Hz", sys_getsr(), x->x_samplerate);
			x->x_skip = 2;
		}
		else if(sys_getsr() / x->x_samplerate == 4.0)
		{
			post("oggcast~: downsampling from %.0f to %d Hz", sys_getsr(), x->x_samplerate);
			x->x_skip = 4;
		}
		else if(sys_getsr() / x->x_samplerate == 3.0)
		{
			post("oggcast~: downsampling from %.0f to %d Hz", sys_getsr(), x->x_samplerate);
			x->x_skip = 3;
		} else post("oggcast~: warning: resampling from %.0f to %d not supported", sys_getsr(), x->x_samplerate);
	}
	if(x->x_vbr == 1)
	{		/* quality based setting */
		if(vorbis_encode_init_vbr(&(x->x_vi), x->x_channels, x->x_samplerate, x->x_quality))
		{
			  post("oggcast~: ogg/vorbis mode initialisation failed: invalid parameters for quality");
			  vorbis_info_clear(&(x->x_vi));
			  return (-1);
		}
	}
	else
	{		/* bitrate based setting */
		if(vorbis_encode_init(&(x->x_vi), x->x_channels, x->x_samplerate, x->x_br_max*1024, x->x_br_nom*1024, x->x_br_min*1024))
		{
			  post("oggcast~: ogg/vorbis mode initialisation failed: invalid parameters for quality");
			  vorbis_info_clear(&(x->x_vi));
			  return (-1);
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
	vorbis_comment_add_tag(&(x->x_vc),"ENCODER","oggcast~ v0.2 for pure-data");

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
			err = send(x->x_fd, x->x_og.header, x->x_og.header_len, SEND_OPT);
			if(err < 0)
			{
				error("oggcast~: could not send ogg header to server (%d)", err);
				x->x_eos = 1;	/* indicate end of stream */
				return (-1);
			} 
			err = send(x->x_fd, x->x_og.body, x->x_og.body_len, SEND_OPT);
			if(err < 0)
			{
				error("oggcast~: could not send ogg body to server (%d)", err);
				x->x_eos = 1;	/* indicate end of stream */
				return (-1);
			} 
		}
	}
	return (0);
}

	/* deinit the ogg/vorbis decoder */
static void oggcast_vorbis_deinit(t_oggcast *x)
{
	vorbis_analysis_wrote(&(x->x_vd),0);
		/* get rid of remaining data in encoder, if any */
	while(vorbis_analysis_blockout(&(x->x_vd),&(x->x_vb))==1)
	{
		vorbis_analysis(&(x->x_vb),NULL);
		vorbis_bitrate_addblock(&(x->x_vb));

		while(vorbis_bitrate_flushpacket(&(x->x_vd),&(x->x_op)))
		{
			ogg_stream_packetin(&(x->x_os),&(x->x_op));
			oggcast_stream(x, x->x_fd);
		}
	} 
		/* clean up and exit.  vorbis_info_clear() must be called last */
	ogg_stream_clear(&(x->x_os));
	vorbis_block_clear(&(x->x_vb));
	vorbis_dsp_clear(&(x->x_vd));
	vorbis_comment_clear(&(x->x_vc));
	vorbis_info_clear(&(x->x_vi));
}

	/* encode ogg/vorbis and stream new data */
static int oggcast_encode(t_oggcast *x, float *buf, int channels, int fifosize, int fd)
{
    unsigned short i, ch;
    int err = 0;
    int n, pages = 0;

		/* expose the buffer to submit data */
	float **inbuffer=vorbis_analysis_buffer(&(x->x_vd),READ * channels);

		/* read from buffer */
	for(n = 0; n < READ; n++)		             /* fill encode buffer */
	{
		for(ch = 0; ch < channels; ch++)
		{
			inbuffer[ch][n] = *buf++;
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
			err = oggcast_stream(x, fd);	/* stream packet to server */
			if(err >= 0)
			{
				pages += err;       /* count pages */
			}
			else return (err);
		}
	}
	return (pages);
}
  
    /* connect to icecast2 server */
static int oggcast_child_connect(char *hostname, char *mountpoint, t_int portno, 
								 char *passwd, char *bcname, char *bcurl,
								 char *bcgenre, t_int bcpublic, t_int br_nom, t_int servertype)
{
    struct          sockaddr_in server;
    struct          hostent *hp;

        /* variables used for communication with server */
    const char      * buf = 0;
    char            resp[STRBUF_SIZE];
    unsigned int    len;
    fd_set          fdset;
    struct timeval  tv;
    t_int           sockfd;                         /* our internal handle for the socket */
    t_int           ret;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        error("oggcast~: internal error while attempting to open socket");
        return (-1);
    }

        /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname);
    if (hp == 0)
    {
        post("oggcast~: bad host?");
        sys_closesocket(sockfd);
        return (-1);
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

        /* assign client port number */
    server.sin_port = htons((unsigned short)portno);

        /* try to connect.  */
    post("oggcast~: connecting to port %d", portno);
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        error("oggcast~: connection failed!\n");
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
        error("oggcast~: can not read from socket");
        sys_closesocket(sockfd);
        return (-1);
    }

	post("oggcast~: logging in to IceCast2 server...");
		/* now try to log in at IceCast2 server using ICE/1.0 scheme */
	if(servertype == 0)
	{
			/* send the request, a string like: "SOURCE /<mountpoint> ICE/1.0\n" */
		buf = "SOURCE ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = "/";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = mountpoint;
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = " ICE/1.0";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* send the ice headers */
					/* password */
		buf = "\nice-password: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = passwd;
		send(sockfd, buf, strlen(buf), SEND_OPT);
					/* name */
		buf = "\r\nice-name: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = bcname;
		send(sockfd, buf, strlen(buf), SEND_OPT);
        		/* url */
		buf = "\r\nice-url: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = bcurl;
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* genre */
		buf = "\r\nice-genre: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = bcgenre;
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* public */
		buf = "\r\nice-public: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		if(bcpublic==0)                            /* set the public flag for broadcast */
		{
			buf = "no";
		}
		else
		{
			buf ="yes";
		}
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* bitrate */
		buf = "\r\nice-bitrate: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		if(sprintf(resp, "%d", br_nom) == -1)    /* convert int to a string */
		{
			error("oggcast~: wrong bitrate");
		}
		send(sockfd, resp, strlen(resp), SEND_OPT);
			/* description */
		buf = "\r\nice-description: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = "ogg/vorbis streamed from pure-data with oggcast~";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* end of header */
		buf = "\r\n\r\n";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* end login for IceCast using ICE/1.0 scheme */
	}
	else	/* or try to log in at IceCast2 server using HTTP/1.0 base auth scheme */
	{
			/* send the request, a string like: "SOURCE /<mountpoint> HTTP/1.0\n\r" */
		buf = "SOURCE /";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = mountpoint;
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = " HTTP/1.0\r\n";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* send basic authorization */
		sprintf(resp, "source:%s", passwd);
		buf = oggcast_util_base64_encode(resp);
		sprintf(resp, "Authorization: Basic %s\r\n", buf);
		send(sockfd, resp, strlen(resp), SEND_OPT);
			/* send content type */
		buf = "Content-Type: application/x-ogg";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* send the ice headers */
					/* password */
		buf = "\r\nice-password: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = passwd;
		send(sockfd, buf, strlen(buf), SEND_OPT);
					/* name */
		buf = "\r\nice-name: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = bcname;
		send(sockfd, buf, strlen(buf), SEND_OPT);
        		/* url */
		buf = "\r\nice-url: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = bcurl;
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* genre */
		buf = "\r\nice-genre: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = bcgenre;
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* public */
		buf = "\r\nice-public: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		if(bcpublic==0)                            /* set the public flag for broadcast */
		{
			buf = "0";
		}
		else
		{
			buf = "1";
		}
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* bitrate */
		buf = "\r\nice-bitrate: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		if(sprintf(resp, "%d", br_nom) == -1)    /* convert int to a string */
		{
			error("oggcast~: wrong bitrate");
		}
		send(sockfd, resp, strlen(resp), SEND_OPT);
			/* description */
		buf = "\r\nice-description: ";
		send(sockfd, buf, strlen(buf), SEND_OPT);
		buf = "Ogg/Vorbis streamed from PureData with oggcast~\r\n";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* end of header: write an empty line */
		buf = "\r\n";
		send(sockfd, buf, strlen(buf), SEND_OPT);
			/* end login for IceCast2 using ICE/1.0 scheme */
	}

		/* check if we can write to server */
	if(oggcast_checkserver(sockfd)!= 0)
	{
		post("oggcast~: error: server refused to receive data");
		return (-1);
	}
    
    post("oggcast~: logged in to http://%s:%d/%s", hp->h_name, portno, mountpoint);

	return (sockfd);
}



static void oggcast_child_disconnect(t_int fd)
{
    sys_closesocket(fd);
	post("oggcast~: connection closed");
}
/************** the child thread which performs data I/O ***********/

#if 0			/* set this to 1 to get debugging output */
static void pute(char *s)   /* debug routine */
{
    write(2, s, strlen(s));
}
#else
#define pute(x)
#endif

#if 1
#define oggcast_cond_wait pthread_cond_wait
#define oggcast_cond_signal pthread_cond_signal
#else
#include <sys/time.h>    /* debugging version... */
#include <sys/types.h>
static void oggcast_fakewait(pthread_mutex_t *b)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 1000000;
    pthread_mutex_unlock(b);
    select(0, 0, 0, 0, &timout);
    pthread_mutex_lock(b);
}

void oggcast_banana( void)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 200000;
    pute("banana1\n");
    select(0, 0, 0, 0, &timout);
    pute("banana2\n");
}


#define oggcast_cond_wait(a,b) oggcast_fakewait(b)
#define oggcast_cond_signal(a) 
#endif

static void *oggcast_child_main(void *zz)
{
    t_oggcast *x = zz;
	time_t         now; /* to get the time */
    pute("1\n");
    pthread_mutex_lock(&x->x_mutex);
    while (1)
    {
    	int fd, fifotail;
		pute("0\n");
		if (x->x_requestcode == REQUEST_NOTHING)
		{
    			pute("wait 2\n");
			oggcast_cond_signal(&x->x_answercondition);
			oggcast_cond_wait(&x->x_requestcondition, &x->x_mutex);
    			pute("3\n");
		}
			// connect to Icecast2 server
		else if (x->x_requestcode == REQUEST_CONNECT)
		{
    		char boo[100];
			int sysrtn, wantbytes;
			
	    		/* copy connect stuff out of the data structure so we can
			relinquish the mutex while we're connecting to server. */
			char *hostname = x->x_hostname;
			char *mountpoint = x->x_mountpoint;
			t_int portno = x->x_port;
			char *passwd = x->x_passwd;
			char *bcname = x->x_bcname;
			char *bcgenre = x->x_bcgenre;
			char *bcurl = x->x_bcurl;
			t_int bcpublic = x->x_bcpublic;
			t_int br_nom = x->x_br_nom;
			t_int servertype = x->x_servertype;
	    		/* alter the request code so that an ensuing "open" will get
			noticed. */
    			pute("4\n");
			x->x_requestcode = REQUEST_BUSY;
			x->x_connecterror = 0;

   	    		/* open the socket with the mutex unlocked in case 
			we're not already connected */
			if(x->x_fd < 0)
			{
				pthread_mutex_unlock(&x->x_mutex);
				fd = oggcast_child_connect(hostname, mountpoint, portno, passwd, bcname,
											bcurl, bcgenre, bcpublic, br_nom, servertype);
				pthread_mutex_lock(&x->x_mutex);
    			pute("5\n");
    	    		/* copy back into the instance structure. */
				x->x_connectstate = 1;
				clock_delay(x->x_clock_connect, 0);
				x->x_fd = fd;
				if (fd < 0)
				{
    	    		x->x_connecterror = fd;
					x->x_connectstate = 0;
					clock_delay(x->x_clock_connect, 0);
    	    		pute("connect failed\n");
					goto lost;
				}
				else
				{
						/* get the time for the DATE comment */
					now=time(NULL);
					x->x_bcdate = pdogg_strdup(ctime(&now)); /*--moo*/
					x->x_pages = 0;
					clock_delay(x->x_clock_pages, 0);
						/* initialise the encoder */
					if(oggcast_vorbis_init(x) == 0)
					{
						post("oggcast~: ogg/vorbis encoder initialised");
						x->x_eos = 0;
						x->x_state = STATE_STREAM;
					}
					else
					{
						post("oggcast~: could not init encoder");
						oggcast_child_disconnect(fd);
						post("oggcast~: connection closed due to initialisation error");
						x->x_fd = -1;
						x->x_connectstate = 0;
						clock_delay(x->x_clock_connect, 0);
    	    			pute("oggcast~: initialisation failed\n");
						goto lost;
					} 
				}
    			x->x_fifotail = fifotail = 0;
	    			/* set fifosize from bufsize.  fifosize must be a
				multiple of the number of bytes eaten for each DSP
				tick.  We pessimistically assume MAXVECSIZE samples
				per tick since that could change.  There could be a
				problem here if the vector size increases while a
				stream is being played...  */
				x->x_fifosize = x->x_bufsize - (x->x_bufsize % (x->x_channels * READ));
					/* arrange for the "request" condition to be signalled x->x_siginterval
					times per buffer */
    			sprintf(boo, "fifosize %d\n", x->x_fifosize);
    			pute(boo);
				x->x_sigcountdown = x->x_sigperiod = (x->x_fifosize / (x->x_siginterval * x->x_channels * x->x_vecsize));
			}
	    		/* check if another request has been made; if so, field it */
			if (x->x_requestcode != REQUEST_BUSY)
	    		goto lost;
    		pute("6\n");

			while (x->x_requestcode == REQUEST_BUSY)
			{
	    		int fifosize = x->x_fifosize, fifotail, channels;
				float *buf = x->x_buf;
    	    	pute("77\n");

				/* if the head is < the tail, we can immediately write
				from tail to end of fifo to disk; otherwise we hold off
				writing until there are at least WRITESIZE bytes in the
				buffer */
				if (x->x_fifohead < x->x_fifotail ||
					x->x_fifohead >= x->x_fifotail + (READ * x->x_channels)
					|| (x->x_requestcode == REQUEST_CLOSE &&
		    			x->x_fifohead != x->x_fifotail))
    	    	{	/* encode audio and send to server */
    	    		pute("8\n");
					fifotail = x->x_fifotail;
					channels = x->x_channels;
					fd = x->x_fd;
	    			pthread_mutex_unlock(&x->x_mutex);
					sysrtn = oggcast_encode(x, buf + fifotail, channels, fifosize, fd);
	    			pthread_mutex_lock(&x->x_mutex);
					if (x->x_requestcode != REQUEST_BUSY &&
	    					x->x_requestcode != REQUEST_CLOSE)
		    				break;
					if (sysrtn < 0)
					{
						post("oggcast~: closing due to error...");
						goto lost;
					}
					else
					{
						x->x_fifotail += (READ * x->x_channels);
						x->x_pages += sysrtn;
						if (x->x_fifotail >= fifosize)
    	    	    				x->x_fifotail = 0;
    	    		}
    	    		sprintf(boo, "after: head %d, tail %d, pages %d\n", x->x_fifohead, x->x_fifotail, sysrtn);
					pute(boo);
				}
				else	/* just wait... */
				{
    	    		pute("wait 7a ...\n");
	    			oggcast_cond_signal(&x->x_answercondition);
					pute("signalled\n");
					oggcast_cond_wait(&x->x_requestcondition,
					&x->x_mutex);
    	    		pute("7a done\n");
					continue;
				}
				/* signal parent in case it's waiting for data */
				oggcast_cond_signal(&x->x_answercondition);
			}
		}

			/* reinit encoder (settings have changed) */
		else if (x->x_requestcode == REQUEST_REINIT)
		{
	    	pthread_mutex_unlock(&x->x_mutex);
			oggcast_vorbis_deinit(x);
    	    oggcast_vorbis_init(x);
   	    	pthread_mutex_lock(&x->x_mutex);
			post("oggcast~: ogg/vorbis encoder reinitialised");
			x->x_state = STATE_STREAM;
			if (x->x_requestcode == REQUEST_REINIT)
	    		x->x_requestcode = REQUEST_CONNECT;
			oggcast_cond_signal(&x->x_answercondition);
		}
			/* close connection to server (disconnect) */
		else if (x->x_requestcode == REQUEST_CLOSE)
		{
lost:
			x->x_state = STATE_IDLE;
    		if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
	    		pthread_mutex_unlock(&x->x_mutex);
				oggcast_vorbis_deinit(x);
    	    	oggcast_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
	    		x->x_fd = -1;
			}
			if (x->x_requestcode == REQUEST_CLOSE)
	    		x->x_requestcode = REQUEST_NOTHING;
			if (x->x_requestcode == REQUEST_BUSY)	/* disconnect due to error */
	    		x->x_requestcode = REQUEST_NOTHING;
			x->x_connectstate = 0;
			clock_delay(x->x_clock_connect, 0);
			x->x_eos = 1;
			oggcast_cond_signal(&x->x_answercondition);
		}
			// quit everything
		else if (x->x_requestcode == REQUEST_QUIT)
		{
			x->x_state = STATE_IDLE;
			if (x->x_fd >= 0)
			{
	    		fd = x->x_fd;
	    		pthread_mutex_unlock(&x->x_mutex);
				oggcast_vorbis_deinit(x);
    	    	oggcast_child_disconnect(fd);
    	    	pthread_mutex_lock(&x->x_mutex);
				x->x_fd = -1;
			}
			x->x_connectstate = 0;
			clock_delay(x->x_clock_connect, 0);
			x->x_requestcode = REQUEST_NOTHING;
			oggcast_cond_signal(&x->x_answercondition);
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

static void oggcast_tick_connect(t_oggcast *x)
{
	pthread_mutex_lock(&x->x_mutex);
	outlet_float(x->x_connection, x->x_connectstate);
	pthread_mutex_unlock(&x->x_mutex);
}

static void oggcast_tick_pages(t_oggcast *x)
{
		/* output new no. of pages if anything changed */
	t_float pages;
    pthread_mutex_lock(&x->x_mutex);
	pages = x->x_pages;	/* get current value with mutex locked */
    pthread_mutex_unlock(&x->x_mutex);
	if(pages != x->x_lastpages)
	{
		outlet_float(x->x_outpages, pages);
		x->x_lastpages = pages;
	}
	clock_delay(x->x_clock_pages, UPDATE_INTERVAL);	/* come back again... */
}

static void *oggcast_new(t_floatarg fnchannels, t_floatarg fbufsize)
{
    t_oggcast *x;
    int nchannels = fnchannels, bufsize = fbufsize * 1024, i;
    float *buf;
    
    if (nchannels < 1)
    	nchannels = 2;		/* two channels as default */
    else if (nchannels > MAXSTREAMCHANS)
    	nchannels = MAXSTREAMCHANS;
		/* check / set buffer size */
    if (bufsize <= 0) bufsize = DEFBUFPERCHAN * nchannels;
    else if (bufsize < MINBUFSIZE)
    	bufsize = MINBUFSIZE;
    else if (bufsize > MAXBUFSIZE)
    	bufsize = MAXBUFSIZE;
    buf = getbytes(bufsize*sizeof(t_float));
    if (!buf) return (0);
    
    x = (t_oggcast *)pd_new(oggcast_class);
    
    for (i = 1; i < nchannels; i++)
		inlet_new (&x->x_obj, &x->x_obj.ob_pd, gensym ("signal"), gensym ("signal"));
    x->x_connection = outlet_new(&x->x_obj, gensym("float"));
    x->x_outpages = outlet_new(&x->x_obj, gensym("float"));
    x->x_ninlets = nchannels;
	x->x_outvec = getbytes(nchannels*sizeof(t_sample *));
	x->x_clock_connect = clock_new(x, (t_method)oggcast_tick_connect);
	x->x_clock_pages = clock_new(x, (t_method)oggcast_tick_pages);

    pthread_mutex_init(&x->x_mutex, 0);
    pthread_cond_init(&x->x_requestcondition, 0);
    pthread_cond_init(&x->x_answercondition, 0);

    x->x_vecsize = 2;
    x->x_state = STATE_IDLE;
    x->x_buf = buf;
    x->x_bufsize = bufsize;
	x->x_siginterval = 32;	/* signal 32 times per buffer */
	                        /* I found this to be most efficient on my machine */
    x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;

	x->x_connectstate = 0;  /* indicating state of connection */
	x->x_outvalue = 0;      /* value at output currently is 0 */
    
    x->x_samplerate = sys_getsr();
	x->x_resample.upsample = x->x_resample.downsample = 1;   /* don't resample */


    x->x_fd = -1;
	x->x_eos = 0;
	x->x_vbr = 1;                   /* use the vbr setting by default */
    x->x_passwd = "letmein";
    x->x_samplerate = sys_getsr();	/* default to Pd's sampling rate */
	x->x_skip = 1;                  /* no resampling supported */
    x->x_quality = 0.4;             /* quality 0.4 gives roughly 128kbps VBR stream */
	x->x_channels = nchannels;      /* stereo */
    x->x_br_max = 144;
    x->x_br_nom = 128;
    x->x_br_min = 96;
	x->x_pages = x->x_lastpages = 0;
	x->x_bcname = pdogg_strdup("ogg/vorbis stream"); /*--moo: added strdup() */
	x->x_bcurl = pdogg_strdup("http://www.akustische-kunst.org/puredata/");
	x->x_bcgenre = pdogg_strdup("experimental");
	x->x_bcdescription = pdogg_strdup("ogg/vorbis stream emitted from pure-data with oggcast~");
	x->x_bcartist = pdogg_strdup("Pd and oggcast~ v0.2");
	x->x_bclocation = pdogg_strdup(x->x_bcurl);
	x->x_bccopyright = pdogg_strdup("");
	x->x_bcperformer = pdogg_strdup("");
	x->x_bccontact = pdogg_strdup("");
	x->x_bcdate = pdogg_strdup("");
	x->x_bcpublic = 1;
	x->x_mountpoint = "puredata.ogg";
	x->x_servertype = 1;			/* HTTP/1.0 protocol for Icecast2 */
    
    logpost(NULL, 4, oggcast_version);
	post("oggcast~: set buffer to %dk bytes", bufsize / 1024);
	post("oggcast~: encoding %d channels @ %d Hz", x->x_channels, x->x_samplerate);

	clock_delay(x->x_clock_pages, 0);
    pthread_create(&x->x_childthread, 0, oggcast_child_main, x);
    return (x);
}

static t_int *oggcast_perform(t_int *w)
{
    t_oggcast *x = (t_oggcast *)(w[1]);
    int vecsize = x->x_vecsize, ninlets = x->x_ninlets, channels = x->x_channels, i, j, skip = x->x_skip;
	float *sp = x->x_buf;
	pthread_mutex_lock(&x->x_mutex);
    if (x->x_state != STATE_IDLE)
    {
    	int wantbytes;
			/* get 'wantbytes' bytes from inlet */
		wantbytes = channels * vecsize / skip;	/* we'll get vecsize bytes per channel */
			/* check if there is enough space in buffer to write all samples */
		while (x->x_fifotail > x->x_fifohead &&
			x->x_fifotail < x->x_fifohead + wantbytes + 1)
		{
			pute("wait...\n");
			oggcast_cond_signal(&x->x_requestcondition);
			oggcast_cond_wait(&x->x_answercondition, &x->x_mutex);
			pute("done\n");
		}

			/* output audio */
		sp += x->x_fifohead;

		if(ninlets >= channels)
		{
			for(j = 0; j < vecsize; j += skip)
			{
				for(i = 0; i < channels; i++)
				{
					*sp++ = x->x_outvec[i][j];
				}
			}
		}
		else if(channels == ninlets * 2)	/* convert mono -> stereo */
		{
			for(j = 0; j < vecsize; j += skip)
			{
				for(i = 0; i < ninlets; i++)
				{
					*sp++ = x->x_outvec[i][j];
					*sp++ = x->x_outvec[i][j];
				}
			}
		}

		
		x->x_fifohead += wantbytes;
		if (x->x_fifohead >= x->x_fifosize)
			x->x_fifohead = 0;
			/* signal the child thread */
		if ((--x->x_sigcountdown) <= 0)
		{
		    pute("signal 1\n");
    		oggcast_cond_signal(&x->x_requestcondition);
			x->x_sigcountdown = x->x_sigperiod;
		}
    }

	pthread_mutex_unlock(&x->x_mutex);

    return (w+2);
}


static void oggcast_disconnect(t_oggcast *x)
{
    	/* LATER rethink whether you need the mutex just to set a variable? */
    pthread_mutex_lock(&x->x_mutex);
	if(x->x_fd >= 0)
	{
		x->x_state = STATE_IDLE;
		x->x_requestcode = REQUEST_CLOSE;
		oggcast_cond_signal(&x->x_requestcondition);
	}
	else post("oggcast~: not connected");
    pthread_mutex_unlock(&x->x_mutex);
}


    /* connect method.  Called as:
    connect <hostname or IP> <mountpoint> <portnumber>
    */

static void oggcast_connect(t_oggcast *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *hostsym = atom_getsymbolarg(0, argc, argv);
    t_symbol *mountsym = atom_getsymbolarg(1, argc, argv);
    t_float portno = atom_getfloatarg(2, argc, argv);
    if (!*hostsym->s_name)	/* check for hostname */
    	return;
    if (!portno)			/* check wether the portnumber is specified */
    	portno = 8000;		/* ...assume port 8000 as standard */
    pthread_mutex_lock(&x->x_mutex);
	if(x->x_fd >= 0)
	{
		post("oggcast~: already connected");
	}
	else
	{
		x->x_requestcode = REQUEST_CONNECT;
		x->x_hostname = hostsym->s_name;
		x->x_mountpoint = mountsym->s_name;
		x->x_port = portno;

		x->x_fifotail = 0;
		x->x_fifohead = 0;
		// if(x->x_recover != 1)x->x_fifobytes = 0;

		x->x_connecterror = 0;
		x->x_state = STATE_STARTUP;
		oggcast_cond_signal(&x->x_requestcondition);
	}
    pthread_mutex_unlock(&x->x_mutex);
}

static void oggcast_float(t_oggcast *x, t_floatarg f)
{
    if (f != 0)
	{
		pthread_mutex_lock(&x->x_mutex);
		if(x->x_fd >= 0)
		{
			post("oggcast~: already connected");
		}
		else
		{
			if(x->x_recover != 1)
			{
				x->x_fifotail = 0;
				x->x_fifohead = 0;
			}
			x->x_requestcode = REQUEST_CONNECT;

			x->x_fifotail = 0;
			x->x_fifohead = 0;

			x->x_connecterror = 0;
			x->x_state = STATE_STARTUP;
			oggcast_cond_signal(&x->x_requestcondition);
		}
		pthread_mutex_unlock(&x->x_mutex);
	}
    else oggcast_disconnect(x);
}

static void oggcast_dsp(t_oggcast *x, t_signal **sp)
{
    int i, ninlets = x->x_ninlets;
    pthread_mutex_lock(&x->x_mutex);
    x->x_vecsize = sp[0]->s_n;
    
    x->x_sigperiod = (x->x_fifosize / (x->x_siginterval * ninlets * x->x_vecsize));
    for (i = 0; i < ninlets; i++)
    	x->x_outvec[i] = sp[i]->s_vec;
    pthread_mutex_unlock(&x->x_mutex);
    dsp_add(oggcast_perform, 1, x);
}

    /* set password for oggcast server */
static void oggcast_password(t_oggcast *x, t_symbol *password)
{
    pthread_mutex_lock(&x->x_mutex);
    x->x_passwd = password->s_name;
    pthread_mutex_unlock(&x->x_mutex);
}
    /* set comment fields for header (reads in just anything) */
static void oggcast_comment(t_oggcast *x, t_symbol *s, t_int argc, t_atom* argv)
{
	t_binbuf *b = binbuf_new();
	char* comment;
	int length;
	binbuf_add(b, argc, argv);
	binbuf_gettext(b, &comment, &length);

    pthread_mutex_lock(&x->x_mutex);
    if(strstr(s->s_name, "ARTIST"))
	{
		if (x->x_bcartist) free(x->x_bcartist);
		x->x_bcartist = pdogg_strdup(comment); /*-- moo: added strdup() */
		post("oggcast~: ARTIST = %s", x->x_bcartist);
	}
	else if(strstr(s->s_name, "GENRE"))
	{
	        free(x->x_bcgenre);
		x->x_bcgenre = pdogg_strdup(comment);
		post("oggcast~: GENRE = %s", x->x_bcgenre);
	}
	else if(strstr(s->s_name, "TITLE"))
	{
	        free(x->x_bcname);
		x->x_bcname = pdogg_strdup(comment);
		post("oggcast~: TITLE = %s", x->x_bcname);
	}
	else if(strstr(s->s_name, "PERFORMER"))
	{
	        free(x->x_bcperformer);
		x->x_bcperformer = pdogg_strdup(comment);
		post("oggcast~: PERFORMER = %s",x->x_bcperformer);
	}
	else if(strstr(s->s_name, "LOCATION"))
	{
	        free(x->x_bclocation);
		x->x_bclocation = pdogg_strdup(comment);
		post("oggcast~: LOCATION = %s",x->x_bclocation);
	}
	else if(strstr(s->s_name, "COPYRIGHT"))
	{
	        free(x->x_bccopyright);
		x->x_bccopyright = pdogg_strdup(comment);
		post("oggcast~: COPYRIGHT = %s", x->x_bccopyright);
	}
	else if(strstr(s->s_name, "CONTACT"))
	{
	        free(x->x_bccontact);
		x->x_bccontact = pdogg_strdup(comment);
		post("oggcast~: CONTACT = %s", x->x_bccontact);
	}
	else if(strstr(s->s_name, "DESCRIPTION"))
	{
	        free(x->x_bcdescription);
		x->x_bcdescription = pdogg_strdup(comment);
		post("oggcast~: DESCRIPTION = %s", x->x_bcdescription);
	}
	else if(strstr(s->s_name, "DATE"))
	{
	        free(x->x_bcdate);
		x->x_bcdate = pdogg_strdup(comment);
		post("oggcast~: DATE = %s", x->x_bcdate);
	}
	else post("oggcast~: no method for %s", s->s_name);
	if(x->x_state == STATE_STREAM)
	{
		x->x_state = STATE_IDLE;
		x->x_requestcode = REQUEST_REINIT;
		oggcast_cond_signal(&x->x_requestcondition);
	}
    pthread_mutex_unlock(&x->x_mutex);
	freebytes(comment, strlen(comment));
	binbuf_free(b);
}
    /* settings for variable bitrate encoding */
static void oggcast_vbr(t_oggcast *x, t_floatarg fsr, t_floatarg fchannels,
                           t_floatarg fquality)
{
    pthread_mutex_lock(&x->x_mutex);
    x->x_vbr = 1;
	x->x_samplerate = (t_int)fsr;
	x->x_quality = fquality;
	x->x_channels = (t_int)fchannels;
	post("oggcast~: %d channels @ %d Hz, quality %.2f", x->x_channels, x->x_samplerate, x->x_quality);
	if(x->x_state == STATE_STREAM)
	{
		x->x_state = STATE_IDLE;
		x->x_requestcode = REQUEST_REINIT;
		oggcast_cond_signal(&x->x_requestcondition);
	}
    pthread_mutex_unlock(&x->x_mutex);
}

    /* settings for bitrate-based vbr encoding */
static void oggcast_vorbis(t_oggcast *x, t_floatarg fsr, t_floatarg fchannels,
                           t_floatarg fmax, t_floatarg fnom, t_floatarg fmin)
{
    pthread_mutex_lock(&x->x_mutex);
    x->x_vbr = 0;
	x->x_samplerate = (t_int)fsr;
	x->x_channels = (t_int)fchannels;
	x->x_br_max = (t_int)fmax;
	x->x_br_nom = (t_int)fnom;
	x->x_br_min = (t_int)fmin;
	post("oggcast~: %d channels @ %d Hz, bitrates: max. %d / nom. %d / min. %d", 
		  x->x_channels, x->x_samplerate, x->x_br_max, x->x_br_nom, x->x_br_min);
	if(x->x_state == STATE_STREAM)
	{
		x->x_state = STATE_IDLE;
		x->x_requestcode = REQUEST_REINIT;
		oggcast_cond_signal(&x->x_requestcondition);
	}
    pthread_mutex_unlock(&x->x_mutex);
}
    /* select server type */
static void oggcast_server(t_oggcast *x, t_floatarg f)
{
    pthread_mutex_lock(&x->x_mutex);
	if(f)
	{
		x->x_servertype = 1;
		post("oggcast~: set server type to new Icecast2 (HTTP/1.0 scheme)");
	}
	else
	{
		x->x_servertype = 0;
		post("oggcast~: set server type to JRoar (ICE/1.0 scheme)");
	}
    pthread_mutex_unlock(&x->x_mutex);
}
    /* print settings to pd's console window */
static void oggcast_print(t_oggcast *x)
{
    pthread_mutex_lock(&x->x_mutex);
	if(x->x_servertype)
		post("oggcast~: server type is Icecast2");
	else
		post("oggcast~: server type is JRoar");
	post("oggcast~: mountpoint at Icecast2: %s", x->x_mountpoint);
	if(x->x_vbr == 1)
	{
		post("oggcast~: Ogg Vorbis encoder: %d channels @ %d Hz, quality %.2f", x->x_channels, x->x_samplerate, x->x_quality);
	}
	else
	{
		post("oggcast~: Ogg Vorbis encoder: %d channels @ %d Hz, bitrates: max. %d, nom. %d, min. %d", 
			  x->x_channels, x->x_samplerate, x->x_br_max, x->x_br_nom, x->x_br_min);
	}
	post("oggcast~: Ogg Vorbis comments:");
	post("          TITLE = %s", x->x_bcname);
	post("          ARTIST = %s", x->x_bcartist);
	post("          PERFORMER = %s", x->x_bcperformer);
	post("          GENRE = %s", x->x_bcgenre);
	post("          LOCATION = %s", x->x_bclocation);
	post("          COPYRIGHT = %s", x->x_bccopyright);
	post("          CONTACT = %s", x->x_bccontact);
	post("          DESCRIPTION = %s", x->x_bcdescription);
	post("          DATE = %s", x->x_bcdate);
    pthread_mutex_unlock(&x->x_mutex);
}


static void oggcast_free(t_oggcast *x)
{
    	/* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_QUIT;
    post("stopping oggcast thread...");
    oggcast_cond_signal(&x->x_requestcondition);
    while (x->x_requestcode != REQUEST_NOTHING)
    {
    	post("signalling...");
		oggcast_cond_signal(&x->x_requestcondition);
    	oggcast_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    pthread_mutex_unlock(&x->x_mutex);
    if (pthread_join(x->x_childthread, &threadrtn))
    	error("oggcast_free: join failed");
    post("... done.");
    
    pthread_cond_destroy(&x->x_requestcondition);
    pthread_cond_destroy(&x->x_answercondition);
    pthread_mutex_destroy(&x->x_mutex);
    freebytes(x->x_buf, x->x_bufsize*sizeof(t_float));
	freebytes(x->x_outvec, x->x_ninlets*sizeof(t_sample *));
	clock_free(x->x_clock_connect);
	clock_free(x->x_clock_pages);

	/*-- moo: free dynamically allocated comment strings --*/
	free(x->x_bcname);
	free(x->x_bcurl);
	free(x->x_bcgenre);
	free(x->x_bcdescription);
	free(x->x_bcartist);
	free(x->x_bclocation);
	free(x->x_bccopyright);
	free(x->x_bcperformer);
	free(x->x_bccontact);
	free(x->x_bcdate);
}

void oggcast_tilde_setup(void)
{
    oggcast_class = class_new(gensym("oggcast~"), (t_newmethod)oggcast_new, 
    	(t_method)oggcast_free, sizeof(t_oggcast), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(oggcast_class, t_oggcast, x_f );
    class_addfloat(oggcast_class, (t_method)oggcast_float);
    class_addmethod(oggcast_class, (t_method)oggcast_disconnect, gensym("disconnect"), 0);
    class_addmethod(oggcast_class, (t_method)oggcast_dsp, gensym("dsp"), 0);
    class_addmethod(oggcast_class, (t_method)oggcast_connect, gensym("connect"), A_GIMME, 0);
    class_addmethod(oggcast_class, (t_method)oggcast_print, gensym("print"), 0);
    class_addmethod(oggcast_class, (t_method)oggcast_password, gensym("passwd"), A_SYMBOL, 0);
    class_addmethod(oggcast_class, (t_method)oggcast_vorbis, gensym("vorbis"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(oggcast_class, (t_method)oggcast_vbr, gensym("vbr"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(oggcast_class, (t_method)oggcast_server, gensym("server"), A_FLOAT, 0);
    class_addanything(oggcast_class, oggcast_comment);
}
