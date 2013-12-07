/* original source (sfwrite~.c) by  Guenter Geiger <geiger@epy.co.at> */
/* added buffering for write-actions to reduce disc-activity <dieb13 at klingt.org>*/

 
#include "m_pd.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/mman.h>
#else
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>


#ifdef _WIN32
#define BINREADMODE "rb"
#define OPENPARAMS O_WRONLY | O_CREAT | O_TRUNC 
#else
#define BINREADMODE "r"
#define OPENPARAMS O_WRONLY | O_CREAT | O_TRUNC 
#endif
#define MAX_CHANS 4


/*
 * ------------------------------------------- sfwrite13~ -------------------------------
 */

#define BLOCKTIME 0.01
#define uint32 unsigned int
#define uint16 unsigned short

static t_class *sfwrite13_class;

typedef struct _sfwrite13
{
     t_object x_obj;
     t_symbol* filename;
     int x_file;
     int do_close;
     short* cbuf;
     short* cbufptr;
     uint32 bufsize;
     t_int rec;
     t_int x_channels;
     uint32  size;
     t_int x_blocked;
     t_int x_blockwarn;
} t_sfwrite13;

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

static void sfwrite13_tempbuf(t_sfwrite13 *x,int size)
{
     post("channels:%d",x->x_channels);
     if (!x->cbuf){
       x->cbuf = getbytes(x->x_channels * size*sizeof(short));
       x->bufsize = size;
     }else{
       x->cbuf = resizebytes(x->cbuf,x->bufsize * x->x_channels * sizeof(short), x->x_channels * size * sizeof(short));
       x->bufsize = size;
     }
     x->cbufptr = x->cbuf;
}

static void sfwrite13_wave_setup(t_sfwrite13* x,t_wave* w) 
{
     
    strncpy(w->w_fileid,"RIFF",4);	    	    /* chunk id 'RIFF'     */
    w->w_chunksize = x->size + sizeof(t_wave) -8;     	    /* chunk size  */
    strncpy(w->w_waveid,"WAVE",4);	    	    /* wave chunk id 'WAVE'  */
    strncpy(w->w_fmtid,"fmt ",4);	    	    /* format chunk id 'fmt '*/
    w->w_fmtchunksize = 16;   	    /* format chunk size          */
    w->w_fmttag = 1;	    	    /* format tag, 1 for PCM      */
    w->w_nchannels = x->x_channels;    	    /* number of channels         */
    w->w_samplespersec = 44100;  	    /* sample rate in hz          */
    w->w_navgbytespersec = 44100*x->x_channels*2; 	    /* average bytes per second   */
    w->w_nblockalign = 4;    	    /* number of bytes per sample */
    w->w_nbitspersample = 16; 	    /* number of bits in a sample */
    strncpy(w->w_datachunkid,"data",4); 	    /* data chunk id 'data'*/
    w->w_datachunksize = x->size;         /* length of data chunk       */
}



static void sfwrite13_dowrite(t_sfwrite13 *x)
{
     double timebefore,timeafter,late;
     int ret;

     timebefore = sys_getrealtime();
     if ((ret =write(x->x_file,x->cbuf,sizeof(short)*(x->cbufptr - x->cbuf))) < (signed int)sizeof(short)*(x->cbufptr - x->cbuf)) {
	       post("sfwrite13: short write %d",ret);
     }
     timeafter = sys_getrealtime();
     late = timeafter - timebefore;

     if (late > BLOCKTIME && x->x_blockwarn) { 
         post("sfwrite13 blocked %f ms",late*1000);
         x->x_blocked++;
         if (x->x_blocked > x->x_blockwarn) {
  	    x->rec = 0;
  	    post("maximum blockcount %d reached, recording stopped (set blockcount with \"block <num>\"",x->x_blockwarn);
         }
     }
     x->size += ret ;
     x->cbufptr = x->cbuf;

}

static void sfwrite13_close(t_sfwrite13 *x)
{
     if (x->x_file > 0) {
	  t_wave w;
	  sfwrite13_dowrite(x);
	  sfwrite13_wave_setup(x,&w);
	  lseek(x->x_file,0,SEEK_SET);
	  write(x->x_file,&w,sizeof(w));
	  close(x->x_file);
     }
     x->x_file = -1;
}


