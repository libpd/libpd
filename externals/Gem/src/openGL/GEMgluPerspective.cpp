////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2005 tigital@mac.com
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
////////////////////////////////////////////////////////

#include "GEMgluPerspective.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMgluPerspective , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMgluPerspective
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMgluPerspective :: GEMgluPerspective	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		fovy(static_cast<GLdouble>(arg0)), 
		aspect(static_cast<GLdouble>(arg1)), 
		m_near(static_cast<GLdouble>(arg2)), 
		m_far(static_cast<GLdouble>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("fovy"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("aspect"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("near"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("far"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMgluPerspective :: ~GEMgluPerspective () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMgluPerspective :: render(GemState *state) {
	gluPerspective (fovy, aspect, m_near, m_far);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMgluPerspective :: fovyMess (t_float arg1) {	// FUN
	fovy = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMgluPerspective :: aspectMess (t_float arg1) {	// FUN
	aspect = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMgluPerspective :: nearMess (t_float arg1) {	// FUN
	m_near = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMgluPerspective :: farMess (t_float arg1) {	// FUN
	m_far = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMgluPerspective :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluPerspective::fovyMessCallback),  	gensym("fovy"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluPerspective::aspectMessCallback),  	gensym("aspect"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluPerspective::nearMessCallback),  	gensym("near"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluPerspective::farMessCallback),  	gensym("far"), A_DEFFLOAT, A_NULL);
};

void GEMgluPerspective :: fovyMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->fovyMess ( static_cast<t_float>(arg0));
}
void GEMgluPerspective :: aspectMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->aspectMess ( static_cast<t_float>(arg0));
}
void GEMgluPerspective :: nearMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nearMess ( static_cast<t_float>(arg0));
}
void GEMgluPerspective :: farMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->farMess ( static_cast<t_float>(arg0));
}
