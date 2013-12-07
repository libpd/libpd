/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block (Linux)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) 2002 James Tittle.  tigital@mac.com
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.


-----------------------------------------------------------------*/
#ifndef _INCLUDE__GEM_PIXES_PIX_FILMDARWIN_H_
#define _INCLUDE__GEM_PIXES_PIX_FILMDARWIN_H_

#include "pix_filmOS.h"

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_filmDarwin
  
  Loads in a film
  
  KEYWORDS
  pix
  
  DESCRIPTION
  
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_filmDarwin : public pix_filmOS
{
  CPPEXTERN_HEADER(pix_filmDarwin, GemBase);

    public:

  //////////
  // Constructor
  pix_filmDarwin(t_symbol *filename);

 protected:

  //////////
  // Destructor
  virtual ~pix_filmDarwin();

  //////////
  // close the movie file
  virtual void closeMess(void);
  //////////
  // open a movie up
  virtual void realOpen(char *filename);

  //////////
  // Do the rendering
  virtual void getFrame();
  
  virtual void postrender(GemState *state);
  
  virtual void startRendering();
  
  virtual void stopRendering();
  
  //////////
  // load film into RAM
  virtual void LoadRam();

  //////////
  // set the playback rate
  virtual void MovRate(float rate);
  
  //////////
  // set the audio volume
  virtual void MovVolume(float volume);
	
  //////////
  // dumps debug info
  virtual void doDebug();     
        
  //-----------------------------------
  // GROUP:	Movie data
  //-----------------------------------
   
  GWorldPtr		m_srcGWorld;
  Rect			m_srcRect;
  TimeValue		m_movieTime;
  Track			m_movieTrack;
  TimeValue		m_timeScale;
  TimeValue		duration;
  float			durationf;
  long			movieDur, movieScale;
  int			m_hiquality;
  int			m_play;
  float			m_rate;
  Fixed			playRate;
  TimeValue 		prevTime;
  TimeValue		curTime;
 // int 			newImage;
  int			m_Task;
  float			m_volume;

private:
  Movie			m_movie;

  //-----------------------------------
  // GROUP:	Texturing
  //-----------------------------------

 protected:
	
  //////////
  // static member functions
  static void openMessCallback   (void *data, t_symbol *filename);
  static void ramCallback(void *data);
  static void hiqualityCallback(void *data, t_floatarg state);
  static void rateCallback(void *data, t_floatarg state);
  static void debugCallback(void *data);
  static void volumeCallback(void *data, t_floatarg state);

};

#endif	// for header file
