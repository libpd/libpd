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

#include "GEMglEndList.h"

CPPEXTERN_NEW ( GEMglEndList );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEndList :: GEMglEndList	(){}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEndList :: ~GEMglEndList () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglEndList :: render(GemState *state) {
	glEndList ();
}
/////////////////////////////////////////////////////////
// static member function
void GEMglEndList :: obj_setupCallback(t_class *classPtr) {}
