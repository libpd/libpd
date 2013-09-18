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

#include "GEMglFinish.h"

CPPEXTERN_NEW ( GEMglFinish );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglFinish :: GEMglFinish	() {}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglFinish :: ~GEMglFinish () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglFinish :: render(GemState *state) {
	glFinish ();
}

/////////////////////////////////////////////////////////
// static member function
void GEMglFinish :: obj_setupCallback(t_class *classPtr) {}
