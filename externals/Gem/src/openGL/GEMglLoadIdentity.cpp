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

#include "GEMglLoadIdentity.h"

CPPEXTERN_NEW ( GEMglLoadIdentity );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLoadIdentity :: GEMglLoadIdentity	(){}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLoadIdentity :: ~GEMglLoadIdentity () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLoadIdentity :: render(GemState *state) {
	glLoadIdentity ();
}
/////////////////////////////////////////////////////////
// static member function
void GEMglLoadIdentity :: obj_setupCallback(t_class *classPtr) {}
