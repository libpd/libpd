/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Interface for the window manager

  Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_OUTPUT_GEMCOCOAWINDOW_H_
#define _INCLUDE__GEM_OUTPUT_GEMCOCOAWINDOW_H_

#import <Cocoa/Cocoa.h>
#include "Base/GemWindow.h"


class GEM_EXTERN gemcocoawindow : public GemWindow
{
  CPPEXTERN_HEADER(gemcocoawindow, GemWindow);

    public:

  //////////
  // Constructor
  gemcocoawindow(void);
  virtual ~gemcocoawindow(void);

  virtual bool create(void);
  virtual void destroy(void);
  virtual void        createMess(std::string);
  virtual void       destroyMess(void);

  // check whether we have a window and if so, make it current
  virtual bool makeCurrent(void);
  virtual void swapBuffers(void);
  virtual void render(void);
  void renderMess(void);

  virtual void dispatch(void);
  void dispatchEvent(NSEvent*);

  virtual void              titleMess(std::string s);
  virtual void dimensionsMess(unsigned int width, unsigned int height);
  virtual void             offsetMess(int x, int y);
  virtual void         fullscreenMess(bool on);
  virtual void             cursorMess(bool on);
  virtual void            menubarMess(int value);
  void		         move(void); // apply dimen/offset to current window
  void		        moved(void); // window dimen/offset changed

 private:
  class PIMPL;
  PIMPL*m_pimpl;
  friend class PIMPL;
};

#endif    // for header file
