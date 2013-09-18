////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file 
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "film.h"
#include "plugins/PluginFactory.h"

gem::plugins::film :: ~film(void) {}

gem::plugins::film*gem::plugins::film::getInstance(void) {
 return NULL;
}

/* initialize the film factory */
static gem::PluginFactoryRegistrar::dummy<gem::plugins::film> fac_filmdummy;
