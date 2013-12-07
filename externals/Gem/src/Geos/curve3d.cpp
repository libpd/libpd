////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Curve3D : create a besier surface
// made by Cyrille Henry
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "curve3d.h"

#include "Gem/State.h"
#include "Utils/Matrix.h"
#include "Utils/Functions.h"
#include <string.h>

CPPEXTERN_NEW_WITH_TWO_ARGS(curve3d, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// curve3d
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
curve3d :: curve3d(t_floatarg sizeX,t_floatarg sizeY )
  : GemShape(1),
    nb_pts_control_X(2), nb_pts_control_Y(2),
    nb_pts_affich_X (30), nb_pts_affich_Y (30),
    m_drawType(FILL),
    m_posXYZ(NULL)
{
  int i, j, a;

  nb_pts_control_X = MAX(2, static_cast<int>(sizeX));
  nb_pts_control_Y = MAX(2, static_cast<int>(sizeY));

  m_posXYZ = new t_float3[nb_pts_control_X*nb_pts_control_Y];

  if(!m_posXYZ){
    nb_pts_control_X=0;
    nb_pts_control_Y=0;
  }

  for (i=0; i < nb_pts_control_X; i++)
    for (j=0; j < nb_pts_control_Y; j++) {
      a= i + j * nb_pts_control_X;
      m_posXYZ[a].x= static_cast<float>(i)/nb_pts_control_X;
      m_posXYZ[a].y= static_cast<float>(j)/nb_pts_control_Y;
      m_posXYZ[a].z= 0.0;
    }

}

//////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
curve3d :: ~curve3d()
{ 
  if(m_posXYZ)delete[]m_posXYZ;
}

//////////////////////////////////////////////////////////
// setMess
//
/////////////////////////////////////////////////////////
void curve3d :: setMess(int X, int Y, float posX, float posY, float posZ){
  if ((X>=0)&(X<nb_pts_control_X)&(Y>=0)&(Y<nb_pts_control_Y))    {
    m_posXYZ[X+Y*nb_pts_control_X].x=posX;
    m_posXYZ[X+Y*nb_pts_control_X].y=posY;
    m_posXYZ[X+Y*nb_pts_control_X].z=posZ;
    setModified();
  }
}

//////////////////////////////////////////////////////////
// resolutionMess
//
/////////////////////////////////////////////////////////
void curve3d :: resolutionMess(int resolutionX, int resolutionY){
  int x=0;
  int y=0;

  nb_pts_control_X = (resolutionX < 2) ? 2 : resolutionX;
  nb_pts_control_Y = (resolutionY < 2) ? 2 : resolutionY;
  if(m_posXYZ)delete[]m_posXYZ;
  m_posXYZ = new t_float3[nb_pts_control_X*nb_pts_control_Y];
  if(!m_posXYZ){
    nb_pts_control_X=0;
    nb_pts_control_Y=0;
  }
  for (x=0; x < nb_pts_control_X; x++)
    for (y=0; y < nb_pts_control_Y; y++) {
      int a= x + y * nb_pts_control_X;
      m_posXYZ[a].x= static_cast<float>(x)/nb_pts_control_X;
      m_posXYZ[a].y= static_cast<float>(y)/nb_pts_control_Y;
      m_posXYZ[a].z= 0.0;
    }
  
  setModified();
}

//////////////////////////////////////////////////////////
// gridMess
//
/////////////////////////////////////////////////////////
void curve3d :: gridMess(int gridX, int gridY){
  nb_pts_affich_X = (gridX < 2) ? 2 : gridX;
  nb_pts_affich_Y = (gridY < 2) ? 2 : gridY;
  setModified();
}

//////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void curve3d :: typeMess(t_symbol *type){
  if (gensym("line")==type) 
    m_drawType = LINE;
  else if (gensym("fill")==type) 
    m_drawType = FILL;
  else if (gensym("point")==type)
    m_drawType = POINT;
  else if (gensym("line1")==type)
    m_drawType = LINE1;
  else if (gensym("line2")==type)
    m_drawType = LINE2;
  else if (gensym("line3")==type)
    m_drawType = LINE3;
  else if (gensym("line4")==type)
    m_drawType = LINE4; 
  else if (gensym("control_fill")==type)
    m_drawType = CONTROL_FILL; 
  else if (gensym("control_point")==type)
    m_drawType = CONTROL_POINT; 
  else if (gensym("control_line")==type)
    m_drawType = CONTROL_LINE; 
  else if (gensym("control_line1")==type)
    m_drawType = CONTROL_LINE1;
  else if (gensym("control_line2")==type)
    m_drawType = CONTROL_LINE2; 
  else if (gensym("default")==type)
    m_drawType = FILL;
  else    {
    error ("unknown draw style: 's'", type->s_name);
    return;
  }
  setModified();
}

