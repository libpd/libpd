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
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "ImageIO.h"
#include "Gem/RTE.h"
#include "Gem/Files.h"

#include "plugins/imagesaver.h"
#include "plugins/PluginFactory.h"

namespace gem { namespace PixImageSaver {
  static gem::plugins::imagesaver*s_instance=NULL;
  static gem::plugins::imagesaver*getInstance(void) {
    if(NULL==s_instance) {
      s_instance=gem::plugins::imagesaver::getInstance();
    }
    return s_instance;
  }
}; };


/***************************************************************************
 *
 * mem2image - Save an image to a file
 *
 ***************************************************************************/
GEM_EXTERN int mem2image(imageStruct* image, const char *filename, const int type)
{
  gem::plugins::imagesaver*piximagesaver=gem::PixImageSaver::getInstance();
  if(piximagesaver) {
    std::string fname=filename;
    std::string mimetype;
    gem::Properties props;
    if(type>0) {
      props.set("quality", (float)type);
    }
    if(piximagesaver->save(*image, filename, mimetype, props)) {
      return (1);
    }
  }
  error("GEM: Unable to save image to '%s'", filename);
  return (0);
}
