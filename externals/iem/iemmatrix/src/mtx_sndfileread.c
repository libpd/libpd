
#include "iemmatrix.h"

#ifdef HAVE_SNDFILE_H
# include <sndfile.h>
#endif

#ifdef __WIN32__
# include <io.h>
# include <stdlib.h>
#else
# include <unistd.h>
#endif

// SNDFILE* sf_open_fd (int fd, int mode, SF_INFO *sfinfo, int close_desc);
// mode 
//    SFM_READ    - read only mode
//    SFM_WRITE   - write only mode
//    SFM_RDWR    - read/write mode
// close_desc=0 if file shouldn't be closed, =1 if the file shall be closed
// in sf_close()
//
// typedef struct
//      {    sf_count_t  frames ;     /* Used to be called samples. */
//           int         samplerate ;
//           int         channels ;
//           int         format ;
//           int         sections ;
//           int         seekable ;
//       } SF_INFO ;
//
// int sf_close (SNDFILE *sndfile);
//
// format
//enum
//      {   /* Major formats. */
//          SF_FORMAT_WAV          = 0x010000,     /* Microsoft WAV format (little endian). */
//          SF_FORMAT_AIFF         = 0x020000,     /* Apple/SGI AIFF format (big endian). */
//          SF_FORMAT_AU           = 0x030000,     /* Sun/NeXT AU format (big endian). */
//          SF_FORMAT_RAW          = 0x040000,     /* RAW PCM data. */
//          SF_FORMAT_PAF          = 0x050000,     /* Ensoniq PARIS file format. */
//          SF_FORMAT_SVX          = 0x060000,     /* Amiga IFF / SVX8 / SV16 format. */
//          SF_FORMAT_NIST         = 0x070000,     /* Sphere NIST format. */
//          SF_FORMAT_VOC          = 0x080000,     /* VOC files. */
//          SF_FORMAT_IRCAM        = 0x0A0000,     /* Berkeley/IRCAM/CARL */
//          SF_FORMAT_W64          = 0x0B0000,     /* Sonic Foundry's 64 bit RIFF/WAV */
//          SF_FORMAT_MAT4         = 0x0C0000,     /* Matlab (tm) V4.2 / GNU Octave 2.0 */
//          SF_FORMAT_MAT5         = 0x0D0000,     /* Matlab (tm) V5.0 / GNU Octave 2.1 */
//          SF_FORMAT_PVF          = 0x0E0000,     /* Portable Voice Format */
//          SF_FORMAT_XI           = 0x0F0000,     /* Fasttracker 2 Extended Instrument */
//          SF_FORMAT_HTK          = 0x100000,     /* HMM Tool Kit format */
//          SF_FORMAT_SDS          = 0x110000,     /* Midi Sample Dump Standard */
//          SF_FORMAT_AVR          = 0x120000,     /* Audio Visual Research */
//          SF_FORMAT_WAVEX        = 0x130000,     /* MS WAVE with WAVEFORMATEX */
//          SF_FORMAT_SD2          = 0x160000,     /* Sound Designer 2 */
//          SF_FORMAT_FLAC         = 0x170000,     /* FLAC lossless file format */
//          SF_FORMAT_CAF          = 0x180000,     /* Core Audio File format */
//
//          /* Subtypes from here on. */
//
//          SF_FORMAT_PCM_S8       = 0x0001,       /* Signed 8 bit data */
//          SF_FORMAT_PCM_16       = 0x0002,       /* Signed 16 bit data */
//          SF_FORMAT_PCM_24       = 0x0003,       /* Signed 24 bit data */
//          SF_FORMAT_PCM_32       = 0x0004,       /* Signed 32 bit data */
//
//          SF_FORMAT_PCM_U8       = 0x0005,       /* Unsigned 8 bit data (WAV and RAW only) */
//
//          SF_FORMAT_FLOAT        = 0x0006,       /* 32 bit float data */
//          SF_FORMAT_DOUBLE       = 0x0007,       /* 64 bit float data */
//
//          SF_FORMAT_ULAW         = 0x0010,       /* U-Law encoded. */
//          SF_FORMAT_ALAW         = 0x0011,       /* A-Law encoded. */
//          SF_FORMAT_IMA_ADPCM    = 0x0012,       /* IMA ADPCM. */
//          SF_FORMAT_MS_ADPCM     = 0x0013,       /* Microsoft ADPCM. */
//
//          SF_FORMAT_GSM610       = 0x0020,       /* GSM 6.10 encoding. */
//          SF_FORMAT_VOX_ADPCM    = 0x0021,       /* Oki Dialogic ADPCM encoding. */
//
//          SF_FORMAT_G721_32      = 0x0030,       /* 32kbs G721 ADPCM encoding. */
//          SF_FORMAT_G723_24      = 0x0031,       /* 24kbs G723 ADPCM encoding. */
//          SF_FORMAT_G723_40      = 0x0032,       /* 40kbs G723 ADPCM encoding. */
//
//          SF_FORMAT_DWVW_12      = 0x0040,       /* 12 bit Delta Width Variable Word encoding. */
//          SF_FORMAT_DWVW_16      = 0x0041,       /* 16 bit Delta Width Variable Word encoding. */
//          SF_FORMAT_DWVW_24      = 0x0042,       /* 24 bit Delta Width Variable Word encoding. */
//          SF_FORMAT_DWVW_N       = 0x0043,       /* N bit Delta Width Variable Word encoding. */
//
//          SF_FORMAT_DPCM_8       = 0x0050,       /* 8 bit differential PCM (XI only) */
//          SF_FORMAT_DPCM_16      = 0x0051,       /* 16 bit differential PCM (XI only) */
//
//          /* Endian-ness options. */
//
//          SF_ENDIAN_FILE         = 0x00000000,   /* Default file endian-ness. */
//          SF_ENDIAN_LITTLE       = 0x10000000,   /* Force little endian-ness. */
//          SF_ENDIAN_BIG          = 0x20000000,   /* Force big endian-ness. */
//          SF_ENDIAN_CPU          = 0x30000000,   /* Force CPU endian-ness. */
//
//          SF_FORMAT_SUBMASK      = 0x0000FFFF,
//          SF_FORMAT_TYPEMASK     = 0x0FFF0000,
//          SF_FORMAT_ENDMASK      = 0x30000000
//          };
//

