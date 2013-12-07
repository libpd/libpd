/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	- file handling with Gem

    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/



#ifndef _INCLUDE__GEM_GEM_FILES_H_
#define _INCLUDE__GEM_GEM_FILES_H_

#include <string>
#include <vector>
#include "Gem/ExportDef.h"

namespace gem {
  
  namespace files {

    GEM_EXTERN std::vector<std::string>getFilenameListing(const std::string&pattern);
    GEM_EXTERN std::string expandEnv(const std::string&, bool bashfilename=false);

    GEM_EXTERN std::string getExtension(const std::string&filename, bool make_lowercase=false);

  };
};

#endif /* _INCLUDE__GEM_GEM_FILES_H_ */
