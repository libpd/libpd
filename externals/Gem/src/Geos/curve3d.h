/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A curve3d

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_CURVE_D_H_
#define _INCLUDE__GEM_GEOS_CURVE_D_H_

#include "Base/GemShape.h"


/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  curve3d
    
  Creates a curve3d

  KEYWORDS
  geo
    
  DESCRIPTION
    
  -----------------------------------------------------------------*/

class GEM_EXTERN curve3d : public GemShape
{
  CPPEXTERN_HEADER(curve3d, GemShape);

    public:

  //////////
  // Constructor
  curve3d(t_floatarg size_X, t_floatarg size_Y);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~curve3d();

  //////////
  // Do the renderShapeing
  virtual void 	renderShape(GemState *state);


  typedef struct {
    GLfloat x,y,z;
  } t_float3;

  int nb_pts_control_X;
  int nb_pts_control_Y;
  int nb_pts_affich_X;
  int nb_pts_affich_Y;
  void resolutionMess(int resX, int resY);
  void gridMess(int gridX, int gridY);
  void setMess(int X,int Y,float posX, float posY,float posZ);

  enum C3dDrawType{LINE, FILL, POINT, LINE1, LINE2, LINE3, LINE4, 
	CONTROL_FILL, CONTROL_POINT, CONTROL_LINE, CONTROL_LINE1, CONTROL_LINE2} 
  m_drawType;

  virtual void	typeMess(t_symbol *type);
 
  t_float3		*m_posXYZ; // attention, valeur critique

 private:
  static void		resolutionMessCallback(void *data, t_floatarg resX, t_floatarg resY );
  static void		gridMessCallback(void *data, t_floatarg gridX, t_floatarg gridY );
  static void		setMessCallback(void *data, t_floatarg X,t_floatarg Y,t_floatarg posX,t_floatarg posY,t_floatarg posZ);
};

#endif	// for header file
