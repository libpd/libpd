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

#include "GEMglPopName.h"

CPPEXTERN_NEW ( GEMglPopName);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPopName :: GEMglPopName	(){
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPopName :: ~GEMglPopName () {}

//////////////////
// extension check
bool GEMglPopName :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglPopName :: render(GemState *state) {
	glPopName ();
}

/////////////////////////////////////////////////////////
// static member functions
//
void GEMglPopName :: obj_setupCallback(t_class *classPtr) {}
