
/* (C) Guenter Geiger <geiger@epy.co.at> */

#include <m_pd.h>
#include "stream.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

/* Utility functions */

static void sys_sockerror(char *s)
{
#ifdef unix
    int err = errno;
#else
    int err = WSAGetLastError();
    if (err == 10054) return;
#endif
    post("%s: %s (%d)\n", s, strerror(err), err);
}



static void sys_closesocket(int fd)
{
#ifdef UNIX
     close(fd); /* shutdown() ?? */
#endif
#ifdef _WIN32
    closesocket(fd);
#endif
}


/* ------------------------ streamout~ ----------------------------- */


static t_class *streamout_class;

typedef struct _streamout
{
     t_object x_obj;
     int x_fd;
     int x_protocol;
     t_tag x_tag;
     t_symbol* hostname;
     int   portno;
     short* cbuf;
     int nsamples;
     int tbufsize;
} t_streamout;



static void streamout_tempbuf(t_streamout *x,int size) {

     if (x->cbuf && x->tbufsize) freebytes(x->cbuf,x->tbufsize);
     x->tbufsize=size*sizeof(float)*x->x_tag.channels;
     if (!x->cbuf) 
	  x->cbuf = getbytes(x->tbufsize);
     x->nsamples = size;
}



static void streamout_disconnect(t_streamout *x)
{
    if (x->x_fd >= 0)
    {
    	sys_closesocket(x->x_fd);
    	x->x_fd = -1;
    	outlet_float(x->x_obj.ob_outlet, 0);
    }
}



static void streamout_connect(t_streamout *x, t_symbol *hostname, t_floatarg fportno)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno = fportno;
    x->hostname = hostname;
    if (!fportno) x->portno = 4267;
    else x->portno = (int) fportno;
    x->x_tag.count = 0;
    

    if (x->x_fd >= 0)
    {
	 post("streamout~: already connected");
	 return;
    }

    /* create a socket */

    sockfd = socket(AF_INET, x->x_protocol, 0);
    if (sockfd < 0)
    {
	 post("streamout: Connection to %s on port %d failed",hostname->s_name,portno); 
	 sys_sockerror("socket");
	 return;
    }

    /* connect socket using hostname provided in command line */

    server.sin_family = AF_INET;
    hp = gethostbyname(x->hostname->s_name);
    if (hp == 0)
    {
	post("bad host?\n");
	return;
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((u_short)portno);

    /* try to connect.  LATER make a separate thread to do this
       because it might block */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
    	sys_sockerror("connecting stream socket");
    	sys_closesocket(sockfd);
    	return;
    }

    post("connected host %s on port %d",hostname->s_name, portno);

    x->x_fd = sockfd;
    outlet_float(x->x_obj.ob_outlet, 1);
}



static t_int *streamout_perform(t_int *w)
{
    t_streamout* x = (t_streamout*) (w[1]);
    t_float *in[4];
    char* bp;
    int n; 
    int length;
    int sent = 0;
    int i;
    int chans = x->x_tag.channels;

    
    for (i=0;i<chans;i++)
      in[i] =  (t_float *)(w[2+i]);
    n = (int)(w[2+i]);
    length = n*SF_SIZEOF(x->x_tag.format)*chans;

    if (n != x->nsamples)
      streamout_tempbuf(x,n);

    x->x_tag.framesize=length;
    x->x_tag.count++;
    /* format the buffer */

    switch (x->x_tag.format) {
    case SF_FLOAT: {
	 float* cibuf =(float*) x->cbuf;
	 bp = (char*) x->cbuf;
	 for (i=0;i<chans;i++)
	   while (n--) 
	     *cibuf++ = *(in[i]++);
	 break;
    }
    case SF_16BIT: {
	 short* cibuf =(short*) x->cbuf;
	 bp = (char*) x->cbuf;
	 for (i=0;i<chans;i++)
	   while (n--) 
	     *cibuf++ = (short) 32767.0 * *in[i]++;
	 break;
    }
    case SF_8BIT: {
	 unsigned char*  cbuf = (char*)  x->cbuf;
	 bp = (char*) x->cbuf;
	 for (i=0;i<chans;i++)
	   while (n--) 
	     *cbuf++ = (unsigned char)(127. * (1.0 + *in[i]++));
	 break;
    }
    default:
	 break;
    }

    if (x->x_fd > 0) {
	 /* send the format tag */
	 
#ifdef __APPLE__
		if (send(x->x_fd,(char*)&x->x_tag,sizeof(t_tag),SO_NOSIGPIPE) < 0)
#elif defined unix
		if (send(x->x_fd,(char*)&x->x_tag,sizeof(t_tag),/*MSG_DONTWAIT|*/MSG_NOSIGNAL) < 0)
#else
		if (send(x->x_fd,(char*)&x->x_tag,sizeof(t_tag),0) < 0)
#endif
		{
	      sys_sockerror("streamout");
	      streamout_disconnect(x);
	      return (w+4);
	 }
	 
	 /* send the buffer */
	 
	 for (sent = 0; sent < length;) {
	      int res = 0;
#ifdef __APPLE__
		  res = send(x->x_fd, bp, length-sent, SO_NOSIGPIPE);
#elif defined unix
		  res = send(x->x_fd, bp, length-sent, /*MSG_DONTWAIT|*/MSG_NOSIGNAL);
#else
		  res = send(x->x_fd, bp, length-sent, 0);
#endif
	      if (res <= 0)
	      {
		   sys_sockerror("streamout");
		   streamout_disconnect(x);
		   break;
	      }
	      else
	      {
		   sent += res;
		   bp += res;
	      }
	 }
    }
    return (w+3+chans);
}



