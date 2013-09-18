/*
 *  GEM - Graphics Environment for Multimedia
 *
 *  rubber.h
 *  based on SGI opengl democode
 *  gem_darwin
 *
 *  Created by Jamie Tittle on Sun Jan 19 2003.
 *  Copyright (c) 2003-2006 tigital. All rights reserved.
 *    For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 *
 */
#ifndef _INCLUDE__GEM_GEOS_RUBBER_H_
#define _INCLUDE__GEM_GEOS_RUBBER_H_

#include "Base/GemShape.h"
#include "Gem/Manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef __ppc__
#include "Utils/Functions.h"
#undef sqrt
#define sqrt fast_sqrtf
#endif

typedef struct {
  float x[3];
  float v[3];
  float t[2];
  int nail;
} MASS;

typedef struct {
  int i, j;
  float r;
} SPRING;

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    rubber
    
    based on the SGI demo distort

KEYWORDS
    geo
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN rubber : public GemShape
{
  CPPEXTERN_HEADER(rubber, GemShape);

    public:

  //////////
  // Constructor
  rubber( t_floatarg width, t_floatarg height);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~rubber();

  //////////
  // The height of the object
  float		ctrX, ctrY;
  void		heightMess(float height);
  void		ctrXMess(float center);
  void		ctrYMess(float center);
         
  //////////
  // Do the rendering
  virtual void 	renderShape(GemState *state);
  
  //////////
  // How the object should be drawn
  virtual void	typeMess(t_symbol *type);
        
  virtual void	rubber_init();
  virtual void	rubber_dynamics();
  virtual void	rubber_bang();
  virtual int	rubber_grab();
		
  //////////
  // The height of the object
  GLfloat	    	m_height;

  //////////
  // The height inlet
  t_inlet       *m_inletH;
  t_inlet		*inletcX;
  t_inlet		*inletcY;
  
  //////////
  // member variables
  
  int		m_speed;
  // index of grabbed mass point
  int		m_grab;
  int		m_alreadyInit;
  float		m_springKS;
  float		m_drag;
  float 	xsize, ysize, ysize0;

  // number of grid-segments in X/Y direction (defaults: 32);
  int           m_grid_sizeX,m_grid_sizeY;
  MASS		*m_mass;
  SPRING	*m_spring;
  int		m_spring_count;
    
 private:

  //////////
  // static member functions
  static void	bangMessCallback(void *data);
  static void 	heightMessCallback(void *data, t_floatarg height);
  static void 	ctrXMessCallback(void *data, t_floatarg center);
  static void 	ctrYMessCallback(void *data, t_floatarg center);
  static void 	dragMessCallback(void *data, t_floatarg drag);
  static void 	springMessCallback(void *data, t_floatarg spring);
};

#endif	// for header file
