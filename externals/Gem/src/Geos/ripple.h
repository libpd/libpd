/*
 *  GEM - Graphics Environment for Multimedia
 *
 *  ripple.h
 *  gem_darwin
 *
 *  Created by Jamie Tittle on Sun Jan 19 2003.
 *  Copyright (c) 2003 tigital. All rights reserved.
 *    For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 *
 */
#ifndef _INCLUDE__GEM_GEOS_RIPPLE_H_
#define _INCLUDE__GEM_GEOS_RIPPLE_H_

#include "Base/GemShape.h"
#include "Gem/Manager.h"
#include <string.h>
#include <math.h>

#ifdef __ppc__
#include "Utils/Functions.h"
#undef sqrt
#define sqrt fast_sqrtf
#endif

#define RIPPLE_LENGTH     2048
#define RIPPLE_CYCLES     18
#define RIPPLE_AMPLITUDE  0.125
#define RIPPLE_STEP	  7
#define RIPPLE_COUNT	  7

#define GRID_SIZE_X   32
#define GRID_SIZE_Y   32

#define GRID_MAX_X   128
#define GRID_MAX_Y   128

#define CLIP_NEAR  0.0
#define CLIP_FAR   1000.0

typedef struct {	/* precomputed displacement vector table */
  float dx[2];
  int r;		/* distance from origin, in pixels */
} RIPPLE_VECTOR;

typedef struct {	/* precomputed ripple amplitude table */
  float amplitude;
} RIPPLE_AMP;

typedef struct {
  float x[2];		/* initial vertex location */
  float t[2];		/* texture coordinate */
  float dt[2];		/* default texture coordinate */
} RIPPLE_VERTEX;

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  ripple
    
  based on the SGI demo distort

  KEYWORDS
  geo
    
  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN ripple : public GemShape
{
  CPPEXTERN_HEADER(ripple, GemShape);

    public:

  //////////
  // Constructor
  ripple( t_floatarg width, t_floatarg height);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~ripple();

  //////////
  // The height of the object
  short		m_ctrX, m_ctrY;
  void	 	heightMess(float height);
  void		ctrXMess(float center);
  void		ctrYMess(float center);
  //////////
  // Do the rendering
  virtual void 	renderShape(GemState *state);
  
  //////////
  // How the object should be drawn
  virtual void	typeMess(t_symbol *type);
  void	ripple_dynamics();
  void	ripple_init();
  float	ripple_distance( int gx, int gy, int cx, int cy);
  int	ripple_max_distance( int gx, int gy );
  void	ripple_bang();
  void	precalc_ripple_vector();
  void	precalc_ripple_amp();
		
  //////////
  // The height of the object
  GLfloat	    	m_height;

  //////////
  // The height inlet
  t_inlet   *m_inletH;
  t_inlet		*m_inletcX;
  t_inlet		*m_inletcY;
  
  //////////
  // getStuff
  int m_gridX, m_gridY;
  
  bool		m_alreadyInit;
  float		m_sizeX, m_sizeY, m_sizeY0;
  RIPPLE_VECTOR m_rippleVector[GRID_MAX_X][GRID_MAX_Y];
  RIPPLE_AMP m_rippleAmp[RIPPLE_LENGTH];
  RIPPLE_VERTEX m_rippleVertex[GRID_MAX_X][GRID_MAX_Y];

  int m_cx[RIPPLE_COUNT];
  int m_cy[RIPPLE_COUNT];
  int m_t[RIPPLE_COUNT];
  int m_max[RIPPLE_COUNT];

  int m_rippleMax;
 private:

  //////////
  // static member functions
  static void	bangMessCallback(void *data);
  static void 	heightMessCallback(void *data, t_floatarg height);
  static void 	ctrXMessCallback(void *data, t_floatarg center);
  static void 	ctrYMessCallback(void *data, t_floatarg center);
};

#endif	// for header file
