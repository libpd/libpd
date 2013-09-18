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

#include "GEMglMapGrid2d.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglMapGrid2d );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMapGrid2d :: GEMglMapGrid2d	(int argc, t_atom*argv)  {
	if (argc>0)un=atom_getint (argv+0);
	if (argc>1)u1=atom_getfloat(argv+1);
	if (argc>2)u2=atom_getfloat(argv+2);
	if (argc>3)vn=atom_getint (argv+3);
	if (argc>4)v1=atom_getfloat(argv+4);
	if (argc>5)v2=atom_getfloat(argv+5);

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("un"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u1"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u2"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("vn"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v1"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v2"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMapGrid2d :: ~GEMglMapGrid2d () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
}

//////////////////
// extension check
bool GEMglMapGrid2d :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglMapGrid2d :: render(GemState *state) {
	glMapGrid2d (un, u1, u2, vn, v1, v2);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMapGrid2d :: unMess (t_float arg1) {	// FUN
	un = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMapGrid2d :: u1Mess (t_float arg1) {	// FUN
	u1 = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglMapGrid2d :: u2Mess (t_float arg1) {	// FUN
	u2 = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglMapGrid2d :: vnMess (t_float arg1) {	// FUN
	vn = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMapGrid2d :: v1Mess (t_float arg1) {	// FUN
	v1 = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglMapGrid2d :: v2Mess (t_float arg1) {	// FUN
	v2 = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglMapGrid2d :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid2d::unMessCallback),  	gensym("un"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid2d::u1MessCallback),  	gensym("u1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid2d::u2MessCallback),  	gensym("u2"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid2d::vnMessCallback),  	gensym("vn"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid2d::v1MessCallback),  	gensym("v1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid2d::v2MessCallback),  	gensym("v2"), A_DEFFLOAT, A_NULL);
}

void GEMglMapGrid2d :: unMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->unMess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid2d :: u1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u1Mess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid2d :: u2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u2Mess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid2d :: vnMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->vnMess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid2d :: v1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->v1Mess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid2d :: v2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->v2Mess ( static_cast<t_float>(arg0));
}
