#ifndef _WIN32
/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file contains, first, a collection of soundfile access routines, a
sort of soundfile library.  Second, the "soundfiler" object is defined which
uses the routines to read or write soundfiles, synchronously, from garrays.
These operations are not to be done in "real time" as they may have to wait
for disk accesses (even the write routine.)  Finally, the realtime objects
readsf~ and writesf~ are defined which confine disk operations to a separate
thread so that they can be used in real time.  The real-time disk access
objects are available for linux only so far, although they could be compiled
for Windows if someone were willing to find a Pthreads package for it. */

/* this is a partial copy of d_soundfile, with some hacking:
	1: a fix in soundfiler to compute the normalization factor only on
		the samples we want to write on the disk...
	2: a version of readsf~ that works with a (positive) speed parameter.

	Antoine Rousseau
*/

#include <pthread.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "m_pd.h"

#define MAXSFCHANS 4

 /* 32 bit "pointer cast" union */
typedef union {
    float f;
    long i;
} ls_pcast32;

/***************** soundfile header structures ************************/

typedef unsigned short uint16;
typedef unsigned long uint32;

#define FORMAT_WAVE 0
#define FORMAT_AIFF 1
#define FORMAT_NEXT 2

/* the NeXTStep sound header structure; can be big or little endian  */

typedef struct _nextstep
{
    char ns_fileid[4]; 	    /* magic number '.snd' if file is big-endian */
    uint32 ns_onset; 	    /* byte offset of first sample */
    uint32 ns_length;	    /* length of sound in bytes */
    uint32 ns_format;        /* format; see below */
    uint32 ns_sr;    	    /* sample rate */
    uint32 ns_nchans;	    /* number of channels */
    char ns_info[4];   	    /* comment */
} t_nextstep;

#define NS_FORMAT_LINEAR_16	3
#define NS_FORMAT_LINEAR_24	4
#define NS_FORMAT_FLOAT         6
#define SCALE (1./(1024. * 1024. * 1024. * 2.))

/* the WAVE header.  All Wave files are little endian.  We assume
    the "fmt" chunk comes first which is usually the case but perhaps not
    always; same for AIFF and the "COMM" chunk.   */

typedef unsigned word;
typedef unsigned long dword;

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

typedef struct _fmt	    /* format chunk */
{
    uint16 f_fmttag;	    	    /* format tag, 1 for PCM      */
    uint16 f_nchannels;    	    /* number of channels         */
    uint32 f_samplespersec;  	    /* sample rate in hz          */
    uint32 f_navgbytespersec; 	    /* average bytes per second   */
    uint16 f_nblockalign;    	    /* number of bytes per frame  */
    uint16 f_nbitspersample; 	    /* number of bits in a sample */
} t_fmt;

typedef struct _wavechunk	    /* ... and the last two items */
{
    char  wc_id[4]; 	    	    /* data chunk id, e.g., 'data' or 'fmt ' */
    uint32 wc_size;         	    /* length of data chunk       */
} t_wavechunk;

/* the AIFF header.  I'm assuming AIFC is compatible but don't really know
    that. */

typedef struct _datachunk
{
    char  dc_id[4]; 	    	    /* data chunk id 'SSND'       */
    uint32 dc_size;         	    /* length of data chunk       */
} t_datachunk;

//#define CHUNKHDRSIZE sizeof(t_datachunk)

typedef struct _comm
{
    uint16 c_nchannels;	            /* number of channels         */
    uint16 c_nframeshi;    	    /* # of sample frames (hi)    */
    uint16 c_nframeslo;    	    /* # of sample frames (lo)    */
    uint16 c_bitspersamp;  	    /* bits per sample            */
    unsigned char c_samprate[10];   /* sample rate, 80-bit float! */
} t_comm;

/* this version is more convenient for writing them out: */
typedef struct _aiff
{
    char  a_fileid[4];	    	    /* chunk id 'FORM'            */
    uint32 a_chunksize;     	    /* chunk size                 */
    char  a_aiffid[4];	    	    /* aiff chunk id 'AIFF'       */
    char  a_fmtid[4];	    	    /* format chunk id 'COMM'     */
    uint32 a_fmtchunksize;   	    /* format chunk size, 18      */
    uint16 a_nchannels;	            /* number of channels         */
    uint16 a_nframeshi;    	    /* # of sample frames (hi)    */
    uint16 a_nframeslo;    	    /* # of sample frames (lo)    */
    uint16 a_bitspersamp;  	    /* bits per sample            */
    unsigned char a_samprate[10];   /* sample rate, 80-bit float! */
} t_aiff;

#define AIFFHDRSIZE 38	    /* probably not what sizeof() gives */


#define AIFFPLUS (AIFFHDRSIZE + 8)  /* header size including first chunk hdr */

#define WHDR1 sizeof(t_nextstep)
#define WHDR2 (sizeof(t_wave) > WHDR1 ? sizeof (t_wave) : WHDR1)
#define WRITEHDRSIZE (AIFFPLUS > WHDR2 ? AIFFPLUS : WHDR2)

#define READHDRSIZE (16 > WHDR2 ? 16 : WHDR2)

#define OBUFSIZE MAXPDSTRING  /* assume MAXPDSTRING is bigger than headers */

#ifdef _WIN32
#include <fcntl.h>
#define BINCREATE _O_WRONLY | _O_CREAT | _O_BINARY | _O_TRUNC |
#else
#define BINCREATE O_WRONLY|O_CREAT|O_TRUNC
#endif

/* this routine returns 1 if the high order byte comes at the lower
address on our architecture (big-endianness.).  It's 1 for Motorola,
0 for Intel: */

extern int garray_ambigendian(void);

/* byte swappers */

static uint32 swap4(uint32 n, int doit)
{
    if (doit)
        return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
                ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
    else return (n);
}

static uint16 swap2(uint32 n, int doit)
{
    if (doit)
        return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
    else return (n);
}

static void swapstring(char *foo, int doit)
{
    if (doit)
    {
        char a = foo[0], b = foo[1], c = foo[2], d = foo[3];
        foo[0] = d;
        foo[1] = c;
        foo[2] = b;
        foo[3] = a;
    }
}

/******************** soundfile access routines **********************/

void readsf_banana( void);    /* debugging */

/* This routine opens a file, looks for either a nextstep or "wave" header,
* seeks to end of it, and fills in bytes per sample and number of channels.
* Only 2- and 3-byte fixed-point samples and 4-byte floating point samples
* are supported.  If "headersize" is nonzero, the
* caller should supply the number of channels, endinanness, and bytes per
* sample; the header is ignored.  Otherwise, the routine tries to read the
* header and fill in the properties.
*/

