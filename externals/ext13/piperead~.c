#include "m_pd.h"

#ifdef __gnu_linux__
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* ------------------------ piperead_tilde~ ----------------------------- */

#ifdef _WIN32
#define BINREADMODE "rb"
#else
#define BINREADMODE "r"
#endif

static t_class *piperead_tilde_class;


typedef struct _piperead_tilde
{
     t_object x_obj;
     void*     x_mapaddr;
     int       x_fd;
     short x_sample;
     t_int   x_play;
     t_int   x_channels;
     int buflen;
     short  buf[32768];
     int  readpointer;
     int writepointer;
} t_piperead_tilde;


void piperead_tilde_open(t_piperead_tilde *x,t_symbol *filename)
{
/*     struct stat  fstate;*/
     char fname[MAXPDSTRING];
     canvas_makefilename(canvas_getcurrent(), filename->s_name,
			 fname, MAXPDSTRING);

     /* close the old file */
     if (x->x_fd >= 0) close(x->x_fd);

     if ((x->x_fd = open(fname,( O_NONBLOCK | O_RDONLY))) < 0)
     {
	  error("can't open %s",fname);
	  return;
     }

    for (x->writepointer=0;x->writepointer<(x->buflen*0.9);x->writepointer++){
      read (x->x_fd,&x->buf[x->writepointer],2);
    }
/*    post ("prebuffering done");*/
    
}

#define MAX_CHANS 4

t_int *piperead_tilde_perform(t_int *w)
{
    t_piperead_tilde*  x = (t_piperead_tilde*)(w[1]);
    int c = x->x_channels;
    int i;
    int erg=0;
    int n;
    t_float* out[MAX_CHANS];
    for (i=0;i<c;i++)      
      out[i] = (t_float *)(w[3+i]);
    n = (int)(w[3+c]);
        
    while (n--) 
    if (x->x_play){ 
      for (i=0;i<c;i++)  
      { 
        if (++x->readpointer>x->buflen){
          x->readpointer=0;
        } 
        if (erg != EAGAIN){
          if (++x->writepointer>x->buflen){
            x->writepointer=0;
          } 
        }
        *out[i]++ = x->buf[x->readpointer]/32768.;
        erg = read (x->x_fd,&x->buf[x->writepointer],2);         
      }
    }
    else
    {
      for (i=0;i<c;i++)  *out[i]++= 0.;
    }
    return (w+c+4);
}


static void piperead_tilde_float(t_piperead_tilde *x, t_floatarg f)
{
     int t = f;
     if (t) {
	  x->x_play=1;
     }
     else {
	  x->x_play=0;
     }

}

static void piperead_tilde_dsp(t_piperead_tilde *x, t_signal **sp)
{
     switch (x->x_channels) {
     case 1:
	  dsp_add(piperead_tilde_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(piperead_tilde_perform, 5, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(piperead_tilde_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec,
		  sp[3]->s_vec,sp[4]->s_vec,
		  sp[0]->s_n);
	  break;
     }
}


static void *piperead_tilde_new(t_floatarg chan, t_floatarg buflen)
{
    t_piperead_tilde *x = (t_piperead_tilde *)pd_new(piperead_tilde_class);
    t_int c = chan;
    t_int bl = buflen;

    if (c<1 || c > MAX_CHANS) c = 1;
    if (bl<8 || bl > 32767) bl = 256;
    x->x_fd = -1;
    x->x_channels = c;
    x->buflen=bl;
    x->x_play = 0;

    while (c--) {
	 outlet_new(&x->x_obj, gensym("signal"));
    }
    return (x);
}

void piperead_tilde_setup(void)
{
    piperead_tilde_class = class_new(gensym("piperead~"), (t_newmethod)piperead_tilde_new, 0,
    	sizeof(t_piperead_tilde), 0,A_DEFFLOAT,A_DEFFLOAT,0);
    class_addmethod(piperead_tilde_class, nullfn, gensym("signal"), 0);
    
    class_addmethod(piperead_tilde_class, (t_method) piperead_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(piperead_tilde_class, (t_method) piperead_tilde_open, gensym("open"), A_SYMBOL,A_NULL);
    class_addfloat(piperead_tilde_class, piperead_tilde_float);
}


