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

#define GEM_CLASSFLAGS CLASS_NOINLET

#include "gemmouse.h"

#include "Gem/Event.h"
#include "Gem/Manager.h"

CPPEXTERN_NEW_WITH_GIMME(gemmouse);

/////////////////////////////////////////////////////////
//
// gemmouse
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemmouse :: gemmouse(int argc, t_atom*argv) :
  m_scaleX(1.0), m_scaleY(1.0), m_scaleMode(NONE)
{
    m_outXPos = outlet_new(this->x_obj, 0);
    m_outYPos = outlet_new(this->x_obj, 0);
    m_outLBut = outlet_new(this->x_obj, 0);
    m_outMBut = outlet_new(this->x_obj, 0);
    m_outRBut = outlet_new(this->x_obj, 0);

    switch(argc){
    case 0:
      m_scaleX = m_scaleY = 0.f;
      break;
    case 1:
      m_scaleX = m_scaleY = atom_getfloat(argv);
      m_scaleMode =HEIGHT;
      break;
    default:
      m_scaleX=atom_getfloat(argv);
      m_scaleY=atom_getfloat(argv+1);
      m_scaleMode =BOTH;
    }

    // register event callback
    setMotionCallback(&gemmouse::mouseMotionCallback, this);
    setButtonCallback(&gemmouse::mouseButtonCallback, this);
    setWheelCallback(&gemmouse::mouseWheelCallback, this);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemmouse :: ~gemmouse()
{
    // remove event callback
    removeMotionCallback(&gemmouse::mouseMotionCallback, this);
    removeButtonCallback(&gemmouse::mouseButtonCallback, this);
    removeWheelCallback(&gemmouse::mouseWheelCallback, this);

    outlet_free(m_outXPos);
    outlet_free(m_outYPos);
    outlet_free(m_outLBut);
    outlet_free(m_outMBut);
    outlet_free(m_outRBut);
}


/////////////////////////////////////////////////////////
// mouseMotion
//
/////////////////////////////////////////////////////////
void gemmouse :: mouseMotion(int x, int y)
{
  t_float scalex = 1., scaley=1.;

  int width, height;
  GemMan::getRealDimen(&width, &height);

  switch(m_scaleMode) {
  case NONE:
    break;
  case BOTH:
    scalex=m_scaleX/width;
    scaley=m_scaleY/height;
    break;
  case WIDTH:
    scalex=m_scaleX/width;
    scaley=scalex;
    break;
  case HEIGHT:
    scaley=m_scaleY/height;
    scalex=scaley;
    break;
  }

  outlet_float(m_outYPos, static_cast<t_float>(y*scaley));
  outlet_float(m_outXPos, static_cast<t_float>(x*scalex));
}

/////////////////////////////////////////////////////////
// mouseButton
//
/////////////////////////////////////////////////////////
void gemmouse :: mouseButton(int which, int state, int x, int y)
{
  switch (which){
  case 0: outlet_float(m_outLBut, static_cast<t_float>(state)); break;
  case 1: outlet_float(m_outMBut, static_cast<t_float>(state)); break;
  case 2: outlet_float(m_outRBut, static_cast<t_float>(state)); break;
  }
}

/////////////////////////////////////////////////////////
// mouseButton
//
/////////////////////////////////////////////////////////
void gemmouse :: mouseWheel(int axis, int value)
{
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemmouse :: obj_setupCallback(t_class *)
{ }
void gemmouse :: mouseMotionCallback(int x, int y, void *data)
{
    (reinterpret_cast<gemmouse*>(data))->mouseMotion(x, y);
}
void gemmouse :: mouseButtonCallback(int which, int state, int x, int y, void *data)
{
    (reinterpret_cast<gemmouse*>(data))->mouseButton(which, state, x, y);
}
void gemmouse :: mouseWheelCallback(int axis, int value, void *data)
{
    (reinterpret_cast<gemmouse*>(data))->mouseWheel(axis, value);
}
