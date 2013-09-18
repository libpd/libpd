/* udpsend~.h modified by Martin Peach from netsend~.h: */
/* ------------------------ netsend~ ------------------------------------------ */
/*                                                                              */
/* Tilde object to send uncompressed audio data to netreceive~.                 */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>.                               */
/* Based on streamout~ by Guenter Geiger.                                       */
/* Get source at http://www.akustische-kunst.org/                               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* This project was commissioned by the Society for Arts and Technology [SAT],  */
/* Montreal, Quebec, Canada, http://www.sat.qc.ca/.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


/* This file is based on and inspired by stream.h (C) Guenter Geiger 1999.   */
/* Some enhancements have been made with the goal of keeping compatibility   */
/* between the stream formats of streamout~/in~ and netsend~/receive~.       */

#define VERSION "0.34"

#define DEFAULT_AUDIO_CHANNELS 32       /* nax. number of audio channels we support */
#define DEFAULT_AUDIO_BUFFER_SIZE 2048 /*1024*/  /* number of samples in one audio block */
#define DEFAULT_UDP_PACKT_SIZE 8192     /* number of bytes we send in one UDP datagram (OS X only) */
#define DEFAULT_PORT 8000               /* default network port number */

#ifdef _WIN32
#ifndef HAVE_INT32_T
typedef int int32_t;
#define HAVE_INT32_T
#endif
#ifndef HAVE_INT16_T
typedef short int16_t;
#define HAVE_INT16_T
#endif
#ifndef HAVE_U_INT32_T
typedef unsigned int u_int32_t;
#define HAVE_U_INT32_T
#endif
#ifndef HAVE_U_INT16_T
typedef unsigned short u_int16_t;
#define HAVE_U_INT16_T
#endif
#endif

typedef union _flint
{
    int i32;
    t_float f32;
} flint;

/* format specific stuff */

#define SF_FLOAT  1
#define SF_DOUBLE 2     /* not implemented */
#define SF_8BIT   10
#define SF_16BIT  11
#define SF_32BIT  12    /* not implemented */
#define SF_ALAW   20    /* not implemented */
#define SF_MP3    30    /* not implemented */
#define SF_AAC    31    /* AAC encoding using FAAC */
#define SF_VORBIS 40    /* not implemented */
#define SF_FLAC   50    /* not implemented */

#define SF_SIZEOF(a) (a == SF_FLOAT ? sizeof(t_float) : \
                     a == SF_16BIT ? sizeof(short) : 1)

typedef struct _tag
{                           /* size (bytes) */
    char    tag[4];         /*  4  */ /*"TAG!"*/
    char    format;         /*  1  */
    int     count;          /*  4  */
    char    channels;       /*  1  */
    int     framesize;      /*  4  */
    char    reserved[2];    /*  2  */ /* pad to 16 bytes */
} t_tag;                    /*-----*/
                            /* 16  */

typedef struct _frame
{
    t_tag  tag;
    char  *data;
} t_frame;

/* fin udpsend~.h */