extern int open_soundfile(const char *dirname, const char *filename, int headersize,
                          int *p_bytespersamp, int *p_bigendian, int *p_nchannels, long *p_bytelimit,
                          long skipframes);


static void soundfile_xferin(int sfchannels, int nvecs, float **vecs,
                             long itemsread, unsigned char *buf, int nitems, int bytespersamp,
                             int bigendian)
{
    int i, j;
    unsigned char *sp, *sp2;
    float *fp;
    int nchannels = (sfchannels < nvecs ? sfchannels : nvecs);
    int bytesperframe = bytespersamp * sfchannels;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                    *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                    *fp = SCALE * ((sp2[1] << 24) | (sp2[0] << 16));
            }
        }
        else if (bytespersamp == 3)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                    *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16)
                                   | (sp2[2] << 8));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                    *fp = SCALE * ((sp2[2] << 24) | (sp2[1] << 16)
                                   | (sp2[0] << 8));
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                    *(long *)fp = ((sp2[0] << 24) | (sp2[1] << 16)
                                   | (sp2[2] << 8) | sp2[3]);
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                    *(long *)fp = ((sp2[3] << 24) | (sp2[2] << 16)
                                   | (sp2[1] << 8) | sp2[0]);
            }
        }
    }
    /* zero out other outputs */
    for (i = sfchannels; i < nvecs; i++)
        for (j = nitems, fp = vecs[i]; j--; )
            *fp++ = 0;

}

static void interpolate(int nvec,float **invec,int nin,
                        float **outvec,int nout)
{

    float r=nin/(float)nout;
    int i,j;


    for(i=0; i<nout; i++)
        for(j=0; j<nvec; j++)
            outvec[j][i]=invec[j][(int)(i*r)];

}
/* soundfiler2_write ...

usage: write [flags] filename table ...
flags:
-nframes <frames>
-skip <frames>
-bytes <bytes per sample>
-normalize
-nextstep
-wave
-big
-little
*/

/* the routine which actually does the work should LATER also be called
from garray_write16. */


/* Parse arguments for writing.  The "obj" argument is only for flagging
errors.  For streaming to a file the "normalize", "onset" and "nframes"
arguments shouldn't be set but the calling routine flags this. */

static int soundfiler2_writeargparse(void *obj, int *p_argc, t_atom **p_argv,
                                     t_symbol **p_filesym,
                                     int *p_filetype, int *p_bytespersamp, int *p_swap, int *p_bigendian,
                                     int *p_normalize, long *p_onset, long *p_nframes,float *p_amp)
{
    int argc = *p_argc;
    t_atom *argv = *p_argv;
    int bytespersamp = 2, bigendian = 0,
        endianness = -1, swap, filetype = FORMAT_WAVE, normalize = 0;
    long onset = 0, nframes = 0x7fffffff;
    float amp=1;
    t_symbol *filesym;
    while (argc > 0 && argv->a_type == A_SYMBOL &&
            *argv->a_w.w_symbol->s_name == '-')
    {
        char *flag = argv->a_w.w_symbol->s_name + 1;
        if (!strcmp(flag, "skip"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                    ((onset = argv[1].a_w.w_float) < 0))
                goto usage;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(flag, "nframes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                    ((nframes = argv[1].a_w.w_float) < 0))
                goto usage;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(flag, "bytes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                    ((bytespersamp = argv[1].a_w.w_float) < 2) ||
                    bytespersamp > 4)
                goto usage;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(flag, "normalize"))
        {
            normalize = 1;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "amp"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT)
                goto usage;
            amp = argv[1].a_w.w_float;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(flag, "wave"))
        {
            filetype = FORMAT_WAVE;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "nextstep"))
        {
            filetype = FORMAT_NEXT;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "aiff"))
        {
            filetype = FORMAT_AIFF;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "big"))
        {
            endianness = 1;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "little"))
        {
            endianness = 1;
            argc -= 1;
            argv += 1;
        }
        else goto usage;
    }
    /* only NextStep handles floating point samples */
    if (bytespersamp == 4)
        filetype = FORMAT_NEXT;

    /* for WAVE force little endian; for nextstep use machine native */
    if (filetype == FORMAT_WAVE)
    {
        bigendian = 0;
        if (endianness == 1)
            pd_error(obj, "WAVE file forced to little endian");
    }
    else if (filetype == FORMAT_AIFF)
    {
        bigendian = 1;
        if (endianness == 0)
            pd_error(obj, "AIFF file forced to big endian");
    }
    else if (endianness == -1)
    {
        bigendian = garray_ambigendian();
    }
    swap = (bigendian != garray_ambigendian());
    if (!argc || argv->a_type != A_SYMBOL)
        goto usage;
    filesym = argv->a_w.w_symbol;
    argc--;
    argv++;

    *p_argc = argc;
    *p_argv = argv;
    *p_filesym = filesym;
    *p_filetype = filetype;
    *p_bytespersamp = bytespersamp;
    *p_swap = swap;
    *p_normalize = normalize;
    *p_onset = onset;
    *p_nframes = nframes;
    *p_bigendian = bigendian;
    *p_amp = amp;
    return (0);
usage:
    return (-1);
}

