/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Interface for the window manager

  Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_OUTPUT_GEMGLUTWINDOW_H_
#define _INCLUDE__GEM_OUTPUT_GEMGLUTWINDOW_H_

#include "Base/GemWindow.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  gemglutwindow

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


class GEM_EXTERN gemglutwindow : public GemWindow
{
  CPPEXTERN_HEADER(gemglutwindow, GemWindow);

    public:

  //////////
  // Constructor
  gemglutwindow(void);

 private:

  //////////
  // Destructor
  virtual ~gemglutwindow(void);

  void doRender(void);

  /* rendering */
  void renderMess(void);

  /* render context (pre creation) */
  void  bufferMess(int buf);
  virtual void    fsaaMess(int value);

  /* window decoration (pre creation) */
  virtual void titleMess(std::string s);

  /* window position/dimension (pre creation) */
  virtual void    dimensionsMess(unsigned int width, unsigned int height);
  virtual void    fullscreenMess(bool on);
  virtual void        offsetMess(int x, int y);

  /* creation/destruction */
  virtual bool        create(void);
  virtual void destroy(void);

  virtual void        createMess(std::string);
  virtual void       destroyMess(void);

  /* post creation */
  virtual void        cursorMess(bool on);

  void menuMess(void);
  void addMenuMess(t_symbol*, int, t_atom*);


  // check whether we have a window and if so, make it current
  virtual bool makeCurrent(void);
  // swap buffers 
  virtual void swapBuffers(void);
  // dispatch events
  virtual void dispatch(void);

 private:

  /* the GLUT window id */
  int m_window;

  //////////
  // glut callbacks 
  static void displayCb(void);
  static void visibleCb(int);
  static void closeCb(void);
  static void keyboardCb(unsigned char, int, int);
  static void specialCb(int, int, int);
  static void reshapeCb(int, int);
  static void mouseCb(int,int,int,int);
  static void motionCb(int,int);
  static void passivemotionCb(int, int);
  static void entryCb(int);
  static void keyboardupCb(unsigned char, int, int);
  static void specialupCb(int, int, int);
  static void joystickCb(unsigned int, int, int, int);
  static void menuCb(int);
  static void menustateCb(int);
  static void menustatusCb(int, int, int);
  static void windowstatusCb(int);
};

#endif    // for header file
