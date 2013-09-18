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

#ifndef _INCLUDE_GEMPLUGIN__VIDEOV4L_VIDEOV4L_H_
#define _INCLUDE_GEMPLUGIN__VIDEOV4L_VIDEOV4L_H_

#include "plugins/videoBase.h"

#ifdef HAVE_LINUX_VIDEODEV_H
# include <linux/videodev.h>
#endif /* HAVE_LINUX_VIDEODEV_H */

#ifdef HAVE_LIBV4L1
# include <libv4l1.h> 
#endif /* HAVE_LIBV4L1 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/types.h>
#include <sys/mman.h>
//#ifdef HAVE_PTHREADS
#include <pthread.h>
//#endif
#define V4L_DEVICENO 0
#define V4L_NBUF 2
#define V4L_COMPOSITEIN 1

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	videoV4L
    
    grab images from a v4l(1) device
    
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
 class GEM_EXPORT videoV4L : public videoBase {
    public:
        //////////
        // Constructor
    	videoV4L(void);
    	    	
    	//////////
    	// Destructor
    	virtual ~videoV4L(void);

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
    	// [out] int - returns 0 if bad
    	bool	   	stopTransfer(void);

	//////////
	// get the next frame
	bool grabFrame(void);

	//////////
	// Set the video dimensions

	virtual bool enumProperties(gem::Properties&readable,
				    gem::Properties&writeable);
	virtual void setProperties(gem::Properties&props);
	virtual void getProperties(gem::Properties&props);

	virtual bool	    	setColor(int);

	virtual std::vector<std::string>enumerate(void);
   
 protected:

  //-----------------------------------
  // GROUP:	Linux specific video data
  //-----------------------------------
	
  struct video_tuner vtuner;
  struct video_picture vpicture;
  struct video_buffer vbuffer;
  struct video_capability vcap;
  struct video_channel vchannel;
  struct video_audio vaudio;
  struct video_mbuf vmbuf;
  struct video_mmap vmmap[V4L_NBUF];
  int tvfd;
  int frame, last_frame;
  unsigned char *videobuf;
  int skipnext;
  int mytopmargin, mybottommargin;
  int myleftmargin, myrightmargin;

  int m_gotFormat; // the format returned by the v4l-device (not an openGL-format!)
  bool m_colorConvert; // do we have to convert the colour-space manually ?

  int m_norm;    // PAL, NTSC,...
  int m_channel;


  unsigned int errorcount;
 };
};};

#endif	// for header file
