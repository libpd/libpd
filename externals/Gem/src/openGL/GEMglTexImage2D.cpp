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

#include "GEMglTexImage2D.h"
#include "Gem/Image.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglTexImage2D );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexImage2D :: GEMglTexImage2D	(int argc, t_atom*argv) :
  target(0), level(0), xoffset(0), yoffset(0), width(0), height(0)
{
	if (argc>0)level  =atom_getint(argv+0);
	if (argc>1)xoffset=atom_getint(argv+1);
	if (argc>2)yoffset=atom_getint(argv+2);
	if (argc>3)width  =atom_getint(argv+3);
	if (argc>4)height =atom_getint(argv+4);
	
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("level"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xoffset"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("yoffset"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("width"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("height"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexImage2D :: ~GEMglTexImage2D () {
  inlet_free(m_inlet[0]);
  inlet_free(m_inlet[1]);
  inlet_free(m_inlet[2]);
  inlet_free(m_inlet[3]);
  inlet_free(m_inlet[4]);
}

//////////////////
// extension check
bool GEMglTexImage2D :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglTexImage2D :: render(GemState *state) {
  if (!state)return;
  pixBlock*img=NULL;  state->get(GemState::_PIX, img);  if(!img || !&img->image)return;
  target=GL_TEXTURE_2D;
  glTexImage2D (target, level, xoffset, yoffset, width, height, 
		   img->image.format, 
		   img->image.type, 
		   img->image.data);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexImage2D :: targetMess (t_float arg1) {	// FUN
  error("target has to be GL_TEXTURE_2D");
}

void GEMglTexImage2D :: levelMess (t_float arg1) {	// FUN
	level = static_cast<GLint>(arg1);
	setModified();
}

void GEMglTexImage2D :: xoffsetMess (t_float arg1) {	// FUN
	xoffset = static_cast<GLint>(arg1);
	setModified();
}

void GEMglTexImage2D :: yoffsetMess (t_float arg1) {	// FUN
	yoffset = static_cast<GLint>(arg1);
	setModified();
}

void GEMglTexImage2D :: widthMess (t_float arg1) {	// FUN
	width = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglTexImage2D :: heightMess (t_float arg1) {	// FUN
	height = static_cast<GLsizei>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexImage2D :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexImage2D::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexImage2D::levelMessCallback),  	gensym("level"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexImage2D::xoffsetMessCallback),  	gensym("xoffset"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexImage2D::yoffsetMessCallback),  	gensym("yoffset"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexImage2D::widthMessCallback),  	gensym("width"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexImage2D::heightMessCallback),  	gensym("height"), A_DEFFLOAT, A_NULL);
}

void GEMglTexImage2D :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglTexImage2D :: levelMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->levelMess ( static_cast<t_float>(arg0));
}
void GEMglTexImage2D :: xoffsetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xoffsetMess ( static_cast<t_float>(arg0));
}
void GEMglTexImage2D :: yoffsetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yoffsetMess ( static_cast<t_float>(arg0));
}
void GEMglTexImage2D :: widthMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->widthMess ( static_cast<t_float>(arg0));
}
void GEMglTexImage2D :: heightMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->heightMess ( static_cast<t_float>(arg0));
}
