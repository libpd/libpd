#ifndef GEM_VERSION_H
#define GEM_VERSION_H

#include "Gem/ExportDef.h"

#define GEM_VERSION_MAJOR 0
#define GEM_VERSION_MINOR 93

namespace gem {
class GEM_EXTERN Version {
  public:
	const static char* versionString(void);
	static bool versionCheck(int major, int minor);
}; };

#define GemVersion gem::Version

#endif

