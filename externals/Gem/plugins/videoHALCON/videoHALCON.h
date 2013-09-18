/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    grab images using HALCON

    HALCON is a proprietary machine vision library by MVtec, that supports a wide range
    of image acquisition devices (most noteable: GigE-cameras)

    see http://halcon.de/

    you will need to get a license key from your vendor


    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "LICENSE.txt" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__VIDEOHALCON_VIDEOHALCON_H_
#define _INCLUDE_GEMPLUGIN__VIDEOHALCON_VIDEOHALCON_H_

#include "plugins/videoBase.h"

#include <map>

#if defined HAVE_LIBHALCON
# define HAVE_HALCON
#endif

#ifdef Status
/* ouch: Xlib.h defines "Status" as "int", but Halcon uses "Status" as a
 * variable name
 */
# undef Status
#endif

#ifdef HAVE_HALCON
# include "HalconCpp.h"
#endif
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	pix_video

    Loads in a video

KEYWORDS
    pix

DESCRIPTION

-----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXPORT videoHALCON : public videoBase {
    public:
        //////////
        // Constructor
    	videoHALCON(void);
    	    	
    	//////////
    	// Destructor
    	virtual ~videoHALCON(void);

#ifdef HAVE_HALCON
	////////
	// open the video-device
      virtual bool           openDevice(gem::Properties&);
      virtual void          closeDevice(void);

    	//////////
    	// Start up the video device
    	// [out] int - returns 0 if bad
    	bool	    	startTransfer(void);
	//////////
    	// Stop the video device
    	// [out] int - returns 0 if bad
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
  Halcon::HFramegrabber*m_grabber;
  std::string m_backendname;
  std::vector<std::string> m_backends;

  std::map<std::string, Halcon::HTuple>m_readable, m_writeable;

  /* short-cut device-name as found by "enumerate"
   * this maps the devicename to the backend that provides the device
   */
  std::map<std::string, std::string>m_device2backend;

#endif /* HAVE_HALCON */

};
};};

#endif	// for header file
