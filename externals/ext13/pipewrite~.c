#include <m_pd.h>
#include "g_canvas.h"

#ifdef __gnu_linux__
#include <sys/mman.h>
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


/*
 * ------------------------------------------- pipewrite_tilde~ -------------------------------
 */

#define MAX_CHANS 4
#define BLOCKTIME 0.001
#define uint32 unsigned int
#define uint16 unsigned short
#define THERATE 22050

typedef struct _wave
{
    char  w_fileid[4];	    	    /* chunk id 'RIFF'            */
    uint32 w_chunksize;     	    /* chunk size                 */
    char  w_waveid[4];	    	    /* wave chunk id 'WAVE'       */
    char  w_fmtid[4];	    	    /* format chunk id 'fmt '     */
    uint32 w_fmtchunksize;   	    /* format chunk size          */
    uint16  w_fmttag;	    	    /* format tag, 1 for PCM      */
    uint16  w_nchannels;    	    /* number of channels         */
    uint32 w_samplespersec;  	    /* sample rate in hz          */
    uint32 w_navgbytespersec; 	    /* average bytes per second   */
    uint16  w_nblockalign;    	    /* number of bytes per sample */
    uint16  w_nbitspersample; 	    /* number of bits in a sample */
    char  w_datachunkid[4]; 	    /* data chunk id 'data'       */
    uint32 w_datachunksize;         /* length of data chunk       */
} t_wave;


static t_class *pipewrite_tilde_class;

typedef struct _pipewrite_tilde
{
     t_object x_obj;
     t_symbol* filename;
     int x_file;
     int finalize;
     t_int rec;
     t_int x_channels;
     t_int  size;
     t_glist * x_glist;
     t_int x_blocked;
     t_int x_blockwarn;
     short maxval;
} t_pipewrite_tilde;

static void pipewrite_tilde_wave_setup(t_pipewrite_tilde* x,t_wave* w) 
{
    strncpy(w->w_fileid,"RIFF",4);	    	    /* chunk id 'RIFF'     */
    w->w_chunksize = x->size + sizeof(t_wave) - 8;     	    /* chunk size  */
    strncpy(w->w_waveid,"WAVE",4);	    	    /* wave chunk id 'WAVE'  */
    strncpy(w->w_fmtid,"fmt ",4);	    	    /* format chunk id 'fmt '*/
    w->w_fmtchunksize = 16;   	    /* format chunk size          */
    w->w_fmttag = 1;	    	    /* format tag, 1 for PCM      */
    w->w_nchannels = x->x_channels;    	    /* number of channels         */
    w->w_samplespersec = THERATE;  	    /* sample rate in hz          */
    w->w_navgbytespersec = THERATE * x->x_channels*2; 	    /* average bytes per second   */
    w->w_nblockalign = 4;    	    /* number of bytes per sample */
    w->w_nbitspersample = 16; 	    /* number of bits in a sample */
    strncpy(w->w_datachunkid,"data",4); 	    /* data chunk id 'data'       */
    w->w_datachunksize = THERATE * 60 * 60 * 24 *365 /* x->size*/ ;         /* length of data chunk       */

}



static void pipewrite_tilde_close(t_pipewrite_tilde *x)
{
     if (x->x_file > 0) {
	  t_wave w;
	  pipewrite_tilde_wave_setup(x,&w);
	  lseek(x->x_file,0,SEEK_SET);
	  write(x->x_file,&w,sizeof(w));
	  close(x->x_file);
     }
     x->x_file = -1;
     x->size=0;
}


static void pipewrite_tilde_open(t_pipewrite_tilde *x,t_symbol *filename)
{
     char fname[MAXPDSTRING];
     t_wave w;

     if (filename == &s_) {
	  post("pipewrite_tilde: open without filename");
	  return;
     }

     canvas_makefilename(glist_getcanvas(x->x_glist), filename->s_name,
			 fname, MAXPDSTRING);
     x->finalize = 0;
     x->x_blocked = 0;
     x->filename = filename;
     x->maxval=0;
     x->size=0;
     post("pipewrite_tilde: filename = %s",x->filename->s_name);

/*
     pipewrite_tilde_close(x);
*/
/*     if ((x->x_file = open(fname,O_WRONLY | O_CREAT | O_NONBLOCK ,0664)) < 0)*/
     if ((x->x_file = open(fname,O_WRONLY | O_CREAT | O_NONBLOCK ,0664)) < 0)

     {
	  error("can't create %s",fname);
	  return;
     }

     
     pipewrite_tilde_wave_setup(x,&w);
    write(x->x_file,&w,sizeof(w));


}

static void pipewrite_tilde_block(t_pipewrite_tilde *x, t_floatarg f)
{
     x->x_blockwarn = f;
}


