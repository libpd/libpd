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

#include "GEMglDepthFunc.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglDepthFunc );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglDepthFunc :: GEMglDepthFunc (int argc, t_atom*argv) :
		func(0)
{
  if(1==argc)funcMess(argv[0]); else if(argc) throw(GemException("invalid number of arguments"));
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("func"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglDepthFunc :: ~GEMglDepthFunc () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglDepthFunc :: render(GemState *state) {
	glDepthFunc (func);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglDepthFunc :: funcMess (t_atom arg) {	// FUN
	func = static_cast<GLenum>(getGLdefine(&arg));
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglDepthFunc :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDepthFunc::funcMessCallback),  	gensym("func"), A_GIMME, A_NULL);
};

void GEMglDepthFunc :: funcMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->funcMess ( argv[0]);
}
