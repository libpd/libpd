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

#include "GEMglPopAttrib.h"

CPPEXTERN_NEW ( GEMglPopAttrib );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPopAttrib :: GEMglPopAttrib	(){}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPopAttrib :: ~GEMglPopAttrib () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPopAttrib :: render(GemState *state) {
	glPopAttrib ();
}
/////////////////////////////////////////////////////////
// static member function
void GEMglPopAttrib :: obj_setupCallback(t_class *classPtr) {}