static void sfwrite13_open(t_sfwrite13 *x,t_symbol *filename)
{
     char fname[MAXPDSTRING];

     if (filename == &s_) {
	  post("sfwrite13: open without filename");
	  return;
     }

     canvas_makefilename(canvas_getcurrent(), filename->s_name,
			 fname, MAXPDSTRING);

     x->x_blocked = 0;
     x->filename = filename;
     post("sfwrite13: filename = %s",x->filename->s_name);

     sfwrite13_close(x);

     if ((x->x_file = sys_open(fname,OPENPARAMS,0664)) < 0)
     {
	  error("can't create %s",fname);
	  return;
     }

     /* skip the header */

     lseek(x->x_file,sizeof(t_wave),SEEK_SET);
     x->size = 0;
     x->do_close=0;


}

static void sfwrite13_block(t_sfwrite13 *x, t_floatarg f)
{
     x->x_blockwarn = f;
}


static void sfwrite13_float(t_sfwrite13 *x, t_floatarg f)
{
  int t = f;
  if (t) {
       post("sfwrite13: start", f); 
       x->rec=1;
  }
  else {
       post("sfwrite13: stop", f); 
       x->rec=0;
       x->do_close  = 1;
  }

}



static t_int *sfwrite13_perform(t_int *w)
{
     t_sfwrite13* x = (t_sfwrite13*)(w[1]);
     t_float * in[4];
     int c = x->x_channels;
     int i,num,n;

     for (i=0;i < c;i++) {
	  in[i] = (t_float *)(w[2+i]);     
     }

     n = num = (int)(w[2+c]);

     if (x->rec && x->x_file) {
	  while (n--) {
	       for (i=0;i<c;i++)  {
                    *x->cbufptr++ = (short)(*(in[i])++ * 32768.);
	       }
	       if ((x->cbufptr - x->cbuf) > (int)(x->bufsize )) {sfwrite13_dowrite(x);}
	  }
     }

     if (x->do_close) {
         sfwrite13_close(x);
     }

     return (w+3+c);
}



static void sfwrite13_dsp(t_sfwrite13 *x, t_signal **sp)
{
     switch (x->x_channels) {
     case 1:
	  dsp_add(sfwrite13_perform, 3, x, sp[0]->s_vec, 
		   sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(sfwrite13_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(sfwrite13_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,
		  sp[2]->s_vec,
		  sp[3]->s_vec,
		  sp[0]->s_n);
	  break;
     }
}

static void sfwrite13_free(t_sfwrite13* x)
{
     sfwrite13_close(x);
}


static void *sfwrite13_new(t_floatarg chan)
{
    t_sfwrite13 *x = (t_sfwrite13 *)pd_new(sfwrite13_class);
    t_int c = chan;

    if (c<1 || c > MAX_CHANS) c = 1;

    x->x_channels = c--;
    x->x_file=0;
    x->rec = 0;
    x->size = 0;
    x->x_blocked = 0;
    x->x_blockwarn = 10;
    while (c--) {
	 inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }
    sfwrite13_tempbuf(x,8192);
    return (x);
}

static void sfwrite13_buffersize(t_sfwrite13 *x,t_floatarg s){
   sfwrite13_tempbuf(x,(int)s);
}

void sfwrite13_setup(void)
{
     sfwrite13_class = class_new(gensym("sfwrite13~"), (t_newmethod)sfwrite13_new, (t_method)sfwrite13_free,
    	sizeof(t_sfwrite13), 0,A_DEFFLOAT,0);
     class_addmethod(sfwrite13_class,nullfn,gensym("signal"), 0);
     class_addmethod(sfwrite13_class, (t_method) sfwrite13_dsp, gensym("dsp"), 0);
     class_addmethod(sfwrite13_class, (t_method) sfwrite13_open, gensym("open"), A_SYMBOL,A_NULL);
     class_addmethod(sfwrite13_class, (t_method) sfwrite13_close, gensym("close"), 0);
     class_addmethod(sfwrite13_class, (t_method) sfwrite13_block,gensym("block"),A_DEFFLOAT,0);
     class_addmethod(sfwrite13_class, (t_method) sfwrite13_buffersize,gensym("buffersize"),A_DEFFLOAT,0);
     class_addfloat(sfwrite13_class, sfwrite13_float);
     
}

void sfwrite13_tilde_setup()
{
  void sfwrite13_setup();
}






