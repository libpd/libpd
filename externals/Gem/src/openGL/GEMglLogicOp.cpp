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

#include "GEMglLogicOp.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglLogicOp);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLogicOp :: GEMglLogicOp	(int argc, t_atom*argv) :
		opcode(0)
{
  if(1==argc)opcodeMess(argv[0]); else if(argc) throw(GemException("invalid number of arguments"));
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("opcode"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLogicOp :: ~GEMglLogicOp () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLogicOp :: render(GemState *state) {
	glLogicOp (opcode);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglLogicOp :: opcodeMess (t_atom arg) {	// FUN
	opcode = static_cast<GLenum>(getGLdefine(&arg));
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglLogicOp :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLogicOp::opcodeMessCallback),  	gensym("opcode"), A_GIMME, A_NULL);
};

void GEMglLogicOp :: opcodeMessCallback (void* data, t_symbol*,int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->opcodeMess ( argv[0]);
}
