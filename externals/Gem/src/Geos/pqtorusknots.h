/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  p,q-torus knots

  Copyright (c) 2004 tigital
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_PQTORUSKNOTS_H_
#define _INCLUDE__GEM_GEOS_PQTORUSKNOTS_H_

#include "Base/GemShape.h"
#include <math.h>

#ifdef __ppc__
#include "Utils/Functions.h"
#undef sqrt
#define sqrt fast_sqrtf
#endif

#include <string.h>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pqtorusknots
    
  Creates a pqtorus with knots

  KEYWORDS
  geo

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN pqtorusknots : public GemShape
{
  CPPEXTERN_HEADER(pqtorusknots, GemShape);

    public:

  //////////
  // Constructor
  pqtorusknots(t_floatarg m_P, t_floatarg m_Q);
 protected:
    	
  //////////
  // Destructor
  virtual ~pqtorusknots();


  //////////
  // The variables of the object
  void	    	scaleMess(float size);
  void	    	stepsMess(float size);
  void	    	facetsMess(float size);
  void	    	thickMess(float size);
  virtual void	clumpMess(float clumps, float clumpOffset, float clumpScale);
  virtual void	uvScaleMess(float uScale, float vScale);
  virtual void	pqMess(float p, float q);
  virtual void	typeMess(t_symbol*s);

  // setup the vertices and normals and...
  virtual void genVert();

  //////////
  // Do the renderShapeing
  virtual void 	renderShape(GemState *state);
  virtual void 	postrenderShape(GemState *state);

  //////////
  // Number of steps in the torus knot
  GLint	    	m_steps;
		
  //////////
  // Number of facets in the torus knot
  GLint	    	m_facets;

  //////////
  // The scale of the object
  GLfloat	m_scale;

  //////////
  // The thickness of the knot
  GLfloat	m_thickness;
		
  //////////
  // The thickness of the knot
  GLfloat	m_clumps;
  GLfloat	m_clumpOffset;
  GLfloat	m_clumpScale;
		
  //////////
  // The coordinate scale of the knot
  GLfloat	m_uScale;
  GLfloat	m_vScale;
		
  //////////
  // The thickness of the knot
  GLfloat	m_P;
  GLfloat	m_Q;
		
  //////////
  // Vertex Array Stuff
  GLfloat*      m_Vertex;
  GLfloat*	m_Normal;
  GLfloat*	m_Texcoord[4];
  GLfloat*      m_texcoords;
  GLuint*	m_Index;
  int		m_Indices;
  int		m_Vertices;
  int		m_PrimitiveType;

 private:

  //////////
  // Static member functions
  static void 	scaleMessCallback(void *data, t_floatarg size);
  static void 	stepsMessCallback(void *data, t_floatarg size);
  static void 	facetsMessCallback(void *data, t_floatarg size);
  static void 	thickMessCallback(void *data, t_floatarg size);
  static void 	clumpMessCallback(void *data, t_floatarg clumps, t_floatarg clumpOffsets, t_floatarg clumpScale);
  static void 	uvScaleMessCallback(void *data, t_floatarg uScale, t_floatarg vScale);
  static void 	pqMessCallback(void *data, t_floatarg p, t_floatarg q);
  static void 	typeMessCallback(void *data, t_symbol*s);

};

#endif	// for header file
