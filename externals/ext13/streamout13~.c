#include "m_pd.h"
#include "stream13.h"

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

/* these pragmas are only used for MSVC, not MinGW or Cygwin <hans@at.or.at> */
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


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


/* ------------------------ streamout13~ ----------------------------- */

/*
#define A1 (4 * (3.14159265/2))
#define A3 (64 * (2.5 - 3.14159265))
#define A5 (1024 * ((3.14159265/2) - 1.5))
*/

static t_class *streamout13_class;

typedef struct _streamout13
{
     t_object x_obj;
     int x_fd;
     int x_protocol;
     int x_format;
     int x_realformat;
     t_symbol* hostname;
     int portno;
     int x_n;
     short* cbuf;
     int nsamples;
     int blockspersend;
     int blockssincesend;
     int newblockspersend;
} t_streamout13;


static void streamout13_connect(t_streamout13 *x, t_symbol *hostname,
    t_floatarg fportno)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno = fportno;
    x->hostname = hostname;
    x->portno = (int) fportno;

    if (x->x_fd >= 0)
    {
    	error("streamout13_connect: already connected - reconnecting");
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
        outlet_float(x->x_obj.ob_outlet, 0);
    }

    	/* create a socket */
    sockfd = socket(AF_INET, x->x_protocol, 0);
    if (sockfd < 0)
    {
	 post("streamout13: Connection to %s on port %d failed",hostname->s_name,portno); 
	 sys_sockerror("socket");
	 return;
    }
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
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

static void streamout13_disconnect(t_streamout13 *x)
{
    if (x->x_fd >= 0)
    {
    	sys_closesocket(x->x_fd);
    	x->x_fd = -1;
    	outlet_float(x->x_obj.ob_outlet, 0);
    }
}

static void streamout13_tempbuf(t_streamout13 *x,int size) {
     int fullsize = size *  x->x_n;
     if (!x->cbuf)                                          
          x->cbuf = getbytes(fullsize*sizeof(t_float)*x->blockspersend+4);
     else                                        
          x->cbuf = resizebytes(x->cbuf,x->nsamples*sizeof(short),fullsize*sizeof(t_float)*x->blockspersend+4);
     x->nsamples = size;                                                              
     x->blockssincesend=0;
}

static t_int *streamout13_perform(t_int *w)
{
  t_streamout13* x = (t_streamout13*) (w[1]);
  int n = (int)(w[2]);
  int i,j,res = 0;
  int offset = 3;
  int sent = 0;
  int length ;
  char* buf = (char *)(w[2]); 
  short* cibuf;
  char* bp;
#ifndef _WIN32
  t_float *in[x->x_n];
#else
  t_float** in = (t_float**) malloc(x->x_n * sizeof(t_float*));
#endif

   for (i=0;i < x->x_n;i++) {
      in[i] = (t_float *)(w[offset+i]);
    }


   if (x->x_fd > 0){

     if (n != x->nsamples)
        streamout13_tempbuf(x,n);

  
     /* format the buffer */
/*     
     cibuf = x->cbuf + x->blockssincesend * n * x->x_n + 2; 
     while (n--){
        for (j=0;j<x->x_n;j++){
            *cibuf++ = (short) 32767.0 * *(in[j]++);
        }
     }
*/

    switch (x->x_format) {
    case SF_FLOAT:{
         t_float* fbuf = (t_float*) x->cbuf + x->blockssincesend * n * x->x_n + 1; 
         while (n--){ 
            for (j=0;j<x->x_n;j++){
                *fbuf++ = *(in[j]++);
            }      
         }         
         break;
    }
    case SF_16BIT: {
         short* sbuf =(short*) x->cbuf + x->blockssincesend * n * x->x_n + 2; /*2 extra bytes for format & grain */
         while (n--){
            for (j=0;j<x->x_n;j++){
                *sbuf++ = (short) 32767.0 * *(in[j]++);
            }
         }
         break;
    }
    case SF_8BIT: {
//         signed char*  cbuf = (signed char*) ibuf;
         signed char*  cbuf = (signed char*) x->cbuf + x->blockssincesend * n * x->x_n + 4; 
         while (n--) 
              for (j=0;j<x->x_n;j++){
                  *cbuf++ = (127.0 * *in[j]++);
              }
         }
         break;
    default:
         break;
    }

     if (!(x->blockssincesend < x->blockspersend - 1)){


        x->blockssincesend=0;
        if (x->x_realformat !=x->x_format){
           x->x_realformat = x->x_format;
           x->blockspersend = (int) (x->blockspersend * 4 / SF_SIZEOF(x->x_format)) ;
           x->nsamples=0;
//           post ("formatchange:%d",x->x_format);
        }
        if (x->blockspersend != (int) (x->newblockspersend * 4 / SF_SIZEOF(x->x_realformat)) ){
          x->blockspersend = (int) (x->newblockspersend * 4 / SF_SIZEOF(x->x_realformat) ) ;
          x->nsamples=0;
//          post ("grainchange:%d",x->blockspersend);
        }                                         
        cibuf=x->cbuf;
        *cibuf=(short)x->x_realformat;
//        *x->cbuf=(short)x->x_realformat;
        cibuf=x->cbuf + 1;
        *cibuf=x->blockspersend;
//        post ("out:cibuf:%d",*cibuf);


         /* send the buffer */
 
        length = x->nsamples *  x->x_n * x->blockspersend *  SF_SIZEOF(x->x_realformat) + 4; 
//        outlet_float(x->x_out2,length);
//        post ("out - length:%d",length);

        for (bp = buf, sent = 0; sent < length;) {
                      res = send(x->x_fd, (char*) x->cbuf, length - sent , 0);
           if (res <= 0)
           {
               sys_sockerror("streamout13~-error");
               streamout13_disconnect(x);
               break;
           } 
           else           
           {
               sent += res;
               bp += res;
           }          
        }
     }else{
        x->blockssincesend++;
     }  
  }
 // post ("b-s-s:%d, length:%d, last:%d, prev:%d",x->blockssincesend,length,*cibuf,prev);
#ifdef _WIN32
  free(in);
#endif
  return (w + 2 + i * 2);
}