static t_class *mtx_sndfileread_class;

typedef struct mtx_sndfileread
{
  t_object x_ob;
#ifdef HAVE_SNDFILE_H
  SNDFILE *x_sndfileread;
  SF_INFO x_sfinfo;
#endif
  t_outlet *x_message_outlet;
  t_outlet *x_readybang_outlet;
  t_canvas *x_canvas;
  float *x_float;
  t_atom *x_outlist;
  int num_chan;
  int num_frames;
} t_mtx_sndfileread;


static void mtx_sndfileread_close (t_mtx_sndfileread *x)
{
#ifdef HAVE_SNDFILE_H
  if(x->x_sndfileread)
    sf_close (x->x_sndfileread);
  x->x_sndfileread=0;
#endif

  if(x->x_outlist)
    freebytes(x->x_outlist, sizeof(t_atom)*(2+x->num_chan*x->num_frames));
  x->x_outlist=0;

  if(x->x_float)
    freebytes(x->x_float, sizeof(float)*(x->num_chan*x->num_frames));
  x->x_float=0;
}

static void mtx_sndfileread_open (t_mtx_sndfileread *x, t_symbol *s, t_symbol*type)
{
#ifdef HAVE_SNDFILE_H
  char filenamebuf[MAXPDSTRING], *filenamebufptr;
  char*dirname=canvas_getdir(x->x_canvas)->s_name;
  int fd;

  mtx_sndfileread_close(x);

  /* directory, filename, extension, dirresult, nameresult, unsigned int size, int bin */
  if ((fd=open_via_path(dirname,
                        s->s_name,"", filenamebuf, &filenamebufptr, MAXPDSTRING,0)) < 0 ) {
    pd_error(x, "%s: failed to open %s", s->s_name, filenamebuf);
    return;
  }
  if (!(x->x_sndfileread = sf_open_fd (fd, SFM_READ, &x->x_sfinfo, 1))) {
    pd_error(x, "%s: failed to open %s", s->s_name, filenamebuf);
    mtx_sndfileread_close(x);
    return;
  }
  x->num_chan = x->x_sfinfo.channels;
#else
  pd_error(x,"mtx_sndfileread: compiled without libsndfile: no file opened!");
#endif
}

