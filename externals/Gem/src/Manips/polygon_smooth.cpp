////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "polygon_smooth.h"

CPPEXTERN_NEW(polygon_smooth);

/////////////////////////////////////////////////////////
//
// polygon_smooth
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
polygon_smooth :: polygon_smooth()
       : m_polygon_smoothState(1)
{}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
polygon_smooth :: ~polygon_smooth()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void polygon_smooth :: render(GemState *)
{
  if (m_polygon_smoothState)    {
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_DONT_CARE); 
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
  }
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void polygon_smooth :: postrender(GemState *)
{
  if (m_polygon_smoothState)
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
}

/////////////////////////////////////////////////////////
// polygon_smoothMess
//
/////////////////////////////////////////////////////////
void polygon_smooth :: polygon_smoothMess(int polygon_smoothState)
{
    m_polygon_smoothState = polygon_smoothState;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void polygon_smooth :: obj_setupCallback(t_class *classPtr)
{
    class_addfloat(classPtr, reinterpret_cast<t_method>(&polygon_smooth::polygon_smoothMessCallback));
}
void polygon_smooth :: polygon_smoothMessCallback(void *data, t_floatarg polygon_smoothState)
{
    GetMyClass(data)->polygon_smoothMess(static_cast<int>(polygon_smoothState));
}
