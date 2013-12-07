/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    video backend for Gem

    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    Linux version by Miller Puckette. msp@ucsd.edu
	
-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__VIDEOAVT_VIDEOAVT_H_
#define _INCLUDE_GEMPLUGIN__VIDEOAVT_VIDEOAVT_H_

#include "plugins/videoBase.h"

#if defined  HAVE_LIBPVAPI
# define HAVE_AVT
#endif

#ifdef HAVE_AVT
/* olala, Prosilica re-invents the wheel and uses non-standard defines for OSs */
# ifdef __linux__
#  define _LINUX
# endif

# ifdef __APPLE__
#  define _OSX
# endif

// _QNX

/* Prosilica's ARCH defines */
# if !defined(_x86) && (defined(_X86_) || defined(__i386__) || defined(__i586__) || defined(__i686__))
#  define _x86 1
# endif

# if !defined(_ppc) && ( defined(__ppc__))
#  define _ppc 1
# endif


/* Prosilica decided to use "Status" as a field-name, but it gets defined to "int" in /usr/include/X11/Xlib.h */
#ifdef Status
# undef Status
#endif

# include "PvApi.h"
#endif
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	videoAVT
    
    Grabs in a video
    
KEYWORDS
    pix
    
DESCRIPTION

    support for the "AVT GiGE SDK" by Prosilica
    
-----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXPORT videoAVT : public videoBase {
    public:
        //////////
        // Constructor
    	videoAVT(void);
    	    	
    	//////////
    	// Destructor
    	virtual ~videoAVT(void);

#ifdef HAVE_AVT
	////////
	// open the video-device
	virtual bool           openDevice(gem::Properties&props);
	virtual void          closeDevice(void);
    
    	//////////
    	// Start up the video device
    	// [out] int - returns 0 if bad
    	virtual bool	    	startTransfer(void);
	//////////
    	// Stop the video device
    	// [out] int - returns 0 if bad
    	virtual bool	   	stopTransfer(void);

	//////////
	// get the next frame
	virtual bool grabFrame(void);

	virtual std::vector<std::string>enumerate(void);

	
	//////////
	// properties
	virtual bool enumProperties(gem::Properties&readable,
				    gem::Properties&writeable);
	virtual void setProperties(gem::Properties&writeprops);
	virtual void getProperties(gem::Properties&readprops);

   
 protected:

  tPvHandle m_grabber;
#define AVT_FRAMESCOUNT 4
  tPvFrame  m_frames[AVT_FRAMESCOUNT];

  pixBlock* getFrame(void);

  virtual void grabbedFrame(const tPvFrame&);
  static void grabCB(tPvFrame*);
  virtual void resizeFrames(unsigned long int);

#endif /* HAVE_AVT */

}; 
};};

#endif	// for header file
