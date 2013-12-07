////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
  
#include "plugins/imagesaverBase.h"

using namespace gem::plugins;

/////////////////////////////////////////////////////////
//
// imagesaverBase
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
imagesaverBase :: imagesaverBase(bool threadable) : m_threadable(m_threadable) {
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
imagesaverBase :: ~imagesaverBase()
{
}

float imagesaverBase ::estimateSave( const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  /* the default is rather bad */
  return 0.;
}
void imagesaverBase ::getWriteCapabilities(std::vector<std::string>&mimetypes, gem::Properties&props) {
  mimetypes.clear();
  props.clear();
}