static int create_soundfile2(t_canvas *canvas, const char *filename,
                             int filetype, int nframes, int bytespersamp,
                             int bigendian, int nchannels, int swap)
{
    char filenamebuf[MAXPDSTRING], buf2[MAXPDSTRING];
    char headerbuf[WRITEHDRSIZE];
    t_wave *wavehdr = (t_wave *)headerbuf;
    t_nextstep *nexthdr = (t_nextstep *)headerbuf;
    t_aiff *aiffhdr = (t_aiff *)headerbuf;
    int fd, headersize = 0;

    strncpy(filenamebuf, filename, MAXPDSTRING-10);
    filenamebuf[MAXPDSTRING-10] = 0;

    if (filetype == FORMAT_NEXT)
    {
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".snd"))
            strcat(filenamebuf, ".snd");
        if (bigendian)
            strncpy(nexthdr->ns_fileid, ".snd", 4);
        else strncpy(nexthdr->ns_fileid, "dns.", 4);
        nexthdr->ns_onset = swap4(sizeof(*nexthdr), swap);
        nexthdr->ns_length = 0;
        nexthdr->ns_format = swap4((bytespersamp == 3 ? NS_FORMAT_LINEAR_24 :
                                    (bytespersamp == 4 ? NS_FORMAT_FLOAT : NS_FORMAT_LINEAR_16)), swap);;
        nexthdr->ns_sr = swap4(44100, swap);     /* lie */
        nexthdr->ns_nchans = swap4(nchannels, swap);
        strcpy(nexthdr->ns_info, "Pd ");
        swapstring(nexthdr->ns_info, swap);
        headersize = sizeof(t_nextstep);
    }
    else if (filetype == FORMAT_AIFF)
    {
        long datasize = nframes * nchannels * bytespersamp;
        long longtmp;
        static unsigned char dogdoo[] =
        {0x40, 0x0e, 0xac, 0x44, 0, 0, 0, 0, 0, 0, 'S', 'S', 'N', 'D'};
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".aif") &&
                strcmp(filenamebuf + strlen(filenamebuf)-5, ".aiff"))
            strcat(filenamebuf, ".aif");
        strncpy(aiffhdr->a_fileid, "FORM", 4);
        aiffhdr->a_chunksize = swap4(datasize + sizeof(*aiffhdr) + 4, swap);
        strncpy(aiffhdr->a_aiffid, "AIFF", 4);
        strncpy(aiffhdr->a_fmtid, "COMM", 4);
        aiffhdr->a_fmtchunksize = swap4(18, swap);
        aiffhdr->a_nchannels = swap2(nchannels, swap);
        longtmp = swap4(nframes, swap);
        memcpy(&aiffhdr->a_nframeshi, &longtmp, 4);
        aiffhdr->a_bitspersamp = swap2(8 * bytespersamp, swap);
        memcpy(aiffhdr->a_samprate, dogdoo, sizeof(dogdoo));
        longtmp = swap4(datasize, swap);
        memcpy(aiffhdr->a_samprate + sizeof(dogdoo), &longtmp, 4);
        headersize = AIFFPLUS;
    }
    else    /* WAVE format */
    {
        long datasize = nframes * nchannels * bytespersamp;
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".wav"))
            strcat(filenamebuf, ".wav");
        strncpy(wavehdr->w_fileid, "RIFF", 4);
        wavehdr->w_chunksize = swap4(datasize + sizeof(*wavehdr) - 8, swap);
        strncpy(wavehdr->w_waveid, "WAVE", 4);
        strncpy(wavehdr->w_fmtid, "fmt ", 4);
        wavehdr->w_fmtchunksize = swap4(16, swap);
        wavehdr->w_fmttag = swap2(1, swap);
        wavehdr->w_nchannels = swap2(nchannels, swap);
        wavehdr->w_samplespersec = swap4(44100, swap);
        wavehdr->w_navgbytespersec = swap4(44100 * nchannels * bytespersamp, swap);
        wavehdr->w_nblockalign = swap2(bytespersamp, swap);
        wavehdr->w_nbitspersample = swap2(8 * bytespersamp, swap);
        strncpy(wavehdr->w_datachunkid, "data", 4);
        wavehdr->w_datachunksize = swap4(datasize, swap);
        headersize = sizeof(t_wave);
    }

    canvas_makefilename(canvas, filenamebuf, buf2, MAXPDSTRING);
    if ((fd = sys_open(buf2, BINCREATE, 0666)) < 0)
        return (-1);

    if (write(fd, headerbuf, headersize) < headersize)
    {
        close (fd);
        return (-1);
    }
    return (fd);
}

static void soundfile_finishwrite(void *obj, char *filename, int fd,
                                  int filetype, long nframes, long itemswritten, int bytesperframe, int swap)
{
    if (itemswritten < nframes)
    {
        if (nframes < 0x7fffffff)
            pd_error(obj, "soundfiler2_write: %d out of %d bytes written",
                     itemswritten, nframes);
        /* try to fix size fields in header */
        if (filetype == FORMAT_WAVE)
        {
            long datasize = itemswritten * bytesperframe, mofo;

            if (lseek(fd,
                      ((char *)(&((t_wave *)0)->w_chunksize)) - (char *)0,
                      SEEK_SET) == 0)
                goto baddonewrite;
            mofo = swap4(datasize + sizeof(t_wave) - 8, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                      ((char *)(&((t_wave *)0)->w_datachunksize)) - (char *)0,
                      SEEK_SET) == 0)
                goto baddonewrite;
            mofo = swap4(datasize, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == FORMAT_AIFF)
        {
            long mofo;
            if (lseek(fd,
                      ((char *)(&((t_aiff *)0)->a_nframeshi)) - (char *)0,
                      SEEK_SET) == 0)
                goto baddonewrite;
            mofo = swap4(nframes, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
    }
    return;
baddonewrite:
    post("%s: %s", filename, strerror(errno));
}

static void soundfile_xferout(int nchannels, float **vecs,
                              unsigned char *buf, int nitems, long onset, int bytespersamp,
                              int bigendian, float normalfactor)
{
    int i, j;
    unsigned char *sp, *sp2;
    float *fp;
    int bytesperframe = bytespersamp * nchannels;
    ls_pcast32 *px;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            float ff = normalfactor * 32768.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp = vecs[i] + onset;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[0] = (xx >> 8);
                    sp2[1] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 3)
        {
            float ff = normalfactor * 8388608.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[0] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[2] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[2] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    float f2 = *fp * normalfactor;
                    px = (ls_pcast32 *)&f2;
                    sp2[0] = ((*px).i >> 24);
                    sp2[1] = ((*px).i >> 24);
                    sp2[2] = ((*px).i >> 24);
                    sp2[3] = (*px).i;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                        j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    float f2 = *fp * normalfactor;
                    px = (ls_pcast32 *)&f2;
                    sp2[3] = ((*px).i >> 24);
                    sp2[2] = ((*px).i >> 24);
                    sp2[1] = ((*px).i >> 24);
                    sp2[0] = (*px).i;
                }
            }
        }
    }
}


/* ------- soundfiler - reads and writes soundfiles to/from "garrays" ---- */
#define DEFMAXSIZE 4000000 	/* default maximum 16 MB per channel */
#define SAMPBUFSIZE 1024


static t_class *soundfiler2_class;

typedef struct _soundfiler2
{
    t_object x_obj;
    t_canvas *x_canvas;
} t_soundfiler2;

