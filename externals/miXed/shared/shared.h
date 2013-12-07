/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __SHARED_H__
#define __SHARED_H__

/* LATER find a proper place for #include <limits.h> */
#ifdef INT_MAX
#define SHARED_INT_MAX  INT_MAX
#else
#define SHARED_INT_MAX  0x7FFFFFFF
#endif
#ifdef INT_MIN
#define SHARED_INT_MIN  INT_MIN
#else
#define SHARED_INT_MIN  ((int)0x80000000)
#endif
/* LATER find a proper place for #include <float.h> */
#ifdef FLT_MAX
#define SHARED_FLT_MAX  FLT_MAX
#else
#define SHARED_FLT_MAX  1E+36
#endif

typedef unsigned long shared_t_bitmask;

/* this is for GNU/Linux and also Debian GNU/Hurd and GNU/kFreeBSD */
#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__) || defined(__GLIBC__)
#include <sys/types.h>
#ifndef int32
typedef int32_t int32;
#endif
#ifndef uint32
typedef u_int32_t uint32;
#endif
#ifndef int16
typedef int16_t int16;
#endif
#ifndef uint16
typedef u_int16_t uint16;
#endif
#ifndef uchar
typedef u_int8_t uchar;
#endif
#include <endian.h>
#if !defined(__BYTE_ORDER) || !defined(__LITTLE_ENDIAN)                         
#error No byte order defined                                                    
#endif                                                                          
#if __BYTE_ORDER == __LITTLE_ENDIAN                                             
#define SHARED_HIOFFSET   1
#define SHARED_LOWOFFSET  0
#else
#define SHARED_HIOFFSET   0
#define SHARED_LOWOFFSET  1
#endif
#endif

#ifdef NT
#ifndef int32
typedef long int32;
#endif
#ifndef uint32
typedef unsigned long uint32;
#endif
#ifndef int16
typedef short int16;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uchar
typedef unsigned char uchar;
#endif
#define SHARED_HIOFFSET   1
#define SHARED_LOWOFFSET  0
#endif

#ifdef MACOSX
#ifndef int32
typedef int int32;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif
#ifndef int16
typedef short int16;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uchar
typedef unsigned char uchar;
#endif
#ifdef __BIG_ENDIAN__
#define SHARED_HIOFFSET   0
#define SHARED_LOWOFFSET  1
#else
#define SHARED_HIOFFSET   1
#define SHARED_LOWOFFSET  0
#endif
#endif

#ifdef IRIX
#ifndef int32
typedef long int32;
#endif
#ifndef uint32
typedef unsigned long uint32;
#endif
#ifndef int16
typedef short int16;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uchar
typedef unsigned char uchar;
#endif
#define SHARED_HIOFFSET   0
#define SHARED_LOWOFFSET  1
#endif

#ifdef __FreeBSD__
#include <sys/types.h>
#ifndef int32
typedef int32_t int32;
#endif
#ifndef uint32
typedef u_int32_t uint32;
#endif
#ifndef int16
typedef int16_t int16;
#endif
#ifndef uint16
typedef u_int16_t uint16;
#endif
#ifndef uchar
typedef u_int8_t uchar;
#endif
#include <machine/endian.h>
#if BYTE_ORDER == LITTLE_ENDIAN
#define SHARED_HIOFFSET   1
#define SHARED_LOWOFFSET  0
#else
#define SHARED_HIOFFSET   0
#define SHARED_LOWOFFSET  1
#endif
#endif

#define SHARED_UNITBIT32  1572864.  /* 3*(2^19) gives 32 fractional bits */
#define SHARED_UNITBIT0  6755399441055744.  /* 3*(2^51), no fractional bits */
#define SHARED_UNITBIT0_HIPART  0x43380000

typedef union _shared_wrappy
{
    double  w_d;
    int32   w_i[2];
} t_shared_wrappy;

typedef union _shared_floatint
{
    t_float  fi_f;
    int32    fi_i;
} t_shared_floatint;

#define SHARED_TRUEBITS  0x3f800000  /* t_float f = 1; *(int32 *)&f */

#define SHARED_PI   3.14159265359
#define SHARED_2PI  6.28318530718

#ifndef PD_BADFLOAT
#ifdef __i386__
#define PD_BADFLOAT(f) ((((*(unsigned int*)&(f))&0x7f800000)==0) || \
    (((*(unsigned int*)&(f))&0x7f800000)==0x7f800000))
#else
#define PD_BADFLOAT(f) 0
#endif
#endif

#ifndef PD_BIGORSMALL
#ifdef __i386__
#define PD_BIGORSMALL(f) ((((*(unsigned int*)&(f))&0x60000000)==0) || \
    (((*(unsigned int*)&(f))&0x60000000)==0x60000000))
#else
#define PD_BIGORSMALL(f) 0
#endif
#endif

#endif