static void streamout_dsp(t_streamout *x, t_signal **sp)
{
  switch (x->x_tag.channels) {
  case 1:
    dsp_add(streamout_perform, 3, x,sp[0]->s_vec, sp[0]->s_n);
    post("one channel mode");
    break;
  case 2:
    dsp_add(streamout_perform, 4, x,sp[0]->s_vec, sp[1]->s_vec,sp[0]->s_n);
    post("two channel mode");
    break;
  case 4:
    dsp_add(streamout_perform, 6, x,sp[0]->s_vec, sp[1]->s_vec,
	    sp[2]->s_vec,sp[3]->s_vec,sp[0]->s_n);
    post("four channel mode");
    break;
  default:
    post("streamout: %d channels not supported",x->x_tag.channels);
  }
}



static void streamout_format(t_streamout *x,t_symbol* form) 
{
    if (!strncmp(form->s_name,"float",5))
      x->x_tag.format = (int) SF_FLOAT;

    if (!strncmp(form->s_name,"16bit",5))
      x->x_tag.format = (int) SF_16BIT;

    if (!strncmp(form->s_name,"8bit",4))
      x->x_tag.format = (int) SF_8BIT;


    post ("format set to %s", form->s_name); 
}



static void streamout_host(t_streamout *x,t_symbol* host) 
{
     if (host != &s_)
	  x->hostname = host;

     if (x->x_fd >= 0) {
	  streamout_connect(x,x->hostname,(float) x->portno); 
     }
}




static void streamout_float(t_streamout* x,t_float arg)
{
     if (arg == 0.0)
	  streamout_disconnect(x);
     else
	  streamout_connect(x,x->hostname,(float) x->portno);
}



static void *streamout_new(t_symbol* prot,float channels)
{
    t_streamout *x = (t_streamout *)pd_new(streamout_class);
    outlet_new(&x->x_obj, &s_float);

    if (channels == 0) channels = 1;

    x->hostname = gensym("localhost");
    x->portno = 3000;
    x->x_fd = -1;
    x->x_protocol = SOCK_STREAM;

    if (prot != &s_)
	 if (!strncmp(prot->s_name,"udp",3))
	      x->x_protocol = SOCK_DGRAM;

    x->x_tag.format = SF_FLOAT;
    x->x_tag.channels = channels;
    x->x_tag.version = 1;
    x->cbuf = NULL;
    streamout_tempbuf(x,64);
    return (x);
}



static void streamout_free(t_streamout* x)
{
     if (x->cbuf && x->tbufsize) freebytes(x->cbuf,x->tbufsize);
}



void streamout_tilde_setup(void)
{
    streamout_class = class_new(gensym("streamout~"), (t_newmethod) streamout_new, (t_method) streamout_free,
    	sizeof(t_streamout), 0, A_DEFSYM,A_DEFFLOAT, 0);
    class_addmethod(streamout_class, (t_method) streamout_connect,
    	gensym("connect"), A_SYMBOL, A_DEFFLOAT, 0);
    class_addmethod(streamout_class, (t_method) streamout_disconnect,
    	gensym("disconnect"), 0);
    class_addfloat(streamout_class,streamout_float);
    class_addmethod(streamout_class, nullfn, gensym("signal"), 0);
    class_addmethod(streamout_class, (t_method) streamout_dsp, gensym("dsp"), 0);
    class_addmethod(streamout_class, (t_method)streamout_format,gensym("format"),A_SYMBOL,0);
    class_addmethod(streamout_class, (t_method)streamout_host,gensym("host"),A_DEFSYM,0);

}
