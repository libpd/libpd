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

#ifndef _INCLUDE_GEMPLUGIN__VIDEODV4L_VIDEODV4L_H_
#define _INCLUDE_GEMPLUGIN__VIDEODV4L_VIDEODV4L_H_
#include "plugins/videoBase.h"

#if defined HAVE_LIBIEC61883 && defined HAVE_LIBRAW1394 && defined HAVE_LIBDV
# define HAVE_DV
#endif

#ifdef HAVE_DV
#include <libraw1394/raw1394.h>
#include <libiec61883/iec61883.h>
#include <libdv/dv.h>


#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#endif


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
 class GEM_EXPORT videoDV4L : public videoBase {
    public:
        //////////
        // Constructor
    	videoDV4L(void);
    	    	
    	//////////
    	// Destructor
    	virtual ~videoDV4L(void);
#ifdef HAVE_DV
	////////
	// open the video-device
	virtual bool           openDevice(gem::Properties&props);
	virtual void          closeDevice(void);
    
  //////////
  // Start up the video device
  // [out] int - returns 0 if bad
  bool	    	startTransfer(void);
	//////////
  // Stop the video device
  // returns TRUE is transfer was running, FALSE is otherwise
  bool	   	stopTransfer(void);

	//////////
	// get the next frame
	bool grabFrame(void);

  int decodeFrame(unsigned char*, int);
  static int iec_frame(unsigned char *data,int len, int complete, void *arg);

	//////////
	// Set the video dimensions
	virtual bool	  setColor(int);
	virtual bool		setQuality(int);

  virtual std::vector<std::string>enumerate(void);


  virtual bool enumProperties(gem::Properties&readable,
			      gem::Properties&writeable);
  virtual void setProperties(gem::Properties&writeprops);
  virtual void getProperties(gem::Properties&readprops);
    
 protected:

  //-----------------------------------
  // GROUP:	Linux specific video data
  //-----------------------------------
  int m_dvfd;

  raw1394handle_t m_raw;
  iec61883_dv_fb_t m_iec;

  ////////
  // the DV-decoder
  dv_decoder_t *m_decoder;

  bool m_parsed;
  uint8_t*m_frame[3];
  int m_pitches[3];

  int m_quality;
#endif /* HAVE_DV */
};
};};

#endif	// for header file