static void pipewrite_tilde_float(t_pipewrite_tilde *x, t_floatarg f)
{
  int t = f;
  if (t) {
       if ( !(x->x_file > 0)){
           post ("pipewrite_tilde:dont have a file to record to");
       }else{
           post("pipewrite_tilde: start", f);
           x->rec=1;
       }
  }
  else {
       post("pipewrite_tilde: stop", f); 
       x->rec=0;
       x->finalize=1;
  }

}


static short out[4*64];

static t_int *pipewrite_tilde_perform(t_int *w)
{
     t_pipewrite_tilde* x = (t_pipewrite_tilde*)(w[1]);
     t_float * in[4];
     int c = x->x_channels;
     int i,num,n;
     short* tout = out;
     int ret;
     double timebefore,timeafter;
     double late;

     for (i=0;i < c;i++) {
	  in[i] = (t_float *)(w[2+i]);     
     }

     n = num = (int)(w[2+c]);

     /* loop */

     if (x->rec && x->x_file) {

	  while (n--) {
	       for (i=0;i<c;i++)  {
                    if (*(in[i]) > 1. ) { *(in[i]) = 1. ; }
                    if (*(in[i]) < -1. ) { *(in[i]) = -1. ; }
		    *tout++ =  (*(in[i])++ * 32768.);
/*
                     if (abs(*tout)>abs(x->maxval)){
                            x->maxval=*tout;
                            post("new maxval:%d, c:%d",x->maxval,c);
                    }
*/
	       }
	  }
	  
	  timebefore = sys_getrealtime();
	  if ((ret =write(x->x_file,out,sizeof(short)*num*c)) < sizeof(short)*num*c) { 
	       post("pipewrite_tilde: short write %d",ret);

	       }
	  timeafter = sys_getrealtime();
	  late = timeafter - timebefore;
          x->size +=ret;
	  /* OK, we let only 10 ms block here */
	  if (late > BLOCKTIME && x->x_blockwarn) { 
	       post("pipewrite_tilde blocked %f ms",late*1000);
	       x->x_blocked++;
	       if (x->x_blocked > x->x_blockwarn) {
/*		    x->rec = 0;*/
		    post("maximum blockcount %d reached, recording normalerweise stopped (set blockcount with \"block <num>\"",x->x_blockwarn);
	       }
	  }
     }
     if (!x->rec && x->finalize){
         pipewrite_tilde_close(x);
         x->finalize = 0;        
     }

     return (w+3+c);
}



static void pipewrite_tilde_dsp(t_pipewrite_tilde *x, t_signal **sp)
{
     switch (x->x_channels) {
     case 1:
	  dsp_add(pipewrite_tilde_perform, 3, x, sp[0]->s_vec, 
		   sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(pipewrite_tilde_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(pipewrite_tilde_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,
		  sp[2]->s_vec,
		  sp[3]->s_vec,
		  sp[0]->s_n);
	  break;
     }
}

static void pipewrite_tilde_free(t_pipewrite_tilde* x)
{
     pipewrite_tilde_close(x);
}


static void *pipewrite_tilde_new(t_floatarg chan)
{
    t_pipewrite_tilde *x = (t_pipewrite_tilde *)pd_new(pipewrite_tilde_class);
    t_int c = chan;

    if (c<1 || c > MAX_CHANS) c = 1;

    x->x_glist = (t_glist*) canvas_getcurrent();
    x->x_channels = c--;
    post("channels:%d",x->x_channels);
    x->x_file=0;
    x->rec = 0;
    x->finalize = 0;
    x->x_blocked = 0;
    x->x_blockwarn = 10;
    while (c--) {
	 inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }


    return (x);
}

void pipewrite_tilde_setup(void)
{
     pipewrite_tilde_class = class_new(gensym("pipewrite~"), (t_newmethod)pipewrite_tilde_new, (t_method)pipewrite_tilde_free,
    	sizeof(t_pipewrite_tilde), 0,A_DEFFLOAT,0);
     class_addmethod(pipewrite_tilde_class,nullfn,gensym("signal"), 0);
     
     class_addmethod(pipewrite_tilde_class, (t_method) pipewrite_tilde_dsp, gensym("dsp"), 0);
     class_addmethod(pipewrite_tilde_class, (t_method) pipewrite_tilde_open, gensym("open"), A_SYMBOL,A_NULL);
     class_addmethod(pipewrite_tilde_class, (t_method) pipewrite_tilde_close, gensym("close"), 0);
     class_addmethod(pipewrite_tilde_class, (t_method)pipewrite_tilde_block,gensym("block"),A_DEFFLOAT,0);
     class_addfloat(pipewrite_tilde_class, pipewrite_tilde_float);
     
}
