/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an SGI picture

Copyright (c) 2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.


-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__IMAGESGI_IMAGESGI_H_
#define _INCLUDE_GEMPLUGIN__IMAGESGI_IMAGESGI_H_
#include "plugins/imageloaderBase.h"
#include <stdio.h>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  imageSGI
  
  Loads in a picture
  
  KEYWORDS
  pix
  
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins { 
class GEM_EXPORT imageSGI : public gem::plugins::imageloaderBase {
 public:

  //////////
  // Constructor
  imageSGI(void);
  virtual ~imageSGI(void);

  //////////
  // read an image
  virtual bool load(std::string filename, imageStruct&result, gem::Properties&props);
  //////////
  // write an image
  virtual bool          save(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props);
  //////////
  // estimate, how well we could save this image
  virtual float estimateSave(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props);

};
};};

#endif	// for header file
