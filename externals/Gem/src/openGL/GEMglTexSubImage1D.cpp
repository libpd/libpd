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

#include "GEMglTexSubImage1D.h"
#include "Gem/Image.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglTexSubImage1D, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexSubImage1D :: GEMglTexSubImage1D(t_floatarg arg0=0,
					 t_floatarg arg1=0,
					 t_floatarg arg2=0) :
  level(static_cast<GLint>(arg0)), 
  xoffset(static_cast<GLint>(arg1)), 
  width(static_cast<GLsizei>(arg2)) 
{
  m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("level"));
  m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xoffset"));
  m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("width"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexSubImage1D :: ~GEMglTexSubImage1D () {
  inlet_free(m_inlet[0]);
  inlet_free(m_inlet[1]);
  inlet_free(m_inlet[2]);
}

//////////////////
// extension check
bool GEMglTexSubImage1D :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglTexSubImage1D :: render(GemState *state) {
  if (!state)return;
  pixBlock*img=NULL;  state->get(GemState::_PIX, img);  if(!img || !&img->image)return;
  target=GL_TEXTURE_1D;
  glTexSubImage1D (target, level, xoffset, width,
		   img->image.format, 
		   img->image.type, 
		   img->image.data);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexSubImage1D :: targetMess (t_float arg1) {	// FUN
  error("target has to be GL_TEXTURE_1D");
}

void GEMglTexSubImage1D :: levelMess (t_float arg1) {	// FUN
	level = static_cast<GLint>(arg1);
	setModified();
}

void GEMglTexSubImage1D :: xoffsetMess (t_float arg1) {	// FUN
	xoffset = static_cast<GLint>(arg1);
	setModified();
}

void GEMglTexSubImage1D :: widthMess (t_float arg1) {	// FUN
	width = static_cast<GLsizei>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexSubImage1D :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexSubImage1D::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexSubImage1D::levelMessCallback),  	gensym("level"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexSubImage1D::xoffsetMessCallback),  	gensym("xoffset"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexSubImage1D::widthMessCallback),  	gensym("width"), A_DEFFLOAT, A_NULL);
}

void GEMglTexSubImage1D :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglTexSubImage1D :: levelMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->levelMess ( static_cast<t_float>(arg0));
}
void GEMglTexSubImage1D :: xoffsetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xoffsetMess ( static_cast<t_float>(arg0));
}
void GEMglTexSubImage1D :: widthMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->widthMess ( static_cast<t_float>(arg0));
}
