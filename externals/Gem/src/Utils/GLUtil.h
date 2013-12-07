/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    GemGLUtil.h
       - contains functions for graphics
       - part of GEM

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_UTILS_GLUTIL_H_
#define _INCLUDE__GEM_UTILS_GLUTIL_H_

#include "Gem/ExportDef.h"

/* for t_symbol/t_atom */
/* LATER get rid of that (std::string) */
#include "Gem/RTE.h"

/* for GLenum */
#include "Gem/GemGL.h"

GEM_EXTERN extern GLenum		glReportError (void);
GEM_EXTERN extern int           getGLdefine(const char *name);
GEM_EXTERN extern int           getGLdefine(const t_symbol *name);
GEM_EXTERN extern int           getGLdefine(const t_atom *name);
GEM_EXTERN extern int           getGLbitfield(int argc, t_atom *argv);
#endif  // for header file

