
/* Written by Guenter Geiger <geiger@xdv.org> (C) 1999 */
/* Thanks to Anthony Lee for Windows bug fixes */

#include <m_pd.h>
#include "stream.h"

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#define SOCKET_ERROR -1
#endif

/* these pragmas are only used for MSVC, not MinGW or Cygwin <hans@at.or.at> */
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef unix
#define NONBLOCKED
#endif

#define MAXFRAMES 128
#define MAXFRAMESIZE 256
#define AVERAGENUM 10

/*#define DEBUGMESS(x) x*/
#define  DEBUGMESS(x) 

/* Utility functions */

/* TODO !!!!
  - check udp support 
*/


#ifdef _WIN32
extern int close(int);
#endif
extern void sys_rmpollfn(int fd);
extern sys_addpollfn(int fd, void* fn, void *ptr);

static void sys_sockerror(char *s)
{
#ifdef _WIN32
    int err = WSAGetLastError();
    if (err == 10054) return;
#else
    int err = errno;
#endif
    post("%s: %s (%d)\n", s, strerror(err), err);
}


static void sys_closesocket(int fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}


int setsocketoptions(int sockfd)
{ 
#ifndef _WIN32
    int sockopt = 1;
	 if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const char*) &sockopt, sizeof(int)) < 0) 
	 {
		 DEBUGMESS(post("setsockopt NODELAY failed\n"));
	 }
	 else
	 {
		 DEBUGMESS(post("TCP_NODELAY set"));
	 }
	 
     
     /* if we don`t use REUSEADDR we have to wait under unix until the 
	address gets freed after a close ... this can be very annoying 
	when working with netsend/netreceive GG
     */

     sockopt = 1;    
     if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0)
	  post("setsockopt failed\n");
#endif
	return 0;
}



/* ------------------------ streamin~ ----------------------------- */


static t_class *streamin_class;



typedef struct _streamin
{
     t_object x_obj;
     int x_socket;
     int x_connectsocket;
     int x_nconnections;
     int x_ndrops;
     int x_fd;
     int x_tcp;

     /* buffering */

     int framein;
     int frameout;
     t_frame frames[MAXFRAMES];
     int maxframes;

     int nbytes;
     int counter;
     int average[AVERAGENUM];
     int averagecur;
     int underflow;
} t_streamin;




static void streamin_datapoll(t_streamin *x)
{
     int ret;
     int n;
     t_tag tag;
     int i;

     n = x->nbytes;
     if (x->nbytes == 0) {      /* get the new tag */
	  ret = recv(x->x_socket, (char*) &x->frames[x->framein].tag,sizeof(t_tag),MSG_PEEK);
	  if (ret != sizeof(t_tag)) {
#ifdef _WIN32
            sys_closesocket(x->x_socket);
            sys_rmpollfn(x->x_socket);
            x->x_socket = -1;
#endif
            return;
          }
	  ret = recv(x->x_socket, (char*) &x->frames[x->framein].tag,sizeof(t_tag),0);

	  if ((x->frames[x->framein].tag.framesize - sizeof(t_tag)) > MAXFRAMESIZE) {
	   error("streamin~: got an invalid frame size of %d, maximum is %d\n",
	           x->frames[x->framein].tag.framesize, MAXFRAMESIZE);
	   x->frames[x->framein].tag.framesize = MAXFRAMESIZE + sizeof(t_tag);
	  }

	  x->nbytes = n = x->frames[x->framein].tag.framesize;
     }

     ret = recv(x->x_socket, (char*) x->frames[x->framein].data + x->frames[x->framein].tag.framesize - n, n, 0);
     if (ret > 0)
	  n-=ret;

     x->nbytes = n;
     if (n == 0) {
	  x->counter++;
	  x->framein++;
	  x->framein %= MAXFRAMES;
     }
}

static void streamin_reset(t_streamin* x,t_floatarg frames)
{
     int i;
     x->counter = 0;
     x->nbytes = 0;
     x->framein = 0;
     x->frameout = 0;
     for (i=0;i<AVERAGENUM;i++)
	  x->average[i] = x->maxframes;
     x->averagecur=0;
     if (frames == 0.0)
	  x->maxframes = MAXFRAMES/2;
     else
	  x->maxframes = frames;
     x->underflow = 0;
}


static void streamin_connectpoll(t_streamin *x)
{
    int fd = accept(x->x_connectsocket, 0, 0);

#ifdef NONBLOCKED
    fcntl(fd,F_SETFL,O_NONBLOCK);
#endif
    if (fd < 0) {
	 post("streamin~: accept failed");
	 return;
    }

    if (x->x_socket > 0) {
	 post("streamin~: new connection");
	 sys_closesocket(x->x_socket);
	 sys_rmpollfn(x->x_socket);
    }

    streamin_reset(x,0);
    x->x_socket = fd;
    sys_addpollfn(fd, streamin_datapoll, x);
}


