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

#include "GEMglDrawElements.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglDrawElements , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglDrawElements :: GEMglDrawElements	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
  mode(static_cast<GLenum>(arg0)), // ub, us, ui
  count(static_cast<GLsizei>(arg1)), 
  type(static_cast<GLenum>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("count"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("type"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list , gensym("indices"));

	len=(count>0)?count:16;

	indices_ui=new GLuint  [len];
	indices_us=new GLushort[len];
	indices_ub=new GLubyte [len];

}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglDrawElements :: ~GEMglDrawElements () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}
//////////////////
// extension check
bool GEMglDrawElements :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglDrawElements :: render(GemState *state) {
  GLvoid *indix;
  switch(type){
  case GL_UNSIGNED_INT:
    indix=indices_ui;    break;
  case GL_UNSIGNED_BYTE:
    indix=indices_ub;    break;
  case GL_UNSIGNED_SHORT:
    indix=indices_us;    break;
  default:
    return;
  }
  glDrawElements (mode, count, type, indix);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglDrawElements :: modeMess (t_float arg1) {	// FUN
	mode = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglDrawElements :: countMess (t_float arg1) {	// FUN
	count = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglDrawElements :: typeMess (t_float arg1) {	// FUN
	type = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglDrawElements :: indicesMess (int argc, t_atom*argv) {	// FUN
  if (argc>len){
    len=argc;
    delete [] indices_ui; indices_ui = new GLuint  [len];
    delete [] indices_ub; indices_ub = new GLubyte [len];
    delete [] indices_us; indices_us = new GLushort[len];
  }
  while(argc--){
    t_float f=atom_getfloat(argv++);
    indices_ui[argc]=static_cast<GLuint>(f);
    indices_ub[argc]=static_cast<GLubyte>(f);
    indices_us[argc]=static_cast<GLushort>(f);
  }
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglDrawElements :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawElements::modeMessCallback),  	gensym("mode"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawElements::countMessCallback),  	gensym("count"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawElements::typeMessCallback),  	gensym("type"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawElements::indicesMessCallback),  	gensym("indices"), A_GIMME, A_NULL);
}

void GEMglDrawElements :: modeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->modeMess (arg0);
}
void GEMglDrawElements :: countMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->countMess (arg0);
}
void GEMglDrawElements :: typeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->typeMess (arg0);
}
void GEMglDrawElements :: indicesMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->indicesMess ( argc, argv );
}