static t_soundfiler2 *soundfiler2_new(void)
{
    t_soundfiler2 *x = (t_soundfiler2 *)pd_new(soundfiler2_class);
    x->x_canvas = canvas_getcurrent();
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

/* soundfiler2_read ...

usage: read [flags] filename table ...
flags:
	-skip <frames> ... frames to skip in file
-nframes <frames>
-onset <frames> ... onset in table to read into (NOT DONE YET)
-raw <headersize channels bytes endian>
-resize
-maxsize <max-size>
*/

static void soundfiler2_read(t_soundfiler2 *x, t_symbol *s,
                             int argc, t_atom *argv)
{
    int headersize = -1, channels = 0, bytespersamp = 0, bigendian = 0,
        resize = 0, i, j;
    long skipframes = 0, nframes = 0, finalsize = 0, itemsleft,
         maxsize = DEFMAXSIZE, itemsread = 0, bytelimit  = 0x7fffffff;
    int fd = -1;
    char endianness, *filename;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes, nitems;
    FILE *fp;
    while (argc > 0 && argv->a_type == A_SYMBOL &&
            *argv->a_w.w_symbol->s_name == '-')
    {
        char *flag = argv->a_w.w_symbol->s_name + 1;
        if (!strcmp(flag, "skip"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                    ((skipframes = argv[1].a_w.w_float) < 0))
                goto usage;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(flag, "nframes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                    ((nframes = argv[1].a_w.w_float) < 0))
                goto usage;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(flag, "raw"))
        {
            if (argc < 5 ||
                    argv[1].a_type != A_FLOAT ||
                    ((headersize = argv[1].a_w.w_float) < 0) ||
                    argv[2].a_type != A_FLOAT ||
                    ((channels = argv[2].a_w.w_float) < 1) ||
                    (channels > MAXSFCHANS) ||
                    argv[3].a_type != A_FLOAT ||
                    ((bytespersamp = argv[3].a_w.w_float) < 2) ||
                    (bytespersamp > 4) ||
                    argv[4].a_type != A_SYMBOL ||
                    ((endianness = argv[4].a_w.w_symbol->s_name[0]) != 'b'
                     && endianness != 'l' && endianness != 'n'))
                goto usage;
            if (endianness == 'b')
                bigendian = 1;
            else if (endianness == 'l')
                bigendian = 0;
            else
                bigendian = garray_ambigendian();
            argc -= 5;
            argv += 5;
        }
        else if (!strcmp(flag, "resize"))
        {
            resize = 1;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "maxsize"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                    ((maxsize = argv[1].a_w.w_float) < 0))
                goto usage;
            resize = 1;     /* maxsize implies resize. */
            argc -= 2;
            argv += 2;
        }
        else goto usage;
    }
    if (argc < 2 || argc > MAXSFCHANS + 1 || argv[0].a_type != A_SYMBOL)
        goto usage;
    filename = argv[0].a_w.w_symbol->s_name;
    argc--;
    argv++;

    for (i = 0; i < argc; i++)
    {
        int vecsize;
        if (argv[i].a_type != A_SYMBOL)
            goto usage;
        if (!(garrays[i] =
                    (t_garray *)pd_findbyclass(argv[i].a_w.w_symbol, garray_class)))
        {
            pd_error(x, "%s: no such table", argv[i].a_w.w_symbol->s_name);
            goto done;
        }
        else if (!garray_getfloatarray(garrays[i], &vecsize, &vecs[i]))
            error("%s: bad template for tabwrite",
                  argv[i].a_w.w_symbol->s_name);
        if (finalsize && finalsize != vecsize && !resize)
        {
            post("soundfiler2_read: arrays have different lengths; resizing...");
            resize = 1;
        }
        finalsize = vecsize;
    }
    fd = open_soundfile(canvas_getdir(x->x_canvas)->s_name, filename,
                        headersize, &bytespersamp, &bigendian, &channels, &bytelimit,
                        skipframes);

    if (fd < 0)
    {
        pd_error(x, "soundfiler2_read: %s: %s", filename, (errno == EIO ?
                 "unknown or bad header format" : strerror(errno)));
        goto done;
    }

    if (resize)
    {
        /* figure out what to resize to */
        long poswas, eofis, framesinfile;

        poswas = lseek(fd, 0, SEEK_CUR);
        eofis = lseek(fd, 0, SEEK_END);
        if (poswas < 0 || eofis < 0)
        {
            pd_error(x, "lseek failed");
            goto done;
        }
        lseek(fd, poswas, SEEK_SET);
        framesinfile = (eofis - poswas) / (channels * bytespersamp);
        if (framesinfile > maxsize)
        {
            pd_error(x, "soundfiler2_read: truncated to %d elements", maxsize);
            framesinfile = maxsize;
        }
        if (framesinfile > bytelimit / bytespersamp)
            framesinfile = bytelimit / bytespersamp;
        finalsize = framesinfile;
        for (i = 0; i < argc; i++)
        {
            int vecsize;

            garray_resize(garrays[i], finalsize);
            /* for sanity's sake let's clear the save-in-patch flag here */
            garray_setsaveit(garrays[i], 0);
            garray_getfloatarray(garrays[i], &vecsize, &vecs[i]);
            /* if the resize failed, garray_resize reported the error */
            if (vecsize != framesinfile)
            {
                pd_error(x, "resize failed");
                goto done;
            }
        }
    }
    if (!finalsize) finalsize = 0x7fffffff;
    if (finalsize > bytelimit / bytespersamp)
        finalsize = bytelimit / bytespersamp;
    fp = fdopen(fd, "rb");
    bufframes = SAMPBUFSIZE / (channels * bytespersamp);

    for (itemsread = 0; itemsread < finalsize; )
    {
        int thisread = finalsize - itemsread;
        thisread = (thisread > bufframes ? bufframes : thisread);
        nitems = fread(sampbuf, channels * bytespersamp, thisread, fp);
        if (nitems <= 0) break;
        soundfile_xferin(channels, argc, vecs, itemsread,
                         (unsigned char *)sampbuf, nitems, bytespersamp, bigendian);
        itemsread += nitems;
    }
    /* zero out remaining elements of vectors */

    for (i = 0; i < argc; i++)
    {
        int nzero, vecsize;
        garray_getfloatarray(garrays[i], &vecsize, &vecs[i]);
        for (j = itemsread; j < vecsize; j++)
            vecs[i][j] = 0;
    }
    /* zero out vectors in excess of number of channels */
    for (i = channels; i < argc; i++)
    {
        int vecsize;
        float *foo;
        garray_getfloatarray(garrays[i], &vecsize, &foo);
        for (j = 0; j < vecsize; j++)
            foo[j] = 0;
    }
    /* do all graphics updates */
    for (i = 0; i < argc; i++)
        garray_redraw(garrays[i]);
    fclose(fp);
    fd = -1;
    goto done;
usage:
    pd_error(x, "usage: read [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -resize -maxsize <n> ...");
    post("-raw <headerbytes> <channels> <bytespersamp> <endian (b, l, or n)>.");
done:
    if (fd >= 0)
        close (fd);
    outlet_float(x->x_obj.ob_outlet, (float)itemsread);
}

/* this is broken out from soundfiler2_write below so garray_write can
call it too... not done yet though. */

static long soundfiler2_dowrite(void *obj, t_canvas *canvas,
                                int argc, t_atom *argv)
{
    int headersize, bytespersamp, bigendian,
        endianness, swap, filetype, normalize, i, j, nchannels;
    long onset, nframes, itemsleft,
         maxsize = DEFMAXSIZE, itemswritten = 0;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes, nitems;
    int fd = -1;
    float normfactor, biggest = 0,ftmp,amp;
    t_symbol *filesym;

    if (soundfiler2_writeargparse(obj, &argc, &argv, &filesym, &filetype,
                                  &bytespersamp, &swap, &bigendian, &normalize, &onset, &nframes,&amp))
        goto usage;
    nchannels = argc;
    if (nchannels < 1 || nchannels > MAXSFCHANS)
        goto usage;

    for (i = 0; i < nchannels; i++)
    {
        int vecsize;

        if (argv[i].a_type != A_SYMBOL) goto usage;

        if (!(garrays[i] =
                    (t_garray *)pd_findbyclass(argv[i].a_w.w_symbol, garray_class)))
        {
            pd_error(obj, "%s: no such table", argv[i].a_w.w_symbol->s_name);
            goto fail;
        }
        else if (!garray_getfloatarray(garrays[i], &vecsize, &vecs[i]))
            error("%s: bad template for tabwrite",argv[i].a_w.w_symbol->s_name);
        if (nframes > vecsize - onset)
            nframes = vecsize - onset;

        for (j = 0; j < nframes; j++) /* FIXED (Antoine Rousseau)*/
        {
            ftmp=vecs[i][j+onset];
            if (ftmp > biggest) biggest = ftmp;
            else if (-ftmp > biggest) biggest = -ftmp;
        }
    }

    if (nframes <= 0)
    {
        pd_error(obj, "soundfiler2_write: no samples at onset %ld", onset);
        goto fail;
    }

    if ((fd = create_soundfile2(canvas, filesym->s_name, filetype,
                                nframes, bytespersamp, bigendian, nchannels,
                                swap)) < 0)
    {
        post("%s: %s\n", filesym->s_name, strerror(errno));
        goto fail;
    }
    if (!normalize)
    {
        if ((bytespersamp != 4) && (biggest > 1))
        {
            //post("%s: normalizing max amplitude %f to 1", filesym->s_name, biggest);
            normalize = 1;
        }
        //else post("%s: biggest amplitude = %f", filesym->s_name, biggest);
    }
    if (normalize)
        normfactor = (biggest > 0 ? 32767./(32768. * biggest) : 1);
    else normfactor = amp;

    /*post("%s: biggest amplitude = %f , normfactor = %f",
    	filesym->s_name, biggest,normfactor);*/

    bufframes = SAMPBUFSIZE / (nchannels * bytespersamp);

    for (itemswritten = 0; itemswritten < nframes; )
    {
        int thiswrite = nframes - itemswritten, nitems, nbytes;
        thiswrite = (thiswrite > bufframes ? bufframes : thiswrite);
        soundfile_xferout(argc, vecs, (unsigned char *)sampbuf, thiswrite,
                          onset, bytespersamp, bigendian, normfactor);
        nbytes = write(fd, sampbuf, nchannels * bytespersamp * thiswrite);
        if (nbytes < nchannels * bytespersamp * thiswrite)
        {
            post("%s: %s", filesym->s_name, strerror(errno));
            if (nbytes > 0)
                itemswritten += nbytes / (nchannels * bytespersamp);
            break;
        }
        itemswritten += thiswrite;
        onset += thiswrite;
    }
    if (fd >= 0)
    {
        soundfile_finishwrite(obj, filesym->s_name, fd,
                              filetype, nframes, itemswritten, nchannels * bytespersamp, swap);
        close (fd);
    }
    return ((float)itemswritten);
usage:
    pd_error(obj, "usage: write [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -bytes <n> -wave -aiff -nextstep ...");
    post("-big -little -normalize -amp <n>");
    post("(defaults to a 16-bit wave file).");
fail:
    if (fd >= 0)
        close (fd);
    return (0);
}

static void soundfiler2_write(t_soundfiler2 *x, t_symbol *s,
                              int argc, t_atom *argv)
{
    long bozo = soundfiler2_dowrite(x, x->x_canvas,
                                    argc, argv);
    outlet_float(x->x_obj.ob_outlet, (float)bozo);
}

void soundfiler2_setup(void)
{
    soundfiler2_class = class_new(gensym("soundfiler2"), (t_newmethod)soundfiler2_new,
                                  0, sizeof(t_soundfiler2), 0, 0);
    class_addmethod(soundfiler2_class, (t_method)soundfiler2_read, gensym("read"),
                    A_GIMME, 0);
    class_addmethod(soundfiler2_class, (t_method)soundfiler2_write,
                    gensym("write"), A_GIMME, 0);
}

/************************* readsf object ******************************/

/* READSF uses the Posix threads package; for the moment we're Linux
only although this should be portable to the other platforms.

Each instance of readsf~ owns a "child" thread for doing the UNIX (NT?) file
reading.  The parent thread signals the child each time:
    (1) a file wants opening or closing;
    (2) we've eaten another 1/16 of the shared buffer (so that the
    	child thread should check if it's time to read some more.)
The child signals the parent whenever a read has completed.  Signalling
is done by setting "conditions" and putting data in mutex-controlled common
areas.
*/

#ifdef __linux__

#define MAXBYTESPERSAMPLE 4
#define MAXVECSIZE 128

#define READSIZE 65536
#define WRITESIZE 65536
#define DEFBUFPERCHAN 262144
#define MINBUFSIZE (4 * READSIZE)
#define MAXBUFSIZE 16777216 	/* arbitrary; just don't want to hang malloc */

#define REQUEST_NOTHING 0
#define REQUEST_OPEN 1
#define REQUEST_CLOSE 2
#define REQUEST_QUIT 3
#define REQUEST_BUSY 4

#define STATE_IDLE 0
#define STATE_STARTUP 1
#define STATE_STREAM 2

static t_class *readsfv_class;

static t_sample *(tmpvec[MAXSFCHANS]);

typedef struct _readsf
{
    t_object x_obj;
    t_canvas *x_canvas;
    t_clock *x_clock;
    char *x_buf;    	    	    	    /* soundfile buffer */
    int x_bufsize;  	    	    	    /* buffer size in bytes */
    int x_noutlets; 	    	    	    /* number of audio outlets */
    t_sample *(x_outvec[MAXSFCHANS]);	    /* audio vectors */
    int x_vecsize;  	    	    	    /* vector size for transfers */
    t_outlet *x_bangout;  	    	    /* bang-on-done outlet */
    int x_state;    	    	    	    /* opened, running, or idle */
    /* parameters to communicate with subthread */
    int x_requestcode;	    /* pending request from parent to I/O thread */
    char *x_filename;	    /* file to open (string is permanently allocated) */
    int x_fileerror;	    /* slot for "errno" return */
    int x_skipheaderbytes;  /* size of header we'll skip */
    int x_bytespersample;   /* bytes per sample (2 or 3) */
    int x_bigendian;        /* true if file is big-endian */
    int x_sfchannels;	    /* number of channels in soundfile */
    long x_onsetframes;	    /* number of sample frames to skip */
    long x_bytelimit;	    /* max number of data bytes to read */
    int x_fd;	    	    /* filedesc */
    int x_fifosize; 	    /* buffer size appropriately rounded down */
    int x_fifohead; 	    /* index of next byte to get from file */
    int x_fifotail; 	    /* index of next byte the ugen will read */
    int x_eof;   	    /* true if fifohead has stopped changing */
    int x_sigcountdown;     /* counter for signalling child for more data */
    int x_sigperiod;	    /* number of ticks per signal */
    int x_filetype; 	    /* writesf~ only; type of file to create */
    int x_itemswritten;     /* writesf~ only; items writen */
    int x_swap; 	    /* writesf~ only; true if byte swapping */
    float x_f; 	    	    /* writesf~ only; scalar for signal inlet */
    /*----HACK------*/
    float x_speed;    /*speed of reading*/
    float x_frac;    /*fractionnal part of sample to play next buffer*/

    pthread_mutex_t x_mutex;
    pthread_cond_t x_requestcondition;
    pthread_cond_t x_answercondition;
    pthread_t x_childthread;
} t_readsf;


/************** the child thread which performs file I/O ***********/

#if 0
static void pute(char *s)   /* debug routine */
{
    write(2, s, strlen(s));
}
#else
#define pute(x)
#endif

#if 1
#define sfread_cond_wait pthread_cond_wait
#define sfread_cond_signal pthread_cond_signal
#else
#include <sys/time.h>    /* debugging version... */
#include <sys/types.h>
static void readsf_fakewait(pthread_mutex_t *b)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 1000000;
    pthread_mutex_unlock(b);
    select(0, 0, 0, 0, &timout);
    pthread_mutex_lock(b);
}

