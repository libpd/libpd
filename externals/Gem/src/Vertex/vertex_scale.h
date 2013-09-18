/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_scale

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_SCALE_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_SCALE_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_scale
    
    Creates a vertex_scale

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_scale : public GemBase
{
  CPPEXTERN_HEADER(vertex_scale, GemBase);

    public:

  //////////
  // Constructor
  vertex_scale(int, t_atom*);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~vertex_scale(void);

  virtual void paramMess(int,t_atom*);
  float	m_x,m_y,m_z,m_w;
  
  virtual void vertexMess(int offset, int count);
  int  m_offset,m_count;

  virtual void modeMess(int,t_atom*);
  bool m_vertex, m_color, m_normal, m_texture;


  t_inlet*m_parmIn, *m_vertIn;
  
  //////////
  // Do the rendering
  virtual void  vertexProcess(int,GLfloat *);
  virtual void 	render(GemState *state);

  static void 	paramMessCallback(void *data, t_symbol*, int, t_atom*);

 private:
        
  static void 	modeMessCallback(void *data, t_symbol*, int, t_atom*);
  static void 	vertexMessCallback(void *data, t_floatarg num, t_floatarg counter);
};

#endif	// for header file
