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

#include "GEMglMap2f.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglMap2f );

/////////////////////////////////////////////////////////
//
// GEMglMap2f
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMap2f :: GEMglMap2f	(int argc, t_atom *argv){
	if (argc>0)target =static_cast<GLenum>(atom_getint(argv+0));
	if (argc>1)u1     =static_cast<GLfloat>(atom_getfloat(argv+1));
	if (argc>2)u2     =static_cast<GLfloat>(atom_getfloat(argv+2));
	if (argc>3)ustride=static_cast<GLint>(atom_getint(argv+3));
	if (argc>4)uorder =static_cast<GLint>(atom_getint(argv+4));
	if (argc>5)v1     =static_cast<GLfloat>(atom_getfloat(argv+5));
	if (argc>6)v2     =static_cast<GLfloat>(atom_getfloat(argv+6));
	if (argc>7)vstride=static_cast<GLint>(atom_getint(argv+7));
	if (argc>8)vorder =static_cast<GLint>(atom_getint(argv+8));

	len=128;
	points = new GLfloat[len];

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u1"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u2"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ustride"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("uorder"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v1"));
	m_inlet[6] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v2"));
	m_inlet[7] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("vstride"));
	m_inlet[8] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("vorder"));
	m_inlet[9] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("points"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMap2f :: ~GEMglMap2f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
inlet_free(m_inlet[6]);
inlet_free(m_inlet[7]);
inlet_free(m_inlet[8]);
inlet_free(m_inlet[9]);
}

//////////////////
// extension check
bool GEMglMap2f :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglMap2f :: render(GemState *state) {
	glMap2f (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMap2f :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglMap2f :: u1Mess (t_float arg1) {	// FUN
	u1 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMap2f :: u2Mess (t_float arg1) {	// FUN
	u2 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMap2f :: ustrideMess (t_float arg1) {	// FUN
	ustride = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMap2f :: uorderMess (t_float arg1) {	// FUN
	uorder = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMap2f :: v1Mess (t_float arg1) {	// FUN
	v1 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMap2f :: v2Mess (t_float arg1) {	// FUN
	v2 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMap2f :: vstrideMess (t_float arg1) {	// FUN
	vstride = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMap2f :: vorderMess (t_float arg1) {	// FUN
	vorder = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMap2f :: pointsMess (int argc, t_atom*argv) {	// FUN
  if (argc>len){
    len=argc;
    delete [] points;
    points = new GLfloat[len];
  }
  while(argc--)points[argc]=atom_getfloat(argv+argc);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglMap2f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::u1MessCallback),  	gensym("u1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::u2MessCallback),  	gensym("u2"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::ustrideMessCallback),  	gensym("ustride"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::uorderMessCallback),  	gensym("uorder"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::v1MessCallback),  	gensym("v1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::v2MessCallback),  	gensym("v2"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::vstrideMessCallback),  	gensym("vstride"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::vorderMessCallback),  	gensym("vorder"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap2f::pointsMessCallback),  	gensym("points"), A_GIMME, A_NULL);
}

void GEMglMap2f :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: u1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u1Mess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: u2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u2Mess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: ustrideMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->ustrideMess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: uorderMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->uorderMess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: v1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->v1Mess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: v2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->v2Mess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: vstrideMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->vstrideMess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: vorderMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->vorderMess ( static_cast<t_float>(arg0));
}
void GEMglMap2f :: pointsMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->pointsMess (argc, argv);
}
