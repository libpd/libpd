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

#include "accumrotate.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(accumrotate);

  /////////////////////////////////////////////////////////
//
// accumrotate
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
accumrotate :: accumrotate(int argc, t_atom *argv)
{
  m_rotMatrix.identity();

  if (argc == 3)
    {
      m_rotMatrix.rotateX(atom_getfloat(&argv[0]));
      m_rotMatrix.rotateY(atom_getfloat(&argv[1]));
      m_rotMatrix.rotateZ(atom_getfloat(&argv[2]));
    }
  else if (argc == 0)
    { }
  else
    {
      throw(GemException("needs 0 or 3 arguments"));
    }

  // create the new inlets
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xVal"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("yVal"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zVal"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
accumrotate :: ~accumrotate()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void accumrotate :: render(GemState *)
{
  glMultMatrixf((float *)(&m_rotMatrix.mat));
}

/////////////////////////////////////////////////////////
// xMess
//
/////////////////////////////////////////////////////////
void accumrotate :: xMess(float val)
{
  m_rotMatrix.rotateX(val);
  setModified();
}

/////////////////////////////////////////////////////////
// yMess
//
/////////////////////////////////////////////////////////
void accumrotate :: yMess(float val)
{
  m_rotMatrix.rotateY(val);
  setModified();
}

/////////////////////////////////////////////////////////
// zMess
//
/////////////////////////////////////////////////////////
void accumrotate :: zMess(float val)
{
  m_rotMatrix.rotateZ(val);
  setModified();
}

/////////////////////////////////////////////////////////
// reset
//
/////////////////////////////////////////////////////////
void accumrotate :: reset()
{
  m_rotMatrix.identity();
  setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void accumrotate :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&accumrotate::xMessCallback),
		  gensym("xVal"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&accumrotate::yMessCallback),
		  gensym("yVal"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&accumrotate::zMessCallback),
		  gensym("zVal"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&accumrotate::resetCallback),
		  gensym("reset"), A_NULL); 
}
void accumrotate :: xMessCallback(void *data, t_floatarg val)
{
  GetMyClass(data)->xMess((float)val);
}
void accumrotate :: yMessCallback(void *data, t_floatarg val)
{
  GetMyClass(data)->yMess((float)val);
}
void accumrotate :: zMessCallback(void *data, t_floatarg val)
{
  GetMyClass(data)->zMess((float)val);
}
void accumrotate :: resetCallback(void *data)
{
  GetMyClass(data)->reset();
}


