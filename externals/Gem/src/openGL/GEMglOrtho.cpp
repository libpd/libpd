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

#include "GEMglOrtho.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglOrtho );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglOrtho :: GEMglOrtho	(int argc, t_atom*argv)
{
  left=right=bottom=top=zNear=zFar=0.0;
  if (argc>0)left  =atom_getfloat(argv);
  if (argc>1)right =atom_getfloat(argv+1);
  if (argc>2)bottom=atom_getfloat(argv+2);
  if (argc>3)top   =atom_getfloat(argv+3);
  if (argc>4)zNear =atom_getfloat(argv+4);
  if (argc>5)zFar  =atom_getfloat(argv+5);

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("left"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("right"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("bottom"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("top"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zNear"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zFar"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglOrtho :: ~GEMglOrtho () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglOrtho :: render(GemState *state) {
	glOrtho (left, right, bottom, top, zNear, zFar);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglOrtho :: leftMess (t_float arg1) {	// FUN
	left = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglOrtho :: rightMess (t_float arg1) {	// FUN
	right = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglOrtho :: bottomMess (t_float arg1) {	// FUN
	bottom = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglOrtho :: topMess (t_float arg1) {	// FUN
	top = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglOrtho :: zNearMess (t_float arg1) {	// FUN
	zNear = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglOrtho :: zFarMess (t_float arg1) {	// FUN
	zFar = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglOrtho :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglOrtho::leftMessCallback),  	gensym("left"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglOrtho::rightMessCallback),  	gensym("right"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglOrtho::bottomMessCallback),  	gensym("bottom"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglOrtho::topMessCallback),  	gensym("top"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglOrtho::zNearMessCallback),  	gensym("zNear"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglOrtho::zFarMessCallback),  	gensym("zFar"), A_DEFFLOAT, A_NULL);
};

void GEMglOrtho :: leftMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->leftMess ( static_cast<t_float>(arg0));
}
void GEMglOrtho :: rightMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->rightMess ( static_cast<t_float>(arg0));
}
void GEMglOrtho :: bottomMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->bottomMess ( static_cast<t_float>(arg0));
}
void GEMglOrtho :: topMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->topMess ( static_cast<t_float>(arg0));
}
void GEMglOrtho :: zNearMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->zNearMess ( static_cast<t_float>(arg0));
}
void GEMglOrtho :: zFarMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->zFarMess ( static_cast<t_float>(arg0));
}
