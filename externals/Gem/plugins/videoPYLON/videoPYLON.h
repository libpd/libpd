/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    Load an video into a pix block

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    Linux version by Miller Puckette. msp@ucsd.edu
	
-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__VIDEOPYLON_VIDEOPYLON_H_
#define _INCLUDE_GEMPLUGIN__VIDEOPYLON_VIDEOPYLON_H_

#include "plugins/videoBase.h"
#include <map>

#ifdef Status
/* ouch: Xlib.h defines "Status" as "int", but Pylon uses "Status" as a
 * variable name
 */
# undef Status
#endif
#ifdef None
# undef None
#endif

#include "pylon/PylonIncludes.h"
#include <pylon/gige/BaslerGigECamera.h>
typedef Pylon::CBaslerGigECamera Camera_t;
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	pix_video
    
    Loads in a video
    
KEYWORDS
    pix
    
DESCRIPTION

    "dimen" (int, int) - set the x,y dimensions
    "zoom" (int, int) - the zoom factor (1.0 is nominal) (num / denom)
    "bright" (int) - the brightnes
    "contrast" (int) - the contrast
    "hue" (int) - the hue
    "sat" (int) - the saturation
    
-----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXPORT videoPYLON : public videoBase {
    public:
    //////////
    // Constructor
    videoPYLON(void);
    	    	
    //////////
    // Destructor
    virtual ~videoPYLON(void);

    ////////
    // open the video-device
    virtual bool           openDevice(gem::Properties&writeprops);
    virtual void          closeDevice(void);
    
    //////////
    // Start up the video device
    // [out] bool - returns FALSE if bad
    bool	    	startTransfer(void);
    //////////
    // Stop the video device
    // [out] bool - returns FALSE if bad
    bool	   	stopTransfer(void);
    
    //////////
    // get the next frame
    bool grabFrame(void);

    virtual std::vector<std::string>enumerate(void);



  virtual bool enumProperties(gem::Properties&readable,
			      gem::Properties&writeable);
  virtual void setProperties(gem::Properties&writeprops);
  virtual void getProperties(gem::Properties&readprops);

   
  protected:
  class CGrabBuffer;

  Pylon::PylonAutoInitTerm autoInitTerm;
  Pylon::CTlFactory*m_factory;
    
  Pylon::CBaslerGigECamera*m_camera;
  Pylon::CBaslerGigEStreamGrabber*m_grabber;

  class Converter;
  Converter*m_converter;

  uint32_t m_numBuffers;
  std::vector<CGrabBuffer*> m_buffers;
  std::map<std::string, Pylon::CDeviceInfo>m_id2device;
}; 
};};

#endif	// for header file