static int streamin_createsocket(t_streamin* x, int portno,t_symbol* prot)
{
    struct sockaddr_in server;
    int sockfd;
    int tcp = x->x_tcp;

     /* create a socket */
    if (!tcp)
      sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    else
      sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
    	sys_sockerror("socket");
    	return (0);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    /* assign server port number */

    server.sin_port = htons((u_short)portno);
    post("listening to port number %d", portno);


    setsocketoptions(sockfd);

	 
    /* name the socket */
    
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
	 sys_sockerror("bind");
	 sys_closesocket(sockfd);
	 return (0);
    }


    if (!tcp) {
	 x->x_socket = sockfd;
	 x->nbytes = 0;
	 sys_addpollfn(sockfd, streamin_datapoll, x);
    }
    else {
	 if (listen(sockfd, 5) < 0) {
	      sys_sockerror("listen");
	      sys_closesocket(sockfd);
	 }
	 else {
	      x->x_connectsocket = sockfd;
	      sys_addpollfn(sockfd, streamin_connectpoll, x);
	 }
    }
    return 1;
}



static void streamin_free(t_streamin *x)
{
     if (x->x_connectsocket > 0) {
	  sys_closesocket(x->x_connectsocket);
	  sys_rmpollfn(x->x_connectsocket);
     }
     sys_rmpollfn(x->x_socket);
     sys_closesocket(x->x_socket);
}


#define QUEUESIZE ((x->framein + MAXFRAMES - x->frameout)%MAXFRAMES)

static t_int *streamin_perform(t_int *w)
{
     t_streamin *x = (t_streamin*) (w[1]);
     t_float *out = (t_float *)(w[2]);
     int n = (int)(w[3]);
     int ret;
     int i = 0;

     if (x->counter < x->maxframes) {
	  return (w+4);
     }

     if (x->framein == x->frameout) {
	  x->underflow++;
	  return w+4;
     }


     /* queue balancing */

     x->average[x->averagecur] = QUEUESIZE;
     x->averagecur++;
     x->averagecur %= AVERAGENUM;

     switch (x->frames[x->frameout].tag.format) {
     case SF_FLOAT: {
	  t_float* buf = (t_float*)(x->frames[x->frameout].data);
	  while (n--) 
	       *out++ = *buf++; 
	  x->frameout++;
	  x->frameout %= MAXFRAMES;
	  break;
     }
     case SF_16BIT:     
     {
	  short* buf = (short*)(x->frames[x->frameout].data);

	  while (n--)
	       *out++ = (float) *buf++*3.051850e-05; 
	  x->frameout++;
	  x->frameout %= MAXFRAMES;

	  break;
     }
     case SF_8BIT:     
     {
	  unsigned char* buf = (char*)(x->frames[x->frameout].data);

	  while (n--)
	       *out++ = (float) (0.0078125 * (*buf++)) - 1.0; 
	  x->frameout++;
	  x->frameout %= MAXFRAMES;
	  break;
     }
     default:
	  post("unknown format %d",x->frames[x->frameout].tag.format);
	  break;
     }

     return (w+4);
}



static void streamin_dsp(t_streamin *x, t_signal **sp)
{
    dsp_add(streamin_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void streamin_print(t_streamin* x)
{
     int i;
     int avg = 0;
     for (i=0;i<AVERAGENUM;i++)
	  avg += x->average[i];
     post("last size = %d, avg size = %d, %d underflows",
	  QUEUESIZE,avg/AVERAGENUM,x->underflow);
}




static void *streamin_new(t_floatarg fportno, t_floatarg prot)
{
    t_streamin *x;
    int i;
    
    if (fportno == 0) fportno = 4267;

    post("port %f",fportno);
    x = (t_streamin *)pd_new(streamin_class);
    
    x->x_connectsocket = -1;
    x->x_socket = -1;
    x->x_tcp = 1;
    outlet_new(&x->x_obj, &s_signal);
    x->x_nconnections = 0;
    x->x_ndrops = 0;
    
    for (i=0;i<MAXFRAMES;i++) {
	 x->frames[i].data = getbytes(MAXFRAMESIZE);
    }
    x->framein = 0;
    x->frameout = 0;
    x->maxframes = MAXFRAMES/2;
    
    if (prot)
	 x->x_tcp = 0;

    streamin_createsocket(x, fportno, gensym("tcp"));

    return (x);
}





void streamin_tilde_setup(void)
{
    streamin_class = class_new(gensym("streamin~"), 
    	(t_newmethod) streamin_new, (t_method) streamin_free,
    	sizeof(t_streamin),  0, A_DEFFLOAT,A_DEFFLOAT, A_NULL);

    class_addmethod(streamin_class, nullfn, gensym("signal"), 0);
    class_addmethod(streamin_class, (t_method) streamin_dsp, gensym("dsp"), 0);
    class_addmethod(streamin_class, (t_method) streamin_print, 
		    gensym("print"), 0);
    class_addmethod(streamin_class, (t_method) streamin_reset, 
		    gensym("reset"),A_DEFFLOAT, 0);
}
