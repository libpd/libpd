/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Interface for the window manager

  Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_OUTPUT_GEMMACWINDOW_H_
#define _INCLUDE__GEM_OUTPUT_GEMMACWINDOW_H_

#include "Base/GemWindow.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  gemmacwindow

  The window manager

  DESCRIPTION

  Access to GemMan.

  "bang"  - swap the buffers
  "render" - render a frame now

  "create" - create a graphics window
  "destroy" - destroy the graphics window


  "buffer" - single or double buffered
  "fsaa" - full screen anti-aliasing

  "title" - set a title for the graphics window
  "border" - whether we want a border as decoration or not

  "dimen" - the window dimensions
  "fullscreen" - fullscreen mode
  "offset" - the window offset
  "secondscreen" - render to the secondscreen (auto-offset)

  "cursor" - whether we want a cursor or not
  "menubar" - hide notorious menubars
  "topmost" - set the window to stay on top

  -----------------------------------------------------------------*/


class GEM_EXTERN gemmacwindow : public GemWindow
{
  CPPEXTERN_HEADER(gemmacwindow, GemWindow);

    public:

  //////////
  // Constructor
  gemmacwindow(void);

 private:

  //////////
  // Destructor
  virtual ~gemmacwindow(void);


  // create window
  virtual bool create(void);

  // destroy window
  virtual void destroy(void);

  // check whether we have a window and if so, make it current
  virtual bool makeCurrent(void);
  
  virtual void swapBuffers(void);

  /* dispatch window events */
  virtual void dispatch(void);

   /* creation/destruction */
  virtual void        createMess(std::string);
  virtual void       destroyMess(void);

  /* decoration,... */
  virtual void        cursorMess(bool on);
  virtual void    dimensionsMess(unsigned int width, unsigned int height);



  //////////
  // Should the window be realized
#warning actuallyDisplay
  bool         m_actuallyDisplay;
  public:
  OSStatus eventHandler (EventRef event);

 private:

  class Info;
  Info*m_info;
  
  static bool     init(void);
};

#endif    // for header file
