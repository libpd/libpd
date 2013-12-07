/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load a digital video (like AVI, Mpeg, Quicktime) into a pix block 
(OS independant parent-class)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__FILMDARWIN_FILMDARWIN_H_
#define _INCLUDE_GEMPLUGIN__FILMDARWIN_FILMDARWIN_H_

#if defined __APPLE__ && !defined __x86_64__
// with OSX10.6, apple has removed loads of Carbon functionality (in 64bit mode)
// LATER make this a real check in configure
# define HAVE_CARBONQUICKTIME
#endif

#include "plugins/filmBase.h"

#ifdef HAVE_CARBONQUICKTIME
# include <Carbon/Carbon.h>
# include <QuickTime/QuickTime.h>
#endif /* HAVE_CARBONQUICKTIME */

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  filmDarwin
    
  film-loader class for MacOS-X (Darwin)
    
  KEYWORDS
  pix film movie
    
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXPORT filmDarwin : public filmBase {
 public:
  
  //////////
  // Constructor
  filmDarwin(void);

  //////////
  // Destructor
  virtual ~filmDarwin(void);

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

 protected:
#ifdef HAVE_CARBONQUICKTIME
  Movie			m_movie; 
  GWorldPtr		m_srcGWorld;
  TimeValue		m_movieTime;
  Track			m_movieTrack;
  Media			m_movieMedia;
  TimeValue		m_timeScale;
  TimeValue		duration;
#endif //HAVE_CARBONQUICKTIME
  double		durationf;

};};};

#endif	// for header file