static void streamout13_dsp(t_streamout13 *x, t_signal **sp)
{
/*
    dsp_add(streamout13_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
*/
  int i;
  t_int** myvec = getbytes(sizeof(t_int)*(x->x_n + 3));
   
  myvec[0] = (t_int*)x;
  myvec[1] = (t_int*)sp[0]->s_n;
  for (i=0;i < x->x_n/*+1*/;i++)
    myvec[2 + i] = (t_int*)sp[i]->s_vec;
  dsp_addv(streamout13_perform, x->x_n + 3, (t_int*)myvec);
  freebytes(myvec,sizeof(t_int)*(x->x_n + 3));
}

static void streamout13_grain(t_streamout13 *x,t_floatarg grain)
{
    if ((grain > 0) && (grain < 24)){
      x->newblockspersend=(int) grain;
      post ("streamout13~:grainsize set to %d blocks",(int) grain);
    }

}

static void streamout13_format(t_streamout13 *x,t_symbol* form) 
{
    if (!strncmp(form->s_name,"float",5))
      x->x_format = (int) SF_FLOAT;

    if (!strncmp(form->s_name,"16bit",5))
      x->x_format = (int) SF_16BIT;

    if (!strncmp(form->s_name,"8bit",4))
      x->x_format = (int) SF_8BIT;


    post ("streamout13~:format set to %s", form->s_name); 
}

static void streamout13_host(t_streamout13 *x,t_symbol* host) 
{
     if (host != &s_){
  	x->hostname = host;
     	if (x->x_fd >= 0) {
    	 	  streamout13_disconnect(x);
		  streamout13_connect(x,x->hostname,(float) x->portno); 
     	}
    }
}


static void streamout13_float(t_streamout13* x,t_float arg)
{

     if (arg == 0.0)
	  streamout13_disconnect(x);
     else
	  streamout13_connect(x,x->hostname,(float) x->portno);
}


static void *streamout13_new(t_symbol* hostname,t_floatarg portno,t_floatarg xn)
{
    int i;
    t_streamout13 *x = (t_streamout13 *)pd_new(streamout13_class);
    outlet_new(&x->x_obj, &s_float);
//    x->x_out2 = outlet_new(&x->x_obj, &s_float);
    if (hostname == &s_)
	 x->hostname = gensym("localhost");
    else 
	 x->hostname = hostname;

    if (portno == 0.0) 
	 x->portno = 3000;
    else
	 x->portno = portno;
    if (xn < 1)
         x->x_n = 1;
    else
         x->x_n = xn;

    for (i = 0 ;i < x->x_n - 1;i++){
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
/*        post("creating ~input nr:%d",i);*/

    }

    x->x_fd = -1;
    x->x_protocol = SOCK_DGRAM;
    x->x_format = SF_16BIT;
    x->x_realformat = SF_16BIT;
    x->cbuf = NULL;
    x->blockspersend=16;
    x->newblockspersend=8;
    x->blockssincesend=0;
    streamout13_tempbuf(x,64);
    return (x);
}

void streamout13_setup(void)
{
    streamout13_class = class_new(gensym("streamout13~"), (t_newmethod) streamout13_new, 0,
    	sizeof(t_streamout13), 0, A_DEFSYM, A_DEFFLOAT,A_DEFFLOAT, 0);
    
    class_addmethod(streamout13_class, (t_method) streamout13_connect,gensym("connect"), A_SYMBOL, A_DEFFLOAT, 0);
    class_addmethod(streamout13_class, (t_method) streamout13_disconnect, gensym("disconnect"), 0);
    class_addfloat(streamout13_class,streamout13_float);
    class_addmethod(streamout13_class, nullfn, gensym("signal"), 0);
    class_addmethod(streamout13_class, (t_method)streamout13_format,gensym("format"),A_SYMBOL,0);
    class_addmethod(streamout13_class, (t_method)streamout13_grain,gensym("grain"),A_DEFFLOAT,0);
    class_addmethod(streamout13_class, (t_method) streamout13_dsp, gensym("dsp"), 0);
    class_addmethod(streamout13_class, (t_method)streamout13_host,gensym("host"),A_DEFSYM,0);

}

void streamout13_tilde_setup()
{
  streamout13_setup();
}