static void mtx_sndfileread_frame (t_mtx_sndfileread *x)
{
#ifdef HAVE_SNDFILE_H
  int n;
  t_atom *ptr;
  
  if ((!x->x_sndfileread)||(x->num_chan<=0)) {
     pd_error(x, "no or damaged file opened for reading");
     return;
  }

  if (!(x->x_float)||(x->num_frames<1)) {
     if(!(x->x_outlist=(t_atom*)getbytes(sizeof(t_atom)*(2+x->num_chan)))) {
	pd_error(x,"out of memory");
	return;
     }
     if (!(x->x_float=(float*)getbytes(sizeof(float)*x->num_chan))) {
	freebytes(x->x_outlist,sizeof(t_atom)*(2+x->num_chan));
	x->x_outlist=0;
	pd_error(x,"out of memory");
	return;
     }
     x->num_frames=1;
  }
  
  if (sf_readf_float(x->x_sndfileread, x->x_float, (sf_count_t)1)<1) {
     mtx_sndfileread_close(x);
     outlet_bang(x->x_readybang_outlet);
  }
  else {
     SETFLOAT(x->x_outlist,(t_float)x->num_chan);
     SETFLOAT(x->x_outlist+1,(t_float)1);
     ptr=x->x_outlist+2;
     for (n=0;n<x->num_chan;n++) {
	SETFLOAT(&ptr[n],x->x_float[n]);
     }
     outlet_anything(x->x_message_outlet,gensym("matrix"),x->num_chan+2,x->x_outlist);
  }
#else
  pd_error(x,"mtx_sndfileread: compiled without libsndfile: no file opened for reading!");
#endif

}

static void mtx_sndfileread_frames (t_mtx_sndfileread *x, t_float f)
{
#ifdef HAVE_SNDFILE_H
  int n,n2,c;
  sf_count_t frames_read;
  int num_frames=(int)f;
  t_atom *ptr;
 
  if ((!x->x_sndfileread)||(x->num_chan<=0)) {
     pd_error(x, "no or damaged file opened for reading");
     return;
  }

  if (!(x->x_float)||(x->num_frames<num_frames)) {
     if(!(x->x_outlist=(t_atom*)getbytes(sizeof(t_atom)*(2+num_frames*x->num_chan)))) {
	pd_error(x,"out of memory");
	return;
     }
     if (!(x->x_float=(float*)getbytes(sizeof(float)*num_frames*x->num_chan))) {
	freebytes(x->x_outlist,sizeof(t_atom)*(2+num_frames*x->num_chan));
	x->x_outlist=0;
	pd_error(x,"out of memory");
	return;
     }
     x->num_frames=num_frames;
  }

  if ((frames_read=sf_readf_float(x->x_sndfileread, 
	      x->x_float,
	      (sf_count_t)num_frames))<1) {
     mtx_sndfileread_close(x);
     outlet_bang(x->x_readybang_outlet);
  }
  else {
     SETFLOAT(x->x_outlist,(t_float)x->num_chan);
     SETFLOAT(x->x_outlist+1,(t_float)frames_read);
     ptr=x->x_outlist+2;
     for (n=0,c=0;c<x->num_chan;c++) {
	for (n2=c; n2<frames_read*x->num_chan; n++, n2+=x->num_chan) {
	   SETFLOAT(&ptr[n],x->x_float[n2]);
	}
     }
     outlet_anything(x->x_message_outlet,gensym("matrix"),frames_read*x->num_chan+2,x->x_outlist);
     if (frames_read<num_frames) {
	mtx_sndfileread_close(x);
	outlet_bang(x->x_readybang_outlet);
     }
  }
#else
  pd_error(x,"mtx_sndfileread: compiled without libsndfile: no file opened!");
#endif
}

static void mtx_sndfileread_free (t_mtx_sndfileread *x)
{
  mtx_sndfileread_close(x);
  outlet_free (x->x_message_outlet);
  outlet_free (x->x_readybang_outlet);
}

static void *mtx_sndfileread_new(void)
{
  t_mtx_sndfileread *x = (t_mtx_sndfileread *)pd_new(mtx_sndfileread_class);
  x->x_message_outlet = outlet_new(&x->x_ob, &s_list);
  x->x_readybang_outlet = outlet_new(&x->x_ob, &s_bang);
#ifdef HAVE_SNDFILE_H
  x->x_sndfileread=0;
#else
  pd_error(x,"mtx_sndfileread won't work: compiled without libsndfile!");
#endif
  x->num_chan=0;
  x->num_frames=0;
  x->x_canvas = canvas_getcurrent();
  return (void *)x;
}

void mtx_sndfileread_setup(void)
{
  mtx_sndfileread_class = class_new(gensym("mtx_sndfileread"), (t_newmethod)mtx_sndfileread_new, 
                                (t_method) mtx_sndfileread_free, sizeof(t_mtx_sndfileread), 0, 0);
  class_addmethod(mtx_sndfileread_class, (t_method)mtx_sndfileread_open, gensym("open"), A_SYMBOL, A_DEFSYM, 0);
  class_addmethod(mtx_sndfileread_class, (t_method)mtx_sndfileread_close, gensym("close"), A_NULL, 0);
  class_addbang(mtx_sndfileread_class, (t_method)mtx_sndfileread_frame);
  class_addfloat(mtx_sndfileread_class, (t_method)mtx_sndfileread_frames);
}

void iemtx_sndfileread_setup (void)
{
   mtx_sndfileread_setup();
}

