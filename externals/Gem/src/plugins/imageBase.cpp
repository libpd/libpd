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
  
#include "plugins/imageBase.h"

using namespace gem::plugins;

/////////////////////////////////////////////////////////
//
// imageBase
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
imageBase :: imageBase(bool threadable)
  : m_threadable(threadable)
{}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
imageBase :: ~imageBase()
{}

bool imageBase :: enumProperties(gem::Properties&readable,
			     gem::Properties&writeable) 
{
  readable.clear();
  writeable.clear();
  return false;
}

void imageBase :: setProperties(gem::Properties&props) {
  // nada
  m_properties=props;
#if 0
  std::vector<std::string> keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    enum gem::Properties::PropertyType typ=props.type(keys[i]);
    //std::cerr  << "key["<<keys[i]<<"]: "<<typ<<" :: ";
    switch(typ) {
    case (gem::Properties::NONE):
      props.erase(keys[i]);
      break;
    case (gem::Properties::DOUBLE):
      //std::cerr << gem::any_cast<double>(props.get(keys[i]));
      break;
    case (gem::Properties::STRING):
      //std::cerr << "'" << gem::any_cast<std::string>(props.get(keys[i])) << "'";
      break;
    default:
      //std::cerr << "<unknown:" << props.get(keys[i]).get_type().name() << ">";
      break;
    }
  }
  //std::cerr << std::endl;
#endif
}

void imageBase :: getProperties(gem::Properties&props) {
  // nada
  std::vector<std::string>keys=props.keys();
  unsigned int i=0;
  for(i=0; i<keys.size(); i++) {
    gem::any unset;
    props.set(keys[i], unset);
  }
}


bool imageBase :: isThreadable(void) {
  return m_threadable;
}
float imageBase::estimateSave( const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  return 0.;
}
void imageBase::getWriteCapabilities(std::vector<std::string>&mimetypes, gem::Properties&props) {
  mimetypes.clear();
  props.clear();
}

