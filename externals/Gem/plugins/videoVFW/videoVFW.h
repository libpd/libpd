/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an video into a pix block: VideoForWindows backend

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__VIDEOVFW_VIDEOVFW_H_
#define _INCLUDE_GEMPLUGIN__VIDEOVFW_VIDEOVFW_H_

#include "plugins/videoBase.h"

#ifdef HAVE_VFW_H
# include <vfw.h>
#endif

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
	pix_video
    
  captures a video on NT machines
    
  KEYWORDS
  pix
    
  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXPORT videoVFW : public videoBase {
  public:
    //////////
    // Constructor
    videoVFW(void);
    	    	
    //////////
    // Destructor
    virtual ~videoVFW(void);

#ifdef HAVE_VFW_H
    ////////
    // open the video-device
    virtual bool           openDevice(gem::Properties&);
    virtual void          closeDevice(void);
    
    //////////
    // Start up the video device
    // [out] int - returns 0 if bad
    bool startTransfer(void);
    //////////
    // Stop the video device
    // [out] int - returns 0 if bad
    bool stopTransfer(void);

    //////////
    // get the next frame
    bool grabFrame(void);

    //////////
    // Set the video dimensions
    virtual bool	    	setColor(int);

    virtual bool enumProperties(gem::Properties&readable, gem::Properties&writeable);
    virtual void setProperties(gem::Properties&);
    virtual void getProperties(gem::Properties&);

  protected:
    HWND		m_hWndC;
    void		videoFrame(LPVIDEOHDR lpVHdr);
  private:
    static void videoFrameCallback(HWND hWnd, LPVIDEOHDR lpVHdr);

#endif /*HAVE_VFW_H */
  }; 
};};

#endif	// for header file
