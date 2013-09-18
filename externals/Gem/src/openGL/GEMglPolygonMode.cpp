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

#include "GEMglPolygonMode.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglPolygonMode);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPolygonMode :: GEMglPolygonMode	(int argc, t_atom*argv) :
		face(0), 
		mode(0)
{
  if(2==argc){faceMess(argv[0]); modeMess(argv[1]);} else if (argc) throw(GemException("invalid number of arguments"));

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("face"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPolygonMode :: ~GEMglPolygonMode () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPolygonMode :: render(GemState *state) {
	glPolygonMode (face, mode);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglPolygonMode :: faceMess (t_atom arg) {	// FUN
  face = static_cast<GLenum>(getGLdefine(&arg));
	setModified();
}

void GEMglPolygonMode :: modeMess (t_atom arg) {	// FUN
  mode = static_cast<GLenum>(getGLdefine(&arg));
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglPolygonMode :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPolygonMode::faceMessCallback),  	gensym("face"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPolygonMode::modeMessCallback),  	gensym("mode"), A_GIMME, A_NULL);
};

void GEMglPolygonMode :: faceMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->faceMess ( argv[0]);
}
void GEMglPolygonMode :: modeMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->modeMess ( argv[0]);
}