static void readsf_banana( void)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 200000;
    pute("banana1\n");
    select(0, 0, 0, 0, &timout);
    pute("banana2\n");
}


#define sfread_cond_wait(a,b) readsf_fakewait(b)
#define sfread_cond_signal(a)
#endif

static void *readsf_child_main(void *zz)
{
    t_readsf *x = zz;
    pute("1\n");
    pthread_mutex_lock(&x->x_mutex);
    while (1)
    {
        int fd, fifohead;
        char *buf;
        pute("0\n");
        if (x->x_requestcode == REQUEST_NOTHING)
        {
            pute("wait 2\n");
            sfread_cond_signal(&x->x_answercondition);
            sfread_cond_wait(&x->x_requestcondition, &x->x_mutex);
            pute("3\n");
        }
        else if (x->x_requestcode == REQUEST_OPEN)
        {
            char boo[80];
            int sysrtn, wantbytes;

            /* copy file stuff out of the data structure so we can
            relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->x_onsetframes;
            long bytelimit = 0x7fffffff;
            int skipheaderbytes = x->x_skipheaderbytes;
            int bytespersample = x->x_bytespersample;
            int sfchannels = x->x_sfchannels;
            int bigendian = x->x_bigendian;
            char *filename = x->x_filename;
            char *dirname = canvas_getdir(x->x_canvas)->s_name;
            /* alter the request code so that an ensuing "open" will get
            noticed. */
            pute("4\n");
            x->x_requestcode = REQUEST_BUSY;
            x->x_fileerror = 0;

            /* if there's already a file open, close it */
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
                if (x->x_requestcode != REQUEST_BUSY)
                    goto lost;
            }
            /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->x_mutex);
            fd = open_soundfile(dirname, filename,
                                skipheaderbytes, &bytespersample, &bigendian,
                                &sfchannels, &bytelimit, onsetframes);
            pthread_mutex_lock(&x->x_mutex);

            pute("5\n");
            /* copy back into the instance structure. */
            x->x_bytespersample = bytespersample;
            x->x_sfchannels = sfchannels;
            x->x_bigendian = bigendian;
            x->x_fd = fd;
            x->x_bytelimit = bytelimit;
            if (fd < 0)
            {
                x->x_fileerror = errno;
                x->x_eof = 1;
                pute("open failed\n");
                pute(filename);
                pute(dirname);
                goto lost;
            }
            /* check if another request has been made; if so, field it */
            if (x->x_requestcode != REQUEST_BUSY)
                goto lost;
            pute("6\n");
            x->x_fifohead = 0;
            /* set fifosize from bufsize.  fifosize must be a
            multiple of the number of bytes eaten for each DSP
            tick.  We pessimistically assume MAXVECSIZE samples
            per tick since that could change.  There could be a
            problem here if the vector size increases while a
            soundfile is being played...  */
            x->x_fifosize = x->x_bufsize - (x->x_bufsize %
                                            (x->x_bytespersample * x->x_sfchannels * MAXVECSIZE));
            /* arrange for the "request" condition to be signalled 16
            times per buffer */
            sprintf(boo, "fifosize %d\n",
                    x->x_fifosize);
            pute(boo);
            x->x_sigcountdown = x->x_sigperiod =
                                    (x->x_fifosize /
                                     (16 * x->x_bytespersample * x->x_sfchannels *
                                      x->x_vecsize));
            /* in a loop, wait for the fifo to get hungry and feed it */

            while (x->x_requestcode == REQUEST_BUSY)
            {
                int fifosize = x->x_fifosize;
                pute("77\n");
                if (x->x_eof)
                    break;
                if (x->x_fifohead >= x->x_fifotail)
                {
                    /* if the head is >= the tail, we can immediately read
                    to the end of the fifo.  Unless, that is, we would
                    read all the way to the end of the buffer and the
                    "tail" is zero; this would fill the buffer completely
                    which isn't allowed because you can't tell a completely
                    full buffer from an empty one. */
                    if (x->x_fifotail || (fifosize - x->x_fifohead > READSIZE))
                    {
                        wantbytes = fifosize - x->x_fifohead;
                        if (wantbytes > READSIZE)
                            wantbytes = READSIZE;
                        if (wantbytes > x->x_bytelimit)
                            wantbytes = x->x_bytelimit;
                        sprintf(boo, "head %d, tail %d, size %d\n",
                                x->x_fifohead, x->x_fifotail, wantbytes);
                        pute(boo);
                    }
                    else
                    {
                        pute("wait 7a ...\n");
                        sfread_cond_signal(&x->x_answercondition);
                        pute("signalled\n");
                        sfread_cond_wait(&x->x_requestcondition,
                                         &x->x_mutex);
                        pute("7a done\n");
                        continue;
                    }
                }
                else
                {
                    /* otherwise check if there are at least READSIZE
                    bytes to read.  If not, wait and loop back. */
                    wantbytes =  x->x_fifotail - x->x_fifohead - 1;
                    if (wantbytes < READSIZE)
                    {
                        pute("wait 7...\n");
                        sfread_cond_signal(&x->x_answercondition);
                        sfread_cond_wait(&x->x_requestcondition,
                                         &x->x_mutex);
                        pute("7 done\n");
                        continue;
                    }
                    else wantbytes = READSIZE;
                }
                pute("8\n");
                fd = x->x_fd;
                buf = x->x_buf;
                fifohead = x->x_fifohead;
                pthread_mutex_unlock(&x->x_mutex);
                sysrtn = read(fd, buf + fifohead, wantbytes);
                pthread_mutex_lock(&x->x_mutex);
                if (x->x_requestcode != REQUEST_BUSY)
                    break;
                if (sysrtn < 0)
                {
                    pute("fileerror\n");
                    x->x_fileerror = errno;
                    break;
                }
                else if (sysrtn == 0)
                {
                    x->x_eof = 1;
                    break;
                }
                else
                {
                    x->x_fifohead += sysrtn;
                    x->x_bytelimit -= sysrtn;
                    if (x->x_bytelimit <= 0)
                    {
                        x->x_eof = 1;
                        break;
                    }
                    if (x->x_fifohead == fifosize)
                        x->x_fifohead = 0;
                }
                sprintf(boo, "after: head %d, tail %d\n",
                        x->x_fifohead, x->x_fifotail);
                pute(boo);
                /* signal parent in case it's waiting for data */
                sfread_cond_signal(&x->x_answercondition);
            }
lost:

            if (x->x_requestcode == REQUEST_BUSY)
                x->x_requestcode = REQUEST_NOTHING;
            /* fell out of read loop: close file if necessary,
            set EOF and signal once more */
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            sfread_cond_signal(&x->x_answercondition);

        }
        else if (x->x_requestcode == REQUEST_CLOSE)
        {
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            if (x->x_requestcode == REQUEST_CLOSE)
                x->x_requestcode = REQUEST_NOTHING;
            sfread_cond_signal(&x->x_answercondition);
        }
        else if (x->x_requestcode == REQUEST_QUIT)
        {
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            x->x_requestcode = REQUEST_NOTHING;
            sfread_cond_signal(&x->x_answercondition);
            break;
        }
        else
        {
            pute("13\n");
        }
    }
    pute("thread exit\n");
    pthread_mutex_unlock(&x->x_mutex);
    return (0);
}

