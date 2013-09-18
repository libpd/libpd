////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//	zmoelnig@iem.kug.ac.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
//  this file has been generated...
////////////////////////////////////////////////////////

#include "GEMglIsEnabled.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglIsEnabled );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIsEnabled :: GEMglIsEnabled (int argc, t_atom*argv) : 
  cap(0) 
{
  if(1==argc)capMess(argv[0]); else if(argc) throw(GemException("invalid number of arguments"));
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("cap"));
  m_outlet=outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIsEnabled :: ~GEMglIsEnabled () {
  inlet_free(m_inlet);
  outlet_free(m_outlet);
}

//////////////////
// extension check
bool GEMglIsEnabled :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIsEnabled :: render(GemState *state) {
  GLboolean b = glIsEnabled (cap);
  outlet_float(m_outlet, b?1.0:0.0);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglIsEnabled :: capMess (t_atom arg) {	// FUN
  cap = static_cast<GLenum>(getGLdefine(&arg));
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//
void GEMglIsEnabled :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIsEnabled::capMessCallback), gensym("cap"), A_GIMME, A_NULL);
}

void GEMglIsEnabled :: capMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
  if(argc==1)GetMyClass(data)->capMess ( argv[0]);
}