//////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void curve3d :: renderShape(GemState *state){
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=FILL;
  float norm[3];
  int n, m;

  glNormal3f(0.0f, 0.0f, 1.0f);
  glLineWidth(m_linewidth);
  glMap2f(GL_MAP2_VERTEX_3, 0.0, 1.0, 3, 
	  static_cast<GLint>(nb_pts_control_X), 
	  0,1,
	  static_cast<GLint>(3*nb_pts_control_X),
	  static_cast<GLint>(nb_pts_control_Y), 
	  &(m_posXYZ[0].x));
  glEnable(GL_MAP2_VERTEX_3);

  GLfloat xsize = 1.0f;
  GLfloat ysize = 1.0f;
  GLfloat ysize0= 0.0f;

  if (GemShape::m_texType && GemShape::m_texNum>=3)
    {
      xsize  = GemShape::m_texCoords[1].s;
      ysize0 = GemShape::m_texCoords[2].t;
      ysize  = GemShape::m_texCoords[1].t;
    }
  GLfloat ysizediff = ysize0 - ysize;

  GLfloat affich_X=static_cast<GLfloat>(nb_pts_affich_X);
  GLfloat affich_Y=static_cast<GLfloat>(nb_pts_affich_Y);

  GLfloat control_X=static_cast<GLfloat>(nb_pts_control_X);
  GLfloat control_Y=static_cast<GLfloat>(nb_pts_control_Y);

  switch (m_drawType){
  case LINE:    { 
    if (GemShape::m_texType)	{
      for (n = 0; n < nb_pts_affich_X+1; n++)   {
	glBegin(GL_LINE_STRIP);
	for (m = 0; m  < nb_pts_affich_Y+1; m++){	
	  glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	  glEvalCoord2f(n/affich_X, m/affich_Y);
	}
	glEnd();
      }
      for(m = 0; m < nb_pts_affich_Y+1; m++) {
	glBegin(GL_LINE_STRIP);
	for(n = 0; n  < nb_pts_affich_X+1; n++){
	  glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	  glEvalCoord2f(n/affich_X, m/affich_Y);
	}
	glEnd();
      }
    }  else {
	  for(n = 0; n < nb_pts_affich_X+1; n++)  {
	    glBegin(GL_LINE_STRIP);
	    for(m = 0; m  < nb_pts_affich_Y+1; m++){	
	      glEvalCoord2f(n/affich_X, m/affich_Y);
	    }
	    glEnd();
	  }
	  for(m = 0; m < nb_pts_affich_Y+1; m++)  {
	    glBegin(GL_LINE_STRIP);
	    for(n = 0; n  < nb_pts_affich_X+1; n++){	
	      glEvalCoord2f(n/affich_X, m/affich_Y);
	    }
	    glEnd();
	  }
    }
  }
    break;
  case FILL:
    {
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_affich_X; n++) {
	  glBegin(GL_TRIANGLE_STRIP);
	  for(m = 0; m  < nb_pts_affich_Y+1; m++)   {	
	    glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	    glTexCoord2f(xsize*(n+1)/affich_X, ysize+ysizediff*m/affich_Y);
	    glEvalCoord2f((n+1)/affich_X, m/affich_Y);
	  }
	  glEnd();
	}
      else
	for(n = 0; n < nb_pts_affich_X; n++) {
	  glBegin(GL_TRIANGLE_STRIP);
	  for(m = 0; m  < nb_pts_affich_Y+1; m++)  {	
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	    glEvalCoord2f((n+1)/affich_X, m/affich_Y);
	  }
	  glEnd();
	}
    }
    break;
  case POINT:
    {
      glBegin(GL_POINTS);
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_affich_X+1; n++) {
	  for(m = 0; m  < nb_pts_affich_Y+1; m++) {
	    glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	  }
	}
      else
	for(n = 0; n < nb_pts_affich_X+1; n++) {
	  for(m = 0; m  < nb_pts_affich_Y+1; m++)
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	}
      glEnd();
    }
    break;

  case LINE1:
    {
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_affich_X; n++) {
	  glBegin(GL_LINE_STRIP);
	  for(m = 0; m  < nb_pts_affich_Y; m++)  {	
	    glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	  }
	  glEnd();
	}
      else
	for(n = 0; n < nb_pts_affich_X; n++) {
	  glBegin(GL_LINE_STRIP);
	  for(m = 0; m  < nb_pts_affich_Y; m++)   {	
	    glEvalCoord2f(n/affich_X, m/affich_Y);	
	  }
	  glEnd();
	}
    }
    break;

  case LINE2:
    {
      if (GemShape::m_texType)
	for(m = 0; m < nb_pts_affich_Y+1; m++) {
	  glBegin(GL_LINE_STRIP);
	  for(n = 0; n  < nb_pts_affich_X+1; n++)  {	
	    glTexCoord2f(xsize*n/affich_X,ysize+ysizediff*m/affich_Y);
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	  }
	  glEnd();
	}
      else
	for(m = 0; m < nb_pts_affich_Y+1; m++) {
	  glBegin(GL_LINE_STRIP);
	  for(n = 0; n  < nb_pts_affich_X+1; n++)  {	
	    glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	    glEvalCoord2f(n/affich_X, m/affich_Y);
	  }
	  glEnd();
	}
    }
    break;

  case LINE3:
    {
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_affich_X; n++) {
	  glBegin(GL_LINES);
	  for(m = 0; m  < nb_pts_affich_Y; m++)
	    {
	      glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	      glEvalCoord2f(n/affich_X, m/affich_Y);
	    }
	  glEnd();
	}
      else
	for(n = 0; n < nb_pts_affich_X; n++) {
	  glBegin(GL_LINES);
	  for(m = 0; m  < nb_pts_affich_Y; m++)  {	
	    glEvalCoord2f(n/affich_X, m/affich_Y);	
	  }
	  glEnd();
	}
    }
    break;

  case LINE4:
    {
      if (GemShape::m_texType)
	for(m = 0; m < nb_pts_affich_Y; m++)
	  {
	    glBegin(GL_LINES);
	    for(n = 0; n  < nb_pts_affich_X; n++) {	
	      glTexCoord2f(xsize*n/affich_X, ysize+ysizediff*m/affich_Y);
	      glEvalCoord2f(n/affich_X, m/affich_Y);
	    }
	    glEnd();
	  }
      else
	for(m = 0; m < nb_pts_affich_Y; m++) {
	  glBegin(GL_LINES);
	  for(n = 0; n  < nb_pts_affich_X; n++) {	
	    glEvalCoord2f(n/affich_X, m/affich_Y);	
	  }
	  glEnd();
	}
    }
    break;

  case CONTROL_FILL:
    {	
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_control_X-1; n++)
	  for(m = 0; m  < nb_pts_control_Y-1; m++)   {
	    Matrix::generateNormal((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X],
                                   (GLfloat*)&m_posXYZ[n+m*nb_pts_control_X+1], 
                                   (GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X], 
                                   norm);
	    glNormal3fv(norm);

	    glBegin(GL_TRIANGLE_STRIP);
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);	    
	    glTexCoord2f(xsize*(n+1.)/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X+1]);
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*(m+1.)/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X]);
	    glEnd();

	    Matrix::generateNormal((GLfloat*)&m_posXYZ[n+1+(m+1)*nb_pts_control_X],
                                   (GLfloat*)&m_posXYZ[n+1+m*nb_pts_control_X], 
                                   (GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X], norm);
	    glNormal3fv(norm);
	    glBegin(GL_TRIANGLE_STRIP);
	    glTexCoord2f(xsize*(n+1.)/(control_X-1.),
                         ysize+ysizediff*(m+1)/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+1+(m+1)*nb_pts_control_X]);
	    glTexCoord2f(xsize*(n+1.)/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+1+m*nb_pts_control_X]);
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*(m+1.)/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X]);
	    glEnd();
	  }
      else
	for(n = 0; n < nb_pts_control_X-1; n++)
	  for(m = 0; m  < nb_pts_control_Y-1; m++)    {
	    Matrix::generateNormal((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X],
                                   (GLfloat*)&m_posXYZ[n+m*nb_pts_control_X+1], 
                                   (GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X], 
                                   norm);
	    glNormal3fv(norm);

	    glBegin(GL_TRIANGLE_STRIP);
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X+1]);
	    glVertex3fv((GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X]);
	    glEnd();

	    Matrix::generateNormal((GLfloat*)&m_posXYZ[n+1+(m+1)*nb_pts_control_X],
                                   (GLfloat*)&m_posXYZ[n+(1+m)*nb_pts_control_X],
                                   (GLfloat*)&m_posXYZ[n+1+m*nb_pts_control_X], 
                                   norm);
	    glNormal3fv(norm);
	    glBegin(GL_TRIANGLE_STRIP);
	    glVertex3fv((GLfloat*)&m_posXYZ[n+1+(m+1)*nb_pts_control_X]);
	    glVertex3fv((GLfloat*)&m_posXYZ[n+(m+1)*nb_pts_control_X]);
	    glVertex3fv((GLfloat*)&m_posXYZ[n+1+m*nb_pts_control_X]);
	    glEnd();
	  }
    }
    break;

  case CONTROL_POINT:
    {
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_control_X; n++)
	  for(m = 0; m  < nb_pts_control_Y; m++)   {	
	    glBegin(GL_POINTS);
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	    glEnd();
	  }
      else
	for(n = 0; n < nb_pts_control_X; n++)
	  for(m = 0; m  < nb_pts_control_Y; m++) {	
	    glBegin(GL_POINTS);	
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	    glEnd();
	  }
    }
    break;

  case CONTROL_LINE:
    {
      if (GemShape::m_texType){
	for(n = 0; n < nb_pts_control_X; n++)   {
	  glBegin(GL_LINE_STRIP);
	  for(m = 0; m  < nb_pts_control_Y; m++){	
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	  }
	  glEnd();
	}

	for(m = 0; m < nb_pts_control_Y; m++)  {
	  glBegin(GL_LINE_STRIP);
	  for(n = 0; n  < nb_pts_control_X; n++){
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	  }
	  glEnd();
	}
      }  else	{
	for(n = 0; n < nb_pts_control_X; n++)  {
	  glBegin(GL_LINE_STRIP);
	  for(m = 0; m  < nb_pts_control_Y; m++)
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	  glEnd();
	}
	for(m = 0; m < nb_pts_control_Y; m++)  {
	  glBegin(GL_LINE_STRIP);
	  for(n = 0; n  < nb_pts_control_X; n++)	
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);				
	  glEnd();
	}
      }
    }
    break;

  case CONTROL_LINE2:
    {
      if (GemShape::m_texType)
	for(m = 0; m < nb_pts_control_Y; m++) {
	  glBegin(GL_LINE_STRIP);
	  for(n = 0; n  < nb_pts_control_X; n++) {
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);			
	  }
	  glEnd();
	}
      else
	for(m = 0; m < nb_pts_control_Y; m++) {
	  glBegin(GL_LINE_STRIP);
	  for(n = 0; n  < nb_pts_control_X; n++)
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);	
	  glEnd();
	}
    }
    break;

  case CONTROL_LINE1:
    {
      if (GemShape::m_texType)
	for(n = 0; n < nb_pts_control_X; n++) {
	  glBegin(GL_LINE_STRIP);
	  for(m = 0; m  < nb_pts_control_Y; m++)  {	
	    glTexCoord2f(xsize*n/(control_X-1.),
                         ysize+ysizediff*m/(control_Y-1.));
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);			
	  }
	  glEnd();
	}
      else
	for(n = 0; n < nb_pts_control_X; n++) {
	  glBegin(GL_LINE_STRIP);
	  for(m = 0; m  < nb_pts_control_Y; m++)
	    glVertex3fv((GLfloat*)&m_posXYZ[n+m*nb_pts_control_X]);
	  glEnd();
	}
    }
    return;
  }
}

//////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void curve3d :: obj_setupCallback(t_class *classPtr){
  class_addmethod(classPtr, reinterpret_cast<t_method>(&curve3d::resolutionMessCallback),
		  gensym("res"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&curve3d::gridMessCallback),
		  gensym("grid"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&curve3d::setMessCallback),
		  gensym("set"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
}

void curve3d :: resolutionMessCallback(void *data, t_floatarg resX, t_floatarg resY)
{
  GetMyClass(data)->resolutionMess(static_cast<int>(resX), static_cast<int>(resY));
}
void curve3d :: gridMessCallback(void *data, t_floatarg gridX, t_floatarg gridY)
{
  GetMyClass(data)->gridMess(static_cast<int>(gridX), static_cast<int>(gridY));
}
void curve3d :: setMessCallback(void *data, float X, float Y, float posX, float posY, float posZ)
{
  GetMyClass(data)->setMess(static_cast<int>(X), static_cast<int>(Y), 
			    posX, posY, posZ);
}