/******** the object proper runs in the calling (parent) thread ****/

static void readsf_tick(t_readsf *x);

static void *readsf_new(t_floatarg fnchannels, t_floatarg fbufsize)
{
    t_readsf *x;
    int nchannels = fnchannels, bufsize = fbufsize, i;
    char *buf;

    if (nchannels < 1)
        nchannels = 1;
    else if (nchannels > MAXSFCHANS)
        nchannels = MAXSFCHANS;
    if (bufsize <= 0) bufsize = DEFBUFPERCHAN * nchannels;
    else if (bufsize < MINBUFSIZE)
        bufsize = MINBUFSIZE;
    else if (bufsize > MAXBUFSIZE)
        bufsize = MAXBUFSIZE;
    buf = getbytes(bufsize);
    if (!buf) return (0);

    x = (t_readsf *)pd_new(readsfv_class);

    for (i = 0; i < nchannels; i++)
        outlet_new(&x->x_obj, gensym("signal"));
    x->x_noutlets = nchannels;
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    pthread_mutex_init(&x->x_mutex, 0);
    pthread_cond_init(&x->x_requestcondition, 0);
    pthread_cond_init(&x->x_answercondition, 0);
    x->x_vecsize = MAXVECSIZE;
    x->x_state = STATE_IDLE;
    x->x_clock = clock_new(x, (t_method)readsf_tick);
    x->x_canvas = canvas_getcurrent();
    x->x_bytespersample = 2;
    x->x_sfchannels = 1;
    x->x_fd = -1;
    x->x_buf = buf;
    x->x_bufsize = bufsize;
    x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;
    pthread_create(&x->x_childthread, 0, readsf_child_main, x);
    return (x);
}

