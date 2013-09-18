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

#include "GEMglMap1f.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglMap1f );

/////////////////////////////////////////////////////////
//
// GEMglMap1f
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMap1f :: GEMglMap1f	(int argc, t_atom*argv){
	if (argc>0)target=static_cast<GLenum>(atom_getint(argv+0));
	if (argc>1)u1    =static_cast<GLfloat>(atom_getfloat(argv+1));
	if (argc>2)u2    =static_cast<GLfloat>(atom_getfloat(argv+2));
	if (argc>3)stride=static_cast<GLint>(atom_getint(argv+3));
	if (argc>4)order =static_cast<GLint>(atom_getint(argv+4));
	
	len=128;
	points = new GLfloat[len];

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u1"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u2"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("stride"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("order"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("points"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMap1f :: ~GEMglMap1f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
}

//////////////////
// extension check
bool GEMglMap1f :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglMap1f :: render(GemState *state) {
	glMap1f (target, u1, u2, stride, order, points);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMap1f :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglMap1f :: u1Mess (t_float arg1) {	// FUN
	u1 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMap1f :: u2Mess (t_float arg1) {	// FUN
	u2 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMap1f :: strideMess (t_float arg1) {	// FUN
	stride = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMap1f :: orderMess (t_float arg1) {	// FUN
	order = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMap1f :: pointsMess (int argc, t_atom*argv) {	// FUN
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

void GEMglMap1f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap1f::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap1f::u1MessCallback),  	gensym("u1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap1f::u2MessCallback),  	gensym("u2"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap1f::strideMessCallback),  	gensym("stride"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap1f::orderMessCallback),  	gensym("order"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMap1f::pointsMessCallback),  	gensym("points"), A_GIMME, A_NULL);
}

void GEMglMap1f :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( arg0);
}
void GEMglMap1f :: u1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u1Mess ( arg0);
}
void GEMglMap1f :: u2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u2Mess ( arg0);
}
void GEMglMap1f :: strideMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->strideMess ( arg0);
}
void GEMglMap1f :: orderMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->orderMess (arg0);
}
void GEMglMap1f :: pointsMessCallback (void* data, t_symbol*s, int argc, t_atom*argv){
	GetMyClass(data)->pointsMess (argc, argv);
}
