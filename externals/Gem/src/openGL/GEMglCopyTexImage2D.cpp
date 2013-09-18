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

#include "GEMglCopyTexImage2D.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglCopyTexImage2D );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglCopyTexImage2D :: GEMglCopyTexImage2D	(int argc,t_atom*argv){
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
	if (argc>6)height        =atom_getint(argv+6);
	if (argc>7)border        =atom_getint(argv+7);


	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("level"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("internalFormat"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("width"));
	m_inlet[6] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("height"));
	m_inlet[7] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("border"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglCopyTexImage2D :: ~GEMglCopyTexImage2D () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
inlet_free(m_inlet[6]);
inlet_free(m_inlet[7]);
}
//////////////////
// extension check
bool GEMglCopyTexImage2D :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglCopyTexImage2D :: render(GemState *state) {
	glCopyTexImage2D (target, level, internalFormat, x, y, width, height, border);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglCopyTexImage2D :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: levelMess (t_float arg1) {	// FUN
	level = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: internalFormatMess (t_float arg1) {	// FUN
	internalFormat = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: widthMess (t_float arg1) {	// FUN
	width = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: heightMess (t_float arg1) {	// FUN
	height = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglCopyTexImage2D :: borderMess (t_float arg1) {	// FUN
	border = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglCopyTexImage2D :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::levelMessCallback),  	gensym("level"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::internalFormatMessCallback),  	gensym("internalFormat"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::widthMessCallback),  	gensym("width"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::heightMessCallback),  	gensym("height"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyTexImage2D::borderMessCallback),  	gensym("border"), A_DEFFLOAT, A_NULL);
}

void GEMglCopyTexImage2D :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: levelMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->levelMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: internalFormatMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->internalFormatMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: widthMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->widthMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: heightMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->heightMess ( static_cast<t_float>(arg0));
}
void GEMglCopyTexImage2D :: borderMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->borderMess ( static_cast<t_float>(arg0));
}
