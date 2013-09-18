 /* Written by Guenter Geiger <geiger@debian.org> (C) 1999, adapted and changed to streamin13~ by d13@klingt.org */

#include "m_pd.h"
#include "stream13.h"

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#define SOCKET_ERROR -1
#endif

/* these pragmas are only used for MSVC, not MinGW or Cygwin <hans@at.or.at> */
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define INBUFSIZE 8192
#define MAXDROPS 99

/* Utility functions */

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

/* ------------------------ streamin13~ ----------------------------- */


static t_class *streamin13_class;

typedef struct _streamin13
{
     t_object x_obj;
     int x_connectsocket;
     int x_nconnections;
     int x_ndrops;
     int x_ndone;
     int x_fd;
     int x_format;
     int x_n;
     short* cbuf;
     int nsamples;
//     t_outlet *x_out2; 
     int blocksperreceive;
     int blockssincereceive;
} t_streamin13;


static void streamin13_tempbuf(t_streamin13 *x,int size)
{
     int fullsize = size * x->x_n;   
     if (!x->cbuf)              
          x->cbuf = getbytes(fullsize*sizeof(t_float)*x->blocksperreceive+4);
     else                                          
          x->cbuf = resizebytes(x->cbuf,x->nsamples*sizeof(t_float),fullsize*sizeof(t_float)*x->blocksperreceive+4);
     x->nsamples = size;
     x->blockssincereceive=0;
}


static int streamin13_listen(t_streamin13 *x,int portno)
{
    struct sockaddr_in server;
    int sockfd;
    static int on = 1;
    
#ifndef _WIN32
    shutdown(x->x_connectsocket,SHUT_RDWR);
#else
    shutdown(x->x_connectsocket,SD_BOTH);
#endif
    sys_closesocket(x->x_connectsocket);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        {
             sys_sockerror("streamin-socket");
             x->x_connectsocket = 0;
             return (0);
        }
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;

    #if defined(__linux__) || defined(__APPLE__)
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        post("setsockopt failed\n");
    #endif

    /* assign server port number */
    server.sin_port = htons((u_short)portno);
    post("listening to port number %d", portno);

    /* name the socket */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        sys_sockerror("streamin-bind");
        sys_closesocket(sockfd);
        x->x_connectsocket = 0;
    }else {
        x->x_connectsocket = sockfd;
        x->x_nconnections = 0;
        x->x_ndrops = 0;
        x->x_format = SF_FLOAT;
        x->cbuf = NULL;
        x->blocksperreceive=16;
        x->blockssincereceive=0;
        streamin13_tempbuf(x,64);
    }

	return(0);
}

static void *streamin13_new(t_floatarg fportno ,t_floatarg xn)
{
    int i;
    t_streamin13 *x = (t_streamin13 *)pd_new(streamin13_class);
    struct sockaddr_in server;
    streamin13_listen(x,(int)fportno); 
    outlet_new(&x->x_obj, &s_signal);
    x->x_n = xn;
    for (i = 0 ;i < x->x_n - 1;i++){
         outlet_new(&x->x_obj, gensym("signal"));
    }

    return (x);
}



static void streamin13_free(t_streamin13 *x)
{
    	/* LATER make me clean up open connections */
}





