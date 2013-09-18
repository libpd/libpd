/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block 
(OS independant parent-class)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__FILMDS_FILMDS_H_
#define _INCLUDE_GEMPLUGIN__FILMDS_FILMDS_H_
#include "plugins/filmBase.h"

#if defined(_WIN32) && defined(HAVE_DIRECTSHOW)
# include <dshow.h>
# include <qedit.h>
#endif

   /*-----------------------------------------------------------------
     -------------------------------------------------------------------
     CLASS
     filmDS
    
     film-loader class for AVI(windoze)
    
     KEYWORDS
     pix film movie
    
     DESCRIPTION

     -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXPORT filmDS : public filmBase {
 public:
  
     //////////
     // Constructor
     filmDS(void);

     //////////
     // Destructor
     virtual ~filmDS(void);

#if defined(_WIN32) && defined(HAVE_DIRECTSHOW)
     //////////
     // open a movie up
     virtual bool open(const std::string filename, const gem::Properties&);
     //////////
     // close the movie file
     virtual void close(void);

     //////////
     // get the next frame
     virtual pixBlock* getFrame(void);

     //////////
     // set the next frame to read;
     virtual errCode changeImage(int imgNum, int trackNum=-1);


     // the raw buffer for decoding...
     int			m_nRawBuffSize;
     unsigned char*	m_RawBuffer;
     GLint                 m_format;

     int 	    	m_reqFrame;

     pixBlock		*m_pixBlock;

     unsigned char*  m_frame;  /* this points to a buffer for decompression */

     int			m_xsize;
     int			m_ysize;
     int			m_csize;

 protected:

 private:
     IBaseFilter				*VideoFilter;		// Base Filter for video
     IBaseFilter				*SampleFilter;		// Sample filter
     IBaseFilter				*NullFilter;		// Null render base Filter for video
     ISampleGrabber			*SampleGrabber;		// Sample grabber
     IGraphBuilder			*FilterGraph;		// Filter Graph for movie playback
     ISampleGrabber			*VideoGrabber;		// Video grabber
     IMediaControl			*MediaControl;		// MediaControl interface
     IMediaSeeking			*MediaSeeking;		// MediaSeeking interface
     IMediaPosition			*MediaPosition;		// MediaPosition interface
     LONGLONG				m_Duration;			// Duration of video
     LONGLONG				m_LastFrame;		// Last frame

     unsigned long		m_GraphRegister;

#endif //DIRECT_SHOW
};};   };

#endif	// for header file
