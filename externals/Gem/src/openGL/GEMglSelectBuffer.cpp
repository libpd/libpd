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

#include "GEMglSelectBuffer.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglSelectBuffer , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglSelectBuffer :: GEMglSelectBuffer	(t_floatarg arg0=16){
	len=-1;
	buffer=0;
	sizeMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("size"));
	m_bufout= outlet_new(this->x_obj, &s_list);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglSelectBuffer :: ~GEMglSelectBuffer () {
  inlet_free(m_inlet);
  outlet_free(m_bufout);
}

//////////////////
// extension check
bool GEMglSelectBuffer :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglSelectBuffer :: render(GemState *state) {
  glSelectBuffer (size, buffer);
}

void GEMglSelectBuffer :: postrender(GemState *state) {
  t_atom*ap=new t_atom[size];
  int i=0;
  for(i=0; i<size; i++) {
    SETFLOAT(ap+i, static_cast<t_float>(buffer[i]));
  }
  outlet_list(m_bufout, gensym("list"), size, ap);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglSelectBuffer :: sizeMess (t_float arg1) {	// FUN
  int i;
  if (arg1<1)return;
  size = static_cast<GLsizei>(arg1);
  if (len<size){
    len=size;
    delete[]buffer;
    buffer = new GLuint[len];
  }
  for(i=0; i<len; i++)
    buffer[i]=0;

  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglSelectBuffer :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglSelectBuffer::sizeMessCallback),  	gensym("size"), A_DEFFLOAT, A_NULL);
}

void GEMglSelectBuffer :: sizeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sizeMess (arg0);
}
