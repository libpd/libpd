#include "ext13.h"
#include "m_pd.h"
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif

/*--*/

/* -------------------------- wavinfo ------------------------------ */
#define BLOCKTIME 0.001
#define uint32 unsigned int
#define uint16 unsigned short

typedef struct _wave
{
    char  w_fileid[4];              /* chunk id 'RIFF'            */
    uint32 w_chunksize;             /* chunk size                 */
    char  w_waveid[4];              /* wave chunk id 'WAVE'       */
    char  w_fmtid[4];               /* format chunk id 'fmt '     */
    uint32 w_fmtchunksize;          /* format chunk size          */
    uint16  w_fmttag;               /* format tag, 1 for PCM      */
    uint16  w_nchannels;            /* number of channels         */
    uint32 w_samplespersec;         /* sample rate in hz          */
    uint32 w_navgbytespersec;       /* average bytes per second   */
    uint16  w_nblockalign;          /* number of bytes per sample */
    uint16  w_nbitspersample;       /* number of bits in a sample */
    char  w_datachunkid[4];         /* data chunk id 'data'       */
    uint32 w_datachunksize;         /* length of data chunk       */
} t_wave;


static t_class *wavinfo_class;

typedef struct _wavinfo
{
    t_object x_obj;
    t_float  x_samplerate;
    t_float  x_bitspersample;
    t_float  x_channels;
    t_float  x_length;
    t_float  x_nsamples;
    int      x_fd;
    t_symbol *x_s;
    t_outlet *x_out0;
    t_outlet *x_out1;
    t_outlet *x_out2;
    t_outlet *x_out3;
    t_canvas* x_canvas;
} t_wavinfo;


static void *wavinfo_new(t_symbol *s)
{
    t_wavinfo *x = (t_wavinfo *)pd_new(wavinfo_class);
    x->x_s = s;
//    outlet_new(&x->x_obj, &s_float);
    x->x_out0 = outlet_new(&x->x_obj, &s_float);
    x->x_out1 = outlet_new(&x->x_obj, &s_float);
    x->x_out2 = outlet_new(&x->x_obj, &s_float);
    x->x_out3 = outlet_new(&x->x_obj, &s_float); 
    x->x_canvas =  canvas_getcurrent();
    return (x);
}

static void wavinfo_bang(t_wavinfo *x)
{
//  outlet_float(x->x_obj.ob_outlet, x->x_f);
}

static void wavinfo_symbol(t_wavinfo *x, t_symbol *filename)
{
   struct stat statbuf;
   t_wave* wavinfo;
   int result;
   char fname[MAXPDSTRING];
   int ok=(stat(filename->s_name, &statbuf) >= 0);
   if (ok>0) {
       canvas_makefilename(x->x_canvas, filename->s_name,fname, MAXPDSTRING);
       if ((x->x_fd = sys_open(fname,( O_NONBLOCK | O_RDONLY))) < 0)
       {
            error("can't open %s",fname);
            return;
       }

       wavinfo = getbytes(sizeof(t_wave));

       result=read (x->x_fd,wavinfo,sizeof(t_wave));
       if (result > 0){
        if (strncmp(wavinfo->w_waveid,"WAVE",4)==0){
            x->x_samplerate=(t_float)wavinfo->w_samplespersec;
            x->x_bitspersample=(t_float)wavinfo->w_nbitspersample;
            x->x_channels=(t_float)wavinfo->w_nchannels;
            x->x_length=(t_float)wavinfo->w_datachunksize;
            x->x_nsamples= x->x_length / (x->x_channels * x->x_bitspersample) * 8;
            outlet_float(x->x_out3,x->x_samplerate);
            outlet_float(x->x_out2,x->x_bitspersample);
            outlet_float(x->x_out1,x->x_channels);
            outlet_float(x->x_out0,x->x_nsamples);
        }else{
              error("not a valid wave-file");
        }
       }else{
             error("could not read wav-header");
       } 
       close (x->x_fd);
   }
   else post ("wavinfo:file not found");
}

void wavinfo_setup(void)
{
    wavinfo_class = class_new(gensym("wavinfo"), (t_newmethod)wavinfo_new, 0,
    	sizeof(t_wavinfo), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)wavinfo_new, gensym("fsize"), A_DEFFLOAT, 0);
    class_addbang(wavinfo_class, wavinfo_bang);
    class_addsymbol(wavinfo_class, wavinfo_symbol);        
}
