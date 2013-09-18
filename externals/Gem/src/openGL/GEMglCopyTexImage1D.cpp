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

#include "GEMglCopyTexImage1D.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglCopyTexImage1D );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglCopyTexImage1D :: GEMglCopyTexImage1D	(int argc, t_atom *argv){
	target=0;
	level=0;
	internalFormat=0;
	x=y=width=0;
	border=0;
	if (argc>0)target        =atom_getint(argv+0);
	if (argc>1)level         =atom_getint(argv+1);
	if (argc>2)internalFormat=atom_getint(argv+2);
	if (argc>3)x             =atom_getint(argv+3);
	if (argc>4)y             =atom_getint(argv+4);
	if (argc>5)width         =atom_getint(argv+5);
	if (argc>6)border        =atom_getint(argv+6);

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("level"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("internalFormat"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("width"));
	m_inlet[6] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("border"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglCopyTexImage1D :: ~GEMglCopyTexImage1D () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
inlet_free(m_inlet[6]);
}
//////////////////
// extension check
bool GEMglCopyTexImage1D :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglCopyTexImage1D :: render(GemState *state) {
	glCopyTexImage1D (target, level, internalFormat, x, y, width, border);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglCopyTexImage1D :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglCopyTexImage1D :: levelMess (t_float arg1) {	// FUN
	level = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyTexImage1D :: internalFormatMess (t_float arg1) {	// FUN
	internalFormat = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglCopyTexImage1D :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyTexImage1D :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyTexImage1D :: widthMess (t_float arg1) {	// FUN
	width = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglCopyTexImage1D :: borderMess (t_float arg1) {	// FUN
	border = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglCopyTexImage1D :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::levelMessCallback),  	gensym("level"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::internalFormatMessCallback),  	gensym("internalFormat"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::widthMessCallback),  	gensym("width"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage1D::borderMessCallback),  	gensym("border"), A_DEFFLOAT, A_NULL);
}

void GEMglCopyTexImage1D :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage1D :: levelMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->levelMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage1D :: internalFormatMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->internalFormatMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage1D :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage1D :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage1D :: widthMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->widthMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage1D :: borderMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->borderMess ( static_cast<t_float>(arg0));
}
