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

#include "GEMglCopyPixels.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglCopyPixels );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglCopyPixels :: GEMglCopyPixels	(int argc, t_atom* argv)
{
  x=y=width=height=0;
  type=0;
  if (argc>0)x=atom_getint(argv);
  if (argc>1)y=atom_getint(argv+1);
  if (argc>2)width=atom_getint(argv+2);
  if (argc>3)height=atom_getint(argv+3);
  if (argc>4)type=atom_getint(argv+4);

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("width"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("height"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("type"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglCopyPixels :: ~GEMglCopyPixels () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglCopyPixels :: render(GemState *state) {
	glCopyPixels (x, y, width, height, type);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglCopyPixels :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyPixels :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLint>(arg1);
	setModified();
}

void GEMglCopyPixels :: widthMess (t_float arg1) {	// FUN
	width = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglCopyPixels :: heightMess (t_float arg1) {	// FUN
	height = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglCopyPixels :: typeMess (t_float arg1) {	// FUN
	type = static_cast<GLenum>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglCopyPixels :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyPixels::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyPixels::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyPixels::widthMessCallback),  	gensym("width"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyPixels::heightMessCallback),  	gensym("height"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCopyPixels::typeMessCallback),  	gensym("type"), A_DEFFLOAT, A_NULL);
};

void GEMglCopyPixels :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglCopyPixels :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
void GEMglCopyPixels :: widthMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->widthMess ( static_cast<t_float>(arg0));
}
void GEMglCopyPixels :: heightMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->heightMess ( static_cast<t_float>(arg0));
}
void GEMglCopyPixels :: typeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->typeMess ( static_cast<t_float>(arg0));
}
