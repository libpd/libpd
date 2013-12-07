/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_set

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_SET_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_SET_H_

#include "vertex_scale.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_set
    
    Creates a vertex_set

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_set : public vertex_scale
{
  CPPEXTERN_HEADER(vertex_set, vertex_scale);

    public:

  //////////
  // Constructor
  vertex_set(int, t_atom*);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~vertex_set(void);

  virtual void paramMess(int,t_atom*);
  float	m_x,m_y,m_z,m_w;
  
  //////////
  // Do the rendering
  virtual void  vertexProcess(int,GLfloat *);
};

#endif	// for header file