static void readsf_tick(t_readsf *x)
{
    outlet_bang(x->x_bangout);
}

static t_int *readsf_perform(t_int *w)
{
    t_readsf *x = (t_readsf *)(w[1]);
    int vecsize = x->x_vecsize, noutlets = x->x_noutlets, i, j,
        bytespersample = x->x_bytespersample,
        bigendian = x->x_bigendian,wantsamples;
    float *fp,tmp,speed=x->x_speed;
    if (x->x_state == STATE_STREAM)
    {
        int wantbytes, nchannels, sfchannels = x->x_sfchannels;
        pthread_mutex_lock(&x->x_mutex);
        tmp=vecsize *speed+x->x_frac;
        wantsamples=(int)tmp;
        x->x_frac=tmp-wantsamples;

        if(!speed) goto idle;

        wantbytes = sfchannels * wantsamples * bytespersample;

        while (
            !x->x_eof && x->x_fifohead >= x->x_fifotail &&
            x->x_fifohead < x->x_fifotail + wantbytes-1)
        {
            pute("wait...\n");
            sfread_cond_signal(&x->x_requestcondition);
            sfread_cond_wait(&x->x_answercondition, &x->x_mutex);
            pute("done\n");
        }
        if (x->x_eof && x->x_fifohead >= x->x_fifotail &&
                x->x_fifohead < x->x_fifotail + wantbytes-1)
        {
            if (x->x_fileerror)
            {
                pd_error(x, "dsp: %s: %s", x->x_filename,
                         (x->x_fileerror == EIO ?
                          "unknown or bad header format" :
                          strerror(x->x_fileerror)));
            }
            clock_delay(x->x_clock, 0);
            x->x_state = STATE_IDLE;
            sfread_cond_signal(&x->x_requestcondition);
            pthread_mutex_unlock(&x->x_mutex);
            goto idle;
        }

        if(speed==1)
            soundfile_xferin(sfchannels, noutlets, x->x_outvec, 0,
                             (unsigned char *)(x->x_buf + x->x_fifotail), vecsize,
                             bytespersample, bigendian);
        else
        {
            soundfile_xferin(sfchannels, noutlets, tmpvec, 0,
                             (unsigned char *)(x->x_buf + x->x_fifotail), wantsamples,
                             bytespersample, bigendian);
            interpolate(noutlets,tmpvec,wantsamples,x->x_outvec,vecsize);
        }

        x->x_fifotail += wantbytes;
        if (x->x_fifotail >= x->x_fifosize)
            x->x_fifotail = 0;
        if ((--x->x_sigcountdown) <= 0)
        {
            sfread_cond_signal(&x->x_requestcondition);
            x->x_sigcountdown = x->x_sigperiod;
        }
        pthread_mutex_unlock(&x->x_mutex);
    }
    else
    {
idle:
        for (i = 0; i < noutlets; i++)
            for (j = vecsize, fp = x->x_outvec[i]; j--; )
                *fp++ = 0;
    }
    return (w+2);
}

