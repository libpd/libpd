/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    a window class to render to 

    Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMWINDOW_H_
#define _INCLUDE__GEM_BASE_GEMWINDOW_H_

#include "Gem/GemGL.h"
#include "Base/CPPExtern.h"

#include <vector>
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemWindow
    
    a window

DESCRIPTION
    
-----------------------------------------------------------------*/
namespace gem {
  class Context;
};

class GEM_EXTERN GemWindow : public CPPExtern
{
 CPPEXTERN_HEADER(GemWindow, CPPExtern);

 private:
  class PIMPL;
  PIMPL*m_pimpl;

 public:
    
  //////////
  // Constructor
  GemWindow(void);
    	
  //////////
  // Destructor
  virtual ~GemWindow(void);

 public:
  /* OUTPUT */

  /* an outlet to propagate information to the patch... mainly callbacks from the context */
  /* LATER think about detaching the output from the stack, so we can e.g. destroy a window from a mouse-callback */
  void info(std::vector<t_atom>);  
  void info(t_symbol*s, int, t_atom*);  
  void info(std::string);
  void info(std::string, t_float);
  void info(std::string, int i);
  void info(std::string, std::string);  

  /* tell downstream objects to render */
  void bang(void);

  /* mouse movement */
  void motion(int x, int y);
  /* mouse buttons */
  void button(int id, int state);
  /* keyboard buttons */
  //  void key(std::string id, int state);
  //void key(int id, int state);
  void key(std::string, int, int state);

  /* window resize/move */
  void dimension(unsigned int, unsigned int);
  void position (int, int);

  /* INPUT */

  /* create a new context */
  static gem::Context*createContext(void);
  /* destroy a given context; 
   * @returns  NULL
   */
  static gem::Context*destroyContext(gem::Context*);

  /* this MUST be called from the derived classes
   * as it will eventually establish a new GemContext (if m_context is non-NULL)
   * if you want to share GemContext's you MUST call
   *   GemWindow::createContext() yourself and set m_context to the result
   *
   * if <tt>false</tt> is returned, you should not continue
   */
  bool createGemWindow(void);
  /* create a new window
   * make sure that this calls the parent's createContext() method
   */
  virtual bool create(void) = 0;

  /* destroy an established context+infrastructuure *
   * make sure that this get's called from your destroy() implementation
   */
  void destroyGemWindow();
  /* create the current window
   * make sure to call GemWindow::destroyGemWindow()
   */
  virtual void destroy(void) = 0;

  /* make the object's context (window,...) the current context
   * this is virtual, so objects can add their own code
   * note however, that they should also call this (parent's) function within
   * typically implementations look like this:
   * bool <mywindow>::makeCurrent(void) {
   *    // do your own stuff
   *
   * is <tt>false</tt> is returned, do not attempt to use it (e.g. draw into it)
   */
  virtual bool makeCurrent(void) = 0;

  /*
   * make the GemWindow current (reset stacks), switch multiContext
   */
  bool pushContext(void);
  /*
   * make uncurrent
   */
  bool popContext (void);

  /* swap back/front buffer
   */
  virtual void swapBuffers(void) = 0;

  /* dispatch messages from the window
   * this might get called more often than the render-cycle
   * it might also be called automatically as soon as the window
   * is create()ed (and until the window is destroy()ed)
   */
  virtual void dispatch(void);

  /* render to this window
   *  the default implementation calls:
   *    if(!makeCurrent())return;
   *    if(!pushContext())return;
   *    bang();
   *    if(m_buffer==2)swap();
   *    popContext();
   * but you can override this, if you want to
   */
  virtual void render(void);

  /* set/get the dimension of the context
   * setting is done by supplying arguments to the method;
   * querying is done by supplying NO arguments
   * this should be kept throughout 
   */
  virtual void dimensionsMess(unsigned int width, unsigned int height) = 0;


  // common property setters
  // by default they will simply set the corresponding values (below in the protected section)
  // to whatever argument is given them
  // so you can use these values when creating the window
  // however, if you need to take immediate action (e.g. because you can), you ought to override these functions

  /* render context (pre creation) */
  virtual void  bufferMess(int buf);
  virtual void    fsaaMess(int value);

  /* window decoration (pre creation) */
  virtual void titleMess(std::string);
  virtual void borderMess(bool on);

  virtual void    fullscreenMess(int on);
  virtual void        offsetMess(int x, int y);
 
  /* creation/destruction */
  virtual void        createMess(std::string);
  virtual void       destroyMess(void);

  /* post creation */
  virtual void        cursorMess(bool on);

  /* print some info */
  virtual void        printMess(void);

 protected:
  unsigned int m_width, m_height;

  // common properties of GemWindow's
  // you can safely ignore these, if they mean nothing to you
  // however, if they do mean something to you, it would be good if you used these
  int          m_xoffset, m_yoffset;
  bool         m_border;
  int          m_fullscreen;

  unsigned int m_buffer;
  std::string  m_title;
  bool         m_cursor;
  int          m_fsaa;

  gem::Context*  m_context;
};



#endif	// for header file
