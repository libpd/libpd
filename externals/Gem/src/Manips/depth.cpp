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

#include "depth.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(depth);

/////////////////////////////////////////////////////////
//
// depth
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
depth :: depth(int argc, t_atom*argv)
       : m_state(1)
{ 
  switch(argc) {
  case 0: 
    break;
  case 1:
    if(A_FLOAT==argv->a_type){
      m_state=(atom_getint(argv)>0);
      break;
    }
  default:
    throw(GemException("invalid argument"));
    break;
  }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
depth :: ~depth()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void depth :: render(GemState *)
{
    if (m_state) glDisable(GL_DEPTH_TEST);
    else         glEnable (GL_DEPTH_TEST);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void depth :: postrender(GemState *)
{
    if (m_state) glEnable(GL_DEPTH_TEST);
}

/////////////////////////////////////////////////////////
// depthMess
//
/////////////////////////////////////////////////////////
void depth :: depthMess(int state)
{
    m_state = state;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void depth :: obj_setupCallback(t_class *classPtr)
{
    class_addfloat(classPtr, reinterpret_cast<t_method>(&depth::depthMessCallback));
}
void depth :: depthMessCallback(void *data, t_floatarg state)
{
    GetMyClass(data)->depthMess(static_cast<int>(state));
}
