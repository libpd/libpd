////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
  
#include "plugins/video.h"
#include "plugins/PluginFactory.h"

gem::plugins::video :: ~video(void) {}

gem::plugins::video*gem::plugins::video::getInstance(void) {
 return NULL;
}

static gem::PluginFactoryRegistrar::dummy<gem::plugins::video> fac_videodummy;
