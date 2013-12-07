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

#include "GEMglBlendFunc.h"
#include "Gem/Exception.h"
CPPEXTERN_NEW_WITH_GIMME ( GEMglBlendFunc );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglBlendFunc :: GEMglBlendFunc (int argc, t_atom*argv) :
		sfactor(0), 
		dfactor(0)
{
  if(2==argc){sfactorMess(argv[0]); dfactorMess(argv[1]);} else if (argc) throw(GemException("invalid number of arguments"));

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("sfactor"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("dfactor"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglBlendFunc :: ~GEMglBlendFunc () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglBlendFunc :: render(GemState *state) {
	glBlendFunc (sfactor, dfactor);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglBlendFunc :: sfactorMess (t_atom arg) {	// FUN
	sfactor = static_cast<GLenum>(getGLdefine(&arg));
	setModified();
}

void GEMglBlendFunc :: dfactorMess (t_atom arg) {	// FUN
	dfactor = static_cast<GLenum>(getGLdefine(&arg));
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglBlendFunc :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBlendFunc::sfactorMessCallback),  	gensym("sfactor"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBlendFunc::dfactorMessCallback),  	gensym("dfactor"), A_GIMME, A_NULL);
};

void GEMglBlendFunc :: sfactorMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->sfactorMess ( argv[0]);
}
void GEMglBlendFunc :: dfactorMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->dfactorMess ( argv[0]);
}