static t_int *streamin13_perform(t_int *w)
{
  t_streamin13* x = (t_streamin13*) (w[1]);
  int offset = 3;
  int i,j;
  int n = (int)(w[2]);
  struct timeval timeout;
  int packsize;
  int ret;
  int length;
  short* cbuf;

#ifndef _WIN32
  t_float *out[x->x_n];
#else
  t_float **out = (t_float**) malloc(x->x_n * sizeof(t_float*));
#endif

#ifndef _WIN32
     fd_set fdset;
#endif
   for (i=0;i < x->x_n;i++) {
      out[i] = (t_float *)(w[offset+i]);
   }
   if (x->x_connectsocket){
	     timeout.tv_sec = 0;
	     timeout.tv_usec = 0;
	
	     if (n != x->nsamples)
		  streamin13_tempbuf(x,n);
  

	//     cbuf = x->cbuf + x->blockssincereceive * n * x->x_n;
     
	#ifndef _WIN32
	     FD_SET(x->x_connectsocket,&fdset);
	     if (!select(x->x_connectsocket+1,&fdset,NULL,NULL,&timeout) 
		 || !FD_ISSET(x->x_connectsocket,&fdset)) {
	       x->x_ndrops++;
	       if (x->x_ndrops <= MAXDROPS) { 
/*		 post("streamin13: drop"); */
		 while (n--){
	            for (j=0;j<x->x_n;j++){
	               *(out[j]++) = 0.0;
	            }
	         }
	       }
	 free(out);	      
	 return (w+offset+1+i);
	     } else {
	        x->x_ndone++;
	     }
	#endif
	     
	     if (x->x_ndrops > MAXDROPS) {
		  post("streamin13: dropped %d signal vectors, received %d sigvecs.",x->x_ndrops,x->x_ndone);
		  x->x_ndrops=0;
	          x->x_ndone=0;
	     }
	     if (!(x->blockssincereceive < x->blocksperreceive -1))    {
	     
	        length=x->x_n * x->nsamples * SF_SIZEOF(x->x_format) *  x->blocksperreceive +4;
	        ret = recv(x->x_connectsocket, (char*) x->cbuf,length, 0);
	        x->blockssincereceive=0;
	        if (x->x_format != *x->cbuf){
	            x->x_format= (short) *x->cbuf;
	        }
	        cbuf = (short*) x->cbuf + 1;
	        if (x->blocksperreceive != (short) *cbuf){
	           x->blocksperreceive=(short) *cbuf;
	           streamin13_tempbuf(x,x->nsamples);
	        }
	     }else{
	        x->blockssincereceive++;
	     }
	     packsize=n * x->x_n * sizeof(short) *  x->blocksperreceive + 2;
	    switch (x->x_format) {
	         case SF_FLOAT:{
	              t_float* fbuf = (t_float*) x->cbuf + x->blockssincereceive * n * x->x_n + 1;
	              while (n--){
	                  for (j=0;j<x->x_n;j++){     
	                      *out[j]++ =  *fbuf++;          
	                  }
	              }
	    
	              break;
	         }
	         case SF_16BIT:     
	         {
	              cbuf = x->cbuf + x->blockssincereceive * n * x->x_n + 2;
	              while (n--){
	                  for (j=0;j<x->x_n;j++){
	                      *out[j]++ = (float) *cbuf++/32767.; 
	                  }
	              }
	              break;
	         }
	         case SF_8BIT: {
	              signed char* cbuf = (signed char*) x->cbuf + x->blockssincereceive * n * x->x_n + 4;
	              while (n--){
	                  for (j=0;j<x->x_n;j++){ 
	                      *out[j]++ = (float) *cbuf++/127.; 
	                  }
	              }
	              break;
	         }
	         default:
	              break;
	     }
     }
#ifdef _WIN32
	free(out);
#endif 
     return (w+offset+1+i);
}
	
	
	
static void streamin13_dsp(t_streamin13 *x, t_signal **sp)
{
  int i;
  t_int** myvec = getbytes(sizeof(t_int)*(x->x_n + 3));
   
  myvec[0] = (t_int*)x;
  myvec[1] = (t_int*)sp[0]->s_n;
  for (i=0;i < x->x_n;i++)
    myvec[2 + i] = (t_int*)sp[i]->s_vec;
  dsp_addv(streamin13_perform, x->x_n + 3, (t_int*)myvec);
  freebytes(myvec,sizeof(t_int)*(x->x_n + 3));

}

static void streamin13_port(t_streamin13 *x,t_floatarg port) 
{
   streamin13_listen(x, (int)port); 
}

void streamin13_setup(void)
{
    streamin13_class = class_new(gensym("streamin13~"), 
    	(t_newmethod) streamin13_new, (t_method) streamin13_free,
    	sizeof(t_streamin13), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    
    class_addmethod(streamin13_class, nullfn, gensym("signal"), 0);
    class_addmethod(streamin13_class, (t_method) streamin13_dsp, gensym("dsp"), 0);
    class_addmethod(streamin13_class, (t_method)streamin13_port,gensym("port"),A_DEFFLOAT,0);
}

void streamin13_tilde_setup()
{
  streamin13_setup();
}
