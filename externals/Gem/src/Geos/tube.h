/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  A tube.
  object by cyrille.henry@la-kitchen.fr
  This primitive create a kind of cilender with paramettre :
  Diameter of the 1st circle (1st base of the object)
  Diameter of the 2nd circle
  X, Y, Z displacement between the 2 circles
  X, Y rotation of the 1st circle
  X, Y rotation of the 2nd circle

  Copyright (c) 1997-1998 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_TUBE_H_
#define _INCLUDE__GEM_GEOS_TUBE_H_

#include "Base/GemShape.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  tube
    
  Creates a tube

  KEYWORDS
  geo
    
  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN tube : public GemShape
{
  CPPEXTERN_HEADER(tube, GemShape);

    public:

  //////////
  // Constructor
  tube(t_floatarg size, t_floatarg size2, t_floatarg high, t_floatarg order);
  //////////
		
 protected:
    	
  //////////
  // Destructor
  virtual ~tube();
  //////////

  // The size2 of the object
  void	    	sizeMess2(float size2);
 
  // The high of the object
  void	    	highMess(float high);

  // The TranslateX of the object
  void	    	TXMess(float TX);

  // The translateY of the object
  void	    	TYMess(float TY);

  // The rotX1 of the object
  void	    	rotX1Mess(float rotX1);

  // The rotY1 of the object
  void	    	rotY1Mess(float rotY1);

  // The rotX2 of the object
  void	    	rotX2Mess(float rotX2);

  // The rotY2 of the object
  void	    	rotY2Mess(float rotY2);

  // The #slices of the object
  void	    	slicesMess(int slices);


  //////////
  // Do the renderShapeing
  virtual void 	renderShape(GemState *state);

  //////////
  // How the object should be drawn
  virtual void	typeMess(t_symbol *type);

  //////////
  // 2 PI
  static const float TWO_PI;

  //////////
  // cos lookup table
  GLfloat		*m_cos;

  //////////
  // sin lookup table
  GLfloat		*m_sin;

  //////////
  // The size2 of the object
  GLfloat	    	m_size2;

  //////////
  // The size2 inlet
  t_inlet         *m_inlet2;

  //////////
  // The high of the object
  GLfloat	    	m_high;

  //////////
  // The high inlet
  t_inlet         *m_inlethigh;

  //////////
  // The TX of the object
  GLfloat	    	m_TX;

  //////////
  // The TX inlet
  t_inlet         *m_inletTX;

  //////////
  // The TY of the object
  GLfloat	    	m_TY;

  //////////
  // The TY inlet
  t_inlet         *m_inletTY;

  //////////
  // The rotX1 of the object
  float	    	cos_rotX1;
  float	    	sin_rotX1;

  //////////
  // The rotX1 inlet
  t_inlet         *m_inletrotX1;

  //////////
  // The rotY1 of the object
  float	    	cos_rotY1;
  float	    	sin_rotY1;

  //////////
  // The rotY1 inlet
  t_inlet         *m_inletrotY1;

  //////////
  // The rotX2 of the object
  float	    	cos_rotX2;
  float	    	sin_rotX2;

  //////////
  // The rotX2 inlet
  t_inlet         *m_inletrotX2;

  //////////
  // The rotY2 of the object
  float	    	cos_rotY2;
  float	    	sin_rotY2;

  //////////
  // The rotY2 inlet
  t_inlet         *m_inletrotY2;

  //////////
  // The order of the object
  int			order;

 private:
  //////////
  // Static member functions
  static void 	sizeMessCallback2(void *data, t_floatarg size2);
  static void 	highMessCallback(void *data, t_floatarg high);
  static void 	TXMessCallback(void *data, t_floatarg TX);
  static void 	TYMessCallback(void *data, t_floatarg TY);
  static void 	rotX1MessCallback(void *data, t_floatarg rotX1);
  static void 	rotY1MessCallback(void *data, t_floatarg rotY1);
  static void 	rotX2MessCallback(void *data, t_floatarg rotX2);
  static void 	rotY2MessCallback(void *data, t_floatarg rotY2);
  static void 	slicesMessCallback(void *data, t_floatarg slice);

};

#endif	// for header file
