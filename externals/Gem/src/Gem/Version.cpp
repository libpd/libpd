/*
 *  GemVersion.cpp
 *  Gem
 *
 *  Created by zmoelnig on 7/30/08.
 *  Copyright 2008 IEM @ KUG. All rights reserved.
 *
 */
#include "Gem/GemConfig.h"
 
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#include "Gem/Version.h"

#ifdef HAVE_VERSION_H
# include "version_current.h"
#endif

#ifndef GEM_VERSION_BUGFIX
# define GEM_VERSION_BUGFIX 3
#endif

#ifndef GEM_VERSION_CODENAME 
# define GEM_VERSION_CODENAME "extended"
#endif


const char* gem::Version :: versionString() {
return ( "" STRINGIFY(GEM_VERSION_MAJOR) "." STRINGIFY(GEM_VERSION_MINOR) "." STRINGIFY(GEM_VERSION_BUGFIX) \
               " " STRINGIFY(GEM_VERSION_CODENAME) );
}

bool gem::Version :: versionCheck(int major, int minor) {
 return ((GEM_VERSION_MAJOR==major) && (GEM_VERSION_MINOR==minor));
}

