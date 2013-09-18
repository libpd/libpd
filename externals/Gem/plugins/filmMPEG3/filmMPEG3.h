/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block (Linux)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.


-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__FILMMPEG3_FILMMPEG3_H_
#define _INCLUDE_GEMPLUGIN__FILMMPEG3_FILMMPEG3_H_
#include "plugins/filmBase.h"
#include <stdio.h>
#ifdef HAVE_LIBMPEG3
#include <libmpeg3.h>
#endif // MPEG3

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  filmMPEG3
  
  Loads in a film
  
  KEYWORDS
  pix
  
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXPORT filmMPEG3 : public filmBase {
 public:

  //////////
  // Constructor
  filmMPEG3(void);

  //////////
  // Destructor
  virtual ~filmMPEG3(void);

#ifdef HAVE_LIBMPEG3
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

  mpeg3_t      *mpeg_file;
#endif
};};};

#endif	// for header file
