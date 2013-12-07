/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load a digital video (quicktime4linux) for linux

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.


-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__FILMQT4L_FILMQT4L_H_
#define _INCLUDE_GEMPLUGIN__FILMQT4L_FILMQT4L_H_
#include "plugins/filmBase.h"
#include <stdio.h>

#include <quicktime.h>
#include <colormodels.h>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  filmQT4L1
  
  Loads in a film
  
  KEYWORDS
  pix
  
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXPORT filmQT4L : public filmBase {
 public:

  //////////
  // Constructor
  filmQT4L(void);

  //////////
  // Destructor
  virtual ~filmQT4L(void);

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
  virtual errCode changeImage(int imgNum, int trackNum = -1);

  //-----------------------------------
  // GROUP:	Movie data
  //-----------------------------------
 protected:

  quicktime_t  *m_quickfile;
  int           m_qtformat;

  imageStruct   m_qtimage;
  int m_lastFrame;

};};};

#endif	// for header file
