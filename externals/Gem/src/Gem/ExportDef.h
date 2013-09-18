/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Export crap

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_EXPORTDEF_H_
#define _INCLUDE__GEM_GEM_EXPORTDEF_H_

#if defined _MSC_VER
/* turn of some warnings on vc-compilers */

/* data conversion with possible loss of data */
# pragma warning( disable : 4244 )
/* identifier conversion with possible loss of data */
# pragma warning( disable : 4305 )
/* declspec without variable declaration */
# pragma warning( disable : 4091 )
/* M$DN classes exporting STL members... */
# pragma warning( disable : 4251 )
/* "switch" without "case" (just "default") */
# pragma warning( disable : 4065 )
/* Exception Specifications! */
# pragma warning (disable : 4290)

// Windows requires explicit import and exporting of functions and classes.
// While this is a pain to do sometimes, in large software development
//      projects, it is very usefull.
# define GEM_EXPORT __declspec(dllexport)
# define GEM_IMPORT __declspec(dllimport)

# define GEM_DEPRECATED __declspec(deprecated)

#elif defined __GNUC__
# define GEM_EXPORT
# define GEM_IMPORT

# define GEM_DEPRECATED __attribute__ ((deprecated))

#else
/* unknown compiler */
# warning set up compiler specific defines
#endif




#ifdef GEM_INTERNAL
# define GEM_EXTERN GEM_EXPORT
#else
# define GEM_EXTERN GEM_IMPORT
#endif

#endif	// for header file
