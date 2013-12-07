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

#include "GEMglFlush.h"

CPPEXTERN_NEW ( GEMglFlush );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglFlush :: GEMglFlush	(){}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglFlush :: ~GEMglFlush () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglFlush :: render(GemState *state) {
	glFlush ();
}

/////////////////////////////////////////////////////////
// static member function
void GEMglFlush :: obj_setupCallback(t_class *classPtr) {}
