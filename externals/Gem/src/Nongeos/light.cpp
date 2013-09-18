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

#include "light.h"

CPPEXTERN_NEW_WITH_ONE_ARG(light, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// light
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
light :: light(t_floatarg lightNum)
  : world_light(lightNum)
{
  m_position[0] = m_position[1] = m_position[2] = 0.0;
  m_position[3] = 1.0;
}

////////////////////////////////////////////////////////
// Destructor
//
////////////////////////////////////////////////////////
light :: ~light()
{ }

////////////////////////////////////////////////////////
// render
//
////////////////////////////////////////////////////////
void light :: renderDebug()
{
  const GLfloat size=0.2f;
  if (m_debug)
    {
      glPushMatrix();
      glDisable(GL_LIGHTING);
      glColor3fv(m_color);
      glTranslatef(m_position[0], m_position[1], m_position[2]);
      gluSphere(m_thing, size, 10, 10);
      glEnable(GL_LIGHTING);
      glPopMatrix();
    }
}

////////////////////////////////////////////////////////
// static member functions
//
////////////////////////////////////////////////////////
void light :: obj_setupCallback(t_class *classPtr)
{ }
