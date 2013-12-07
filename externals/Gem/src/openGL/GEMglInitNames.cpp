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

#include "GEMglInitNames.h"

CPPEXTERN_NEW( GEMglInitNames);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglInitNames :: GEMglInitNames(){
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglInitNames :: ~GEMglInitNames () {}

//////////////////
// extension check
bool GEMglInitNames :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglInitNames :: render(GemState *state) {
	glInitNames ();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglInitNames :: obj_setupCallback(t_class *classPtr) {}
