#ifndef _INCLUDE_GEMPLUGIN__VIDEOPYLON_STREAMGRABBERPROPERTIES_H_
#define _INCLUDE_GEMPLUGIN__VIDEOPYLON_STREAMGRABBERPROPERTIES_H_

#include "pylon/PylonIncludes.h"
# include <pylon/gige/BaslerGigECamera.h>

#include <string>
#include "Gem/Properties.h"

namespace gem { namespace pylon { namespace streamgrabberproperties {
                    void init(void);

                    gem::Properties&getKeys(void);
                    gem::Properties&setKeys(void);

                    void get(Pylon::CBaslerGigEStreamGrabber*device, 
                             std::string key,
                             gem::any&result);

                    bool set(Pylon::CBaslerGigEStreamGrabber*device, 
                             std::string key,
                             gem::Properties&props);

                  };};};

#endif /* _INCLUDE_GEMPLUGIN__VIDEOPYLON_STREAMGRABBERPROPERTIES_H_ */
