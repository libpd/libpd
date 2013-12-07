/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <g_canvas.h>
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
#define OPENPARAMS O_WRONLY | O_CREAT | O_NONBLOCK | O_TRUNC 
#endif
#define MAX_CHANS 4


/*
 * ------------------------------------------- sfwrite~ -------------------------------
 */

#define BLOCKTIME 0.01
#define uint32 unsigned int
#define uint16 unsigned short

static t_class *sfwrite_class;

typedef struct _sfwrite
{
     t_object x_obj;
     t_symbol* filename;
     int x_file;

     t_int rec;
     t_int x_channels;
     uint32  size;
     t_glist * x_glist;
     t_int x_blocked;
     t_int x_blockwarn;
} t_sfwrite;

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


static void sfwrite_wave_setup(t_sfwrite* x,t_wave* w) 
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



static void sfwrite_close(t_sfwrite *x)
{
     if (x->x_file > 0) {
	  t_wave w;
	  sfwrite_wave_setup(x,&w);
	  lseek(x->x_file,0,SEEK_SET);
	  write(x->x_file,&w,sizeof(w));
	  close(x->x_file);
     }
     x->x_file = -1;
}


static void sfwrite_open(t_sfwrite *x,t_symbol *filename)
{
     char fname[MAXPDSTRING];

     if (filename == &s_) {
	  post("sfwrite: open without filename");
	  return;
     }

     canvas_makefilename(glist_getcanvas(x->x_glist), filename->s_name,
			 fname, MAXPDSTRING);

     x->x_blocked = 0;
     x->filename = filename;
     post("sfwrite: filename = %s",x->filename->s_name);

     sfwrite_close(x);

     if ((x->x_file = sys_open(fname,OPENPARAMS,0664)) < 0)
     {
	  error("can't create %s",fname);
	  return;
     }

     /* skip the header */

     lseek(x->x_file,sizeof(t_wave),SEEK_SET);
     x->size = 0;


}

static void sfwrite_block(t_sfwrite *x, t_floatarg f)
{
     x->x_blockwarn = f;
}


static void sfwrite_float(t_sfwrite *x, t_floatarg f)
{
  int t = f;
  if (t) {
       post("sfwrite: start", f); 
       x->rec=1;
  }
  else {
       post("sfwrite: stop", f); 
       x->rec=0;
  }

}


static short out[4*64];

static t_int *sfwrite_perform(t_int *w)
{
     t_sfwrite* x = (t_sfwrite*)(w[1]);
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
		    *tout++ = *(in[i])++ * 32768.;
	       }
	  }
	  
	  timebefore = sys_getrealtime();
	  if ((ret =write(x->x_file,out,sizeof(short)*num*c)) < (signed int)sizeof(short)*num*c) {
	       post("sfwrite: short write %d",ret);

	       }
	  timeafter = sys_getrealtime();
	  late = timeafter - timebefore;

	  /* OK, we let only 10 ms block here */
	  if (late > BLOCKTIME && x->x_blockwarn) { 
	       post("sfwrite blocked %f ms",late*1000);
	       x->x_blocked++;
	       if (x->x_blocked > x->x_blockwarn) {
		    x->rec = 0;
		    post("maximum blockcount %d reached, recording stopped (set blockcount with \"block <num>\"",x->x_blockwarn);
	       }
	  }
	  x->size +=64*x->x_channels*sizeof(short) ;
     }

     return (w+3+c);
}



static void sfwrite_dsp(t_sfwrite *x, t_signal **sp)
{
     switch (x->x_channels) {
     case 1:
	  dsp_add(sfwrite_perform, 3, x, sp[0]->s_vec, 
		   sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(sfwrite_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(sfwrite_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,
		  sp[2]->s_vec,
		  sp[3]->s_vec,
		  sp[0]->s_n);
	  break;
     }
}

static void sfwrite_free(t_sfwrite* x)
{
     sfwrite_close(x);
}


static void *sfwrite_new(t_floatarg chan)
{
    t_sfwrite *x = (t_sfwrite *)pd_new(sfwrite_class);
    t_int c = chan;

    if (c<1 || c > MAX_CHANS) c = 1;

    x->x_glist = (t_glist*) canvas_getcurrent();
    x->x_channels = c--;
    x->x_file=0;
    x->rec = 0;
    x->size = 0;
    x->x_blocked = 0;
    x->x_blockwarn = 10;
    while (c--) {
	 inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }


    return (x);
}

void sfwrite_tilde_setup(void)
{
     sfwrite_class = class_new(gensym("sfwrite~"), (t_newmethod)sfwrite_new, (t_method)sfwrite_free,
    	sizeof(t_sfwrite), 0,A_DEFFLOAT,0);
     class_addmethod(sfwrite_class,nullfn,gensym("signal"), 0);
     class_addmethod(sfwrite_class, (t_method) sfwrite_dsp, gensym("dsp"), 0);
     class_addmethod(sfwrite_class, (t_method) sfwrite_open, gensym("open"), A_SYMBOL,A_NULL);
     class_addmethod(sfwrite_class, (t_method) sfwrite_close, gensym("close"), 0);
    class_addmethod(sfwrite_class, (t_method)sfwrite_block,gensym("block"),A_DEFFLOAT,0);
     class_addfloat(sfwrite_class, sfwrite_float);
     
}








