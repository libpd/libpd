/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Interface for the window manager

  Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_OUTPUT_GEMGLXWINDOW_H_
#define _INCLUDE__GEM_OUTPUT_GEMGLXWINDOW_H_

#include "Base/GemWindow.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  gemglxwindow

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


class GEM_EXTERN gemglxwindow : public GemWindow
{
  CPPEXTERN_HEADER(gemglxwindow, GemWindow);

    public:

  //////////
  // Constructor
  gemglxwindow(void);

 private:

  //////////
  // Destructor
  virtual ~gemglxwindow(void);


  // create window
  virtual bool create(void);

  // destroy window
  virtual void destroy(void);

  // check whether we have a window and if so, make it current
  virtual bool makeCurrent(void);
  virtual void swapBuffers(void);

  /* dispatch window events */
  void dispatch(void);

  /* render context (pre creation) */
  virtual void  bufferMess(int buf);
  virtual void    fsaaMess(int value);

  /* window decoration (pre creation) */
  virtual void titleMess(std::string);
  virtual void borderMess(bool on);

  /* window position/dimension (pre creation) */
  virtual void    dimensionsMess(unsigned int width, unsigned int height);

  virtual void    fullscreenMess(bool on);
  virtual void        offsetMess(int x, int y);
 
  /* creation/destruction */
  virtual void        createMess(std::string);
  virtual void       destroyMess(void);

  /* post creation */
  virtual void        cursorMess(bool on);

  //// X display specification (e.g. "remote:0.1")
  std::string m_display;

  void       print(void);
 private:

  class PIMPL;
  PIMPL*m_pimpl;
};

#endif    // for header file