static void readsf_start(t_readsf *x)
{
    /* start making output.  If we're in the "startup" state change
    to the "running" state. */
    if (x->x_state == STATE_STARTUP)
        x->x_state = STATE_STREAM;
    else pd_error(x, "readsf: start requested with no prior 'open'");
}

static void readsf_stop(t_readsf *x)
{
    /* LATER rethink whether you need the mutex just to set a variable? */
    pthread_mutex_lock(&x->x_mutex);
    x->x_state = STATE_IDLE;
    x->x_requestcode = REQUEST_CLOSE;
    sfread_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}

static void readsf_float(t_readsf *x, t_floatarg f)
{
    if (f != 0)
        readsf_start(x);
    else readsf_stop(x);
}

static void readsf_speed(t_readsf *x, t_floatarg f)
{
    if((f>=0)&&(f<=8))
        x->x_speed=f;
}

/* open method.  Called as:
open filename [skipframes headersize channels bytespersamp endianness]
	(if headersize is zero, header is taken to be automatically
detected; thus, use the special "-1" to mean a truly headerless file.)
*/

static void readsf_open(t_readsf *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *filesym = atom_getsymbolarg(0, argc, argv);
    t_float onsetframes = atom_getfloatarg(1, argc, argv);
    t_float headerbytes = atom_getfloatarg(2, argc, argv);
    t_float channels = atom_getfloatarg(3, argc, argv);
    t_float bytespersamp = atom_getfloatarg(4, argc, argv);
    t_symbol *endian = atom_getsymbolarg(5, argc, argv);
    if (!*filesym->s_name)
        return;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_OPEN;
    x->x_filename = filesym->s_name;
    x->x_fifotail = 0;
    x->x_fifohead = 0;
    if (*endian->s_name == 'b')
        x->x_bigendian = 1;
    else if (*endian->s_name == 'l')
        x->x_bigendian = 0;
    else if (*endian->s_name)
        pd_error(x, "endianness neither 'b' nor 'l'");
    else x->x_bigendian = garray_ambigendian();
    x->x_onsetframes = (onsetframes > 0 ? onsetframes : 0);
    x->x_skipheaderbytes = (headerbytes > 0 ? headerbytes :
                            (headerbytes == 0 ? -1 : 0));
    x->x_sfchannels = (channels >= 1 ? channels : 1);
    x->x_bytespersample = (bytespersamp > 2 ? bytespersamp : 2);
    x->x_eof = 0;
    x->x_fileerror = 0;
    x->x_state = STATE_STARTUP;
    x->x_speed=1;
    x->x_frac=0;
    sfread_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}

static void readsf_dsp(t_readsf *x, t_signal **sp)
{
    int i, noutlets = x->x_noutlets;
    pthread_mutex_lock(&x->x_mutex);
    x->x_vecsize = sp[0]->s_n;

    x->x_sigperiod = (x->x_fifosize /
                      (x->x_bytespersample * x->x_sfchannels * x->x_vecsize));
    for (i = 0; i < noutlets; i++)
        x->x_outvec[i] = sp[i]->s_vec;
    pthread_mutex_unlock(&x->x_mutex);
    dsp_add(readsf_perform, 1, x);
}

static void readsf_print(t_readsf *x)
{
    post("state %d", x->x_state);
    post("fifo head %d", x->x_fifohead);
    post("fifo tail %d", x->x_fifotail);
    post("fifo size %d", x->x_fifosize);
    post("fd %d", x->x_fd);
    post("eof %d", x->x_eof);
}

static void readsf_free(t_readsf *x)
{
    /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_QUIT;
    post("stopping readsf thread...");
    sfread_cond_signal(&x->x_requestcondition);
    while (x->x_requestcode != REQUEST_NOTHING)
    {
        post("signalling...");
        sfread_cond_signal(&x->x_requestcondition);
        sfread_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    pthread_mutex_unlock(&x->x_mutex);
    if (pthread_join(x->x_childthread, &threadrtn))
        error("readsf_free: join failed");
    post("... done.");

    pthread_cond_destroy(&x->x_requestcondition);
    pthread_cond_destroy(&x->x_answercondition);
    pthread_mutex_destroy(&x->x_mutex);
    freebytes(x->x_buf, x->x_bufsize);
    clock_free(x->x_clock);
}

#endif /* __linux__ */

void readsfv_tilde_setup(void)
{
#ifdef __linux__
    int i;

    readsfv_class = class_new(gensym("readsfv~"), (t_newmethod)readsf_new,
                              (t_method)readsf_free, sizeof(t_readsf), 0, A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addfloat(readsfv_class, (t_method)readsf_float);
    class_addmethod(readsfv_class, (t_method)readsf_speed, gensym("speed"), A_FLOAT,0);
    class_addmethod(readsfv_class, (t_method)readsf_start, gensym("start"), 0);
    class_addmethod(readsfv_class, (t_method)readsf_stop, gensym("stop"), 0);
    class_addmethod(readsfv_class, (t_method)readsf_dsp, gensym("dsp"), 0);
    class_addmethod(readsfv_class, (t_method)readsf_open, gensym("open"),
                    A_GIMME, 0);
    class_addmethod(readsfv_class, (t_method)readsf_print, gensym("print"), 0);

    for(i=0; i<MAXSFCHANS; i++)
        tmpvec[i]=getbytes(sizeof(t_sample)*8*1024);
#endif /* __linux__ */
}



#endif /* NOT _WIN32 */
