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

#include "plugins/record.h"
#include "plugins/PluginFactory.h"

gem::plugins::record :: ~record(void) {}

gem::plugins::record*gem::plugins::record::getInstance(void) {
  return NULL;
}

static gem::PluginFactoryRegistrar::dummy<gem::plugins::record> fac_recorddummy;
