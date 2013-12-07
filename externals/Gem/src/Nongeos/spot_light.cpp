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
//    Copyright (c) 2005 Pierre-Olivier Charlebois McGill Electrical Engineering
//
//        For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "spot_light.h"

CPPEXTERN_NEW_WITH_ONE_ARG(spot_light, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// spot_light
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

spot_light :: spot_light(t_floatarg lightNum)
  : world_light(lightNum)
{
  // position
  m_position[0] = 0.0;
  m_position[1] = 0.0;
  m_position[2] = 0.0;
  m_position[3] = 1.0;

  spotDirection[0] = 0.0;
  spotDirection[1] = 0.0;
  spotDirection[2] = -1.0;

  constantAttenuation = 0.0;
  linearAttenuation = 0.1;
  quadraticAttenuation = 0.0;
  spotCutoff = 30.0;
  spotExponent = 2.5;

  // create the parameters inlet
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("paramlist"));
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
spot_light :: ~spot_light()
{

}

////////////////////////////////////////////////////////
// lightParamMess
//
////////////////////////////////////////////////////////
void spot_light :: lightParamMess(float linAtt, float cutoff, float exponent)
{
  // convert from spherical coordinates
  // needs to be positive?
  if (linAtt >= 0) linearAttenuation = linAtt;
  
  // spotCutoff needs to be 0-90 or 180
  if ( (cutoff >= 0) && (cutoff <= 90) ) spotCutoff = cutoff;
  else if (cutoff == 180) spotCutoff = cutoff;
  
  // only 0-128
  if ( (exponent >= 0) && (exponent <= 128) ) spotExponent = exponent;
  m_change = 1;
  setModified();
}

////////////////////////////////////////////////////////
// render
//
////////////////////////////////////////////////////////

void spot_light :: renderDebug()
{
  if (m_debug) { // display source
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3fv(m_color);
    gluCylinder(m_thing, 0.2f, 0, 0.4f, 10, 10);
    glEnable(GL_LIGHTING);
    glPopMatrix();
  }
}
void spot_light :: render(GemState *state)
{
  if (!m_light)return;

  if (m_change) {
    m_change = 0;
    if ( !m_on ){
      glDisable(m_light);
      return;
    }

    glEnable(m_light);          

    // setGlobal parameter
    glLightfv(m_light, GL_DIFFUSE,  m_color);
    glLightfv(m_light, GL_SPECULAR, m_color);
    glLightf(m_light, GL_LINEAR_ATTENUATION, linearAttenuation);
  }

  if (m_on) {
    // Set spot parameters
    glLightfv(m_light, GL_POSITION, m_position);
    glLightf(m_light, GL_SPOT_EXPONENT, spotExponent);
    glLightfv(m_light, GL_SPOT_DIRECTION, spotDirection);
    glLightf(m_light, GL_SPOT_CUTOFF, spotCutoff);

    renderDebug();
  }
}

////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void spot_light :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&spot_light::lightParamMessCallback),
                  gensym("paramlist"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
}
void spot_light :: lightParamMessCallback(void *data, 
					  t_floatarg linAtt, 
					  t_floatarg cutoff, 
					  t_floatarg exponent)
{
  GetMyClass(data)->lightParamMess(linAtt, cutoff, exponent);
}
