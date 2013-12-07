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

#include "GEMglGetIntegerv.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglGetIntegerv );

/////////////////////////////////////////////////////////
//
// GEMglGetIntegerv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGetIntegerv :: GEMglGetIntegerv	(int argc, t_atom*argv) {
  if(1==argc)pnameMess(argv[0]); else if(argc) throw(GemException("invalid number of arguments"));

  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
  m_outlet = outlet_new(this->x_obj, 0);

}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGetIntegerv :: ~GEMglGetIntegerv () {
  inlet_free(m_inlet);
  outlet_free(m_outlet);
}

//////////////////
// extension check
bool GEMglGetIntegerv :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglGetIntegerv :: render(GemState *state) {
  GLint mi[16]={0};

  glGetIntegerv(pname,mi);

  SETFLOAT(m_alist+0, mi[0]);
  SETFLOAT(m_alist+1, mi[1]);
  SETFLOAT(m_alist+2, mi[2]);
  SETFLOAT(m_alist+3, mi[3]);
  SETFLOAT(m_alist+4, mi[4]);
  SETFLOAT(m_alist+5, mi[5]);
  SETFLOAT(m_alist+6, mi[6]);
  SETFLOAT(m_alist+7, mi[7]);
  SETFLOAT(m_alist+8, mi[8]);
  SETFLOAT(m_alist+9, mi[9]);
  SETFLOAT(m_alist+10, mi[10]);
  SETFLOAT(m_alist+11, mi[11]);
  SETFLOAT(m_alist+12, mi[12]);
  SETFLOAT(m_alist+13, mi[13]);
  SETFLOAT(m_alist+14, mi[14]);
  SETFLOAT(m_alist+15, mi[15]);

  outlet_list(m_outlet, gensym("list"), 16, m_alist);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglGetIntegerv :: pnameMess (t_atom arg) {	// FUN
  pname=static_cast<GLenum>(getGLdefine(&arg));
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglGetIntegerv :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGetIntegerv::pnameMessCallback),  	gensym("pname"), A_GIMME, A_NULL);
}

void GEMglGetIntegerv :: pnameMessCallback (void* data, t_symbol*, int argc, t_atom*argv) {
  if(argc==1)GetMyClass(data)->pnameMess ( argv[0]);
}
