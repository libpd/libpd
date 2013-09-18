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

#ifndef _INCLUDE__GEM_PLUGINS_FILMBASE_H_
#define _INCLUDE__GEM_PLUGINS_FILMBASE_H_

#include "plugins/film.h"
#include "Gem/Image.h"

#include <string>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  film
    
  parent class for the system- and library-dependent film-loader classes
    
  KEYWORDS
  pix film movie
    
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXTERN filmBase : public film
{
 private:
  class PIMPL;
  PIMPL*m_pimpl;

  
  //////////
  // Constructor
 protected:  
  /* initialize the filmloader
   *
   * set 'threadable' to false, if your backend is known to be not threadsafe
   */
  filmBase(bool threadable);

 public:
  filmBase(void);
  ////////
  // Destructor
  /* free what is apropriate */
  virtual ~filmBase(void);

 public:
  ////////
  // returns true if instance can be used in thread
  virtual bool isThreadable(void);


  void close(void);

  
  /**
   * list all properties the currently opened film supports
   * if no film is opened, this returns generic backend properties 
   * which can be different from media specific properties
   * after calling, "readable" will hold a list of all properties that can be read
   * and "writeable" will hold a list of all properties that can be set
   * if the enumeration fails, this returns <code>false</code>
   */

  virtual bool enumProperties(gem::Properties&readable,
			      gem::Properties&writeable);

  /**
   * properties implemented in here:
   * - "auto"
   * - "colorspace"
   */
  virtual void setProperties(gem::Properties&props);

  /**
   * properties implemented in here:
   * - "width"
   * - "height"
   * - "frames"
   * - "fps"
   */
  virtual void getProperties(gem::Properties&props);




  //////////
  // set the wanted color-space
  /* could be used for switching the colourspace on the fly 
   * normally the colour-space of a film could be set when opening a movie
   */
  virtual void requestColor(GLenum format){m_wantedFormat=format;}

  //////////
  // get the number of frames
  /* the number of frames can depend on the track
   * so this will return the framenum of the current track
   */
  virtual int getFrameNum(void){return m_numFrames;}

  // get the frames per seconds (or "-1" if unknown)
  virtual double getFPS(void);

  // get xsize of the frame
  virtual int getWidth(void) {return m_image.image.xsize;}
  // get ysize of the frame
  virtual int getHeight(void) {return m_image.image.ysize;}


  virtual void setAuto(double);

  //////////
  // Change which image to display
  /* this is the second core function of this class:
   * most decoding-libraries can set the frame-number on a random-access basis.
   * some cannot, then this might do nothing
   * you could also switch between various tracks of a file (if the format supports it)
   * specifying trackNum as -1 means "same track as before"
   */
  virtual errCode changeImage(int imgNum, int trackNum=-1);

 protected:
  /* i guess every child-class might need (some of the) following variables  */

  /* here the frame is stored
   */
  pixBlock m_image;
  /* this is the colour-space the user requested (like GL_RGBA)
   */
  GLenum  m_wantedFormat;

  /* probably a good idea to know how many frames/tracks there are in this movie
   * the number of frames might vary between tracks, so this refers to the current track
   */
  int  m_numFrames, m_numTracks;
  /* most often we will also want to know what the current frame/track is...
   */
  int  m_curFrame, m_curTrack;

  /* if the (frame,track) is the same as the last time, 
   * we probably don't want to decode this frame again.
   * if so m_readNext should be FALSE
   */
  bool m_readNext;

  // auto increment
  double m_auto;

  //////////////////////
  // the frame-rate
  double m_fps;

  bool m_newfilm;
};

};}; // namespace gem::plugins

#endif	// for header file
