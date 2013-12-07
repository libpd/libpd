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

#include "GEMglPushMatrix.h"

CPPEXTERN_NEW ( GEMglPushMatrix );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPushMatrix :: GEMglPushMatrix	() {}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPushMatrix :: ~GEMglPushMatrix () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPushMatrix :: render(GemState *state) {
	glPushMatrix ();
}
/////////////////////////////////////////////////////////
// static member function
void GEMglPushMatrix :: obj_setupCallback(t_class *classPtr) {}
